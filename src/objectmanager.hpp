#ifndef OPENLWM2M_OBJECTMANAGER_HPP_
#define OPENLWM2M_OBJECTMANAGER_HPP_

#include "object.hpp"

namespace openlwm2m {

class ObjectManager {
private:
    friend class Client;
    friend class RegHandler;

    Object::Storage mObjectStorage;

    ObjectManager();
    ~ObjectManager();

    void init();

    Object* createObject(uint16_t id, Object::Instance instance, size_t maxInstances, Object::Mandatory mandatory,
                         uint16_t interfaces, Status* status = NULL);
    Object* getObject(Interface interface, uint16_t id);

    Object* getFirstObject(Interface interface);
    Object* getNextObject(Interface interface);
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_OBJECTMANAGER_HPP_ */