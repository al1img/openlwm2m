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

Object::Object(uint16_t id, uint16_t interfaces, bool single, bool mandatory, size_t maxInstances)
    : ItemBase(NULL, id), mInterfaces(interfaces), mSingle(single), mMandatory(mandatory), mInitialized(false)
{
    if (single) {
        maxInstances = 1;
    }

    for (size_t i = 0; i < maxInstances; i++) {
        mInstanceStorage.pushItem(new ObjectInstance(this));
    }
}

Object::~Object()
{
}

void Object::init()
{
    if (mInitialized) {
        return;
    }

    LOG_DEBUG("Create /%d", getId());

    mInitialized = true;

    // Appendix D.1
    // If the Object field “Mandatory” is “Mandatory” and the Object field “Instances” is “Single”then, the number
    // of Object Instance MUST be 1.
    if (mSingle && mMandatory) {
        ObjectInstance* instance = createInstance(0);
        ASSERT(instance);
    }
}

void Object::release()
{
    if (!mInitialized) {
        return;
    }

    LOG_DEBUG("Delete /%d", getId());

    mInitialized = false;

    mInstanceStorage.clear();
}

Status Object::createResourceString(uint16_t id, uint16_t operations, bool single, bool mandatory, size_t maxInstances,
                                    size_t maxLen, ResourceInfo::ValueChangeCbk callback, void* context)
{
    return createResource(id, operations, DATA_TYPE_STRING, single, mandatory, maxInstances,
                          (ResourceInfo::Min){.minUint = 0}, (ResourceInfo::Max){.maxUint = maxLen}, callback, context);
}

Status Object::createResourceInt(uint16_t id, uint16_t operations, bool single, bool mandatory, size_t maxInstances,
                                 int64_t min, int64_t max, ResourceInfo::ValueChangeCbk callback, void* context)
{
    return createResource(id, operations, DATA_TYPE_INT, single, mandatory, maxInstances,
                          (ResourceInfo::Min){.minInt = min}, (ResourceInfo::Max){.maxInt = max}, callback, context);
}

Status Object::createResourceUint(uint16_t id, uint16_t operations, bool single, bool mandatory, size_t maxInstances,
                                  uint64_t min, uint64_t max, ResourceInfo::ValueChangeCbk callback, void* context)
{
    return createResource(id, operations, DATA_TYPE_UINT, single, mandatory, maxInstances,
                          (ResourceInfo::Min){.minUint = min}, (ResourceInfo::Max){.maxUint = max}, callback, context);
}

Status Object::createResourceBool(uint16_t id, uint16_t operations, bool single, bool mandatory, size_t maxInstances,
                                  ResourceInfo::ValueChangeCbk callback, void* context)
{
    return createResource(id, operations, DATA_TYPE_BOOL, single, mandatory, maxInstances,
                          (ResourceInfo::Min){.minUint = 0}, (ResourceInfo::Max){.maxUint = 0}, callback, context);
}

Status Object::createResourceOpaque(uint16_t id, uint16_t operations, bool single, bool mandatory, size_t maxInstances,
                                    size_t minSize, size_t maxSize, ResourceInfo::ValueChangeCbk callback,
                                    void* context)
{
    return createResource(id, operations, DATA_TYPE_OPAQUE, single, mandatory, maxInstances,
                          (ResourceInfo::Min){.minUint = minSize}, (ResourceInfo::Max){.maxUint = maxSize}, callback,
                          context);
}

Status Object::createResourceNone(uint16_t id, uint16_t operations, bool single, bool mandatory, size_t maxInstances,
                                  ResourceInfo::ValueChangeCbk callback, void* context)
{
    return createResource(id, operations, DATA_TYPE_NONE, single, mandatory, maxInstances,
                          (ResourceInfo::Min){.minUint = 0}, (ResourceInfo::Max){.maxUint = 0}, callback, context);
}

ObjectInstance* Object::createInstance(uint16_t id, Status* status)
{
    if (!mInitialized) {
        if (status) *status = STS_ERR_NOT_ALLOWED;
        return NULL;
    }

    ObjectInstance* instance = mInstanceStorage.allocateItem(id, status);

    if (instance) {
        sInstanceChanged = true;
    }

    return instance;
}

