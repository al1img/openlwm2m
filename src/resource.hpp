#ifndef OPENLWM2M_RESOURCE_HPP_
#define OPENLWM2M_RESOURCE_HPP_

#include <stdint.h>

namespace openlwm2m {

enum ResourceMandatory { RES_MANDATORY, RES_OPTIONAL };

enum ResourceInstance { RES_SINGLE, RES_MULTIPLE };

enum ResourceType {
    TYPE_STRING,
    TYPE_INT8,
    TYPE_INT16,
    TYPE_INT32,
    TYPE_INT64,
    TYPE_UINT8,
    TYPE_UINT16,
    TYPE_UINT32,
    TYPE_UINT64,
    TYPE_FLOAT32,
    TYPE_FLOAT64,
    TYPE_BOOL,
    TYPE_OPAQUE,
    TYPE_TIME,
    TYPE_OBJLINK,
    TYPE_CORELINK,
    TYPE_NONE
};

enum ResourceOperation { OP_NONE = 0x00, OP_READ = 0x01, OP_WRITE = 0x02, OP_READWRITE = 0x03, OP_EXECUTE = 0x04 };

class ResourceDesc {
private:
    friend class Object;

    uint16_t mId;
    uint16_t mOperations;
    ResourceInstance mInstance;
    int mMaxInstances;
    ResourceMandatory mMandatory;
    ResourceType mType;
    int mMin;
    int mMax;

    ResourceDesc(uint16_t id, uint16_t operations, ResourceInstance instance, int maxInstances,
                 ResourceMandatory mandatory, ResourceType type, int min, int max);
    ~ResourceDesc();
};

class Resource {
public:
    Resource(ResourceDesc* desc);

private:
    ResourceDesc* mDesc;
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_RESOURCE_HPP_ */