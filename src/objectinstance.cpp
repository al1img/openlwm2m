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
        Resource* resource = mResourceStorage.createItem(node->get()->getId(), *node->get());
        ASSERT(resource);
        node = node->next();
    }
}

ObjectInstance::~ObjectInstance() {}

void ObjectInstance::init()
{
    LOG_DEBUG("Create object instance /%d/%d", getParent()->getId(), getId());

    mResourceStorage.init();
}

void ObjectInstance::release()
{
    LOG_DEBUG("Delete object instance /%d/%d", getParent()->getId(), getId());

    mResourceStorage.release();
}

}  // namespace openlwm2m