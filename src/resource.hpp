#ifndef OPENLWM2M_RESOURCE_HPP_
#define OPENLWM2M_RESOURCE_HPP_

#include <stdint.h>
#include <cstddef>

namespace openlwm2m {

class ResourceDesc;

class Resource {
public:
    enum Mandatory { MANDATORY, OPTIONAL };

    enum Instance { SINGLE, MULTIPLE };

    enum Type {
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

    enum Operation { OP_NONE = 0x00, OP_READ = 0x01, OP_WRITE = 0x02, OP_READWRITE = 0x03, OP_EXECUTE = 0x04 };

    Resource(ResourceDesc* desc);

private:
    ResourceDesc* mDesc;
};

class ResourceDesc {
private:
    friend class Object;

    uint16_t mId;
    uint16_t mOperations;
    Resource::Instance mInstance;
    size_t mMaxInstances;
    Resource::Mandatory mMandatory;
    Resource::Type mType;
    int mMin;
    int mMax;

    ResourceDesc(uint16_t id, uint16_t operations, Resource::Instance instance, size_t maxInstances,
                 Resource::Mandatory mandatory, Resource::Type type, int min, int max);
    ~ResourceDesc();
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_RESOURCE_HPP_ */