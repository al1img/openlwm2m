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

Status RegHandler::connect()
{
    Status status = STS_OK;

    ResourceInstance* shortServerIdInstance =
        mClient.getResourceInstance(ITF_REGISTER, OBJ_LWM2M_SERVER, getId(), RES_SHORT_SERVER_ID);

    if (!shortServerIdInstance) {
        return STS_ERR_NOT_EXIST;
    }

    ResourceInstance* serverUriInstance =
        mClient.getResourceInstance(ITF_REGISTER, OBJ_LWM2M_SECURITY, getId(), RES_LWM2M_SERVER_URI);

    if (!serverUriInstance) {
        return STS_ERR_NOT_EXIST;
    }

    const char* serverUri = serverUriInstance->getString();

    LOG_DEBUG("Reg handler /%d, connect to: %s", getId(), serverUri);

    mConnection = mClient.getTransport().createConnection(serverUri, &status);

    return status;
}

void RegHandler::init()
{
    LOG_DEBUG("Create reg handler /%d", getId());
}

void RegHandler::release()
{
    LOG_DEBUG("Delete reg handler /%d", getId());

    if (mConnection) {
        Status status = mClient.getTransport().deleteConnection(mConnection);

        if (status != STS_OK) {
            LOG_ERROR("Can't delete connection: %d", status);
        }
    }
}

}  // namespace openlwm2m