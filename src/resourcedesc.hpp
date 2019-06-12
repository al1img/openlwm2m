#ifndef OPENLWM2M_RESOURCEDESC_HPP_
#define OPENLWM2M_RESOURCEDESC_HPP_

#include <stdint.h>
#include <cstddef>

#include "itembase.hpp"
#include "storage.hpp"

// LwM2M Object: LwM2M Security
#define RES_LWM2M_SERVER_URI 0
#define RES_BOOTSTRAP_SERVER 1
#define RES_SECURITY_SHORT_SERVER_ID 10

// LwM2M Object: LwM2M Server
#define RES_SHORT_SERVER_ID 0
#define RES_LIFETIME 1
#define RES_BINDING 7
#define RES_REGISTRATION_PRIORITY_ORDER 13
#define RES_INITIAL_REGISTRATION_DELAY 14

namespace openlwm2m {

class ResourceInstance;

class ResourceDesc : public ItemBase {
public:
    typedef void (*ValueChangeCbk)(void* context, ResourceInstance* resInstance);

    enum Mandatory { MANDATORY, OPTIONAL };

    enum Instance { SINGLE, MULTIPLE };

    enum Type {
        TYPE_STRING,
        TYPE_INT,
        TYPE_UINT,
        TYPE_FLOAT,
        TYPE_BOOL,
        TYPE_OPAQUE,
        TYPE_TIME,
        TYPE_OBJLINK,
        TYPE_CORELINK,
        TYPE_NONE
    };

    enum Operation { OP_NONE = 0x00, OP_READ = 0x01, OP_WRITE = 0x02, OP_READWRITE = 0x03, OP_EXECUTE = 0x04 };

private:
    struct Params {
        uint16_t operations;
        Instance instance;
        size_t maxInstances;
        Mandatory mandatory;
        Type type;
        ResourceDesc::ValueChangeCbk cbk;
        void* context;
        union {
            int64_t minInt;
            uint64_t minUint;
            double minFloat;
        };
        union {
            int64_t maxInt;
            uint64_t maxUint;
            double maxFloat;
        };
    };

    friend class Object;
    friend class ObjectInstance;
    friend class Resource;
    friend class ResourceInstance;
    friend class StorageList<ResourceDesc>;
    friend class Lwm2mStorage<ResourceDesc, Params>;

    typedef Lwm2mStorage<ResourceDesc, Params> Storage;

    Params mParams;

    ResourceDesc(ItemBase* parent, uint16_t id, Params params);
    ~ResourceDesc();

    void init();
    void release();
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_RESOURCE_HPP_ */
