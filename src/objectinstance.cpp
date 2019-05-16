#include "objectinstance.hpp"
#include "log.hpp"
#include "resource.hpp"

#define LOG_MODULE "ObjectInstance"

namespace openlwm2m {

ObjectInstance::ObjectInstance(uint16_t id, uint16_t objectId, List& resourceDescList) : mId(id), mObjectId(objectId)
{
    LOG_DEBUG("Create object instance /%d/%d", mObjectId, mId);

    Node* node = resourceDescList.begin();

    while (node) {
        Resource* resource = new Resource(mId, *static_cast<ResourceDesc*>(node->get()));

        mResourceList.append(resource);
        node = node->next();
    }
}

ObjectInstance::~ObjectInstance()
{
    LOG_DEBUG("Delete object instance /%d/%d", mObjectId, mId);

    Node* node = mResourceList.begin();

    while (node) {
        delete static_cast<Resource*>(node->get());
        node = node->next();
    }
}

}  // namespace openlwm2m