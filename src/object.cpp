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

Object::Object(uint16_t id, uint16_t interfaces, ItemInstance instance, size_t maxInstances, ItemMandatory mandatory)
    : ItemBase(NULL, id), mInterfaces(interfaces), mInstance(instance), mMandatory(mandatory)
{
    for (size_t i = 0; i < maxInstances; i++) {
        mInstanceStorage.addItem(new ObjectInstance(this));
    }
}

Object::~Object()
{
}

void Object::init()
{
    LOG_DEBUG("Create /%d", getId());

    // Appendix D.1
    // If the Object field “Mandatory” is “Mandatory” and the Object field “Instances” is “Single”then, the number
    // of Object Instance MUST be 1.
    if (mInstance == SINGLE && mMandatory == MANDATORY) {
        createInstance(0);
    }
}

void Object::release()
{
    LOG_DEBUG("Delete /%d", getId());

    mInstanceStorage.clear();
}

Status Object::createResourceString(uint16_t id, uint16_t operations, ItemInstance instance, size_t maxInstances,
                                    ItemMandatory mandatory, size_t maxLen, ResourceInfo::ValueChangeCbk callback,
                                    void* context)
{
    return createResource(id, operations, instance, maxInstances, mandatory, ResourceInfo::TYPE_STRING,
                          (ResourceInfo::Min){.minUint = 0}, (ResourceInfo::Max){.maxUint = maxLen}, callback, context);
}

#if 0
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
#endif

ObjectInstance* Object::getInstanceById(uint16_t id)
{
    return mInstanceStorage.getItemById(id);
}

ObjectInstance* Object::createInstance(uint16_t id, Status* status)
{
    ObjectInstance* instance = mInstanceStorage.newItem(id, status);

    if (instance) {
        sInstanceChanged = true;
    }

    return instance;
}

ObjectInstance* Object::getFirstInstance()
{
    return mInstanceStorage.getFirstItem();
}

ObjectInstance* Object::getNextInstance()
{
    return mInstanceStorage.getNextItem();
}

ResourceInstance* Object::getResourceInstance(uint16_t objInstanceId, uint16_t resId, uint16_t resInstanceId)
{
    ObjectInstance* objInstance = getInstanceById(objInstanceId);

    if (!objInstance) {
        return NULL;
    }

    return objInstance->getResourceInstance(resId, resInstanceId);
}

Status Object::setResourceChangedCbk(uint16_t resourceId, ResourceInfo::ValueChangeCbk callback, void* context)
{
    ResourceInfo* info = mResourceInfoStorage.getItemById(resourceId);

    if (!info) {
        return STS_ERR_NOT_FOUND;
    }

    info->setValueChangedCbk(callback, context);

    return STS_OK;
}

/*******************************************************************************
 * Private
 ******************************************************************************/

Status Object::createResource(uint16_t id, uint16_t operations, ItemInstance instance, size_t maxInstances,
                              ItemMandatory mandatory, ResourceInfo::DataType type, ResourceInfo::Min min,
                              ResourceInfo::Max max, ResourceInfo::ValueChangeCbk callback, void* context)
{
    // Appendix D.1
    // Resource which supports “Execute” operation MUST have “Single” as value of the “Instances” field.
    if (operations & ResourceInfo::OP_EXECUTE && instance != SINGLE) {
        return STS_ERR_INVALID_VALUE;
    }

    if (instance == SINGLE) {
        maxInstances = 1;
    }

    LOG_DEBUG("Create resource /%d/%d", getId(), id);

    ResourceInfo* info = new ResourceInfo(id, operations, instance, maxInstances, mandatory, type, min, max);

    info->setValueChangedCbk(callback, context);

    return mResourceInfoStorage.addItem(info);
}

}  // namespace openlwm2m
