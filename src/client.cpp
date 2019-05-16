#include "client.hpp"
#include "interface.hpp"
#include "log.hpp"

#define LOG_MODULE "Client"

namespace openlwm2m {

Client::Client() : mState(STATE_INIT)
{
    LOG_DEBUG("Create client");

    // LwM2M Object: LwM2M Security
    Object* object = createObject(0, Object::MULTIPLE, CONFIG_MAX_OBJ_SECURITY, Object::MANDATORY, ITF_BOOTSTRAP);
    // LWM2M Server URI
    object->createResource(0, Resource::OP_NONE, Resource::SINGLE, 0, Resource::MANDATORY, Resource::TYPE_STRING, 0,
                           255);
    // Bootstrap-Server
    object->createResource(1, Resource::OP_NONE, Resource::SINGLE, 0, Resource::MANDATORY, Resource::TYPE_BOOL);
    // Security Mode
    object->createResource(2, Resource::OP_NONE, Resource::SINGLE, 0, Resource::MANDATORY, Resource::TYPE_INT8, 0, 4);

    // LLwM2M Object: LwM2M Server
    object = createObject(1, Object::MULTIPLE, CONFIG_MAX_OBJ_SERVER, Object::MANDATORY, ITF_ALL);

    // LLwM2M Object: Device
    object = createObject(3, Object::SINGLE, 0, Object::MANDATORY, ITF_ALL);
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

Object* Client::createObject(uint16_t id, Object::Instance instance, size_t maxInstances, Object::Mandatory mandatory,
                             uint16_t interfaces, Status* status)
{
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

    mObjectList.insert(object);

    return object;
}

Status Client::startBootstrap()
{
    LOG_DEBUG("Start client");
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
