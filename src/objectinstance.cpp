#include "objectinstance.hpp"
#include "log.hpp"
#include "resource.hpp"

#define LOG_MODULE "ObjectInstance"

namespace openlwm2m {

/*******************************************************************************
 * Public
 ******************************************************************************/

Resource* ObjectInstance::getResourceById(uint16_t id)
{
    return mResourceStorage.getItemById(id);
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

ObjectInstance::ObjectInstance(ItemBase* parent, ResourceDesc::Storage& resourceDescStorage)
    : ItemBase(parent), mResourceStorage()
{
    Node<ResourceDesc>* node = resourceDescStorage.begin();

    while (node) {
        Resource* resource = mResourceStorage.newItem(this, node->get()->getId(), *node->get());
        ASSERT(resource);
        node = node->next();
    }
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
    LOG_DEBUG("Delete /%d/%d", getParent()->getId(), getId());

    mResourceStorage.release();
}

}  // namespace openlwm2m
