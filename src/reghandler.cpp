#include "reghandler.hpp"

#include <cstdio>

#include "client.hpp"
#include "log.hpp"

#define LOG_MODULE "RegHandler"

namespace openlwm2m {

/*******************************************************************************
 * Private
 ******************************************************************************/

RegHandler::RegHandler(ItemBase* parent, Param param)
    : ItemBase(parent), mParam(param), mSession(NULL), mTimer(INVALID_ID)
{
}

RegHandler::~RegHandler()
{
}

void RegHandler::init()
{
    LOG_DEBUG("Create /%d", getId());

    mTimer.setId(getId());
    mSession = mServerInstance = mSecurityInstance = NULL;
    mState = STATE_INIT;
}

void RegHandler::release()
{
    LOG_DEBUG("Delete /%d", getId());

    if (mTransport && mSession) {
        Status status = mTransport->deleteSession(mSession);

        if (status != STS_OK) {
            LOG_ERROR("Can't delete session: %d", status);
        }
    }

    mTimer.stop();
    mState = STATE_INIT;
}

void RegHandler::setTransport(TransportItf* transport)
{
    mTransport = transport;
}

Status RegHandler::bind(ObjectInstance* serverInstance)
{
    mServerInstance = serverInstance;

    Object* object = mParam.objectManager.getObject(ITF_CLIENT, OBJ_LWM2M_SECURITY);
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

    LOG_DEBUG("Bind /%d to: %s", getId(), serverUri);

    ASSERT(mTransport);

    Status status = STS_OK;

    mSession = mTransport->createSession(serverUri, &status);

    return status;
}

Status RegHandler::startRegistration()
{
    ResourceInstance* initRegDelay = mServerInstance->getResourceInstance(RES_INITIAL_REGISTRATION_DELAY);

    uint64_t initDelayMs = 0;

    if (initRegDelay) {
        initDelayMs = initRegDelay->getUint();
    }

    LOG_DEBUG("Start /%d, delay: %lu", getId(), initDelayMs);

    mTimer.start(initDelayMs, &RegHandler::timerCallback, this, true);

    mState = STATE_INIT_DELAY;

    return STS_OK;
}

Status RegHandler::timerCallback(void* context)
{
    return static_cast<RegHandler*>(context)->onTimerCallback();
}

Status RegHandler::onTimerCallback()
{
    Status status = STS_OK;

    switch (mState) {
        case STATE_INIT_DELAY:
            mState = STATE_REGISTRATION;
            char objectsStr[CONFIG_DEFAULT_STRING_LEN];

            status = getObjectsStr(objectsStr, CONFIG_DEFAULT_STRING_LEN);

            if (status != STS_OK) {
                registrationStatus(status);
                return status;
            }

            LOG_DEBUG("Send reg request /%d, objects: %s", getId(), objectsStr);

            ASSERT(mTransport)

            status = mTransport->registrationRequest(
                mSession, mParam.clientName, mServerInstance->getResourceInstance(RES_LIFETIME)->getInt(),
                LWM2M_VERSION, mServerInstance->getResourceInstance(RES_BINDING)->getString(), mParam.queueMode, NULL,
                objectsStr, &RegHandler::registrationCallback, this);

            if (status != STS_OK) {
                registrationStatus(status);
                return status;
            }

            break;

        default:
            break;
    }

    return STS_OK;
}

void RegHandler::registrationCallback(void* context, Status status)
{
    static_cast<RegHandler*>(context)->onRegistrationCallback(status);
}

void RegHandler::onRegistrationCallback(Status status)
{
    if (mParam.pollRequest) {
        mParam.pollRequest();
    }
}

Status RegHandler::getObjectsStr(char* str, int maxSize)
{
    int size = 0;

    Object* object = mParam.objectManager.getFirstObject(ITF_REGISTER);

    while (object) {
        ObjectInstance* instance = object->getFirstInstance();

        int ret = 0;

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

        object = mParam.objectManager.getNextObject(ITF_REGISTER);
    }

    if (size > 0 && str[size - 1] == ',') {
        str[size - 1] = 0;
    }

    return STS_OK;
}

void RegHandler::registrationStatus(Status status)
{
    if (status == STS_OK) {
        LOG_INFO("Registration success /%d", getId());
    }
    else {
        LOG_ERROR("Registration failed /%d, status: %d", getId(), status);
    }
}

}  // namespace openlwm2m
