/** \file interface.hpp
 * LWM2M Interfaces.
 */

#ifndef OPENLWM2M_INTERFACE_HPP_
#define OPENLWM2M_INTERFACE_HPP_

#include <stdint.h>
#include <cstddef>

#include "status.hpp"

namespace openlwm2m {

/**
 * Define LWM2M Interfaces.
 *
 * See: 6. Interfaces.
 */
enum Interface {
    ITF_CLIENT = 0x00,     ///< Client interface: this interface access all objects.
    ITF_BOOTSTRAP = 0x01,  ///< Bootstrap interface.
    ITF_REGISTER = 0x02,   ///< Client registration interface.
    ITF_DEVICE = 0x04,     ///< Device Management and Service Enablement Interface.
    ITF_REPORTTING = 0x08  ///< Information Reporting Interface.
};

/**
 * Defines all interfaces.
 */
#define ITF_ALL (ITF_BOOTSTRAP | ITF_REGISTER | ITF_DEVICE | ITF_REPORTTING)

class TransportItf {
public:
    virtual void* createSession(const char* uri, Status* status = NULL) = 0;
    virtual Status deleteSession(void* session) = 0;

    typedef void (*RequestHandler)(void* context, Status status);

    // Bootstrap

    virtual void bootstrapRequest(void* session, RequestHandler handler, void* context) = 0;

    // Registration

    virtual void registrationRequest(void* session, const char* clientName, uint64_t lifetime, const char* version,
                                     const char* bindingMode, bool queueMode, const char* smsNumber,
                                     const char* objects, RequestHandler handler, void* context) = 0;
    virtual void registrationUpdate(void* session, const uint32_t* lifetime, const char* bindingMode,
                                    const char* smsNumber, const char* objects, RequestHandler handler,
                                    void* context) = 0;
    virtual void registrationDeregister(void* session, RequestHandler handler, void* context) = 0;

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
    virtual void bootstrapWrite() = 0;
    virtual void bootstrapDelete() = 0;

    // Device
    virtual void deviceRead() = 0;
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