#include "client.hpp"
#include "interface.hpp"
#include "log.hpp"

#define LOG_MODULE "Client"

namespace openlwm2m {

/*******************************************************************************
 * Client
 ******************************************************************************/

Client::Client(TransportItf& transport) : mTransport(transport), mObjectStorage(NULL), mState(STATE_INIT)
{
    LOG_DEBUG("Create client");

    Status status = STS_OK;

    /***************************************************************************
     * E.1 LwM2M Object: LwM2M Security
     **************************************************************************/
    Object* object = createObject(0, Object::MULTIPLE, CONFIG_NUM_SERVERS + CONFIG_BOOTSTRAP_SERVER, Object::MANDATORY,
                                  ITF_BOOTSTRAP);
    LWM2M_ASSERT(object);
    // LWM2M Server URI
    status = object->createResource(0, ResourceDesc::OP_NONE, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY,
                                    ResourceDesc::TYPE_STRING, 0, 255);
    LWM2M_ASSERT(status == STS_OK);
    // Bootstrap-Server
    status = object->createResource(1, ResourceDesc::OP_NONE, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY,
                                    ResourceDesc::TYPE_BOOL);
    LWM2M_ASSERT(status == STS_OK);
    // Security Mode
    status = object->createResource(2, ResourceDesc::OP_NONE, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY,
                                    ResourceDesc::TYPE_INT8, 0, 4);
    LWM2M_ASSERT(status == STS_OK);
    // Public Key or Identity
    status = object->createResource(3, ResourceDesc::OP_NONE, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY,
                                    ResourceDesc::TYPE_OPAQUE, 0, CONFIG_CLIENT_PUBLIC_KEY_MAX_LEN);
    LWM2M_ASSERT(status == STS_OK);
    // Server Public Key
    status = object->createResource(4, ResourceDesc::OP_NONE, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY,
                                    ResourceDesc::TYPE_OPAQUE, 0, CONFIG_SERVER_PUBLIC_KEY_MAX_LEN);
    LWM2M_ASSERT(status == STS_OK);
    // Secret Key
    status = object->createResource(5, ResourceDesc::OP_NONE, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY,
                                    ResourceDesc::TYPE_OPAQUE, 0, CONFIG_CLIENT_PRIVATE_KEY_MAX_LEN);
    LWM2M_ASSERT(status == STS_OK);

    /***************************************************************************
     * E.2 LwM2M Object: LwM2M Server
     **************************************************************************/
    object = createObject(1, Object::MULTIPLE, CONFIG_NUM_SERVERS, Object::MANDATORY, ITF_ALL);
    LWM2M_ASSERT(object);
    // Short Server ID
    status = object->createResource(0, ResourceDesc::OP_READ, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY,
                                    ResourceDesc::TYPE_UINT16, 1, 65535);
    LWM2M_ASSERT(status == STS_OK);
    // Lifetime
    status = object->createResource(1, ResourceDesc::OP_READWRITE, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY,
                                    ResourceDesc::TYPE_INT32);
    LWM2M_ASSERT(status == STS_OK);
    // Notification Storing When Disabled or Offline
    status = object->createResource(6, ResourceDesc::OP_READWRITE, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY,
                                    ResourceDesc::TYPE_BOOL);
    LWM2M_ASSERT(status == STS_OK);
    // Binding
    status = object->createResource(7, ResourceDesc::OP_READWRITE, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY,
                                    ResourceDesc::TYPE_STRING, 0, CONFIG_BINDING_STR_MAX_LEN);
    LWM2M_ASSERT(status == STS_OK);
    // Registration Update Trigger
    status = object->createResource(8, ResourceDesc::OP_EXECUTE, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY,
                                    ResourceDesc::TYPE_NONE);
    LWM2M_ASSERT(status == STS_OK);

    /***************************************************************************
     * E.4 LwM2M Object: Device
     **************************************************************************/
    object = createObject(3, Object::SINGLE, 0, Object::MANDATORY, ITF_ALL);
    LWM2M_ASSERT(object);
    // Reboot
    status = object->createResource(4, ResourceDesc::OP_EXECUTE, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY,
                                    ResourceDesc::TYPE_NONE);
    LWM2M_ASSERT(status == STS_OK);
    // Error Code
    status = object->createResource(11, ResourceDesc::OP_READ, ResourceDesc::MULTIPLE, CONFIG_ERR_CODE_MAX_SIZE,
                                    ResourceDesc::MANDATORY, ResourceDesc::TYPE_INT8, 0, 8);
    LWM2M_ASSERT(status == STS_OK);
    // Supported Binding and Modes
    status = object->createResource(16, ResourceDesc::OP_READ, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY,
                                    ResourceDesc::TYPE_STRING, 0, 2);
    LWM2M_ASSERT(status == STS_OK);
}

Client::~Client() { LOG_DEBUG("Delete client"); }

/*******************************************************************************
 * Public
 ******************************************************************************/

Object* Client::createObject(uint16_t id, Object::Instance instance, size_t maxInstances, Object::Mandatory mandatory,
                             uint16_t interfaces, Status* status)
{
    if (mState != STATE_INIT) {
        if (status) *status = STS_ERR_STATE;
        return NULL;
    }

    if (instance == Object::SINGLE) {
        maxInstances = 1;
    }

    Object::Params params = {instance, mandatory, interfaces, maxInstances};

    return mObjectStorage.newItem(id, params, status);
}

Object* Client::getObject(uint16_t id, Interface interface, Status* status)
{
    Object* object = mObjectStorage.getItemById(id);

    if (!object) {
        if (status) *status = STS_ERR_EXIST;
        return NULL;
    }

    if (object->mParams.mInterfaces & interface) {
        return object;
    }

    if (status) *status = STS_ERR_EXIST;
    return NULL;
}

Status Client::init()
{
    if (mState != STATE_INIT) {
        return STS_ERR_STATE;
    }

    Node<Object>* node = mObjectStorage.begin();

    while (node) {
        Status status = node->get()->init();
        if (status != STS_OK) {
            return status;
        }
        node = node->next();
    }

    return STS_OK;
}

Status Client::bootstrapStart()
{
    LOG_DEBUG("Start bootstrap");

    mState = STATE_BOOTSTRAP;

    return STS_OK;
}

Status Client::bootstrapFinish()
{
    LOG_DEBUG("Finish bootstrap");

    if (mState != STATE_BOOTSTRAP) {
        return STS_ERR_STATE;
    }

    mState = STATE_REGISTERING;

    return STS_OK;
}

void Client::bootstrapDiscover() {}
void Client::bootstrapRead() {}
void Client::bootstrapWrite() {}
void Client::bootstrapDelete() {}

void Client::deviceRead() {}
void Client::deviceDiscover() {}
void Client::deviceWrite() {}
void Client::deviceWriteAttributes() {}
void Client::deviceExecute() {}
void Client::deviceCreate() {}
void Client::deviceDelete() {}
void Client::readComposite() {}
void Client::writeComposite() {}

void Client::reportingObserve() {}
void Client::reportingCancelObservation() {}
void Client::reportingObserveComposite() {}
void Client::reportingCancelObserveComposite() {}

}  // namespace openlwm2m
