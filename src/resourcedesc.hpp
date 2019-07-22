#ifndef OPENLWM2M_RESOURCEDESC_HPP_
#define OPENLWM2M_RESOURCEDESC_HPP_

#include <stdint.h>
#include <cstddef>

#include "dataformat.hpp"
#include "itembase.hpp"
#include "storage.hpp"

// clang-format off

// LwM2M Object: LwM2M Security
#define RES_LWM2M_SERVER_URI            0
#define RES_BOOTSTRAP_SERVER            1
#define RES_SECURITY_SHORT_SERVER_ID    10

// LwM2M Object: LwM2M Server
#define RES_SHORT_SERVER_ID             0
#define RES_LIFETIME                    1
#define RES_DEFAULT_MIN_PERIOD          2
#define RES_DEFAULT_MAX_PERIOD          3
#define RES_DISBALE                     4
#define RES_DISABLE_TIMEOUT             5
#define RES_NOTIFICATION_STORING        6
#define RES_BINDING                     7
#define RES_REGISTRATION_UPDATE_TRIGGER 8
#define RES_BOOTSTRAP_REQUEST_TRIGGER   9
#define RES_APN_LINK                    10
#define RES_TLS_DTLS_ALERT_CODE         11
#define RES_LAST_BOOTSTRAPPED           12
#define RES_REGISTRATION_PRIORITY_ORDER 13
#define RES_INITIAL_REGISTRATION_DELAY  14

// clang-format on

namespace openlwm2m {

class ResourceInstance;

class ResourceDesc : public ItemBase {
public:
    typedef void (*ValueChangeCbk)(void* context, ResourceInstance* resInstance);

    enum Mandatory { MANDATORY, OPTIONAL };

    enum Instance { SINGLE, MULTIPLE };

    enum Operation { OP_NONE = 0x00, OP_READ = 0x01, OP_WRITE = 0x02, OP_READWRITE = 0x03, OP_EXECUTE = 0x04 };

private:
    struct Params {
        uint16_t operations;
        Instance instance;
        size_t maxInstances;
        Mandatory mandatory;
        DataType type;
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
    friend class ResourceString;
    friend class ResourceInt;
    friend class ResourceUint;
    friend class ResourceBool;
    friend class Lwm2mStorage<ResourceDesc, Params>;

    typedef Lwm2mStorage<ResourceDesc, Params> Storage;

    Params mParams;

    ResourceDesc(ItemBase* parent, Params params);
    ~ResourceDesc();

    void init();
    void release();
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_RESOURCE_HPP_ */
