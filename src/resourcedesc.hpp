#ifndef OPENLWM2M_RESOURCEDESC_HPP_
#define OPENLWM2M_RESOURCEDESC_HPP_

#include <stdint.h>
#include <cstddef>

#include "lwm2mbase.hpp"

namespace openlwm2m {

class ResourceDesc : public Lwm2mBase {
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

private:
    friend class Object;
    friend class Resource;

    uint16_t mOperations;
    ResourceDesc::Instance mInstance;
    size_t mMaxInstances;
    ResourceDesc::Mandatory mMandatory;
    ResourceDesc::Type mType;
    int mMin;
    int mMax;

    ResourceDesc(Lwm2mBase* parent, uint16_t id, uint16_t operations, ResourceDesc::Instance instance,
                 size_t maxInstances, ResourceDesc::Mandatory mandatory, ResourceDesc::Type type, int min, int max);
    ~ResourceDesc();
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_RESOURCE_HPP_ */