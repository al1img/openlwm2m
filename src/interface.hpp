#include <stdint.h>

namespace openlwm2m {

#define ALL_ITF (BOOTSTRAP_ITF | REGISTER_ITF | DEVICE_ITF | REPORTTING_ITF)

enum Interfaces { BOOTSTRAP_ITF = 0x00, REGISTER_ITF = 0x01, DEVICE_ITF = 0x02, REPORTTING_ITF = 0x04 };

class BootstrapUlItf {
public:
    typedef void (*RequestHandler)(void* context);

    virtual void bootstrapRequest(RequestHandler handler, void* context) = 0;
};

class BootstrapDlItf {
public:
    virtual void bootstrapFinish() = 0;
    virtual void bootstrapDiscover() = 0;
    virtual void bootstrapRead() = 0;
    virtual void bootstrapWrite() = 0;
    virtual void bootstrapDelete() = 0;
};

class RegisterUlItf {
public:
    typedef void (*RequestHandler)(void* context);
    typedef void (*UpdateHandler)(void* context);
    typedef void (*DeregisterHandler)(void* context);

    virtual void registrationRequest(const char* clientName, uint32_t lifetime, const char* version,
                                     const char* bindingMode, bool queueMode, const char* smsNumber,
                                     const char* objects, RequestHandler handler, void* context) = 0;
    virtual void registrationUpdate(const uint32_t* lifetime, const char* bindingMode, const char* smsNumber,
                                    const char* objects, UpdateHandler handler, void* context) = 0;
    virtual void registrationDeregister(DeregisterHandler handler, void* context) = 0;
};

class DeviceUlItf {
public:
    typedef void (*SendHandler)(void* context);

    virtual void deviceSend(SendHandler handler, void* context) = 0;
};

class DeviceDlItf {
public:
    virtual void deviceRead() = 0;
    virtual void deviceDiscover() = 0;
    virtual void deviceWrite() = 0;
    virtual void deviceWriteAttributes() = 0;
    virtual void deviceExecute() = 0;
    virtual void deviceCreate() = 0;
    virtual void deviceDelete() = 0;
    virtual void readComposite() = 0;
    virtual void writeComposite() = 0;
};

class ReportingUlItf {
public:
    virtual void reportingNotify() = 0;
};

class ReportingDlItf {
public:
    virtual void reportingObserve() = 0;
    virtual void reportingCancelObservation() = 0;
    virtual void reportingObserveComposite() = 0;
    virtual void reportingCancelObserveComposite() = 0;
};

}  // namespace openlwm2m