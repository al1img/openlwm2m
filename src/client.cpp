#include "client.hpp"
#include "interface.hpp"
#include "log.hpp"

#define LOG_MODULE "Client"

namespace openlwm2m {

Client::Client()
{
    LOG_INFO("Create client");

    // LwM2M Object: LwM2M Security
    Object* object = createObject(0, OBJ_MULTIPLE, CONFIG_MAX_OBJ_SECURITY, OBJ_MANDATORY, ITF_BOOTSTRAP);
    // LWM2M Server URI
    object->createResource(0, OP_NONE, RES_SINGLE, 0, RES_MANDATORY, TYPE_STRING, 0, 255);
    // Bootstrap-Server
    object->createResource(1, OP_NONE, RES_SINGLE, 0, RES_MANDATORY, TYPE_BOOL);
    // Security Mode
    object->createResource(2, OP_NONE, RES_SINGLE, 0, RES_MANDATORY, TYPE_INT8, 0, 4);

    // LLwM2M Object: LwM2M Server
    object = createObject(1, OBJ_MULTIPLE, CONFIG_MAX_OBJ_SERVER, OBJ_MANDATORY, ITF_ALL);
}

Client::~Client()
{
    Node* node = mObjectList.begin();

    while (node) {
        delete static_cast<Object*>(node->get());
        node = node->next();
    }

    LOG_INFO("Delete client");
}

Object* Client::createObject(uint16_t id, ObjectInstance instance, int maxInstances, ObjectMandatory mandatory,
                             uint16_t interfaces)
{
    Object* object = new Object(id, instance, maxInstances, mandatory, interfaces);

    mObjectList.append(object);

    return object;
}

}  // namespace openlwm2m
