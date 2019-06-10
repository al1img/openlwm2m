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

    return STS_OK;
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
}

}  // namespace openlwm2m