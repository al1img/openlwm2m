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

Object* ObjectManager::createObject(uint16_t id, bool single, bool mandatory, size_t maxInstances, Status* status)
{
    if (single) {
        maxInstances = 1;
    }

    LOG_DEBUG("Create object /%d", id);

    Object* object = new Object(id, single, mandatory, maxInstances);

    Status retStatus = mObjectStorage.pushItem(object);

    if (status) {
        *status = retStatus;
    }

    return object;
}

Object* ObjectManager::getObjectById(uint16_t id)
{
    return mObjectStorage.getItemById(id);
}

Object* ObjectManager::getFirstObject()
{
    return mObjectStorage.getFirstItem();
}

Object* ObjectManager::getNextObject()
{
    return mObjectStorage.getNextItem();
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

    if ((status = converter->startDecoding(data, size)) != STS_OK) {
        return status;
    }

    Object* object = getObjectById(objectId);

    if (!object) {
        return STS_ERR_NOT_FOUND;
    }

    if (objectInstanceId == INVALID_ID) {
        if ((status = writeObject(object, converter)) != STS_OK) {
            return status;
        }

        return STS_OK;
    }

    bool instanceCreated = false;
    ObjectInstance* objectInstance = object->getInstanceById(objectInstanceId);

    if (!objectInstance) {
        objectInstance = object->createInstance(objectInstanceId, &status);
        if (!objectInstance) {
            return status;
        }

        instanceCreated = true;
    }

    if (resourceId == INVALID_ID) {
        if ((status = writeObjectInstance(objectInstance, converter)) != STS_OK) {
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

    if ((status = writeResource(resource, converter)) != STS_OK) {
        if (instanceCreated) {
            object->deleteInstance(objectInstanceId);
        }

        return status;
    }

    return STS_OK;
}

DataConverter* ObjectManager::getConverterById(uint16_t id)
{
    return mConverterStorage.getItemById(id);
}

/*******************************************************************************
 * Private
 ******************************************************************************/

/*******************************************************************************
 * E.1 LwM2M Object: LwM2M Security
 ******************************************************************************/

void ObjectManager::createSecurityObject()
{
    Status status = STS_OK;

    Object* object = createObject(OBJ_LWM2M_SECURITY, false, true, CONFIG_NUM_SERVERS + CONFIG_BOOTSTRAP_SERVER);
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

    Object* object = createObject(OBJ_LWM2M_SERVER, false, true, CONFIG_NUM_SERVERS);
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
    status = object->createExecutableResource(RES_REGISTRATION_UPDATE_TRIGGER, true);
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

    Object* object = createObject(OBJ_DEVICE, true, true);
    ASSERT(object);
    // Reboot
    status = object->createExecutableResource(RES_REBOOT, true);
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

Status ObjectManager::writeObject(Object* object, DataConverter* converter, bool store, bool checkOperation,
                                  bool ignoreMissing, bool replace)
{
    Status status = STS_OK;

    if (store && (status = storeObject(object)) != STS_OK) {
        LOG_ERROR("Can't store object: /%d, status: %d", object->getId(), status);
        return status;
    }

    if ((status = object->write(converter, checkOperation, ignoreMissing, replace)) != STS_OK) {
        LOG_ERROR("Can't write object: /%d, status: %d", object->getId(), status);

        if (store) {
            Status restoreStatus = restoreObject(object);

            if (restoreStatus != STS_OK) {
                LOG_ERROR("Can't restore object: /%d, status: %d", object->getId(), status);
                status = restoreStatus;
            }
        }

        return status;
    }

    return STS_OK;
}

Status ObjectManager::writeObjectInstance(ObjectInstance* objectInstance, DataConverter* converter, bool store,
                                          bool checkOperation, bool ignoreMissing, bool replace)
{
    Status status = STS_OK;

    if (store && (status = storeObjectInstance(objectInstance)) != STS_OK) {
        LOG_ERROR("Can't store object instance: /%d/%d, status: %d", objectInstance->getParent()->getId(),
                  objectInstance->getId(), status);
        return status;
    }

    if ((status = objectInstance->write(converter, checkOperation, ignoreMissing, replace)) != STS_OK) {
        LOG_ERROR("Can't write object instance: /%d/%d, status: %d", objectInstance->getParent()->getId(),
                  objectInstance->getId(), status);

        if (store) {
            Status restoreStatus = restoreObjectInstance(objectInstance);

            if (restoreStatus != STS_OK) {
                LOG_ERROR("Can't restore object instance: /%d/%d, status: %d", objectInstance->getParent()->getId(),
                          objectInstance->getId(), status);
                status = restoreStatus;
            }
        }

        return status;
    }

    return STS_OK;
}

Status ObjectManager::writeResource(Resource* resource, DataConverter* converter, bool store, bool checkOperation,
                                    bool replace)
{
    Status status = STS_OK;

    if (store && (status = storeResource(resource)) != STS_OK) {
        LOG_ERROR("Can't store resource: /%d/%d/%d, status: %d", resource->getParent()->getParent()->getId(),
                  resource->getParent()->getId(), resource->getId(), status);
        return status;
    }

    if ((status = resource->write(converter, checkOperation, replace)) != STS_OK) {
        LOG_ERROR("Can't write resource: /%d/%d/%d, status: %d", resource->getParent()->getParent()->getId(),
                  resource->getParent()->getId(), resource->getId(), status);

        if (store) {
            Status restoreStatus = restoreResource(resource);

            if (restoreStatus != STS_OK) {
                LOG_ERROR("Can't restore resource: /%d/%d/%d, status: %d", resource->getParent()->getParent()->getId(),
                          resource->getParent()->getId(), resource->getId(), status);
                status = restoreStatus;
            }
        }

        return status;
    }

    return STS_OK;
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

}  // namespace openlwm2m
