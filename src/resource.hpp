#ifndef OPENLWM2M_RESOURCE_HPP_
#define OPENLWM2M_RESOURCE_HPP_

#include "itembase.hpp"
#include "lwm2m.hpp"
#include "resourceinstance.hpp"
#include "storage.hpp"

// clang-format off

// LwM2M Object: LwM2M Security
#define RES_LWM2M_SERVER_URI            0
#define RES_BOOTSTRAP_SERVER            1
#define RES_SECURITY_MODE               2
#define RES_PUBLIC_KEY_OR_IDENTITY      3
#define RES_SERVER_PUBLIC_KEY           4
#define RES_SECRET_KEY                  5
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
#define RES_REG_FAILURE_BLOCK           15
#define RES_BOOTSTRAP_ON_REG_FAILURE    16
#define RES_SEQUENCE_DELAY_TIMER        19
#define RES_SEQUENCE_RETRY_COUNT        20

// clang-format on

namespace openlwm2m {

class ObjectInstance;

class ResourceInfo : public ItemBase {
public:
    typedef void (*ValueChangeCbk)(void* context, ResourceInstance* resourceInstance);
    typedef Lwm2mStorage<ResourceInfo> Storage;

    union Min {
        int64_t minInt;
        uint64_t minUint;
        double minFloat;
    };

    union Max {
        int64_t maxInt;
        uint64_t maxUint;
        double maxFloat;
    };

    ResourceInfo(uint16_t id, uint16_t operations, DataType type, bool single, bool mandatory, size_t maxInstances,
                 Min min, Max max);
    ~ResourceInfo();

    void init();
    void release();

    void setValueChangedCbk(ValueChangeCbk callback, void* context);
    void valueChanged(ResourceInstance* instance);

    bool isSingle() const { return mSingle; }
    bool isMandatory() const { return mMandatory; }
    DataType getType() const { return mType; }
    size_t getMaxInstances() const { return mMaxInstances; }
    bool checkOperation(Operation operation) const { return (mOperations & operation) != 0; }
    Min min() const { return mMin; }
    Max max() const { return mMax; }

private:
    uint16_t mOperations;
    DataType mType;
    bool mSingle;
    bool mMandatory;
    size_t mMaxInstances;
    Min mMin;
    Max mMax;
    ValueChangeCbk mCallback;
    void* mContext;
};

class Resource : public ItemBase {
public:
    typedef Lwm2mStorage<Resource> Storage;

    Resource(ObjectInstance* parent, ResourceInfo& info);
    ~Resource();

    void init();
    void release();

    ObjectInstance* getObjectInstance() const;

    ResourceInstance* createInstance(uint16_t id = INVALID_ID, Status* status = NULL);
    Status deleteInstance(ResourceInstance* instance);

    ResourceInstance* getInstanceById(uint16_t id);
    ResourceInstance* getFirstInstance();
    ResourceInstance* getNextInstance();

    ResourceInfo& getInfo() const { return mInfo; }

private:
    ResourceInfo& mInfo;
    ResourceInstance::Storage mInstanceStorage;

    static ResourceInstance* newInstance(Resource* parent);
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_RESOURCE_HPP_ */