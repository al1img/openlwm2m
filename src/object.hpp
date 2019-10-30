#ifndef OPENLWM2M_OBJECT_HPP_
#define OPENLWM2M_OBJECT_HPP_

#include <stdint.h>
#include <cfloat>
#include <climits>

#include "interface.hpp"
#include "itembase.hpp"
#include "lwm2m.hpp"
#include "objectinstance.hpp"
#include "resource.hpp"
#include "storage.hpp"

namespace openlwm2m {

// clang-format off

#define OBJ_LWM2M_SECURITY  0
#define OBJ_LWM2M_SERVER    1
#define OBJ_DEVICE          3

// clang-format on

/**
 * lwm2m object.
 */
class Object : public ItemBase {
public:
    typedef Lwm2mStaticStorage<Object> Storage;

    Object(uint16_t id, bool single, bool manadatory, size_t maxInstances = 1);
    ~Object();

    void init();
    void release();

    Status createResourceString(uint16_t id, uint16_t operations, bool single, bool mandatory, size_t maxInstances = 1,
                                size_t maxLen = CONFIG_DEFAULT_STRING_LEN, ResourceInfo::Callback callback = 0,
                                void* context = NULL);
    Status createResourceInt(uint16_t id, uint16_t operations, bool single, bool mandatory, size_t maxInstances = 1,
                             int64_t min = LONG_MIN, int64_t max = LONG_MAX, ResourceInfo::Callback callback = 0,
                             void* context = NULL);
    Status createResourceUint(uint16_t id, uint16_t operations, bool single, bool mandatory, size_t maxInstances = 1,
                              uint64_t min = 0, uint64_t max = ULONG_MAX, ResourceInfo::Callback callback = 0,
                              void* context = NULL);
    Status createResourceFloat(uint16_t id, uint16_t operations, bool single, bool mandatory, size_t maxInstances = 1,
                               double min = -DBL_MAX, double max = DBL_MAX, ResourceInfo::Callback callback = 0,
                               void* context = NULL);
    Status createResourceBool(uint16_t id, uint16_t operations, bool single, bool mandatory, size_t maxInstances = 1,
                              ResourceInfo::Callback callback = 0, void* context = NULL);
    Status createResourceOpaque(uint16_t id, uint16_t operations, bool single, bool mandatory, size_t maxInstances = 1,
                                size_t minSize = 0, size_t maxSize = 0, ResourceInfo::Callback callback = 0,
                                void* context = NULL);
    Status createExecutableResource(uint16_t id, bool mandatory = true, ResourceInfo::Callback callback = 0,
                                    void* context = NULL);

    Status setResourceCallback(uint16_t resourceId, ResourceInfo::Callback callback, void* context);

    ObjectInstance* createInstance(uint16_t id = INVALID_ID, Status* status = NULL);
    Status deleteInstance(uint16_t id);
    ObjectInstance* getInstanceById(uint16_t id) { return mInstanceStorage.getItemById(id); }
    ObjectInstance* getFirstInstance() { return mInstanceStorage.getFirstItem(); }
    ObjectInstance* getNextInstance() { return mInstanceStorage.getNextItem(); }

    ResourceInstance* getResourceInstance(uint16_t objectInstanceId, uint16_t resourceId,
                                          uint16_t resourceInstanceId = 0);

    Status write(DataConverter* converter, bool checkOperation = false, bool ignoreMissing = true,
                 bool replace = false);

    Status read(DataConverter* converter, bool checkOperation = false);

    static bool isInstanceChanged()
    {
        bool result = sInstanceChanged;
        sInstanceChanged = false;

        return result;
    }

private:
    static bool sInstanceChanged;

    bool mSingle;
    bool mMandatory;

    bool mInitialized;

    ResourceInfo::Storage mResourceInfoStorage;
    ObjectInstance::Storage mInstanceStorage;

    Status createResource(uint16_t id, uint16_t operations, DataType type, bool single, bool mandatory,
                          size_t maxInstances, ResourceInfo::Min min, ResourceInfo::Max max,
                          ResourceInfo::Callback callback, void* context);
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_OBJECT_HPP_ */