Status Object::deleteInstance(uint16_t id)
{
    if (!mInitialized) {
        return STS_ERR_NOT_ALLOWED;
    }

    if (mSingle && mMandatory) {
        return STS_ERR_NOT_ALLOWED;
    }

    ObjectInstance* instance = mInstanceStorage.getItemById(id);

    if (!instance) {
        return STS_ERR_NOT_FOUND;
    }

    Status status = mInstanceStorage.deallocateItem(instance);

    if (status == STS_OK) {
        sInstanceChanged = true;
    }

    return status;
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

Status Object::write(DataConverter* converter, bool checkOperation, bool ignoreMissing, bool replace)
{
    Status status = STS_OK;
    DataConverter::ResourceData resourceData;

    LOG_DEBUG("Write /%d", getId());

    if (replace) {
        mInstanceStorage.clear();

        if (mSingle && mMandatory) {
            ObjectInstance* instance = createInstance(0);
            ASSERT(instance);
        }
    }

    while ((status = converter->nextDecoding(&resourceData)) == STS_OK) {
        if (resourceData.objectId != getId() || resourceData.objectInstanceId == INVALID_ID ||
            resourceData.resourceId == INVALID_ID) {
            LOG_ERROR("Unexpected path: /%d/%d/%d", resourceData.objectId, resourceData.objectInstanceId,
                      resourceData.resourceId);
            return STS_ERR_FORMAT;
        }

        ObjectInstance* objectInstance = getInstanceById(resourceData.objectInstanceId);

        if (!objectInstance) {
            objectInstance = createInstance(resourceData.objectInstanceId, &status);
            if (!objectInstance) {
                LOG_ERROR("Can't create object instance: /%d/%d, status: %d", resourceData.objectId,
                          resourceData.objectInstanceId, status);
                return status;
            }
        }

        Resource* resource = objectInstance->getResourceById(resourceData.resourceId);

        if (!resource) {
            if (!ignoreMissing) {
                LOG_ERROR("Can't find resource: /%d/%d/%d", resourceData.objectId, resourceData.objectInstanceId,
                          resourceData.resourceId);
                return STS_ERR_NOT_FOUND;
            }
            else {
                continue;
            }
        }

        if (checkOperation && !resource->getInfo().checkOperation(OP_WRITE)) {
            return STS_ERR_NOT_ALLOWED;
        }

        if (resource->getInfo().isSingle()) {
            if (resourceData.resourceInstanceId != INVALID_ID) {
                LOG_ERROR("Address single resource as multiple: /%d/%d/%d/%d", resourceData.objectId,
                          resourceData.objectInstanceId, resourceData.resourceId, resourceData.resourceInstanceId);
                return STS_ERR_NOT_FOUND;
            }
            else {
                resourceData.resourceInstanceId = 0;
            }
        }

        ResourceInstance* resourceInstance = resource->getInstanceById(resourceData.resourceInstanceId);

        if (!resourceInstance) {
            resourceInstance = resource->createInstance(resourceData.resourceInstanceId, &status);
            if (!resourceInstance) {
                LOG_ERROR("Can't find resource instance: /%d/%d/%d/%d", resourceData.objectId,
                          resourceData.objectInstanceId, resourceData.resourceId, resourceData.resourceInstanceId);
                return status;
            }
        }

        if ((status = resourceInstance->write(&resourceData)) != STS_OK) {
            LOG_ERROR("Can't write resource instance: /%d/%d/%d/%d, status: %d", resourceData.objectId,
                      resourceData.objectInstanceId, resourceData.resourceId, resourceData.resourceInstanceId, status);
            return status;
        }
    }

    if (status != STS_ERR_NOT_FOUND) {
        return status;
    }

    return STS_OK;
}

Status Object::read(DataConverter* converter, bool checkOperation)
{
    Status status = STS_OK;

    for (ObjectInstance* instance = getFirstInstance(); instance != NULL; instance = getNextInstance()) {
        if ((status = instance->read(converter, checkOperation)) != STS_OK) {
            return status;
        }
    }

    return STS_OK;
}

/*******************************************************************************
 * Private
 ******************************************************************************/

Status Object::createResource(uint16_t id, uint16_t operations, DataType type, bool single, bool mandatory,
                              size_t maxInstances, ResourceInfo::Min min, ResourceInfo::Max max,
                              ResourceInfo::ValueChangeCbk callback, void* context)
{
    if (mInitialized) {
        return STS_ERR_NOT_ALLOWED;
    }

    // Appendix D.1
    // Resource which supports “Execute” operation MUST have “Single” as value of the “Instances” field.
    if (operations & OP_EXECUTE && !single) {
        return STS_ERR_INVALID_VALUE;
    }

    if (single) {
        maxInstances = 1;
    }

    LOG_DEBUG("Create resource type %d /%d/%d", type, getId(), id);

    ResourceInfo* info = new ResourceInfo(id, operations, type, single, mandatory, maxInstances, min, max);

    info->setValueChangedCbk(callback, context);

    Status status = STS_OK;

    if ((status = mResourceInfoStorage.pushItem(info)) != STS_OK) {
        return status;
    }

    ObjectInstance* objectInstance = mInstanceStorage.getFirstFreeItem();

    while (objectInstance) {
        objectInstance->addResource(*info);

        objectInstance = mInstanceStorage.getNextFreeItem();
    }

    return STS_OK;
}

}  // namespace openlwm2m
