#include "objectinstance.hpp"
#include "log.hpp"
#include "resource.hpp"

#define LOG_MODULE "ObjectInstance"

namespace openlwm2m {

/*******************************************************************************
 * Private
 ******************************************************************************/

ObjectInstance::ObjectInstance(ItemBase* parent, uint16_t id, ResourceDesc::Storage& resourceDescStorage)
    : ItemBase(parent, id), mResourceStorage(this)
{
    Node<ResourceDesc>* node = resourceDescStorage.begin();

    while (node) {
        Resource* resource = mResourceStorage.newItem(node->get()->getId(), *node->get());
        LWM2M_ASSERT(resource);
        node = node->next();
    }
}

ObjectInstance::~ObjectInstance() {}

void ObjectInstance::create()
{
    LOG_DEBUG("Create object instance /%d/%d", getParent()->getId(), getId());

    Node<Resource>* node = mResourceStorage.begin();

    while (node) {
        node->get()->init();
        node = node->next();
    }
}
void ObjectInstance::release()
{
    LOG_DEBUG("Delete object instance /%d/%d", getParent()->getId(), getId());
    Node<Resource>* node = mResourceStorage.begin();

    while (node) {
        node->get()->destroy();
        node = node->next();
    }
}

}  // namespace openlwm2m