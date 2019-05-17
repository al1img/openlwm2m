#include "object.hpp"
#include "log.hpp"

#define LOG_MODULE "Object"

namespace openlwm2m {

Object::Object(uint16_t id, Instance instance, size_t maxInstances, Mandatory mandatory, uint16_t interfaces)
    : Lwm2mBase(NULL, id),
      mInstance(instance),
      mMaxInstances(maxInstances),
      mMandatory(mandatory),
      mInterfaces(interfaces),
      mStarted(false)
#ifdef RESERVE_MEMORY
      ,
      mInstanceStorage(maxInstances)
#endif
{
    LOG_DEBUG("Create object /%d", getId());
}

Object::~Object()
{
    LOG_DEBUG("Delete object /%d", getId());

    deleteInstances();
    deleteResources();
}

Status Object::createResource(uint16_t id, uint16_t operations, ResourceDesc::Instance instance, size_t maxInstances,
                              ResourceDesc::Mandatory mandatory, ResourceDesc::Type type, int min, int max)
{
    if (mStarted) {
        return STS_ERR_STATE;
    }

    // Appendix D.1
    // Resource which supports “Execute”operation MUST have “Single” as value of the “Instances” field.
    if (operations & ResourceDesc::OP_EXECUTE && instance != ResourceDesc::SINGLE) {
        return STS_ERR_VALUE;
    }

    if (instance == ResourceDesc::SINGLE) {
        maxInstances = 1;
    }

    ResourceDesc* resourceDesc =
        new ResourceDesc(this, id, operations, instance, maxInstances, mandatory, type, min, max);

    mResourceDescList.append(resourceDesc);

    return STS_OK;
}

Status Object::start()
{
    LOG_DEBUG("Start object /%d", getId());

#ifdef RESERVE_MEMORY
    for (size_t i = 0; i < mMaxInstances; i++) {
        ObjectInstance* instance = new ObjectInstance(this, i, mResourceDescList);
        mInstanceStorage.pushItem(instance);
    }
#endif

    // Appendix D.1
    // If the Object field “Mandatory” is “Mandatory” and the Object field “Instances” is “Single”then, the number of
    // Object Instance MUST be 1.
    if (mInstance == SINGLE && mMandatory == MANDATORY) {
        if (getInstanceCount() == 0) {
            Status status;

            if (createInstance(&status) == NULL) {
                return status;
            }
        }
    }

    return STS_OK;
}

bool Object::hasFreeInstance()
{
#ifdef RESERVE_MEMORY
    return mInstanceStorage.size() < mInstanceStorage.maxSize();
#else
    return mMaxInstances == 0 || mInstanceList.size() < mMaxInstances;
#endif
}

size_t Object::getInstanceCount()
{
#ifdef RESERVE_MEMORY
    return mInstanceStorage.size();
#else
    return mInstanceList.size();
#endif
}

ObjectInstance* Object::createInstance(Status* status)
{
    if (status) {
        *status = STS_OK;
    }

    // Check size
    if (!hasFreeInstance()) {
        if (status) {
            *status = STS_ERR_MEM;
        }

        return NULL;
    }

    ObjectInstance* instance;

#ifdef RESERVE_MEMORY

    instance = static_cast<ObjectInstance*>(mInstanceStorage.newItem(status));

    if (instance) {
        LOG_DEBUG("Create object instance /%d/%d", getId(), instance->getId());
    }

#else
    Node* node;
    uint16_t id = 0;

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
    instance = new ObjectInstance(this, id, mResourceDescList);

    if (node) {
        mInstanceList.insert(instance, node);
    }
    else {
        mInstanceList.append(instance);
    }

#endif

    return instance;
}

void Object::deleteInstances()
{
#ifdef RESERVE_MEMORY
    ObjectInstance* instance = static_cast<ObjectInstance*>(mInstanceStorage.begin(true));

    while (instance) {
        delete instance;
        instance = static_cast<ObjectInstance*>(mInstanceStorage.next(true));
    }
#else
    Node* node = mInstanceList.begin();

    while (node) {
        delete static_cast<ObjectInstance*>(node->get());
        node = node->next();
    }
#endif
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
