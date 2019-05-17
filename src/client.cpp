#include "client.hpp"
#include "interface.hpp"
#include "log.hpp"

#define LOG_MODULE "Client"

namespace openlwm2m {

/*******************************************************************************
 * Client
 ******************************************************************************/

Client::Client() : mState(STATE_INIT)
{
    LOG_DEBUG("Create client");

    /***************************************************************************
     * E.1 LwM2M Object: LwM2M Security
     **************************************************************************/
    Object* object = createObject(0, Object::MULTIPLE, CONFIG_NUM_SERVERS + CONFIG_BOOTSTRAP_SERVER, Object::MANDATORY,
                                  ITF_BOOTSTRAP);
    // LWM2M Server URI
    object->createResource(0, ResourceDesc::OP_NONE, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY,
                           ResourceDesc::TYPE_STRING, 0, 255);
    // Bootstrap-Server
    object->createResource(1, ResourceDesc::OP_NONE, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY,
                           ResourceDesc::TYPE_BOOL);
    // Security Mode
    object->createResource(2, ResourceDesc::OP_NONE, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY,
                           ResourceDesc::TYPE_INT8, 0, 4);
    // Public Key or Identity
    object->createResource(3, ResourceDesc::OP_NONE, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY,
                           ResourceDesc::TYPE_OPAQUE, 0, CONFIG_CLIENT_PUBLIC_KEY_MAX_LEN);
    // Server Public Key
    object->createResource(4, ResourceDesc::OP_NONE, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY,
                           ResourceDesc::TYPE_OPAQUE, 0, CONFIG_SERVER_PUBLIC_KEY_MAX_LEN);
    // Secret Key
    object->createResource(5, ResourceDesc::OP_NONE, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY,
                           ResourceDesc::TYPE_OPAQUE, 0, CONFIG_CLIENT_PRIVATE_KEY_MAX_LEN);

    /***************************************************************************
     * E.2 LwM2M Object: LwM2M Server
     **************************************************************************/
    object = createObject(1, Object::MULTIPLE, CONFIG_NUM_SERVERS, Object::MANDATORY, ITF_ALL);
    // Short Server ID
    object->createResource(0, ResourceDesc::OP_READ, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY,
                           ResourceDesc::TYPE_UINT16, 1, 65535);
    // Lifetime
    object->createResource(1, ResourceDesc::OP_READWRITE, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY,
                           ResourceDesc::TYPE_INT32);
    // Notification Storing When Disabled or Offline
    object->createResource(6, ResourceDesc::OP_READWRITE, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY,
                           ResourceDesc::TYPE_BOOL);
    // Binding
    object->createResource(7, ResourceDesc::OP_READWRITE, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY,
                           ResourceDesc::TYPE_STRING, 0, CONFIG_BINDING_STR_MAX_LEN);
    // Registration Update Trigger
    object->createResource(8, ResourceDesc::OP_EXECUTE, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY,
                           ResourceDesc::TYPE_NONE);

    /***************************************************************************
     * E.4 LwM2M Object: Device
     **************************************************************************/
    object = createObject(3, Object::SINGLE, 0, Object::MANDATORY, ITF_ALL);
    // Reboot
    object->createResource(4, ResourceDesc::OP_EXECUTE, ResourceDesc::SINGLE, 0, ResourceDesc::MANDATORY,
                           ResourceDesc::TYPE_NONE);
    // Error Code
    object->createResource(11, ResourceDesc::OP_EXECUTE, ResourceDesc::MULTIPLE, CONFIG_ERR_CODE_MAX_SIZE,
                           ResourceDesc::MANDATORY, ResourceDesc::TYPE_INT8, 0, 8);
    // Error Code
    object->createResource(11, ResourceDesc::OP_EXECUTE, ResourceDesc::MULTIPLE, CONFIG_ERR_CODE_MAX_SIZE,
                           ResourceDesc::MANDATORY, ResourceDesc::TYPE_INT8, 0, 8);
}

Client::~Client()
{
    LOG_DEBUG("Delete client");

    Node* node = mObjectList.begin();

    while (node) {
        delete static_cast<Object*>(node->get());
        node = node->next();
    }
}

/*******************************************************************************
 * Public
 ******************************************************************************/

Object* Client::createObject(uint16_t id, Object::Instance instance, size_t maxInstances, Object::Mandatory mandatory,
                             uint16_t interfaces, Status* status)
{
    if (status) {
        *status = STS_OK;
    }

    if (mState != STATE_INIT) {
        if (status) {
            *status = STS_ERR_STATE;
        }

        return NULL;
    }

    if (instance == Object::SINGLE) {
        maxInstances = 1;
    }

    Object* object = new Object(id, instance, maxInstances, mandatory, interfaces);

    mObjectList.append(object);

    return object;
}

Object* Client::getObject(uint16_t id, Interface interface, Status* status)
{
    if (status) {
        *status = STS_OK;
    }

    Node* node = mObjectList.begin();

    while (node) {
        Object* object = static_cast<Object*>(node->get());

        if (object->getId() == id && (object->mInterfaces & interface)) {
            return object;
        }

        node = node->next();
    }

    if (status) {
        *status = STS_ERR_EXIST;
    }

    return NULL;
}

Status Client::startBootstrap()
{
    LOG_DEBUG("Start bootstrap");
    Node* node = mObjectList.begin();

    while (node) {
        Status status = static_cast<Object*>(node->get())->start();
        if (status != STS_OK) {
            return status;
        }
        node = node->next();
    }

    return STS_OK;
}

}  // namespace openlwm2m
