#include "objectinstance.hpp"
#include "log.hpp"
#include "object.hpp"
#include "resource.hpp"

#define LOG_MODULE "ObjectInstance"

namespace openlwm2m {

/*******************************************************************************
 * Public
 ******************************************************************************/

ObjectInstance::ObjectInstance(Object* parent) : ItemBase(parent)
{
}

ObjectInstance::~ObjectInstance()
{
}

void ObjectInstance::init()
{
    LOG_DEBUG("Create /%d/%d", getParent()->getId(), getId());

    mResourceStorage.init();
}

void ObjectInstance::release()
{
    mResourceStorage.release();

    LOG_DEBUG("Delete /%d/%d", getParent()->getId(), getId());
}

Status ObjectInstance::addResource(ResourceInfo& info)
{
    return mResourceStorage.pushItem(new Resource(this, info));
}

ResourceInstance* ObjectInstance::getResourceInstance(uint16_t resId, uint16_t resInstanceId)
{
    Resource* resource = getResourceById(resId);

    if (!resource) {
        return NULL;
    }

    return resource->getInstanceById(resInstanceId);
}

/*******************************************************************************
 * Private
 ******************************************************************************/

}  // namespace openlwm2m
