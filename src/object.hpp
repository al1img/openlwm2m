#ifndef OPENLWM2M_OBJECT_HPP_
#define OPENLWM2M_OBJECT_HPP_

#include <stdint.h>
#include <climits>

#include "interface.hpp"
#include "itembase.hpp"
#include "objectinstance.hpp"
#include "resource.hpp"
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
    typedef Lwm2mStorage<Object> Storage;

    Object(uint16_t id, uint16_t interfaces, ItemInstance instance, size_t maxInstances, ItemMandatory mandatory);
    ~Object();

    void init();
    void release();

    Status createResourceString(uint16_t id, uint16_t operations, ItemInstance instance, size_t maxInstances,
                                ItemMandatory mandatory, size_t maxLen = CONFIG_DEFAULT_STRING_LEN,
                                ResourceInfo::ValueChangeCbk callback = 0, void* context = NULL);

#if 0                                
    Status createResourceInt(uint16_t id, uint16_t operations, ItemInstance instance, size_t maxInstances,
                             ItemMandatory mandatory, int64_t min = LONG_MIN, int64_t max = LONG_MAX,
                             ResourceInfo::ValueChangeCbk cbk = 0, void* context = NULL);
    Status createResourceUint(uint16_t id, uint16_t operations, ItemInstance instance, size_t maxInstances,
                              ItemMandatory mandatory, uint64_t min = 0, uint64_t max = ULONG_MAX,
                              ResourceInfo::ValueChangeCbk cbk = 0, void* context = NULL);
    Status createResourceBool(uint16_t id, uint16_t operations, ItemInstance instance, size_t maxInstances,
                              ItemMandatory mandatory, ResourceInfo::ValueChangeCbk cbk = 0, void* context = NULL);
    Status createResourceOpaque(uint16_t id, uint16_t operations, ItemInstance instance, size_t maxInstances,
                                ItemMandatory mandatory, size_t minSize = 0, size_t maxSize = 0,
                                ResourceInfo::ValueChangeCbk cbk = 0, void* context = NULL);
    Status createResourceNone(uint16_t id, uint16_t operations, ItemInstance instance, size_t maxInstances,
                              ItemMandatory mandatory, ResourceInfo::ValueChangeCbk cbk = 0, void* context = NULL);
#endif
    ObjectInstance* createInstance(uint16_t id = INVALID_ID, Status* status = NULL);
    ObjectInstance* getInstanceById(uint16_t id);
    ObjectInstance* getFirstInstance();
    ObjectInstance* getNextInstance();
    ResourceInstance* getResourceInstance(uint16_t objectInstanceId, uint16_t resourceId,
                                          uint16_t resourceInstanceId = 0);

    Status setResourceChangedCbk(uint16_t resourceId, ResourceInfo::ValueChangeCbk callback, void* context);

    static bool isInstanceChanged()
    {
        bool result = sInstanceChanged;
        sInstanceChanged = false;

        return result;
    }

private:
    static bool sInstanceChanged;

    uint16_t mInterfaces;
    ItemInstance mInstance;
    ItemMandatory mMandatory;

    ResourceInfo::Storage mResourceInfoStorage;
    ObjectInstance::Storage mInstanceStorage;

    Status createResource(uint16_t id, uint16_t operations, ItemInstance instance, size_t maxInstances,
                          ItemMandatory mandatory, ResourceInfo::DataType type, ResourceInfo::Min min,
                          ResourceInfo::Max max, ResourceInfo::ValueChangeCbk callback, void* context);
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_OBJECT_HPP_ */
