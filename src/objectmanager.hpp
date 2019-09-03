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

    Status write(Interface interface, const char* path, DataFormat format, void* data, size_t size);
    Status read(Interface interface, const char* path, DataFormat inFormat, void* inData, size_t inSize,
                DataFormat reqFormat, void* outData, size_t* outSize, DataFormat* outFormat);

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

    Status bootstrapWrite(DataConverter* converter, const char* path, void* data, size_t size);
    Status writeResource(ResourceInstance* instance, DataConverter::ResourceData* resourceData);

    Status readResourceInstance(DataConverter* converter, ResourceInstance* resourceInstance);
    Status readResource(DataConverter* converter, Resource* resource);
    Status readObjectInstance(DataConverter* converter, ObjectInstance* objectInstance);
    Status readObject(DataConverter* converter, Object* object);
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_OBJECTMANAGER_HPP_ */
