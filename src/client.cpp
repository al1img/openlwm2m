#include "client.hpp"
#include "interface.hpp"
#include "log.hpp"

#define LOG_MODULE "Client"

namespace openlwm2m {

Client::Client()
{
    LOG_INFO("Create client");

    // Create security object
    Object* object = createObject(0, 0, true, BOOTSTRAP_ITF);
}

Client::~Client()
{
    Node* node = mObjectList.begin();

    while (node != nullptr) {
        delete static_cast<Object*>(node->get());
        node = node->next();
    }

    LOG_INFO("Delete client");
}

Object* Client::createObject(uint16_t id, int maxInstances, bool mandatory, uint16_t interfaces)
{
    Object* object = new Object(id, maxInstances, mandatory, interfaces);

    mObjectList.append(object);

    return object;
}

}  // namespace openlwm2m
