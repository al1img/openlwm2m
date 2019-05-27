#include "resourceinstance.hpp"
#include "log.hpp"

#define LOG_MODULE "ResourceInstance"

namespace openlwm2m {

/*******************************************************************************
 * Private
 ******************************************************************************/

ResourceInstance::ResourceInstance(ItemBase* parent, uint16_t id, ResourceDesc& desc)
    : ItemBase(parent, id), mDesc(desc)
{
}

ResourceInstance::~ResourceInstance()
{
}

void ResourceInstance::init()
{
    LOG_DEBUG("Create resource instance /%d/%d/%d/%d", getParent()->getParent()->getParent()->getId(),
              getParent()->getParent()->getId(), getParent()->getId(), getId());
}
void ResourceInstance::release()
{
    LOG_DEBUG("Delete resource instance /%d/%d/%d/%d", getParent()->getParent()->getParent()->getId(),
              getParent()->getParent()->getId(), getParent()->getId(), getId());
}

}  // namespace openlwm2m