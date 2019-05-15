#include "object.hpp"
#include "log.hpp"

#define LOG_MODULE "Object"

namespace openlwm2m {

Object::Object(uint16_t id, int maxInstances, bool mandatory, uint16_t interfaces)
    : mId(id), mMaxInstances(maxInstances), mMandatory(mandatory), mInterfaces(interfaces)
{
    LOG_DEBUG("Create object %d", id);
}

Object::~Object()
{
    Node* node = mResourceList.begin();

    while (node != nullptr) {
        delete static_cast<Resource*>(node->get());
        node = node->next();
    }

    LOG_DEBUG("Delete object %d", mId);
}

Resource* Object::createResource(uint16_t id, uint16_t operations, int maxInstances, bool mandatory, ResourceType type,
                                 int min, int max)
{
    Resource* resource = new Resource(id, operations, maxInstances, mandatory, type, min, max);

    mResourceList.append(resource);

    return resource;
}

}  // namespace openlwm2m
