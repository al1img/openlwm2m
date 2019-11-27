#include "bootstraphandler.hpp"

#include <cstdio>

#include "log.hpp"
#include "utils.hpp"

#define LOG_MODULE "BootstrapHandler"

namespace openlwm2m {

/*******************************************************************************
 * Public
 ******************************************************************************/

BootstrapHandler::BootstrapHandler(const char* clientName, ObjectManager& objectManager)
    : mClientName(clientName),
      mObjectManager(objectManager),
      mTransport(NULL),
      mSession(NULL),
      mState(STATE_INIT),
      mTimer(0)
{
}

BootstrapHandler::~BootstrapHandler()
{
    if (mState == STATE_BOOTSTRAPING) {
        mTimer.stop();
        mTransport->deleteSession(mSession);
    }
}

Status BootstrapHandler::bind(TransportItf* transport)
{
    mTransport = transport;

    return STS_OK;
}

Status BootstrapHandler::bootstrapRequest(RequestHandler handler, void* context)
{
    Status status = STS_OK;

    if (mState != STATE_INIT && mState != STATE_BOOTSTAPPED) {
        return STS_ERR_NOT_ALLOWED;
    }

    Object* object = mObjectManager.getObjectById(OBJ_LWM2M_SECURITY);
    ASSERT(object);

    ObjectInstance* securityInstance = NULL;

    for (ObjectInstance* instance = object->getFirstInstance(); instance; instance = object->getNextInstance()) {
        if (static_cast<ResourceBool*>(instance->getResourceInstance(RES_BOOTSTRAP_SERVER))->getValue()) {
            securityInstance = instance;
            break;
        }
    }

    if (!securityInstance) {
        return STS_ERR_NOT_FOUND;
    }

    const char* serverUri =
        static_cast<ResourceString*>(securityInstance->getResourceInstance(RES_LWM2M_SERVER_URI))->getValue();

    LOG_INFO("Bootstrap request to: %s", serverUri);

    mSession = mTransport->createSession(serverUri, &status);

    if (!mSession) {
        return status;
    }

    DataFormat preferredFormat = CONFIG_DEFAULT_DATA_FORMAT;

    if ((status = mTransport->bootstrapRequest(mSession, mClientName, &preferredFormat,
                                               &BootstrapHandler::bootstrapRequstCallback, this)) != STS_OK) {
        return status;
    }

    mRequestHandler = handler;
    mRequestContext = context;

    mState = STATE_BOOTSTRAPING;

    return STS_OK;
}

Status BootstrapHandler::bootstrapFinish()
{
    Status status = STS_OK;

    if (mState != STATE_BOOTSTRAPING) {
        return STS_ERR_NOT_ALLOWED;
    }

    LOG_INFO("Bootstrap finish");

    bootstrapFinish(status);

    return status;
}

Status BootstrapHandler::discover(char* data, size_t* size, uint16_t objectId)
{
    size_t curSize = 0;

    if (mState != STATE_BOOTSTRAPING) {
        return STS_ERR_NOT_ALLOWED;
    }

    LOG_INFO("Bootstrap discover");

    int ret = snprintf(data, *size, "lwm2m=\"%s\"", LWM2M_VERSION);

    if (ret < 0) {
        return STS_ERR_NO_MEM;
    }

    curSize += ret;

    if (objectId == INVALID_ID) {
        for (Object* object = mObjectManager.getFirstObject(); object != NULL;
             object = mObjectManager.getNextObject()) {
            ret = discoverObject(&data[curSize], *size - curSize, object);

            if (ret < 0) {
                return STS_ERR_NO_MEM;
            }

            curSize += ret;
        }
    }
    else {
        Object* object = mObjectManager.getObjectById(objectId);

        if (!object) {
            return STS_ERR_NOT_FOUND;
        }

        ret = discoverObject(&data[curSize], *size - curSize, object);

        if (ret < 0) {
            return STS_ERR_NO_MEM;
        }

        curSize += ret;
    }

    *size = curSize;

    return STS_OK;
}

/*******************************************************************************
 * Private
 ******************************************************************************/

Status BootstrapHandler::timerCallback(void* context)
{
    return static_cast<BootstrapHandler*>(context)->onTimerCallback();
}

Status BootstrapHandler::onTimerCallback()
{
    bootstrapFinish(STS_ERR_TIMEOUT);

    return STS_OK;
}

void BootstrapHandler::bootstrapRequstCallback(void* context, void* data, Status status)
{
    static_cast<BootstrapHandler*>(context)->onBootstrapRequestCallback(status);
}

void BootstrapHandler::onBootstrapRequestCallback(Status status)
{
    if (status == STS_OK) {
        LOG_INFO("Bootstrap request success");

        mTimer.start(sBootstrapTimeoutMs, &BootstrapHandler::timerCallback, this, true);
    }
    else {
        bootstrapFinish(status);
    }
}

void BootstrapHandler::bootstrapFinish(Status status)
{
    if (status == STS_OK) {
        LOG_INFO("Bootstrap finished");
    }
    else {
        LOG_ERROR("Bootstrap failed, status: %d", status);
    }

    mState = STATE_BOOTSTAPPED;
    mTimer.stop();
    mTransport->deleteSession(mSession);

    if (mRequestHandler) {
        mRequestHandler(mRequestContext, this, status);
    }
}

int BootstrapHandler::discoverObject(char* data, size_t maxSize, Object* object)
{
    char buf[16];
    int curSize = 0;

    for (ObjectInstance* instance = object->getFirstInstance(); instance != NULL;
         instance = object->getNextInstance()) {
        int ret = Utils::makePath(buf, sizeof(buf), object->getId(), instance->getId());
        if (ret < 0) {
            return -1;
        }

        if (object->getId() == OBJ_LWM2M_SECURITY &&
            !static_cast<ResourceBool*>(instance->getResourceInstance(RES_BOOTSTRAP_SERVER))->getValue()) {
            ret = snprintf(
                &data[curSize], maxSize - curSize, ",<%s>;ssid=%ld;uri=%s", buf,
                static_cast<ResourceInt*>(instance->getResourceInstance(RES_SECURITY_SHORT_SERVER_ID))->getValue(),
                static_cast<ResourceString*>(instance->getResourceInstance(RES_LWM2M_SERVER_URI))->getValue());
        }
        else {
            ret = snprintf(&data[curSize], maxSize - curSize, ",<%s>", buf);
        }

        curSize += ret;
    }

    return curSize;
}

#if 0
Status ServerHandler::getObjectsStr(char* str, int maxSize)
{
    int ret = 0;
    int size = 0;

    if (mObjectManager.isFormatSupported(DATA_FMT_SENML_JSON)) {
        ret = snprintf(&str[size], maxSize, "</>;rt=\"oma.lwm2m\";ct=%d,", DATA_FMT_SENML_JSON);

        if (ret < 0) {
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
                return STS_ERR;
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
    strncpy(mBindingStr, static_cast<ResourceString*>(mServerInstance->getResourceInstance(RES_BINDING))->getValue(),
            CONFIG_BINDING_STR_MAX_LEN);
    mBindingStr[CONFIG_BINDING_STR_MAX_LEN] = '\0';

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
        strncpy(mObjectsStr, objectsStr, CONFIG_DEFAULT_STRING_LEN);
        mObjectsStr[CONFIG_DEFAULT_STRING_LEN] = '\n';
        objectsPtr = mObjectsStr;
    }

    if (strcmp(mBindingStr,
               static_cast<ResourceString*>(mServerInstance->getResourceInstance(RES_BINDING))->getValue()) != 0) {
        strncpy(mBindingStr,
                static_cast<ResourceString*>(mServerInstance->getResourceInstance(RES_BINDING))->getValue(),
                CONFIG_BINDING_STR_MAX_LEN);
        mBindingStr[CONFIG_BINDING_STR_MAX_LEN] = '\n';
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
#endif

}  // namespace openlwm2m
