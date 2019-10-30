#ifndef OPENLWM2M_OBJECTMANAGER_HPP_
#define OPENLWM2M_OBJECTMANAGER_HPP_

#include "object.hpp"

namespace openlwm2m {

class ObjectManager {
public:
    ObjectManager();
    ~ObjectManager();

    void init();

    Object* createObject(uint16_t id, bool single, bool mandatory, size_t maxInstances = 1, Status* status = NULL);
    Object* getObjectById(uint16_t id);

    Object* getFirstObject();
    Object* getNextObject();

    Status bootstrapWrite(DataFormat dataFormat, void* data, size_t size, uint16_t objectId,
                          uint16_t objectInstanceId = INVALID_ID, uint16_t resourceId = INVALID_ID);

    Status bootstrepRead(DataFormat* dataFormat, void* data, size_t* size, uint16_t objectId,
                         uint16_t objectInstanceId = INVALID_ID);

    Status addConverter(DataConverter* converter);
    bool isFormatSupported(DataFormat format);

    ObjectInstance* getServerInstance(uint16_t shortServerId);

private:
    Object::Storage mObjectStorage;
    DataConverter::Storage mConverterStorage;

    void createSecurityObject();
    void createServerObject();
    void createDeviceObject();

    void createConverters();

    static void resBootstrapChanged(void* context, ResourceInstance* resInstance);

    // TODO store/restore items
    Status storeObject(Object* object);
    Status restoreObject(Object* object);
    Status storeObjectInstance(ObjectInstance* objectInstance);
    Status restoreObjectInstance(ObjectInstance* objectInstance);
    Status storeResource(Resource* resoure);
    Status restoreResource(Resource* resource);

    Status writeObject(Object* object, DataConverter* converter, bool checkOperation = false, bool ignoreMissing = true,
                       bool replace = false);
    Status writeObjectInstance(ObjectInstance* objectInstance, DataConverter* converter, bool checkOperation = false,
                               bool ignoreMissing = true, bool replace = false);
    Status writeResource(Resource* resource, DataConverter* converter, bool checkOperation = false,
                         bool replace = false);
    Status writeResourceInstance(ResourceInstance* resourceInstance, DataConverter* converter,
                                 bool checkOperation = false);

    Status readResourceInstance(DataConverter* converter, ResourceInstance* resourceInstance);
    Status readResource(DataConverter* converter, Resource* resource);
    Status readObjectInstance(DataConverter* converter, ObjectInstance* objectInstance);
    Status readObject(DataConverter* converter, Object* object);
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_OBJECTMANAGER_HPP_ */
