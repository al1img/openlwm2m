#include "resourceinstance.hpp"
#include "log.hpp"

#define LOG_MODULE "ResourceInstance"

namespace openlwm2m {

ResourceInstance::ResourceInstance(Lwm2mBase* parent, uint16_t id, ResourceDesc& desc)
    : Lwm2mBase(parent, id), mDesc(desc)
{
    LOG_DEBUG("Create resource instance /%d/%d/%d/%d", getParent()->getParent()->getParent()->getId(),
              getParent()->getParent()->getId(), getParent()->getId(), getId());
}

ResourceInstance::~ResourceInstance()
{
    LOG_DEBUG("Delete resource instance /%d/%d/%d/%d", getParent()->getParent()->getParent()->getId(),
              getParent()->getParent()->getId(), getParent()->getId(), getId());
}

}  // namespace openlwm2m