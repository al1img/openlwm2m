#include "objectmanager.hpp"

#define LOG_MODULE "ObjectInstance"

namespace openlwm2m {

/*******************************************************************************
 * Public
 ******************************************************************************/

ObjectManager::ObjectManager()
{
}

ObjectManager::~ObjectManager()
{
    mObjectStorage.release();
}

void ObjectManager::init()
{
    mObjectStorage.init();
}

Object* ObjectManager::createObject(uint16_t id, Object::Instance instance, size_t maxInstances,
                                    Object::Mandatory mandatory, uint16_t interfaces, Status* status)
{
    if (instance == Object::SINGLE) {
        maxInstances = 1;
    }

    LOG_DEBUG("Create object /%d", id);

    Object::Params params = {instance, mandatory, interfaces, maxInstances};

    return mObjectStorage.newItem(NULL, id, params, status);
}

Object* ObjectManager::getObject(Interface interface, uint16_t id)
{
    Object* object = mObjectStorage.getItemById(id);

    if (interface && object && !(object->mParams.interfaces & interface)) {
        LOG_DEBUG("Object /%d not accesible by interface %d", id, interface);
        return NULL;
    }

    return object;
}

Object* ObjectManager::getFirstObject(Interface interface)
{
    Object* object = mObjectStorage.getFirstItem();

    while (object) {
        if (interface && !(object->mParams.interfaces & interface)) {
            LOG_DEBUG("Object /%d not accesible by interface %d", object->getId(), interface);

            object = mObjectStorage.getNextItem();

            continue;
        }

        return object;
    }

    return NULL;
}

Object* ObjectManager::getNextObject(Interface interface)
{
    Object* object = mObjectStorage.getNextItem();

    while (object) {
        if (interface && !(object->mParams.interfaces & interface)) {
            LOG_DEBUG("Object /%d not accesible by interface %d", object->getId(), interface);

            object = mObjectStorage.getNextItem();

            continue;
        }

        return object;
    }

    return NULL;
}

Status ObjectManager::addConverter(DataConverter* converter)
{
    return mConverterStorage.addItem(converter);
}

Status ObjectManager::write(Interface interface, DataFormat format, const char* path, void* data, size_t size)
{
    DataConverter* converter = mConverterStorage.getItemById(format);

    if (converter == NULL) {
        return STS_ERR_NOT_EXIST;
    }

    //    converter->dataToObjects(data, size);

    return STS_OK;
}

/*******************************************************************************
 * Private
 ******************************************************************************/

}  // namespace openlwm2m
