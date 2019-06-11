#include "reghandler.hpp"
#include "client.hpp"
#include "log.hpp"

#define LOG_MODULE "RegHandler"

namespace openlwm2m {

/*******************************************************************************
 * Private
 ******************************************************************************/

RegHandler::RegHandler(ItemBase* parent, uint16_t id, Client& client)
    : ItemBase(parent, id), mClient(client), mConnection(NULL)
{
}

RegHandler::~RegHandler()
{
}

void RegHandler::init()
{
    LOG_DEBUG("Create /%d", getId());

    mConnection = mServerInstance = mSecurityInstance = NULL;
    mState = STATE_INIT;
}

void RegHandler::release()
{
    LOG_DEBUG("Delete /%d", getId());

    if (mConnection) {
        Status status = mClient.getTransport().deleteConnection(mConnection);

        if (status != STS_OK) {
            LOG_ERROR("Can't delete connection: %d", status);
        }
    }

    mTimer.stop();
    mState = STATE_INIT;
}

Status RegHandler::bind(ObjectInstance* serverInstance)
{
    mServerInstance = serverInstance;

    Object* object = mClient.getObject(ITF_CLIENT, OBJ_LWM2M_SECURITY);
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

    Status status = STS_OK;

    mConnection = mClient.getTransport().createConnection(serverUri, &status);

    return status;
}

Status RegHandler::startRegistration()
{
    LOG_DEBUG("Start /%d", getId());

    ResourceInstance* initRegDelay = mServerInstance->getResourceInstance(RES_INITIAL_REGISTRATION_DELAY);

    uint64_t initDelayMs = 0;

    if (initRegDelay) {
        initDelayMs = initRegDelay->getUint();
    }

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
    LOG_DEBUG("Timer /%d, state: %d", getId(), mState);

    switch (mState) {
        case STATE_INIT_DELAY:
            mState = STATE_REGISTRATION;
            mClient.mTransport.registrationRequest(
                mClient.mName, mServerInstance->getResourceInstance(RES_LIFETIME)->getInt(), LWM2M_VERSION,
                mServerInstance->getResourceInstance(RES_BINDING)->getString(), mClient.mQueueMode, NULL, NULL,
                &RegHandler::registrationCallback, this);
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
}

}  // namespace openlwm2m