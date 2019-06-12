#include "client.hpp"
#include "interface.hpp"
#include "log.hpp"
#include "timer.hpp"

#define LOG_MODULE "Client"

namespace openlwm2m {

/*******************************************************************************
 * Client
 ******************************************************************************/

Client::Client(const char* name, bool queueMode, TransportItf& transport)
    : mName(name),
      mQueueMode(queueMode),
      mTransport(transport),
      mObjectStorage(),
      mObjectNode(NULL),
      mRegHandlerStorage(*this, CONFIG_NUM_SERVERS),
      mState(STATE_INIT)
{
    LOG_DEBUG("Create client");

    Status status = STS_OK;

    /***************************************************************************
     * E.1 LwM2M Object: LwM2M Security
     **************************************************************************/
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
                                        ResourceDesc::MANDATORY, &Client::resBootstrapChanged, this);
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
    // Short Server ID
    status = object->createResourceInt(RES_SECURITY_SHORT_SERVER_ID, ResourceDesc::OP_NONE, ResourceDesc::SINGLE, 0,
                                       ResourceDesc::OPTIONAL, 1, 65534);
    ASSERT(status == STS_OK);

    /***************************************************************************
     * E.2 LwM2M Object: LwM2M Server
     **************************************************************************/
    object = createObject(OBJ_LWM2M_SERVER, Object::MULTIPLE, CONFIG_NUM_SERVERS, Object::MANDATORY, ITF_ALL);
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
    status =
        object->createResourceBool(6, ResourceDesc::OP_READWRITE, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY);
    ASSERT(status == STS_OK);
    // Binding
    status = object->createResourceString(RES_BINDING, ResourceDesc::OP_READWRITE, ResourceDesc::SINGLE, 0,
                                          ResourceDesc::MANDATORY, CONFIG_BINDING_STR_MAX_LEN);
    ASSERT(status == STS_OK);
    // Registration Update Trigger
    status = object->createResourceNone(8, ResourceDesc::OP_EXECUTE, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY);
    ASSERT(status == STS_OK);
    // Registration Priority Oreder
    status = object->createResourceUint(RES_REGISTRATION_PRIORITY_ORDER, ResourceDesc::OP_NONE, ResourceDesc::SINGLE, 0,
                                        ResourceDesc::OPTIONAL);
    ASSERT(status == STS_OK);
    // Initial Registration Delay Timer
    status = object->createResourceUint(RES_INITIAL_REGISTRATION_DELAY, ResourceDesc::OP_NONE, ResourceDesc::SINGLE, 0,
                                        ResourceDesc::OPTIONAL);
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

Status Client::poll(uint64_t currentTimeMs, uint64_t* pollInMs)
{
    if (pollInMs) {
        *pollInMs = ULONG_MAX;
    }

    Status status = STS_OK;

    if ((status = Timer::poll(currentTimeMs, pollInMs)) != STS_OK) {
        return status;
    }

    return STS_OK;
}

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

    return mObjectStorage.newItem(NULL, id, params, status);
}

Object* Client::getObject(Interface interface, uint16_t id)
{
    Object* object = mObjectStorage.getItemById(id);

    if (interface && object && !(object->mParams.interfaces & interface)) {
        LOG_DEBUG("Object /%d not accesible by interface %d", id, interface);
        return NULL;
    }

    return object;
}

Object* Client::getFirstObject(Interface interface)
{
    mObjectNode = mObjectStorage.begin();

    while (mObjectNode) {
        if (mObjectNode) {
            if (interface && mObjectNode->get() && !(mObjectNode->get()->mParams.interfaces & interface)) {
                LOG_DEBUG("Object /%d not accesible by interface %d", mObjectNode->get()->getId(), interface);
            }
            else {
                return mObjectNode->get();
            }
        }

        mObjectNode = mObjectNode->next();
    }

    return NULL;
}

