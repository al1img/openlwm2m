#ifndef COAP_TRANSPORT_HPP_
#define COAP_TRANSPORT_HPP_

#include "interface.hpp"

class CoapTransport : public openlwm2m::TransportItf {
public:
    void createConnection();
    void deleteConnection();

    // Bootstrap
    void bootstrapRequest(BootstrapRequestHandler handler, void* context);

    // Registration
    void registrationRequest(const char* clientName, uint32_t lifetime, const char* version, const char* bindingMode,
                             bool queueMode, const char* smsNumber, const char* objects,
                             RegistrationRequestHandler handler, void* context);
    void registrationUpdate(const uint32_t* lifetime, const char* bindingMode, const char* smsNumber,
                            const char* objects, RegistrationUpdateHandler handler, void* context);
    void registrationDeregister(RegistrationDeregisterHandler handler, void* context);

    // Device
    void deviceSend(DeviceSendHandler handler, void* context);

    // Reporting
    void reportingNotify();
};

#endif /* COAP_TRANSPORT_HPP_ */