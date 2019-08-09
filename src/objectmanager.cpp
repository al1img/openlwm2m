#include "objectmanager.hpp"

#if CONFIG_DATA_FORMAT_SENML_JSON
#include "jsonconverter.hpp"
#endif

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

Object* ObjectManager::createObject(uint16_t id, Object::Instance instance, size_t maxInstances,
                                    Object::Mandatory mandatory, uint16_t interfaces, Status* status)
{
    if (instance == Object::SINGLE) {
        maxInstances = 1;
    }

    LOG_DEBUG("Create object /%d", id);

    Object::Params params = {instance, mandatory, interfaces, maxInstances};

    return mObjectStorage.newItem(NULL, id, params, status);
}

Object* ObjectManager::getObject(Interface interface, uint16_t id)
{
    Object* object = mObjectStorage.getItemById(id);

    if (interface && object && !(object->mParams.interfaces & interface)) {
        return NULL;
    }

    return object;
}

Object* ObjectManager::getFirstObject(Interface interface)
{
    Object* object = mObjectStorage.getFirstItem();

    while (object) {
        if (interface && !(object->mParams.interfaces & interface)) {
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
        if (interface && !(object->mParams.interfaces & interface)) {
            object = mObjectStorage.getNextItem();

            continue;
        }

        return object;
    }

    return NULL;
}

Status ObjectManager::addConverter(DataConverter* converter)
{
    return mConverterStorage.addItem(converter);
}

Status ObjectManager::write(Interface interface, DataFormat format, const char* path, void* data, size_t size)
{
    DataConverter* converter = mConverterStorage.getItemById(format);

    if (converter == NULL) {
        return STS_ERR_NOT_EXIST;
    }

    switch (interface) {
        case ITF_BOOTSTRAP:
            return bootstrapWrite(converter, path, data, size);

        default:
            return STS_ERR_NOT_EXIST;
    }
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

    Object* object = createObject(OBJ_LWM2M_SECURITY, Object::MULTIPLE,
                                  CONFIG_NUM_SERVERS == 0 ? 0 : CONFIG_NUM_SERVERS + CONFIG_BOOTSTRAP_SERVER,
                                  Object::MANDATORY, ITF_BOOTSTRAP);
    ASSERT(object);
    // LWM2M Server URI
    status = object->createResourceString(RES_LWM2M_SERVER_URI, ResourceDesc::OP_NONE, ResourceDesc::SINGLE, 0,
                                          ResourceDesc::MANDATORY, 255);
    ASSERT(status == STS_OK);
    // Bootstrap-Server
    status = object->createResourceBool(RES_BOOTSTRAP_SERVER, ResourceDesc::OP_NONE, ResourceDesc::SINGLE, 0,
                                        ResourceDesc::MANDATORY, &ObjectManager::resBootstrapChanged, this);
    ASSERT(status == STS_OK);
    // Security Mode
    status = object->createResourceInt(RES_SECURITY_MODE, ResourceDesc::OP_NONE, ResourceDesc::SINGLE, 0,
                                       ResourceDesc::MANDATORY, 0, 4);
    ASSERT(status == STS_OK);
    // Public Key or Identity
    status = object->createResourceOpaque(RES_PUBLIC_KEY_OR_IDENTITY, ResourceDesc::OP_NONE, ResourceDesc::SINGLE, 0,
                                          ResourceDesc::MANDATORY, 0, CONFIG_CLIENT_PUBLIC_KEY_MAX_LEN);
    ASSERT(status == STS_OK);
    // Server Public Key
    status = object->createResourceOpaque(RES_SERVER_PUBLIC_KEY, ResourceDesc::OP_NONE, ResourceDesc::SINGLE, 0,
                                          ResourceDesc::MANDATORY, 0, CONFIG_SERVER_PUBLIC_KEY_MAX_LEN);
    ASSERT(status == STS_OK);
    // Secret Key
    status = object->createResourceOpaque(RES_SECRET_KEY, ResourceDesc::OP_NONE, ResourceDesc::SINGLE, 0,
                                          ResourceDesc::MANDATORY, 0, CONFIG_CLIENT_PRIVATE_KEY_MAX_LEN);
    ASSERT(status == STS_OK);
    // Short Server ID
    status = object->createResourceInt(RES_SECURITY_SHORT_SERVER_ID, ResourceDesc::OP_NONE, ResourceDesc::SINGLE, 0,
                                       ResourceDesc::OPTIONAL, 1, 65534);
    ASSERT(status == STS_OK);
}

/*******************************************************************************
 * E.2 LwM2M Object: LwM2M Server
 ******************************************************************************/

void ObjectManager::createServerObject()
{
    Status status = STS_OK;

    Object* object = createObject(OBJ_LWM2M_SERVER, Object::MULTIPLE, CONFIG_NUM_SERVERS, Object::MANDATORY, ITF_ALL);
    ASSERT(object);
    // Short Server ID
    status = object->createResourceInt(RES_SHORT_SERVER_ID, ResourceDesc::OP_READ, ResourceDesc::SINGLE, 0,
                                       ResourceDesc::MANDATORY, 1, 65535);
    ASSERT(status == STS_OK);
    // Lifetime
    status = object->createResourceInt(RES_LIFETIME, ResourceDesc::OP_READWRITE, ResourceDesc::SINGLE, 0,
                                       ResourceDesc::MANDATORY);
    ASSERT(status == STS_OK);
    // Notification Storing When Disabled or Offline
    status = object->createResourceBool(RES_NOTIFICATION_STORING, ResourceDesc::OP_READWRITE, ResourceDesc::SINGLE, 0,
                                        ResourceDesc::MANDATORY);
    ASSERT(status == STS_OK);
    // Binding
    status = object->createResourceString(RES_BINDING, ResourceDesc::OP_READWRITE, ResourceDesc::SINGLE, 0,
                                          ResourceDesc::MANDATORY, CONFIG_BINDING_STR_MAX_LEN);
    ASSERT(status == STS_OK);
    // Registration Update Trigger
    status = object->createResourceNone(8, ResourceDesc::OP_EXECUTE, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY);
    ASSERT(status == STS_OK);

#if CONFIG_MINIMAL_CLIENT == 0
    // Registration Priority Oreder
    status = object->createResourceUint(RES_REGISTRATION_PRIORITY_ORDER, ResourceDesc::OP_NONE, ResourceDesc::SINGLE, 0,
                                        ResourceDesc::OPTIONAL);
    ASSERT(status == STS_OK);
    // Initial Registration Delay Timer
    status = object->createResourceUint(RES_INITIAL_REGISTRATION_DELAY, ResourceDesc::OP_NONE, ResourceDesc::SINGLE, 0,
                                        ResourceDesc::OPTIONAL);
    ASSERT(status == STS_OK);
    // Communication Sequence Delay Timer
    status = object->createResourceUint(RES_SEQUENCE_DELAY_TIMER, ResourceDesc::OP_NONE, ResourceDesc::SINGLE, 0,
                                        ResourceDesc::OPTIONAL);
    ASSERT(status == STS_OK);
    // Communication Sequence Retry Count
    status = object->createResourceUint(RES_SEQUENCE_RETRY_COUNT, ResourceDesc::OP_NONE, ResourceDesc::SINGLE, 0,
                                        ResourceDesc::OPTIONAL);
    ASSERT(status == STS_OK);
#endif
}

/*******************************************************************************
 * E.4 LwM2M Object: Device
 ******************************************************************************/
void ObjectManager::createDeviceObject()
{
    Status status = STS_OK;

    Object* object = createObject(3, Object::SINGLE, 0, Object::MANDATORY, ITF_ALL);
    ASSERT(object);
    // Reboot
    status = object->createResourceNone(4, ResourceDesc::OP_EXECUTE, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY);
    ASSERT(status == STS_OK);
    // Error Code
    status = object->createResourceInt(11, ResourceDesc::OP_READ, ResourceDesc::MULTIPLE, CONFIG_ERR_CODE_MAX_SIZE,
                                       ResourceDesc::MANDATORY, 0, 8);
    ASSERT(status == STS_OK);
    // Supported Binding and Modes
    status =
        object->createResourceString(16, ResourceDesc::OP_READ, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY, 2);
    ASSERT(status == STS_OK);
}

void ObjectManager::createConverters()
{
    Status status = STS_OK;

#if CONFIG_DATA_FORMAT_SENML_JSON
    status = addConverter(new JsonConverter());
    ASSERT(status == STS_OK);
#endif
}

void ObjectManager::resBootstrapChanged(void* context, ResourceInstance* resInstance)
{
    // E.1 This Resource MUST be set when the Bootstrap-Server Resource has a value of 'false'.
    ObjectInstance* securityObjectInstance = static_cast<ObjectInstance*>(resInstance->getParent()->getParent());
    ASSERT(securityObjectInstance);

    Resource* resShortServerId = securityObjectInstance->getResourceById(RES_SECURITY_SHORT_SERVER_ID);
    ASSERT(resShortServerId);

    if (!resInstance->getBool()) {
        if (!resShortServerId->getFirstInstance()) {
            resShortServerId->createInstance();
        }
    }
    else {
        resShortServerId->deleteInstance(resShortServerId->getFirstInstance());
    }
}

Status ObjectManager::bootstrapWrite(DataConverter* converter, const char* path, void* data, size_t size)
{
    // 6.1.7.5. BOOTSTRAP WRITE

    Status status = STS_OK;
    DataConverter::ResourceData resourceData;

    LOG_DEBUG("Bootstrap write");

    // Check format: just to empty path
    if ((status = converter->startDecoding(path, data, size)) != STS_OK) {
        return status;
    }

    while ((status = converter->nextDecoding(&resourceData)) == STS_OK) {
    }

    if (status != STS_ERR_NOT_EXIST) {
        return status;
    }

    if ((status = converter->startDecoding(path, data, size)) != STS_OK) {
        return status;
    }

    while ((status = converter->nextDecoding(&resourceData)) == STS_OK) {
        Object* object = getObject(ITF_BOOTSTRAP, resourceData.objectId);

        if (!object) {
            LOG_WARNING("Skip not existed object: /%d", resourceData.objectId);
            continue;
        }

        ObjectInstance* objectInstance = object->getInstanceById(resourceData.objectInstanceId);

        if (!objectInstance) {
            objectInstance = object->createInstance(resourceData.objectInstanceId, &status);

            if (!objectInstance) {
                LOG_ERROR("Can't create object instance: /%d/%d, status: %d", resourceData.objectId,
                          resourceData.objectInstanceId, status);
                continue;
            }
        }

        Resource* resource = objectInstance->getResourceById(resourceData.resourceId);

        if (!resource) {
            LOG_WARNING("Skip not existed resource: /%d/%d/%d", resourceData.objectId, resourceData.objectInstanceId,
                        resourceData.resourceId);
            continue;
        }

        if (resourceData.resourceInstanceId == INVALID_ID) {
            resourceData.resourceInstanceId = 0;
        }

        ResourceInstance* resourceInstance = resource->getInstanceById(resourceData.resourceInstanceId);

        if (!resourceInstance) {
            resourceInstance = resource->createInstance(resourceData.resourceInstanceId, &status);

            if (!resourceInstance) {
                LOG_ERROR("Can't create resource instance: /%d/%d/%d/%d, status: %d", resourceData.objectId,
                          resourceData.objectInstanceId, resourceData.resourceId, resourceData.resourceInstanceId,
                          status);
                continue;
            }
        }

        if ((status = writeResource(resourceInstance, &resourceData)) != STS_OK) {
            LOG_ERROR("Can't write resource instance: /%d/%d/%d/%d, status: %d", resourceData.objectId,
                      resourceData.objectInstanceId, resourceData.resourceId, resourceData.resourceInstanceId, status);
        }
    }

    return STS_OK;
}

Status ObjectManager::writeResource(ResourceInstance* instance, DataConverter::ResourceData* resourceData)
{
    switch (resourceData->dataType) {
        case DATA_TYPE_STRING:
            return instance->setString(resourceData->strValue);

        case DATA_TYPE_INT:
            return instance->setInt(resourceData->intValue);

        case DATA_TYPE_UINT:
            return instance->setUint(resourceData->uintValue);

        case DATA_TYPE_FLOAT:
            return instance->setFloat(resourceData->floatValue);

        case DATA_TYPE_BOOL:
            return instance->setBool(resourceData->boolValue);

        case DATA_TYPE_OPAQUE:
        case DATA_TYPE_TIME:
        case DATA_TYPE_OBJLINK:
        case DATA_TYPE_CORELINK:
        default:
            return STS_ERR_INVALID_VALUE;
    }
}

ObjectInstance* ObjectManager::getServerInstance(uint16_t shortServerId)
{
    Object* object = mObjectStorage.getItemById(OBJ_LWM2M_SERVER);

    if (!object) {
        return NULL;
    }

    ObjectInstance* objectInstance = object->getFirstInstance();

    while (objectInstance) {
        if (objectInstance->getResourceInstance(RES_SHORT_SERVER_ID)->getInt() == shortServerId) {
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
