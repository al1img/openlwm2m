#include "coaptransport.hpp"

void CoapTransport::createConnection() {}
void CoapTransport::deleteConnection() {}

// Bootstrap
void CoapTransport::bootstrapRequest(BootstrapRequestHandler handler, void* context) {}

// Registration
void CoapTransport::registrationRequest(const char* clientName, uint32_t lifetime, const char* version,
                                        const char* bindingMode, bool queueMode, const char* smsNumber,
                                        const char* objects, RegistrationRequestHandler handler, void* context)
{
}

void CoapTransport::registrationUpdate(const uint32_t* lifetime, const char* bindingMode, const char* smsNumber,
                                       const char* objects, RegistrationUpdateHandler handler, void* context)
{
}

void CoapTransport::registrationDeregister(RegistrationDeregisterHandler handler, void* context) {}

// Device
void CoapTransport::deviceSend(DeviceSendHandler handler, void* context) {}

// Reporting
void CoapTransport::reportingNotify() {}
