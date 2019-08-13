#include "reghandler.hpp"

#include <cstdio>
#include <cstring>

#include "client.hpp"
#include "log.hpp"
#include "utils.hpp"

#define LOG_MODULE "RegHandler"

namespace openlwm2m {

/*******************************************************************************
 * Public
 ******************************************************************************/

RegHandler::RegHandler(ItemBase* parent, Params params)
    : ItemBase(parent), mParams(params), mTransport(NULL), mSession(NULL), mTimer(INVALID_ID)
{
}

RegHandler::~RegHandler()
{
}

void RegHandler::init()
{
    LOG_DEBUG("Create %d", getId());

    mTimer.setId(getId());
    mSession = mServerInstance = mSecurityInstance = NULL;
    mState = STATE_INIT;
}

void RegHandler::release()
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

Status RegHandler::bind(TransportItf* transport)
{
    mTransport = transport;

    mServerInstance = mParams.objectManager.getServerInstance(getId());
    ASSERT(mServerInstance);

    Object* object = mParams.objectManager.getObject(ITF_CLIENT, OBJ_LWM2M_SECURITY);
    ASSERT(object);

    mSecurityInstance = object->getFirstInstance();

    while (mSecurityInstance) {
        if (mSecurityInstance->getResourceInstance(RES_BOOTSTRAP_SERVER)->getBool()) {
            continue;
        }

        if (mSecurityInstance->getResourceInstance(RES_SECURITY_SHORT_SERVER_ID)->getInt() ==
            mServerInstance->getResourceInstance(RES_SHORT_SERVER_ID)->getInt()) {
            break;
        }

        mSecurityInstance = object->getNextInstance();
    }

    if (!mSecurityInstance) {
        return STS_ERR_NOT_EXIST;
    }

    const char* serverUri = mSecurityInstance->getResourceInstance(RES_LWM2M_SERVER_URI)->getString();

    LOG_INFO("Bind %d to: %s", getId(), serverUri);

    Status status = STS_OK;

    mSession = mTransport->createSession(serverUri, &status);

    return status;
}

Status RegHandler::registration(bool ordered, RegistrationHandler handler, void* context)
{
    if (mState != STATE_INIT && mState != STATE_DEREGISTERED) {
        return STS_ERR_INVALID_STATE;
    }

    mRegistrationContext = (ContextHandler){handler, context};

    mCurrentSequence = 0;

    ResourceInstance* initRegDelay = mServerInstance->getResourceInstance(RES_INITIAL_REGISTRATION_DELAY);

    uint64_t initDelayMs = 0;

    if (initRegDelay) {
        initDelayMs = initRegDelay->getUint() * 1000;
    }

    LOG_INFO("Start %d, delay: %lu", getId(), initDelayMs);

    mTimer.start(initDelayMs, &RegHandler::timerCallback, this, true);

    mState = STATE_INIT_DELAY;
    mOrdered = ordered;

    return STS_OK;
}

Status RegHandler::deregistration(RegistrationHandler handler, void* context)
{
    Status status = STS_OK;

    if (mState != STATE_REGISTERED) {
        return STS_ERR_INVALID_STATE;
    }

    mDeregistrationContext = (ContextHandler){handler, context};

    mTimer.stop();

    mState = STATE_DEREGISTRATION;

    LOG_INFO("Send deregistration reqest");

    if ((status = mTransport->deregistrationRequest(mSession, mLocation, &RegHandler::deregistrationCallback, this)) !=
        STS_OK) {
        return status;
    }

    return STS_OK;
}

Status RegHandler::updateRegistration()
{
    if (mState != STATE_REGISTERED) {
        return STS_ERR_INVALID_STATE;
    }

    mTimer.start(sUpdateRegistrationTimeout, &RegHandler::timerCallback, this, true);

    return STS_OK;
}

/*******************************************************************************
 * Private
 ******************************************************************************/

Status RegHandler::timerCallback(void* context)
{
    return static_cast<RegHandler*>(context)->onTimerCallback();
}

Status RegHandler::onTimerCallback()
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

void RegHandler::registrationCallback(void* context, void* data, Status status)
{
    static_cast<RegHandler*>(context)->onRegistrationCallback(static_cast<char*>(data), status);
}

void RegHandler::onRegistrationCallback(char* location, Status status)
{
    if (mState != STATE_REGISTRATION) {
        return;
    }

    if (location) {
        Utils::strCopy(mLocation, location, CONFIG_DEFAULT_STRING_LEN);
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

        mTimer.start(mLifetime * CONFIG_LIFETIME_SCALE * 1000, &RegHandler::timerCallback, this, true);
    }
    else {
        LOG_ERROR("Registration failed %d, status: %d", getId(), status);

        ResourceInstance* failureBlock = mServerInstance->getResourceInstance(RES_REG_FAILURE_BLOCK);

        if (mOrdered && (!failureBlock || !failureBlock->getBool())) {
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

    if (mParams.pollRequest) {
        mParams.pollRequest();
    }
}

void RegHandler::updateCallback(void* context, void* data, Status status)
{
    static_cast<RegHandler*>(context)->onUpdateCallback(status);
}

void RegHandler::onUpdateCallback(Status status)
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

    mTimer.start(timeMs, &RegHandler::timerCallback, this, true);

    if (mParams.pollRequest) {
        mParams.pollRequest();
    }
}

void RegHandler::deregistrationCallback(void* context, void* data, Status status)
{
    static_cast<RegHandler*>(context)->onDeregistrationCallback(status);
}

void RegHandler::onDeregistrationCallback(Status status)
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

    if (mParams.pollRequest) {
        mParams.pollRequest();
    }
}

Status RegHandler::getObjectsStr(char* str, int maxSize)
{
    int ret = 0;
    int size = 0;

    Object* object = mParams.objectManager.getFirstObject(ITF_REGISTER);

    if (mParams.objectManager.isFormatSupported(DATA_FMT_SENML_JSON)) {
        if (object) {
            ret = snprintf(&str[size], maxSize, "</>;rt=\"oma.lwm2m\";ct=%d,", DATA_FMT_SENML_JSON);
        }
        else {
            ret = snprintf(&str[size], maxSize, "</>;rt=\"oma.lwm2m\";ct=%d", DATA_FMT_SENML_JSON);
        }

        if (ret < 0) {
            return STS_ERR;
        }

        size += ret;
    }

    while (object) {
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

        object = mParams.objectManager.getNextObject(ITF_REGISTER);
    }

    if (size > 0 && str[size - 1] == ',') {
        str[size - 1] = 0;
    }

    return STS_OK;
}

Status RegHandler::sendRegistration()
{
    Status status = STS_OK;

    mState = STATE_REGISTRATION;

    if ((status = getObjectsStr(mObjectsStr, CONFIG_DEFAULT_STRING_LEN)) != STS_OK) {
        return status;
    }

    mLifetime = mServerInstance->getResourceInstance(RES_LIFETIME)->getInt();
    strncpy(mBindingStr, mServerInstance->getResourceInstance(RES_BINDING)->getString(), CONFIG_BINDING_STR_MAX_LEN);
    mBindingStr[CONFIG_BINDING_STR_MAX_LEN] = '\0';

    LOG_INFO("Send registration request %d, lifetime: %d, objects: %s, bindings: %s", getId(), mLifetime, mObjectsStr,
             mBindingStr);

    if ((status = mTransport->registrationRequest(mSession, mParams.clientName, mLifetime, LWM2M_VERSION, mBindingStr,
                                                  mParams.queueMode, NULL, mObjectsStr,
                                                  &RegHandler::registrationCallback, this)) != STS_OK) {
        return status;
    }

    return STS_OK;
}

Status RegHandler::sendUpdate()
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

