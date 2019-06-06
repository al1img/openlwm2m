#ifndef OPENLWM2M_OBJECT_HPP_
#define OPENLWM2M_OBJECT_HPP_

#include <stdint.h>

#include "interface.hpp"
#include "itembase.hpp"
#include "objectinstance.hpp"
#include "resourcedesc.hpp"
#include "status.hpp"
#include "storage.hpp"

namespace openlwm2m {

#define OBJ_LWM2M_SECURITY 0
#define OBJ_LWM2M_SERVER 1

/**
 * lwm2m object.
 */
class Object : public ItemBase {
public:
    enum Mandatory { MANDATORY, OPTIONAL };

    enum Instance { SINGLE, MULTIPLE };

    Status createResource(uint16_t id, uint16_t operations, ResourceDesc::Instance instance, size_t maxInstances,
                          ResourceDesc::Mandatory mandatory, ResourceDesc::Type type, int min = 0, int max = 0);

    ObjectInstance* createInstance(uint16_t id = INVALID_ID, Status* status = NULL);
    ObjectInstance* getInstanceById(uint16_t id);
    ObjectInstance* getFirstInstance();
    ObjectInstance* getNextInstance();
    ResourceInstance* getResourceInstance(uint16_t objInstanceId, uint16_t resId, uint16_t resInstanceId = 0);

private:
    struct Params {
        Instance instance;
        Mandatory mandatory;
        uint16_t interfaces;
        size_t maxInstances;
    };

    friend class Client;
    friend class StorageBase<Object>;
    friend class StorageArray<Object, Params>;

    typedef StorageArray<Object, Params> Storage;

    Params mParams;

    ResourceDesc::Storage mResourceDescStorage;
    ObjectInstance::Storage* mInstanceStorage;
    ObjectInstance::StorageNode* mInstanceNode;

    Object(ItemBase* parent, uint16_t id, Params params);
    virtual ~Object();

    void init();
    void release();
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_OBJECT_HPP_ */