Object* Client::getNextObject(Interface interface)
{
    if (mObjectNode) {
        mObjectNode = mObjectNode->next();

        if (mObjectNode) {
            if (interface && mObjectNode->get() && !(mObjectNode->get()->mParams.interfaces & interface)) {
                LOG_DEBUG("Object /%d not accesible by interface %d", mObjectNode->get()->getId(), interface);
            }
            else {
                return mObjectNode->get();
            }

            mObjectNode = mObjectNode->next();
        }
    }

    return NULL;
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

    Status status = STS_OK;

    if ((status = createRegHandlers()) != STS_OK) {
        mRegHandlerStorage.clear();
        return status;
    }

    if ((status = startNextPriorityReg()) != STS_OK) {
        mRegHandlerStorage.clear();
        return status;
    }

    // Starts all non priority handlers

    for (RegHandler::StorageNode* node = mRegHandlerStorage.begin(); node; node = node->next()) {
        // Skip already started handlers
        if (node->get()->mState != RegHandler::STATE_INIT) {
            continue;
        }

        // Skip handlers with priority order
        if (node->get()->mServerInstance->getResourceInstance(RES_REGISTRATION_PRIORITY_ORDER)) {
            continue;
        }

        if ((status = node->get()->startRegistration()) != STS_OK) {
            mRegHandlerStorage.clear();
            return status;
        }
    }

    return status;
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

/*******************************************************************************
 * Private
 ******************************************************************************/

void Client::resBootstrapChanged(void* context, ResourceInstance* resInstance)
{
    // E.1 ThisResource MUST be set when the Bootstrap-ServerResource has a value of 'false'.
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

Status Client::createRegHandlers()
{
    Object* object = getObject(ITF_REGISTER, OBJ_LWM2M_SERVER);
    ASSERT(object);

    ObjectInstance* serverInstance = object->getFirstInstance();

    while (serverInstance) {
        ResourceInstance* shortServerIdInstance = serverInstance->getResourceInstance(RES_SHORT_SERVER_ID);
        ASSERT(shortServerIdInstance);

        RegHandler* handler = mRegHandlerStorage.newItem(NULL, INVALID_ID, *this);
        ASSERT(handler);

        Status status = STS_OK;

        if ((status = handler->bind(serverInstance)) != STS_OK) {
            LOG_ERROR("Can't bind to lwm2m server %d", status);

            mRegHandlerStorage.deleteItem(handler);
        }

        serverInstance = object->getNextInstance();
    }

    if (mRegHandlerStorage.size() == 0) {
        LOG_ERROR("No valid lwm2m servers found");

        return STS_ERR_NOT_EXIST;
    }

    return STS_OK;
}

Status Client::startNextPriorityReg()
{
    RegHandler* minPriorityHandler = NULL;
    uint64_t minPriority = ULONG_MAX;

    for (RegHandler::StorageNode* node = mRegHandlerStorage.begin(); node; node = node->next()) {
        // Skip already started handlers
        if (node->get()->mState != RegHandler::STATE_INIT) {
            continue;
        }

        ResourceInstance* regPriority =
            node->get()->mServerInstance->getResourceInstance(RES_REGISTRATION_PRIORITY_ORDER);

        // Skip handlers without priority order
        if (!regPriority) {
            continue;
        }

        if (regPriority->getUint() <= minPriority) {
            minPriorityHandler = node->get();
            minPriority = regPriority->getUint();
        }
    }

    if (minPriorityHandler) {
        return minPriorityHandler->startRegistration();
    }

    return STS_OK;
}

void Client::registrationStatus(RegHandler* handler, Status status)
{
    LOG_DEBUG("Handler /%d reg status: %d", handler->getId(), status);

    // if priority handler finished, start next priority handler
    if (handler->mServerInstance->getResourceInstance(RES_REGISTRATION_PRIORITY_ORDER)) {
        startNextPriorityReg();
    }
}

}  // namespace openlwm2m
