#include "resource.hpp"
#include "log.hpp"

#define LOG_MODULE "Resource"

namespace openlwm2m {

Resource::Resource(Lwm2mBase* parent, ResourceDesc& desc) : Lwm2mBase(parent, desc.getId()), mDesc(desc)
{
    LOG_DEBUG("Create resource /%d/%d/%d", getParent()->getParent()->getId(), getParent()->getId(), getId());
}

Resource::~Resource()
{
    LOG_DEBUG("Delete resource /%d/%d/%d", getParent()->getParent()->getId(), getParent()->getId(), getId());
}

}  // namespace openlwm2m
