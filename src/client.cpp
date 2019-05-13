#include "client.hpp"
#include "log.hpp"

#define LOG_MODULE "Client"

namespace openlwm2m {

Client::Client() { LOG_INFO("Create client"); }

Client::~Client()
{
    LOG_INFO("Delete client");

    Node<Object*>* node = mObjectList.begin();

    while (node != nullptr) {
        delete node->get();
        node = node->next();
    }
}

Object* Client::createObject(uint16_t id, int maxInstances, bool mandatory, uint16_t interfaces)
{
    Object* object = new Object();

    mObjectList.append(object);

    return object;
}

}  // namespace openlwm2m
