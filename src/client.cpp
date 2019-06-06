#include "client.hpp"
#include "interface.hpp"
#include "log.hpp"

#define LOG_MODULE "Client"

namespace openlwm2m {

/*******************************************************************************
 * Client
 ******************************************************************************/

Client::Client(TransportItf& transport)
    : mTransport(transport),
      mObjectStorage(NULL),
      mRegHandlerStorage(NULL, *this, CONFIG_NUM_SERVERS),
      mState(STATE_INIT)
{
    LOG_DEBUG("Create client");

    Status status = STS_OK;

    /***************************************************************************
     * E.1 LwM2M Object: LwM2M Security
     **************************************************************************/
    Object* object = createObject(OBJ_LWM2M_SECURITY, Object::MULTIPLE,
                                  CONFIG_NUM_SERVERS == 0 ? 0 : CONFIG_NUM_SERVERS + CONFIG_BOOTSTRAP_SERVER,
                                  Object::MANDATORY, ITF_BOOTSTRAP | ITF_REGISTER);
    ASSERT(object);
    // LWM2M Server URI
    status = object->createResourceString(RES_LWM2M_SERVER_URI, ResourceDesc::OP_NONE, ResourceDesc::SINGLE, 0,
                                          ResourceDesc::MANDATORY, 255);
    ASSERT(status == STS_OK);
    // Bootstrap-Server
    status = object->createResourceBool(1, ResourceDesc::OP_NONE, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY);
    ASSERT(status == STS_OK);
    // Security Mode
    status =
        object->createResourceInt(2, ResourceDesc::OP_NONE, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY, 0, 4);
    ASSERT(status == STS_OK);
    // Public Key or Identity
    status = object->createResourceOpaque(3, ResourceDesc::OP_NONE, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY, 0,
                                          CONFIG_CLIENT_PUBLIC_KEY_MAX_LEN);
    ASSERT(status == STS_OK);
    // Server Public Key
    status = object->createResourceOpaque(4, ResourceDesc::OP_NONE, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY, 0,
                                          CONFIG_SERVER_PUBLIC_KEY_MAX_LEN);
    ASSERT(status == STS_OK);
    // Secret Key
    status = object->createResourceOpaque(5, ResourceDesc::OP_NONE, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY, 0,
                                          CONFIG_CLIENT_PRIVATE_KEY_MAX_LEN);
    ASSERT(status == STS_OK);

    /***************************************************************************
     * E.2 LwM2M Object: LwM2M Server
     **************************************************************************/
    object = createObject(OBJ_LWM2M_SERVER, Object::MULTIPLE, CONFIG_NUM_SERVERS, Object::MANDATORY, ITF_ALL);
    ASSERT(object);
    // Short Server ID
    status = object->createResourceUint(RES_SHORT_SERVER_ID, ResourceDesc::OP_READ, ResourceDesc::SINGLE, 0,
                                        ResourceDesc::MANDATORY, 1, 65535);
    ASSERT(status == STS_OK);
    // Lifetime
    status = object->createResourceInt(1, ResourceDesc::OP_READWRITE, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY);
    ASSERT(status == STS_OK);
    // Notification Storing When Disabled or Offline
    status =
        object->createResourceBool(6, ResourceDesc::OP_READWRITE, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY);
    ASSERT(status == STS_OK);
    // Binding
    status = object->createResourceString(7, ResourceDesc::OP_READWRITE, ResourceDesc::SINGLE, 0,
                                          ResourceDesc::MANDATORY, CONFIG_BINDING_STR_MAX_LEN);
    ASSERT(status == STS_OK);
    // Registration Update Trigger
    status = object->createResourceNone(8, ResourceDesc::OP_EXECUTE, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY);
    ASSERT(status == STS_OK);

    /***************************************************************************
     * E.4 LwM2M Object: Device
     **************************************************************************/
    object = createObject(3, Object::SINGLE, 0, Object::MANDATORY, ITF_ALL);
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

Client::~Client()
{
    LOG_DEBUG("Delete client");

    mRegHandlerStorage.clear();
    mObjectStorage.release();
}

/*******************************************************************************
 * Public
 ******************************************************************************/

Object* Client::createObject(uint16_t id, Object::Instance instance, size_t maxInstances, Object::Mandatory mandatory,
                             uint16_t interfaces, Status* status)
{
    if (mState != STATE_INIT) {
        if (status) *status = STS_ERR_INVALID_STATE;
        return NULL;
    }

    if (instance == Object::SINGLE) {
        maxInstances = 1;
    }

    Object::Params params = {instance, mandatory, interfaces, maxInstances};

    return mObjectStorage.createItem(id, params, status);
}

Object* Client::getObject(Interface interface, uint16_t id)
{
    Object* object = mObjectStorage.getItemById(id);

    if (object && !(object->mParams.interfaces & interface)) {
        LOG_WARNING("Object /%d not accesible by interface %d", id, interface);
        return NULL;
    }

    return object;
}

ResourceInstance* Client::getResourceInstance(Interface interface, uint16_t objId, uint16_t objInstanceId,
                                              uint16_t resId, uint16_t resInstanceId)
{
    Object* object = getObject(interface, objId);

    if (!object) {
        return NULL;
    }

    return object->getResourceInstance(objInstanceId, resId, resInstanceId);
}

Status Client::init()
{
    LOG_DEBUG("Init client");

    if (mState != STATE_INIT) {
        return STS_ERR_INVALID_STATE;
    }

    mObjectStorage.init();

    mState = STATE_INITIALIZED;

    return STS_OK;
}

Status Client::registration()
{
    LOG_DEBUG("Register client");

    if (mState != STATE_INITIALIZED && mState != STATE_BOOTSTRAP) {
        return STS_ERR_INVALID_STATE;
    }

    Object* object = getObject(ITF_REGISTER, OBJ_LWM2M_SERVER);

    if (!object) {
        return STS_ERR_NOT_EXIST;
    }

    ObjectInstance* serverInstance = object->getFirstInstance();

    while (serverInstance) {
        ResourceInstance* shortServerIdInstance = serverInstance->getResourceInstance(RES_SHORT_SERVER_ID);
        ASSERT(shortServerIdInstance);

        RegHandler* handler = mRegHandlerStorage.newItem(serverInstance->getId(), *this);

        if (!handler) {
            LOG_ERROR("Can't create reg handler: %d", STS_ERR_NO_MEM);
            continue;
        }

        Status status = STS_OK;

        if ((status = handler->connect()) != STS_OK) {
            LOG_ERROR("Can't connect to lwm2m server: %d", status);

            mRegHandlerStorage.deleteInstance(handler);
        }

        serverInstance = object->getNextInstance();
    }

    if (mRegHandlerStorage.size() == 0) {
        LOG_ERROR("No valid lwm2m servers found");

        return STS_ERR_NOT_EXIST;
    }

    return STS_OK;
}

void Client::bootstrapDiscover()
{
}
void Client::bootstrapRead()
{
}
void Client::bootstrapWrite()
{
}
void Client::bootstrapDelete()
{
}

void Client::deviceRead()
{
}
void Client::deviceDiscover()
{
}
void Client::deviceWrite()
{
}
void Client::deviceWriteAttributes()
{
}
void Client::deviceExecute()
{
}
void Client::deviceCreate()
{
}
void Client::deviceDelete()
{
}
void Client::readComposite()
{
}
void Client::writeComposite()
{
}

void Client::reportingObserve()
{
}
void Client::reportingCancelObservation()
{
}
void Client::reportingObserveComposite()
{
}
void Client::reportingCancelObserveComposite()
{
}

}  // namespace openlwm2m