    if (strcmp(mBindingStr, mServerInstance->getResourceInstance(RES_BINDING)->getString()) != 0) {
        strncpy(mBindingStr, mServerInstance->getResourceInstance(RES_BINDING)->getString(),
                CONFIG_BINDING_STR_MAX_LEN);
        mBindingStr[CONFIG_BINDING_STR_MAX_LEN] = '\n';
        bindingPtr = mBindingStr;
    }

    if (mLifetime != mServerInstance->getResourceInstance(RES_LIFETIME)->getInt()) {
        mLifetime = mServerInstance->getResourceInstance(RES_LIFETIME)->getInt();
        lifetimePtr = &mLifetime;
    }

    LOG_INFO("Send registration update %d, lifetime: %d, objects: %s, bindings: %s", getId(), mLifetime, mObjectsStr,
             mBindingStr);

    if ((status = mTransport->registrationUpdate(mSession, mLocation, lifetimePtr, bindingPtr, NULL, objectsPtr,
                                                 &RegHandler::updateCallback, this)) != STS_OK) {
        return status;
    }

    return STS_OK;
}

bool RegHandler::setupRetry()
{
    // Table: 6.2.1.1-1 Registration Procedures Default Values
    uint64_t delay = 60 * 60 * 24;
    uint64_t count = 1;

    ResourceInstance* timerInstance = mServerInstance->getResourceInstance(RES_SEQUENCE_DELAY_TIMER);
    ResourceInstance* countInstance = mServerInstance->getResourceInstance(RES_SEQUENCE_RETRY_COUNT);

    if (countInstance) {
        count = countInstance->getUint();
    }

    if (timerInstance) {
        delay = timerInstance->getUint();
    }

    if (delay < ULONG_MAX && mCurrentSequence < count) {
        mTimer.start(delay * 1000, &RegHandler::timerCallback, this, true);
        mCurrentSequence++;

        return true;
    }

    return false;
}

}  // namespace openlwm2m
