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
    if (mInitialized) {
        return STS_ERR_STATE;
    }

    // Appendix D.1
    // Resource which supports “Execute” operation MUST have “Single” as value of the “Instances” field.
    if (operations & ResourceDesc::OP_EXECUTE && instance != ResourceDesc::SINGLE) {
        return STS_ERR_VALUE;
    }

    if (instance == ResourceDesc::SINGLE) {
        maxInstances = 1;
    }

    ResourceDesc::Params params = {operations, instance, mandatory, type, min, max, maxInstances};

    Status status = STS_OK;

    mResourceDescStorage.createItem(id, params, &status);

    return status;
}

ObjectInstance* Object::createInstance(uint16_t id, Status* status)
{
    // Check state
    if (!mInitialized) {
        if (status) *status = STS_ERR_STATE;
        return NULL;
    }

    // Check size
    if (!mInstanceStorage || !mInstanceStorage->hasFreeItem()) {
        if (status) *status = STS_ERR_MEM;
        return NULL;
    }

    // Create instance
    return mInstanceStorage->newItem(id, mResourceDescStorage, status);
}

/*******************************************************************************
 * Private
 ******************************************************************************/

Object::Object(ItemBase* parent, uint16_t id, Params params)
    : ItemBase(parent, id), mParams(params), mInitialized(false), mResourceDescStorage(this), mInstanceStorage(NULL)
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

    mInitialized = true;

    mInstanceStorage = new ObjectInstance::Storage(this, mResourceDescStorage, mParams.mMaxInstances);

    // Appendix D.1
    // If the Object field “Mandatory” is “Mandatory” and the Object field “Instances” is “Single”then, the number
    // of Object Instance MUST be 1.
    if (mParams.mInstance == SINGLE && mParams.mMandatory == MANDATORY) {
        if (mInstanceStorage->size() == 0) {
            ObjectInstance* instance = createInstance();
            ASSERT(instance);
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
