#ifndef COAPTRANSPORT_HPP_
#define COAPTRANSPORT_HPP_

#include <coap2/coap.h>

#include "interface.hpp"

class CoapTransport : public openlwm2m::TransportItf {
public:
    CoapTransport();
    ~CoapTransport();

    void* createConnection(const char* uri, openlwm2m::Status* status = NULL);
    openlwm2m::Status deleteConnection(void* connection);

    // Bootstrap
    void bootstrapRequest(BootstrapRequestHandler handler, void* context);

    // Registration
    void registrationRequest(const char* clientName, uint64_t lifetime, const char* version, const char* bindingMode,
                             bool queueMode, const char* smsNumber, const char* objects,
                             RegistrationRequestHandler handler, void* context);
    void registrationUpdate(const uint32_t* lifetime, const char* bindingMode, const char* smsNumber,
                            const char* objects, RegistrationUpdateHandler handler, void* context);
    void registrationDeregister(RegistrationDeregisterHandler handler, void* context);

    // Device
    void deviceSend(DeviceSendHandler handler, void* context);

    // Reporting
    void reportingNotify();

private:
    coap_context_t* mContext;

    openlwm2m::Status resolveAddress(const char* uri, coap_address_t* dst);
};

#endif /* COAPTRANSPORT_HPP_ */