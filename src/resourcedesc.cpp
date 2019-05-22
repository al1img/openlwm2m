#include "resourcedesc.hpp"
#include "log.hpp"

#define LOG_MODULE "ResourceDesc"

namespace openlwm2m {

/*******************************************************************************
 * Private
 ******************************************************************************/

ResourceDesc::ResourceDesc(ItemBase* parent, uint16_t id, Params params) : ItemBase(parent, id), mParams(params)
{
    LOG_DEBUG("Create resource desc /%d/%d", getParent()->getId(), getId());
}

ResourceDesc::~ResourceDesc() { LOG_DEBUG("Delete resource desc /%d/%d", getParent()->getId(), getId()); }

}  // namespace openlwm2m