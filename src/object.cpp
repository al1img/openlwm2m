#include "object.hpp"
#include "log.hpp"

#define LOG_MODULE "Object"

namespace openlwm2m {

/*******************************************************************************
 * Public
 ******************************************************************************/

Status Object::createResource(uint16_t id, uint16_t operations, ResourceDesc::Instance instance, size_t maxInstances,
                              ResourceDesc::Mandatory mandatory, ResourceDesc::Type type, int min, int max)
{
    if (mInstanceStorage) {
        return STS_ERR_INVALID_STATE;
    }

    // Appendix D.1
    // Resource which supports “Execute” operation MUST have “Single” as value of the “Instances” field.
    if (operations & ResourceDesc::OP_EXECUTE && instance != ResourceDesc::SINGLE) {
        return STS_ERR_INVALID_VALUE;
    }

    if (instance == ResourceDesc::SINGLE) {
        maxInstances = 1;
    }

    ResourceDesc::Params params = {operations, instance, mandatory, type, min, max, maxInstances};

    Status status = STS_OK;

    mResourceDescStorage.createItem(id, params, &status);

    return status;
}

ObjectInstance* Object::getInstanceById(uint16_t id)
{
    return mInstanceStorage->getItemById(id);
}

ObjectInstance* Object::createInstance(uint16_t id, Status* status)
{
    if (!mInstanceStorage) {
        if (status) *status = STS_ERR_INVALID_STATE;
        return NULL;
    }

    // Create instance
    return mInstanceStorage->newItem(id, mResourceDescStorage, status);
}

ObjectInstance* Object::getFirstInstance()
{
    if (mInstanceStorage) {
        mInstanceNode = mInstanceStorage->begin();

        if (mInstanceNode) {
            return mInstanceNode->get();
        }
    }

    return NULL;
}

ObjectInstance* Object::getNextInstance()
{
    if (mInstanceNode) {
        mInstanceNode = mInstanceNode->next();

        if (mInstanceNode) {
            return mInstanceNode->get();
        }
    }

    return NULL;
}

ResourceInstance* Object::getResourceInstance(uint16_t objInstanceId, uint16_t resId, uint16_t resInstanceId)
{
    ObjectInstance* objInstance = getInstanceById(objInstanceId);

    if (!objInstance) {
        return NULL;
    }

    return objInstance->getResourceInstance(resId, resInstanceId);
}

/*******************************************************************************
 * Private
 ******************************************************************************/

Object::Object(ItemBase* parent, uint16_t id, Params params)
    : ItemBase(parent, id), mParams(params), mResourceDescStorage(this), mInstanceStorage(NULL), mInstanceNode(NULL)
{
    LOG_DEBUG("Create object /%d", getId());
}

Object::~Object()
{
    LOG_DEBUG("Delete object /%d", getId());

    delete mInstanceStorage;
}

void Object::init()
{
    //    LOG_DEBUG("Init object /%d", getId());

    if (!mInstanceStorage) {
        mInstanceStorage = new ObjectInstance::Storage(this, mResourceDescStorage, mParams.maxInstances);

        // Appendix D.1
        // If the Object field “Mandatory” is “Mandatory” and the Object field “Instances” is “Single”then, the number
        // of Object Instance MUST be 1.
        if (mParams.instance == SINGLE && mParams.mandatory == MANDATORY) {
            if (mInstanceStorage->size() == 0) {
                ObjectInstance* instance = createInstance();
                ASSERT(instance);
            }
        }
    }
}

void Object::release()
{
    //    LOG_DEBUG("Release object /%d", getId());

    if (mInstanceStorage) {
        mInstanceStorage->clear();
    }

    mResourceDescStorage.release();
}

}  // namespace openlwm2m
