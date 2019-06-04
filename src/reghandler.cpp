#include "reghandler.hpp"
#include "log.hpp"

#define LOG_MODULE "RegHandler"

namespace openlwm2m {

/*******************************************************************************
 * Private
 ******************************************************************************/

RegHandler::RegHandler(ItemBase* parent, uint16_t id, TransportItf& transport)
    : ItemBase(parent, id), mTransport(transport)
{
}

RegHandler::~RegHandler()
{
}

void RegHandler::init()
{
    LOG_DEBUG("Create reg handler /%d", getId());
}

void RegHandler::release()
{
    LOG_DEBUG("Delete reg handler /%d", getId());
}

}  // namespace openlwm2m