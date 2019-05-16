#include "object.hpp"
#include "log.hpp"

#define LOG_MODULE "Object"

namespace openlwm2m {

Object::Object(uint16_t id, Instance instance, size_t maxInstances, Mandatory mandatory, uint16_t interfaces)
    : mId(id),
      mInstance(instance),
      mMaxInstances(maxInstances),
      mMandatory(mandatory),
      mInterfaces(interfaces),
      mStarted(false)
{
    LOG_DEBUG("Create object /%d", mId);
}

Object::~Object()
{
    LOG_DEBUG("Delete object /%d", mId);

    deleteInstances();
    deleteResources();
}

Status Object::createResource(uint16_t id, uint16_t operations, Resource::Instance instance, size_t maxInstances,
                              Resource::Mandatory mandatory, Resource::Type type, int min, int max)
{
    if (mStarted) {
        return STS_ERR_STATE;
    }

    if (instance == Resource::SINGLE) {
        maxInstances = 1;
    }

    ResourceDesc* resourceDesc =
        new ResourceDesc(id, mId, operations, instance, maxInstances, mandatory, type, min, max);

    mResourceDescList.append(resourceDesc);

    return STS_OK;
}

Status Object::start()
{
    LOG_DEBUG("Start object /%d", mId);

    // Appendix D.1
    // Create single mandatory instance
    if (mInstance == SINGLE && mMandatory == MANDATORY) {
        if (getInstanceCount() == 0) {
            Status status;

            if (createInstance(ITF_BOOTSTRAP, &status) == NULL) {
                return status;
            }
        }
    }

    return STS_OK;
}

bool Object::hasFreeInstance() { return mMaxInstances == 0 || mInstanceList.size() < mMaxInstances; }

size_t Object::getInstanceCount() { return mInstanceList.size(); }

ObjectInstance* Object::createInstance(Interface interface, Status* status)
{
    Status retStatus = STS_OK;
    uint16_t id = 0;
    Node* node;
    ObjectInstance* instance;

    // Check interface
    if ((interface & mInterfaces) == 0) {
        retStatus = STS_ERR_ACCESS;
        goto error;
    }

    // Check size
    if (!hasFreeInstance()) {
        retStatus = STS_ERR_MEM;
        goto error;
    }

    // Get free id
    node = mInstanceList.begin();

    while (node) {
        instance = static_cast<ObjectInstance*>(node->get());

        if (id < instance->getId()) {
            break;
        }

        id++;
        node = node->next();
    }

    // Create instance
    instance = new ObjectInstance(id, mId, mResourceDescList);

    if (node) {
        mInstanceList.insert(instance, node);
    }
    else {
        mInstanceList.append(instance);
    }

    return instance;

error:

    if (status) {
        *status = retStatus;
    }

    return NULL;
}

void Object::deleteInstances()
{
    Node* node = mInstanceList.begin();

    while (node) {
        delete static_cast<ObjectInstance*>(node->get());
        node = node->next();
    }
}

void Object::deleteResources()
{
    Node* node = mResourceDescList.begin();

    while (node) {
        delete static_cast<ResourceDesc*>(node->get());
        node = node->next();
    }
}

}  // namespace openlwm2m
