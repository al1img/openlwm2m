/** \file interface.hpp
 * LWM2M Interfaces.
 */

#ifndef OPENLWM2M_INTERFACE_HPP_
#define OPENLWM2M_INTERFACE_HPP_

#include <stdint.h>
#include <cstddef>

#include "dataformat.hpp"
#include "lwm2m.hpp"

namespace openlwm2m {

class ClientItf;

class TransportItf {
public:
    virtual void setClient(ClientItf* client) = 0;

    virtual void* createSession(const char* uri, Status* status = NULL) = 0;
    virtual Status deleteSession(void* session) = 0;

    typedef void (*RequestHandler)(void* context, void* data, Status status);

    // Bootstrap

    virtual void bootstrapRequest(void* session, RequestHandler handler, void* context) = 0;

    // Registration

    virtual Status registrationRequest(void* session, const char* clientName, int64_t lifetime, const char* version,
                                       const char* bindingMode, bool queueMode, const char* smsNumber,
                                       const char* objects, RequestHandler handler, void* context) = 0;
    virtual Status registrationUpdate(void* session, const char* location, const int64_t* lifetime,
                                      const char* bindingMode, const char* smsNumber, const char* objects,
                                      RequestHandler handler, void* context) = 0;
    virtual Status deregistrationRequest(void* session, const char* location, RequestHandler handler,
                                         void* context) = 0;

    // Device

    virtual void deviceSend(void* session, RequestHandler handler, void* context) = 0;

    // Reporting
    virtual void reportingNotify(void* session, RequestHandler handler, void* context) = 0;
};

class ClientItf {
public:
    // Bootstrap
    virtual void bootstrapDiscover() = 0;
    virtual void bootstrapRead() = 0;
    virtual Status bootstrapWrite(DataFormat dataFormat, void* data, size_t size, uint16_t objectId,
                                  uint16_t objectInstanceId = INVALID_ID, uint16_t resourceId = INVALID_ID) = 0;
    virtual void bootstrapDelete() = 0;

    // Device
    virtual Status deviceRead(const char* path, DataFormat reqFormat, void* data, size_t* size, DataFormat* format) = 0;
    virtual void deviceDiscover() = 0;
    virtual void deviceWrite() = 0;
    virtual void deviceWriteAttributes() = 0;
    virtual void deviceExecute() = 0;
    virtual void deviceCreate() = 0;
    virtual void deviceDelete() = 0;
    virtual void readComposite() = 0;
    virtual void writeComposite() = 0;

    // Reporting
    virtual void reportingObserve() = 0;
    virtual void reportingCancelObservation() = 0;
    virtual void reportingObserveComposite() = 0;
    virtual void reportingCancelObserveComposite() = 0;
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_INTERFACE_HPP_ */
