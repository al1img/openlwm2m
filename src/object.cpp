#include "object.hpp"
#include "log.hpp"

#define LOG_MODULE "Object"

namespace openlwm2m {

Object::Object(uint16_t id, ObjectInstance instance, int maxInstances, ObjectMandatory mandatory, uint16_t interfaces)
    : mId(id), mInstance(instance), mMaxInstances(maxInstances), mMandatory(mandatory), mInterfaces(interfaces)
{
    LOG_DEBUG("Create object %d", id);
}

Object::~Object()
{
    Node* node = mResourceDescList.begin();

    while (node) {
        delete static_cast<ResourceDesc*>(node->get());
        node = node->next();
    }

    LOG_DEBUG("Delete object %d", mId);
}

void Object::createResource(uint16_t id, uint16_t operations, ResourceInstance instance, int maxInstances,
                            ResourceMandatory mandatory, ResourceType type, int min, int max)
{
    ResourceDesc* resourceDesc = new ResourceDesc(id, operations, instance, maxInstances, mandatory, type, min, max);

    mResourceDescList.append(resourceDesc);
}

}  // namespace openlwm2m
