#include "object.hpp"
#include "log.hpp"

#define LOG_MODULE "Object"

namespace openlwm2m {

/*******************************************************************************
 * Static
 ******************************************************************************/

bool Object::sInstanceChanged = false;

/*******************************************************************************
 * Public
 ******************************************************************************/

Status Object::createResourceString(uint16_t id, uint16_t operations, ResourceDesc::Instance instance,
                                    size_t maxInstances, ResourceDesc::Mandatory mandatory, size_t maxLen,
                                    ResourceDesc::ValueChangeCbk cbk, void* context)
{
    ResourceDesc::Params params = {operations, instance, maxInstances, mandatory, DATA_TYPE_STRING, cbk, context};

    params.minUint = 0;
    params.maxUint = maxLen;

    return createResource(id, params);
}

Status Object::createResourceInt(uint16_t id, uint16_t operations, ResourceDesc::Instance instance, size_t maxInstances,
                                 ResourceDesc::Mandatory mandatory, int64_t min, int64_t max,
                                 ResourceDesc::ValueChangeCbk cbk, void* context)
{
    ResourceDesc::Params params = {operations, instance, maxInstances, mandatory, DATA_TYPE_INT, cbk, context};

    params.minInt = min;
    params.maxInt = max;

    return createResource(id, params);
}

Status Object::createResourceUint(uint16_t id, uint16_t operations, ResourceDesc::Instance instance,
                                  size_t maxInstances, ResourceDesc::Mandatory mandatory, uint64_t min, uint64_t max,
                                  ResourceDesc::ValueChangeCbk cbk, void* context)
{
    ResourceDesc::Params params = {operations, instance, maxInstances, mandatory, DATA_TYPE_UINT, cbk, context};

    params.minUint = min;
    params.maxUint = max;

    return createResource(id, params);
}

Status Object::createResourceBool(uint16_t id, uint16_t operations, ResourceDesc::Instance instance,
                                  size_t maxInstances, ResourceDesc::Mandatory mandatory,
                                  ResourceDesc::ValueChangeCbk cbk, void* context)
{
    ResourceDesc::Params params = {operations, instance, maxInstances, mandatory, DATA_TYPE_BOOL, cbk, context};

    params.minUint = 0;
    params.maxUint = 0;

    return createResource(id, params);
}

Status Object::createResourceOpaque(uint16_t id, uint16_t operations, ResourceDesc::Instance instance,
                                    size_t maxInstances, ResourceDesc::Mandatory mandatory, size_t minSize,
                                    size_t maxSize, ResourceDesc::ValueChangeCbk cbk, void* context)
{
    ResourceDesc::Params params = {operations, instance, maxInstances, mandatory, DATA_TYPE_OPAQUE, cbk, context};

    params.minUint = minSize;
    params.maxUint = maxSize;

    return createResource(id, params);
}

Status Object::createResourceNone(uint16_t id, uint16_t operations, ResourceDesc::Instance instance,
                                  size_t maxInstances, ResourceDesc::Mandatory mandatory,
                                  ResourceDesc::ValueChangeCbk cbk, void* context)
{
    ResourceDesc::Params params = {operations, instance, maxInstances, mandatory, DATA_TYPE_NONE, cbk, context};

    params.minUint = 0;
    params.maxUint = 0;

    return createResource(id, params);
}

ObjectInstance* Object::getInstanceById(uint16_t id)
{
    if (!mInstanceStorage) {
        return NULL;
    }

    return mInstanceStorage->getItemById(id);
}

ObjectInstance* Object::createInstance(uint16_t id, Status* status)
{
    if (!mInstanceStorage) {
        if (status) *status = STS_ERR_INVALID_STATE;
        return NULL;
    }

    ObjectInstance* instance = mInstanceStorage->newItem(id, status);

    if (instance) {
        sInstanceChanged = true;
    }

    return instance;
}

ObjectInstance* Object::getFirstInstance()
{
    if (mInstanceStorage) {
        return mInstanceStorage->getFirstItem();
    }

    return NULL;
}

ObjectInstance* Object::getNextInstance()
{
    if (mInstanceStorage) {
        return mInstanceStorage->getNextItem();
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

Status Object::setResourceChangedCbk(uint16_t resourceId, ResourceDesc::ValueChangeCbk cbk, void* context)
{
    ResourceDesc* resourceDesc = mResourceDescStorage.getItemById(resourceId);

    if (!resourceDesc) {
        return STS_ERR_NO_MEM;
    }

    resourceDesc->setValueChangedCbk(cbk, context);

    return STS_OK;
}

/*******************************************************************************
 * Private
 ******************************************************************************/

Object::Object(ItemBase* parent, Params params) : ItemBase(parent), mParams(params), mInstanceStorage(NULL)
{
}

Object::~Object()
{
    delete mInstanceStorage;
}

void Object::init()
{
    LOG_DEBUG("Create /%d", getId());

    if (!mInstanceStorage) {
        mResourceDescStorage.init();

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
    LOG_DEBUG("Delete /%d", getId());

    if (mInstanceStorage) {
        mInstanceStorage->clear();
    }

    mResourceDescStorage.release();
}

Status Object::createResource(uint16_t id, ResourceDesc::Params& params)
{
    if (mInstanceStorage) {
        return STS_ERR_INVALID_STATE;
    }

    // Appendix D.1
    // Resource which supports “Execute” operation MUST have “Single” as value of the “Instances” field.
    if (params.operations & ResourceDesc::OP_EXECUTE && params.instance != ResourceDesc::SINGLE) {
        return STS_ERR_INVALID_VALUE;
    }

    if (params.instance == ResourceDesc::SINGLE) {
        params.maxInstances = 1;
    }

    LOG_DEBUG("Create resource /%d/%d", getId(), id);

    Status status = STS_OK;

    mResourceDescStorage.newItem(this, id, params, &status);

    return status;
}

}  // namespace openlwm2m
