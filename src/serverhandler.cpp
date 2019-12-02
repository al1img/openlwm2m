#include "serverhandler.hpp"

#include <cstdio>
#include <cstring>

#include "log.hpp"
#include "utils.hpp"

#define LOG_MODULE "ServerHandler"

namespace openlwm2m {

/*******************************************************************************
 * Public
 ******************************************************************************/

ServerHandler::ServerHandler(const char* clientName, bool queueMode, ObjectManager& objectManager)
    : ItemBase(NULL),
      mClientName(clientName),
      mQueueMode(queueMode),
      mObjectManager(objectManager),
      mTransport(NULL),
      mSession(NULL),
      mTimer(INVALID_ID)
{
}

ServerHandler::~ServerHandler()
{
}

void ServerHandler::init()
{
    LOG_DEBUG("Create %d", getId());

    mTimer.setId(getId());
    mSession = mServerInstance = mSecurityInstance = NULL;
    mState = STATE_INIT;
}

void ServerHandler::release()
{
    LOG_DEBUG("Delete %d", getId());

    if (mTransport && mSession) {
        Status status = mTransport->deleteSession(mSession);

        if (status != STS_OK) {
            LOG_ERROR("Can't delete session: %d", status);
        }
    }

    mTimer.stop();
    mState = STATE_INIT;
}

Status ServerHandler::bind(TransportItf* transport)
{
    mTransport = transport;

    mServerInstance = mObjectManager.getServerInstance(getId());
    ASSERT(mServerInstance);

    Object* object = mObjectManager.getObjectById(OBJ_LWM2M_SECURITY);
    ASSERT(object);

    mSecurityInstance = object->getFirstInstance();

    for (; mSecurityInstance; mSecurityInstance = object->getNextInstance()) {
        if (static_cast<ResourceBool*>(mSecurityInstance->getResourceInstance(RES_BOOTSTRAP_SERVER))->getValue()) {
            continue;
        }

        if (static_cast<ResourceInt*>(mSecurityInstance->getResourceInstance(RES_SECURITY_SHORT_SERVER_ID))
                ->getValue() ==
            static_cast<ResourceInt*>(mServerInstance->getResourceInstance(RES_SHORT_SERVER_ID))->getValue()) {
            break;
        }
    }

    if (!mSecurityInstance) {
        return STS_ERR_NOT_FOUND;
    }

    const char* serverUri =
        static_cast<ResourceString*>(mSecurityInstance->getResourceInstance(RES_LWM2M_SERVER_URI))->getValue();

    LOG_INFO("Bind %d to: %s", getId(), serverUri);

    Status status = STS_OK;

    mSession = mTransport->createSession(serverUri, &status);

    return status;
}

Status ServerHandler::registration(bool ordered, RegistrationHandler handler, void* context)
{
    if (mState != STATE_INIT && mState != STATE_DEREGISTERED) {
        return STS_ERR_NOT_ALLOWED;
    }

    mRegistrationContext = (ContextHandler){handler, context};

    mCurrentSequence = 0;

    ResourceInstance* initRegDelay = mServerInstance->getResourceInstance(RES_INITIAL_REGISTRATION_DELAY);

    uint64_t initDelayMs = 0;

    if (initRegDelay) {
        initDelayMs = static_cast<ResourceUint*>(initRegDelay)->getValue() * 1000;
    }

    LOG_INFO("Start %d, delay: %lu", getId(), initDelayMs);

    mTimer.start(initDelayMs, &ServerHandler::timerCallback, this, true);

    mState = STATE_INIT_DELAY;
    mOrdered = ordered;

    return STS_OK;
}

Status ServerHandler::deregistration(RegistrationHandler handler, void* context)
{
    Status status = STS_OK;

    if (mState != STATE_REGISTERED) {
        return STS_ERR_NOT_ALLOWED;
    }

    mDeregistrationContext = (ContextHandler){handler, context};

    mTimer.stop();

    mState = STATE_DEREGISTRATION;

    LOG_INFO("Send deregistration reqest");

    if ((status = mTransport->deregistrationRequest(mSession, mLocation, &ServerHandler::deregistrationCallback,
                                                    this)) != STS_OK) {
        return status;
    }

    return STS_OK;
}

Status ServerHandler::updateRegistration()
{
    if (mState != STATE_REGISTERED) {
        return STS_ERR_NOT_ALLOWED;
    }

    mTimer.start(sUpdateRegistrationTimeoutMs, &ServerHandler::timerCallback, this, true);

    return STS_OK;
}

Status ServerHandler::discover(void* data, size_t* size, uint16_t objectId, uint16_t objectInstanceId,
                               uint16_t resourceId)
{
    if (mState != STATE_REGISTERED) {
        return STS_ERR_NOT_ALLOWED;
    }

    LOG_DEBUG("Device discover /%d/%d/%d", objectId, objectInstanceId, resourceId);

    int ret;

    if (objectId == OBJ_LWM2M_SECURITY || objectId == OBJ_OSCORE) {
        return STS_ERR_NOT_ALLOWED;
    }

    Object* object = mObjectManager.getObjectById(objectId);

    if (!object) {
        return STS_ERR_NOT_FOUND;
    }

    if (objectInstanceId == INVALID_ID) {
        if ((ret = discoverObject(static_cast<char*>(data), *size, object)) < 0) {
            return STS_ERR_NO_MEM;
        }

        *size = ret;

        return STS_OK;
    }

    ObjectInstance* objectInstance = object->getInstanceById(objectInstanceId);

    if (!objectInstance) {
        return STS_ERR_NOT_FOUND;
    }

    if (resourceId == INVALID_ID) {
        if ((ret = discoverObjectInstance(static_cast<char*>(data), *size, objectInstance)) < 0) {
            return STS_ERR_NO_MEM;
        }

        *size = ret;

        return STS_OK;
    }

    Resource* resource = objectInstance->getResourceById(resourceId);

    // Only instanciated resources
    if (!resource || !resource->getFirstInstance()) {
        return STS_ERR_NOT_FOUND;
    }

    if ((ret = discoverResource(static_cast<char*>(data), *size, resource)) < 0) {
        return STS_ERR_NO_MEM;
    }

    *size = ret;

    return STS_OK;
}

/*******************************************************************************
 * Private
 ******************************************************************************/

Status ServerHandler::timerCallback(void* context)
{
    return static_cast<ServerHandler*>(context)->onTimerCallback();
}

Status ServerHandler::onTimerCallback()
{
    switch (mState) {
        case STATE_INIT_DELAY:
            // fallthrough
        case STATE_REGISTRATION:
            return sendRegistration();

        case STATE_REGISTERED:
            return sendUpdate();

        default:
            return STS_ERR;
    }
}

void ServerHandler::registrationCallback(void* context, void* data, Status status)
{
    static_cast<ServerHandler*>(context)->onRegistrationCallback(static_cast<char*>(data), status);
}

void ServerHandler::onRegistrationCallback(char* location, Status status)
{
    if (mState != STATE_REGISTRATION) {
        return;
    }

    if (location) {
        Utils::strCopy(mLocation, location, sizeof(mLocation));
    }
    else {
        mLocation[0] = '\0';
    }

    if (status == STS_OK) {
        LOG_INFO("Registration success %d, location: %s", getId(), mLocation);

        mState = STATE_REGISTERED;

        if (mRegistrationContext.handler) {
            mRegistrationContext.handler(mRegistrationContext.context, this, status);
        }

        mTimer.start(mLifetime * CONFIG_LIFETIME_SCALE * 1000, &ServerHandler::timerCallback, this, true);
    }
    else {
        LOG_ERROR("Registration failed %d, status: %d", getId(), status);

        ResourceInstance* failureBlock = mServerInstance->getResourceInstance(RES_REG_FAILURE_BLOCK);

        if (mOrdered && (!failureBlock || !static_cast<ResourceBool*>(failureBlock)->getValue())) {
            mState = STATE_DEREGISTERED;

            if (mRegistrationContext.handler) {
                mRegistrationContext.handler(mRegistrationContext.context, this, status);
            }
        }
        else if (!setupRetry()) {
            mState = STATE_DEREGISTERED;

            if (mRegistrationContext.handler) {
                mRegistrationContext.handler(mRegistrationContext.context, this, status);
            }
        }
    }
}

void ServerHandler::updateCallback(void* context, void* data, Status status)
{
    static_cast<ServerHandler*>(context)->onUpdateCallback(status);
}

void ServerHandler::onUpdateCallback(Status status)
{
    uint64_t timeMs = mLifetime * CONFIG_LIFETIME_SCALE * 1000;

    if (mState != STATE_REGISTERED) {
        return;
    }

    if (status == STS_OK) {
        LOG_INFO("Update success %d", getId());
    }
    else {
        LOG_ERROR("Update failed %d, status: %d", getId(), status);

        if (status != STS_ERR_TIMEOUT) {
            mState = STATE_REGISTRATION;
            timeMs = 0;
            mCurrentSequence = 0;
        }
    }

    mTimer.start(timeMs, &ServerHandler::timerCallback, this, true);
}

void ServerHandler::deregistrationCallback(void* context, void* data, Status status)
{
    static_cast<ServerHandler*>(context)->onDeregistrationCallback(status);
}

void ServerHandler::onDeregistrationCallback(Status status)
{
    if (mState != STATE_DEREGISTRATION) {
        return;
    }

    mState = STATE_DEREGISTERED;

    if (status == STS_OK) {
        LOG_INFO("Deregistration success %d", getId());
    }
    else {
        LOG_ERROR("Deregistration failed %d, status: %d", getId(), status);
    }

    if (mDeregistrationContext.handler) {
        mDeregistrationContext.handler(mDeregistrationContext.context, this, status);
    }
}

// TODO: move to object manager
Status ServerHandler::getObjectsStr(char* str, int maxSize)
{
    int ret = 0;
    int size = 0;

    if (mObjectManager.isFormatSupported(DATA_FMT_SENML_JSON)) {
        ret = snprintf(&str[size], maxSize, "</>;rt=\"oma.lwm2m\";ct=%d,", DATA_FMT_SENML_JSON);

        if (ret < 0 || ret >= maxSize) {
            return STS_ERR;
        }

        size += ret;
    }

    for (Object* object = mObjectManager.getFirstObject(); object; object = mObjectManager.getNextObject()) {
        if (object->getId() == OBJ_LWM2M_SECURITY || object->getId() == OBJ_OSCORE) {
            continue;
        }

        ObjectInstance* instance = object->getFirstInstance();

        for (;;) {
            if (!instance) {
                ret = snprintf(&str[size], maxSize - size, "<%d>,", object->getId());
            }
            else {
                ret = snprintf(&str[size], maxSize - size, "<%d/%d>,", object->getId(), instance->getId());

                instance = object->getNextInstance();
            }

            if (ret < 0) {
                return STS_ERR_INVALID_VALUE;
            }

            if (ret >= (maxSize - size)) {
                return STS_ERR_NO_MEM;
            }

            size += ret;

            if (!instance) {
                break;
            }
        }
    }

    if (size > 0 && str[size - 1] == ',') {
        str[size - 1] = 0;
    }

    return STS_OK;
}

Status ServerHandler::sendRegistration()
{
    Status status = STS_OK;

    mState = STATE_REGISTRATION;

    if ((status = getObjectsStr(mObjectsStr, CONFIG_DEFAULT_STRING_LEN)) != STS_OK) {
        return status;
    }

    mLifetime = static_cast<ResourceInt*>(mServerInstance->getResourceInstance(RES_LIFETIME))->getValue();

    if (Utils::strCopy(mBindingStr,
                       static_cast<ResourceString*>(mServerInstance->getResourceInstance(RES_BINDING))->getValue(),
                       sizeof(mBindingStr)) < 0) {
        return STS_ERR_NO_MEM;
    }

    LOG_INFO("Send registration request %d, lifetime: %d, objects: %s, bindings: %s", getId(), mLifetime, mObjectsStr,
             mBindingStr);

    if ((status = mTransport->registrationRequest(mSession, mClientName, mLifetime, LWM2M_VERSION, mBindingStr,
                                                  mQueueMode, NULL, mObjectsStr, &ServerHandler::registrationCallback,
                                                  this)) != STS_OK) {
        return status;
    }

    return STS_OK;
}

Status ServerHandler::sendUpdate()
{
    Status status = STS_OK;
    int64_t* lifetimePtr = NULL;
    char* bindingPtr = NULL;
    char* objectsPtr = NULL;

    char objectsStr[CONFIG_DEFAULT_STRING_LEN + 1];

    if ((status = getObjectsStr(objectsStr, CONFIG_DEFAULT_STRING_LEN)) != STS_OK) {
        return status;
    }

    if (strcmp(mObjectsStr, objectsStr) != 0) {
        if (Utils::strCopy(mObjectsStr, objectsStr, sizeof(mObjectsStr)) < 0) {
            return STS_ERR_NO_MEM;
        }

        objectsPtr = mObjectsStr;
    }

    if (strcmp(mBindingStr,
               static_cast<ResourceString*>(mServerInstance->getResourceInstance(RES_BINDING))->getValue()) != 0) {
        if (Utils::strCopy(mBindingStr,
                           static_cast<ResourceString*>(mServerInstance->getResourceInstance(RES_BINDING))->getValue(),
                           sizeof(mBindingStr)) < 0) {
            return STS_ERR_NO_MEM;
        }

        bindingPtr = mBindingStr;
    }

    if (mLifetime != static_cast<ResourceInt*>(mServerInstance->getResourceInstance(RES_LIFETIME))->getValue()) {
        mLifetime = static_cast<ResourceInt*>(mServerInstance->getResourceInstance(RES_LIFETIME))->getValue();
        lifetimePtr = &mLifetime;
    }

    LOG_INFO("Send registration update %d, lifetime: %d, objects: %s, bindings: %s", getId(), mLifetime, mObjectsStr,
             mBindingStr);

    if ((status = mTransport->registrationUpdate(mSession, mLocation, lifetimePtr, bindingPtr, NULL, objectsPtr,
                                                 &ServerHandler::updateCallback, this)) != STS_OK) {
        return status;
    }

    return STS_OK;
}

bool ServerHandler::setupRetry()
{
    // Table: 6.2.1.1-1 Registration Procedures Default Values
    uint64_t delay = 60 * 60 * 24;
    uint64_t count = 1;

    ResourceInstance* timerInstance = mServerInstance->getResourceInstance(RES_SEQUENCE_DELAY_TIMER);
    ResourceInstance* countInstance = mServerInstance->getResourceInstance(RES_SEQUENCE_RETRY_COUNT);

    if (countInstance) {
        count = static_cast<ResourceUint*>(countInstance)->getValue();
    }

    if (timerInstance) {
        delay = static_cast<ResourceUint*>(timerInstance)->getValue();
    }

    if (delay < ULONG_MAX && mCurrentSequence < count) {
        mTimer.start(delay * 1000, &ServerHandler::timerCallback, this, true);
        mCurrentSequence++;

        return true;
    }

    return false;
}

int ServerHandler::discoverObject(char* data, size_t maxSize, Object* object)
{
    size_t curSize = 0;

    if (maxSize == 0) {
        return -1;
    }

    data[curSize++] = '<';
    if (curSize >= maxSize) {
        return -1;
    }

    int ret = Utils::makePath(&data[curSize], maxSize - curSize, object->getId());

    if (ret < 0) {
        return -1;
    }

    curSize += ret;

    data[curSize++] = '>';
    if (curSize >= maxSize) {
        return -1;
    }

    if (object->getFirstInstance()) {
        data[curSize++] = ',';
        if (curSize >= maxSize) {
            return -1;
        }
    }

    for (ObjectInstance* instance = object->getFirstInstance(); instance != NULL;
         instance = object->getNextInstance()) {
        if ((ret = discoverObjectInstance(&data[curSize], maxSize - curSize, instance)) < 0) {
            return -1;
        }

        curSize += ret;
    }

    return curSize;
}

int ServerHandler::discoverObjectInstance(char* data, size_t maxSize, ObjectInstance* instance)
{
    char buf[16];
    size_t curSize = 0;

    int ret = Utils::makePath(buf, sizeof(buf), instance->getParent()->getId(), instance->getId());

    if (ret < 0) {
        return -1;
    }

    ret = snprintf(&data[curSize], maxSize - curSize, "<%s>", buf);

    if (ret < 0 || static_cast<size_t>(ret) >= maxSize - curSize) {
        return -1;
    }

    curSize += ret;

    for (Resource* resource = instance->getFirstResource(); resource != NULL; resource = instance->getNextResource()) {
        if (resource->getFirstInstance()) {
            data[curSize++] = ',';
            if (curSize >= maxSize) {
                return -1;
            }
        }

        if ((ret = discoverResource(&data[curSize], maxSize - curSize, resource)) < 0) {
            return -1;
        }

        curSize += ret;
    }

    return curSize;
}

int ServerHandler::discoverResource(char* data, size_t maxSize, Resource* resource, bool includeInstances)
{
    char buf[16];
    int curSize = 0;

    int ret = Utils::makePath(buf, sizeof(buf), resource->getParent()->getParent()->getId(),
                              resource->getParent()->getId(), resource->getId());

    if (ret < 0) {
        return -1;
    }

    ret = snprintf(&data[curSize], maxSize - curSize, "<%s>", buf);

    if (ret < 0 || static_cast<size_t>(ret) >= maxSize - curSize) {
        return -1;
    }

    curSize += ret;

    if (includeInstances && !resource->getInfo().isSingle() && resource->getFirstInstance()) {
        for (ResourceInstance* instance = resource->getFirstInstance(); instance;
             instance = resource->getNextInstance()) {
            int ret = Utils::makePath(buf, sizeof(buf), resource->getParent()->getParent()->getId(),
                                      resource->getParent()->getId(), resource->getId(), instance->getId());

            if (ret < 0) {
                return -1;
            }

            ret = snprintf(&data[curSize], maxSize - curSize, ",<%s>", buf);

            if (ret < 0 || static_cast<size_t>(ret) >= maxSize - curSize) {
                return -1;
            }

            curSize += ret;
        }
    }

    return curSize;
}

}  // namespace openlwm2m
