#include "resourcedesc.hpp"
#include "log.hpp"

#define LOG_MODULE "ResourceDesc"

namespace openlwm2m {

/*******************************************************************************
 * Private
 ******************************************************************************/

ResourceDesc::ResourceDesc(ItemBase* parent, uint16_t id, Params params) : ItemBase(parent, id), mParams(params)
{
    LOG_DEBUG("Create /%d/%d", getParent()->getId(), getId());
}

ResourceDesc::~ResourceDesc()
{
    LOG_DEBUG("Delete /%d/%d", getParent()->getId(), getId());
}

void ResourceDesc::init()
{
}

void ResourceDesc::release()
{
}

}  // namespace openlwm2m
