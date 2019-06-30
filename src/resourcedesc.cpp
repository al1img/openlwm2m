#include "resourcedesc.hpp"
#include "log.hpp"

#define LOG_MODULE "ResourceDesc"

namespace openlwm2m {

/*******************************************************************************
 * Private
 ******************************************************************************/

ResourceDesc::ResourceDesc(ItemBase* parent, Params params) : ItemBase(parent), mParams(params)
{
}

ResourceDesc::~ResourceDesc()
{
}

void ResourceDesc::init()
{
    LOG_DEBUG("Create /%d/%d", getParent()->getId(), getId());
}

void ResourceDesc::release()
{
    LOG_DEBUG("Delete /%d/%d", getParent()->getId(), getId());
}

}  // namespace openlwm2m
