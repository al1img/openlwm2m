#ifndef OPENLWM2M_OBJECTMANAGER_HPP_
#define OPENLWM2M_OBJECTMANAGER_HPP_

#include "object.hpp"

namespace openlwm2m {

class ObjectManager {
public:
    ObjectManager();
    ~ObjectManager();

    void init();

    Object* createObject(uint16_t id, Object::Instance instance, size_t maxInstances, Object::Mandatory mandatory,
                         uint16_t interfaces, Status* status = NULL);
    Object* getObject(Interface interface, uint16_t id);

    Object* getFirstObject(Interface interface);
    Object* getNextObject(Interface interface);

    Status write(Interface interface, DataFormat format, const char* path, void* data, size_t size);

    Status addConverter(DataConverter* converter);

private:
    Object::Storage mObjectStorage;
    DataConverter::Storage mConverterStorage;
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_OBJECTMANAGER_HPP_ */