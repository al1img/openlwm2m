#include "objectmanager.hpp"

#include <cstdlib>

#if CONFIG_DATA_FORMAT_SENML_JSON
#include "jsonconverter.hpp"
#endif
#include "config.hpp"
#include "lwm2m.hpp"
#include "textconverter.hpp"
#include "utils.hpp"

#define LOG_MODULE "ObjectManager"

namespace openlwm2m {

/*******************************************************************************
 * Public
 ******************************************************************************/

ObjectManager::ObjectManager()
{
    createSecurityObject();
    createServerObject();
    createDeviceObject();

    createConverters();
}

ObjectManager::~ObjectManager()
{
    mObjectStorage.release();
}

void ObjectManager::init()
{
    mObjectStorage.init();
}

Object* ObjectManager::createObject(uint16_t id, uint16_t interfaces, bool single, bool mandatory, size_t maxInstances,
                                    Status* status)
{
    if (single) {
        maxInstances = 1;
    }

    LOG_DEBUG("Create object /%d", id);

    Object* object = new Object(id, interfaces, single, mandatory, maxInstances);

    Status retStatus = mObjectStorage.pushItem(object);

    if (status) {
        *status = retStatus;
    }

    return object;
}

Object* ObjectManager::getObjectById(Interface interface, uint16_t id)
{
    Object* object = mObjectStorage.getItemById(id);

    if (object && !(object->checkInterface(interface))) {
        return NULL;
    }

    return object;
}

Object* ObjectManager::getFirstObject(Interface interface)
{
    Object* object = mObjectStorage.getFirstItem();

    while (object) {
        if (!object->checkInterface(interface)) {
            object = mObjectStorage.getNextItem();
            continue;
        }

        return object;
    }

    return NULL;
}

Object* ObjectManager::getNextObject(Interface interface)
{
    Object* object = mObjectStorage.getNextItem();

    while (object) {
        if (!object->checkInterface(interface)) {
            object = mObjectStorage.getNextItem();
            continue;
        }

        return object;
    }

    return NULL;
}

Status ObjectManager::addConverter(DataConverter* converter)
{
    return mConverterStorage.pushItem(converter);
}

Status ObjectManager::bootstrapWrite(DataFormat dataFormat, void* data, size_t size, uint16_t objectId,
                                     uint16_t objectInstanceId, uint16_t resourceId)
{
    // 6.1.7.5. BOOTSTRAP WRITE

    Status status = STS_OK;
    DataConverter* converter = mConverterStorage.getItemById(dataFormat);

    if (converter == NULL) {
        return STS_ERR_FORMAT;
    }

    if ((converter->startDecoding(data, size)) != STS_OK) {
        return status;
    }

    Object* object = getObjectById(ITF_BOOTSTRAP, objectId);

    if (!object) {
        return STS_ERR_NOT_FOUND;
    }

    if (objectInstanceId == INVALID_ID) {
        if ((writeObject(object, converter)) != STS_OK) {
            return status;
        }

        return STS_OK;
    }

    bool instanceCreated = false;
    ObjectInstance* objectInstance = object->getInstanceById(objectInstanceId);

    if (!objectInstanceId) {
        objectInstance = object->createInstance(objectInstanceId, &status);
        if (!objectInstance) {
            return status;
        }

        instanceCreated = true;
    }

    if (resourceId == INVALID_ID) {
        if ((writeObjectInstance(objectInstance, converter)) != STS_OK) {
            if (instanceCreated) {
                object->deleteInstance(objectInstanceId);
            }

            return status;
        }

        return STS_OK;
    }

    Resource* resource = objectInstance->getResourceById(resourceId);
    if (!resource) {
        if (instanceCreated) {
            object->deleteInstance(objectInstanceId);
        }

        return STS_ERR_NOT_FOUND;
    }

    if ((writeResource(resource, converter)) != STS_OK) {
        if (instanceCreated) {
            object->deleteInstance(objectInstanceId);
        }

        return status;
    }

    return STS_OK;
}

Status ObjectManager::bootstrepRead(DataFormat* dataFormat, void* data, size_t* size, uint16_t objectId,
                                    uint16_t objectInstanceId)
{
    Status status = STS_OK;

    if (*dataFormat == DATA_FMT_ANY) {
        *dataFormat = CONFIG_DEFAULT_DATA_FORMAT;
    }

    DataConverter* outConverter = mConverterStorage.getItemById(*dataFormat);

    if (outConverter == NULL) {
        return STS_ERR_NOT_ALLOWED;
    }

    if ((status = outConverter->startEncoding(data, *size)) != STS_OK) {
        return status;
    }

    Object* object = getObjectById(ITF_BOOTSTRAP, objectId);

    if (!object) {
        return STS_ERR_NOT_FOUND;
    }

    if (objectInstanceId == INVALID_ID) {
        if ((status = object->read(outConverter)) != STS_OK) {
            return status;
        }
    }
    else {
        ObjectInstance* objectInstance = object->getInstanceById(objectInstanceId);

        if ((status = objectInstance->read(outConverter)) != STS_OK) {
            return status;
        }
    }

    return outConverter->finishEncoding(size);
}

#if 0
Status ObjectManager::readBootstrap(Interface interface, const char* path, DataFormat inFormat, void* inData,
                                    size_t inSize, DataFormat reqFormat, void* outData, size_t* outSize,
                                    DataFormat* outFormat)
{
    Status status = STS_OK;
    DataConverter::ResourceData resourceData;

    if (reqFormat != DATA_FMT_ANY) {
        outConverter = mConverterStorage.getItemById(reqFormat);
    }

    if (inConverter == NULL || outConverter == NULL) {
        return STS_ERR_FORMAT;
    }

    if ((status = inConverter->startDecoding(path, inData, inSize)) != STS_OK) {
        return status;
    }

    if ((status = outConverter->startEncoding(outData, *outSize)) != STS_OK) {
        return status;
    }

    while ((status = inConverter->nextDecoding(&resourceData)) == STS_OK) {
        Object* object = NULL;
        ObjectInstance* objectInstance = NULL;
        Resource* resource = NULL;
        ResourceInstance* resourceInstance = NULL;

        object = getObjectById(interface, resourceData.objectId);

        if (object) {
            objectInstance = object->getInstanceById(resourceData.objectInstanceId);
        }

        if (objectInstance) {
            resource = objectInstance->getResourceById(resourceData.resourceId);
        }

        if (resource) {
            if (resource->getInfo().isSingle()) {
                resourceInstance = resource->getFirstInstance();
            }
            else {
                resourceInstance = resource->getInstanceById(resourceData.resourceInstanceId);
            }
        }

        if (object && objectInstance && resource && resourceInstance) {
            if (!resource->getInfo().checkOperation(OP_READ)) {
                return STS_ERR_NOT_ALLOWED;
            }

            if (inFormat == DATA_FMT_TEXT && reqFormat == DATA_FMT_ANY && outConverter->getId() != DATA_FMT_TEXT) {
                outConverter = mConverterStorage.getItemById(DATA_FMT_TEXT);

                if (outConverter == NULL) {
                    return STS_ERR_FORMAT;
                }

                if ((status = outConverter->startEncoding(outData, *outSize)) != STS_OK) {
                    return status;
                }
            }

            if ((status = readResourceInstance(outConverter, resourceInstance)) != STS_OK) {
                return status;
            }
        }
        else if (object && objectInstance && resource) {
            if (!resource->getInfo().checkOperation(OP_READ)) {
                return STS_ERR_NOT_ALLOWED;
            }

            if ((status = readResource(outConverter, resource)) != STS_OK) {
                return status;
            }
        }
        else if (object && objectInstance) {
            if ((status = readObjectInstance(outConverter, objectInstance)) != STS_OK) {
                return status;
            }
        }
        else if (object) {
            if ((status = readObject(outConverter, object)) != STS_OK) {
                return status;
            }
        }
        else {
            return STS_ERR_NOT_FOUND;
        }
    }

    if (status != STS_ERR_NOT_FOUND) {
        return status;
    }

    *outFormat = static_cast<DataFormat>(outConverter->getId());

    return outConverter->finishEncoding(outSize);
}

#endif

/*******************************************************************************
 * Private
 ******************************************************************************/

/*******************************************************************************
 * E.1 LwM2M Object: LwM2M Security
 ******************************************************************************/

void ObjectManager::createSecurityObject()
{
    Status status = STS_OK;

    Object* object =
        createObject(OBJ_LWM2M_SECURITY, ITF_BOOTSTRAP, false, true, CONFIG_NUM_SERVERS + CONFIG_BOOTSTRAP_SERVER);
    ASSERT(object);
    // LWM2M Server URI
    status = object->createResourceString(RES_LWM2M_SERVER_URI, OP_NONE, true, true, 1, 255);
    ASSERT(status == STS_OK);
    // Bootstrap-Server
    status = object->createResourceBool(RES_BOOTSTRAP_SERVER, OP_NONE, true, true, 1,
                                        &ObjectManager::resBootstrapChanged, this);
    ASSERT(status == STS_OK);
    // Security Mode
    status = object->createResourceInt(RES_SECURITY_MODE, OP_NONE, true, true, 1, 0, 4);
    ASSERT(status == STS_OK);
    // Public Key or Identity
    status = object->createResourceOpaque(RES_PUBLIC_KEY_OR_IDENTITY, OP_NONE, true, true, 1, 0,
                                          CONFIG_CLIENT_PUBLIC_KEY_MAX_LEN);
    ASSERT(status == STS_OK);
    // Server Public Key
    status = object->createResourceOpaque(RES_SERVER_PUBLIC_KEY, OP_NONE, true, true, 1, 0,
                                          CONFIG_SERVER_PUBLIC_KEY_MAX_LEN);
    ASSERT(status == STS_OK);
    // Secret Key
    status = object->createResourceOpaque(RES_SECRET_KEY, OP_NONE, true, true, 1, 0, CONFIG_CLIENT_PRIVATE_KEY_MAX_LEN);
    ASSERT(status == STS_OK);
    // Short Server ID
    status = object->createResourceInt(RES_SECURITY_SHORT_SERVER_ID, OP_NONE, true, false, 1, 1, 65534);
    ASSERT(status == STS_OK);
}

/*******************************************************************************
 * E.2 LwM2M Object: LwM2M Server
 ******************************************************************************/

void ObjectManager::createServerObject()
{
    Status status = STS_OK;

    Object* object = createObject(OBJ_LWM2M_SERVER, ITF_ALL, false, true, CONFIG_NUM_SERVERS);
    ASSERT(object);
    // Short Server ID
    status = object->createResourceInt(RES_SHORT_SERVER_ID, OP_READ, true, true, 1, 1, 65535);
    ASSERT(status == STS_OK);
    // Lifetime
    status = object->createResourceInt(RES_LIFETIME, OP_READWRITE, true, true);
    ASSERT(status == STS_OK);
    // Notification Storing When Disabled or Offline
    status = object->createResourceBool(RES_NOTIFICATION_STORING, OP_READWRITE, true, true);
    ASSERT(status == STS_OK);
    // Binding
    status = object->createResourceString(RES_BINDING, OP_READWRITE, true, true, 1, CONFIG_BINDING_STR_MAX_LEN);
    ASSERT(status == STS_OK);
    // Registration Update Trigger
    status = object->createResourceNone(RES_REGISTRATION_UPDATE_TRIGGER, OP_EXECUTE, true, true);
    ASSERT(status == STS_OK);

#if CONFIG_MINIMAL_CLIENT == 0
    // Registration Priority Oreder
    status = object->createResourceUint(RES_REGISTRATION_PRIORITY_ORDER, OP_NONE, true, false);
    ASSERT(status == STS_OK);
    // Initial Registration Delay Timer
    status = object->createResourceUint(RES_INITIAL_REGISTRATION_DELAY, OP_NONE, true, false);
    ASSERT(status == STS_OK);

    // Registration Failure Block
    status = object->createResourceBool(RES_REG_FAILURE_BLOCK, OP_NONE, true, false);
    ASSERT(status == STS_OK);

    // Bootstrap on Registration Failure
    status = object->createResourceBool(RES_BOOTSTRAP_ON_REG_FAILURE, OP_NONE, true, false);
    ASSERT(status == STS_OK);

    // Communication Sequence Delay Timer
    status = object->createResourceUint(RES_SEQUENCE_DELAY_TIMER, OP_NONE, true, false);
    ASSERT(status == STS_OK);
    // Communication Sequence Retry Count
    status = object->createResourceUint(RES_SEQUENCE_RETRY_COUNT, OP_NONE, true, false);
    ASSERT(status == STS_OK);
#endif
}

/*******************************************************************************
 * E.4 LwM2M Object: Device
 ******************************************************************************/
void ObjectManager::createDeviceObject()
{
    Status status = STS_OK;

    Object* object = createObject(OBJ_DEVICE, ITF_ALL, true, true);
    ASSERT(object);
    // Reboot
    status = object->createResourceNone(RES_REBOOT, OP_EXECUTE, true, true);
    ASSERT(status == STS_OK);
    // Error Code
    status = object->createResourceInt(RES_ERR_CODE, OP_READ, false, true, CONFIG_ERR_CODE_MAX_SIZE, 0, 8);
    ASSERT(status == STS_OK);
    // Supported Binding and Modes
    status = object->createResourceString(RES_BINDING_AND_MODES, OP_READ, true, true, 1, 2);
    ASSERT(status == STS_OK);
}

void ObjectManager::createConverters()
{
    Status status = STS_OK;

#if CONFIG_DATA_FORMAT_SENML_JSON
    status = addConverter(new JsonConverter());
    ASSERT(status == STS_OK);
#endif

    status = addConverter(new TextConverter());
    ASSERT(status == STS_OK);
}

void ObjectManager::resBootstrapChanged(void* context, ResourceInstance* resInstance)
{
    // E.1 This Resource MUST be set when the Bootstrap-Server Resource has a value of 'false'.
    ObjectInstance* securityObjectInstance = resInstance->getResource()->getObjectInstance();
    ASSERT(securityObjectInstance);

    Resource* resShortServerId = securityObjectInstance->getResourceById(RES_SECURITY_SHORT_SERVER_ID);
    ASSERT(resShortServerId);

    if (!static_cast<ResourceBool*>(resInstance)->getValue()) {
        if (!resShortServerId->getFirstInstance()) {
            resShortServerId->createInstance();
        }
    }
    else {
        resShortServerId->deleteInstance(0);
    }
}

Status ObjectManager::storeObject(Object* object)
{
    return STS_OK;
}

Status ObjectManager::restoreObject(Object* object)
{
    return STS_OK;
}

Status ObjectManager::storeObjectInstance(ObjectInstance* objectInstance)
{
    return STS_OK;
}

Status ObjectManager::restoreObjectInstance(ObjectInstance* objectInstance)
{
    return STS_OK;
}

Status ObjectManager::storeResource(Resource* resoure)
{
    return STS_OK;
}

Status ObjectManager::restoreResource(Resource* resource)
{
    return STS_OK;
}

Status ObjectManager::writeObject(Object* object, DataConverter* converter, bool checkOperation, bool ignoreMissing,
                                  bool replace)
{
    Status status = STS_OK;

    if ((status = storeObject(object)) != STS_OK) {
        LOG_ERROR("Can't store object: /%d, status: %d", object->getId(), status);
        return status;
    }

    if ((status = object->write(converter, checkOperation, ignoreMissing, replace)) != STS_OK) {
        LOG_ERROR("Can't write object: /%d, status: %d", object->getId(), status);

        Status restoreStatus = restoreObject(object);

        if (restoreStatus != STS_OK) {
            LOG_ERROR("Can't restore object: /%d, status: %d", object->getId(), status);
            status = restoreStatus;
        }

        return status;
    }

    return STS_OK;
}

Status ObjectManager::writeObjectInstance(ObjectInstance* objectInstance, DataConverter* converter, bool checkOperation,
                                          bool ignoreMissing, bool replace)
{
    Status status = STS_OK;

    if ((status = storeObjectInstance(objectInstance)) != STS_OK) {
        LOG_ERROR("Can't store object instance: /%d/%d, status: %d", objectInstance->getParent()->getId(),
                  objectInstance->getId(), status);
        return status;
    }

    if ((status = objectInstance->write(converter, checkOperation, ignoreMissing, replace)) != STS_OK) {
        LOG_ERROR("Can't write object instance: /%d/%d, status: %d", objectInstance->getParent()->getId(),
                  objectInstance->getId(), status);

        Status restoreStatus = restoreObjectInstance(objectInstance);

        if (restoreStatus != STS_OK) {
            LOG_ERROR("Can't restore object instance: /%d/%d, status: %d", objectInstance->getParent()->getId(),
                      objectInstance->getId(), status);
            status = restoreStatus;
        }

        return status;
    }

    return STS_OK;
}

Status ObjectManager::writeResource(Resource* resource, DataConverter* converter, bool checkOperation, bool replace)
{
    Status status = STS_OK;

    if (resource)

        if (resource->getInfo().isSingle()) {
            ResourceInstance* resourceInstance = resource->getFirstInstance();

            if (!resourceInstance) {
                resourceInstance = resource->createInstance(0, &status);

                if (!resourceInstance) {
                    return status;
                }
            }

            if ((writeResourceInstance(resourceInstance, converter, checkOperation)) != STS_OK) {
                return status;
            }
        }

    if ((status = storeResource(resource)) != STS_OK) {
        LOG_ERROR("Can't store resource: /%d/%d/%d, status: %d", resource->getParent()->getParent()->getId(),
                  resource->getParent()->getId(), resource->getId(), status);
        return status;
    }

    if ((status = resource->write(converter, checkOperation, replace)) != STS_OK) {
        LOG_ERROR("Can't write resource: /%d/%d/%d, status: %d", resource->getParent()->getParent()->getId(),
                  resource->getParent()->getId(), resource->getId(), status);

        Status restoreStatus = restoreResource(resource);

        if (restoreStatus != STS_OK) {
            LOG_ERROR("Can't restore resource: /%d/%d/%d, status: %d", resource->getParent()->getParent()->getId(),
                      resource->getParent()->getId(), resource->getId(), status);
            status = restoreStatus;
        }

        return status;
    }

    return STS_OK;
}

Status ObjectManager::writeResourceInstance(ResourceInstance* resourceInstance, DataConverter* converter,
                                            bool checkOperation)
{
    if (checkOperation && !resourceInstance->getResource()->getInfo().checkOperation(OP_WRITE)) {
        return STS_ERR_NOT_ALLOWED;
    }

    DataConverter::ResourceData data;

    Status status = converter->nextDecoding(&data);

    if (status != STS_OK) {
        if (status == STS_ERR_NOT_FOUND) {
            return STS_ERR_FORMAT;
        }

        return status;
    }

    return resourceInstance->write(&data);
}

ObjectInstance* ObjectManager::getServerInstance(uint16_t shortServerId)
{
    Object* object = mObjectStorage.getItemById(OBJ_LWM2M_SERVER);

    if (!object) {
        return NULL;
    }

    ObjectInstance* objectInstance = object->getFirstInstance();

    while (objectInstance) {
        if (static_cast<ResourceInt*>(objectInstance->getResourceInstance(RES_SHORT_SERVER_ID))->getValue() ==
            shortServerId) {
            return objectInstance;
        }

        objectInstance = object->getNextInstance();
    }

    return NULL;
}

bool ObjectManager::isFormatSupported(DataFormat format)
{
    return mConverterStorage.getItemById(format);
}

Status ObjectManager::readResourceInstance(DataConverter* converter, ResourceInstance* resourceInstance)
{
    DataConverter::ResourceData resourceData = {resourceInstance->getParent()->getParent()->getParent()->getId(),
                                                resourceInstance->getParent()->getParent()->getId(),
                                                resourceInstance->getParent()->getId(), resourceInstance->getId()};
    /*
        if (resourceInstance->getDesc().isSingle()) {
            resourceData.resourceInstanceId = INVALID_ID;
        }

        resourceData.dataType = resourceInstance->getDesc().getDataType();

        switch (resourceData.dataType) {
            case DATA_TYPE_STRING:
                resourceData.strValue = const_cast<char*>(resourceInstance->getString());
                break;

            case DATA_TYPE_INT:
                resourceData.intValue = resourceInstance->getInt();
                break;

            case DATA_TYPE_UINT:
                resourceData.uintValue = resourceInstance->getUint();
                break;

            case DATA_TYPE_FLOAT:
                resourceData.floatValue = resourceInstance->getFloat();
                break;

            case DATA_TYPE_BOOL:
                resourceData.boolValue = resourceInstance->getBool();
                break;
            default:
                return STS_ERR;
        }
    */
    return converter->nextEncoding(&resourceData);
}

Status ObjectManager::readResource(DataConverter* converter, Resource* resource)
{
    /*
    ResourceInstance* resourceInstance = resource->getFirstInstance();

    while (resourceInstance) {
        Status status = readResourceInstance(converter, resourceInstance);

        if (status != STS_OK) {
            return status;
        }

        resourceInstance = resource->getNextInstance();
    }
*/
    return STS_OK;
}

Status ObjectManager::readObjectInstance(DataConverter* converter, ObjectInstance* objectInstance)
{
    /*
    Resource* resource = objectInstance->getFirstResource();

    while (resource) {
        if (resource->getDesc().checkOperation(ResourceDesc::OP_READ)) {
            Status status = readResource(converter, resource);

            if (status != STS_OK) {
                return status;
            }
        }

        resource = objectInstance->getNextResource();
    }
*/
    return STS_OK;
}

Status ObjectManager::readObject(DataConverter* converter, Object* object)
{
    /*
    ObjectInstance* objectInstance = object->getFirstInstance();

    while (objectInstance) {
        Status status = readObjectInstance(converter, objectInstance);

        if (status != STS_OK) {
            return status;
        }

        objectInstance = object->getNextInstance();
    }
*/
    return STS_OK;
}

}  // namespace openlwm2m
