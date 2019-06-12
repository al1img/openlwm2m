#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "coaptransport.hpp"

/*******************************************************************************
 * CoapTransport
 ******************************************************************************/

CoapTransport::CoapTransport()
{
    coap_startup();

    mContext = coap_new_context(NULL);
}

CoapTransport::~CoapTransport()
{
    coap_cleanup();
    if (mContext) {
        coap_free_context(mContext);
    }
}

/*******************************************************************************
 * Public
 ******************************************************************************/

void* CoapTransport::createSession(const char* uri, openlwm2m::Status* status)
{
    coap_address_t dst;

    openlwm2m::Status retStatus = resolveAddress(uri, &dst);
    if (retStatus != openlwm2m::STS_OK) {
        if (status) *status = retStatus;
        return NULL;
    }

    return coap_new_client_session(mContext, NULL, &dst, COAP_PROTO_UDP);
}

openlwm2m::Status CoapTransport::deleteSession(void* session)
{
    coap_session_release(static_cast<coap_session_t*>(session));

    return openlwm2m::STS_OK;
}

// Bootstrap
void CoapTransport::bootstrapRequest(void* session, BootstrapRequestHandler handler, void* context)
{
}

// Registration
void CoapTransport::registrationRequest(void* session, const char* clientName, uint64_t lifetime, const char* version,
                                        const char* bindingMode, bool queueMode, const char* smsNumber,
                                        const char* objects, RegistrationRequestHandler handler, void* context)
{
}

void CoapTransport::registrationUpdate(void* session, const uint32_t* lifetime, const char* bindingMode,
                                       const char* smsNumber, const char* objects, RegistrationUpdateHandler handler,
                                       void* context)
{
}

void CoapTransport::registrationDeregister(void* session, RegistrationDeregisterHandler handler, void* context)
{
}

// Device
void CoapTransport::deviceSend(void* session, DeviceSendHandler handler, void* context)
{
}

// Reporting
void CoapTransport::reportingNotify(void* session)
{
}

/*******************************************************************************
 * Private
 ******************************************************************************/

openlwm2m::Status CoapTransport::resolveAddress(const char* uri, coap_address_t* dst)
{
    char* host;
    char* port;

    if (0 == strncmp(uri, "coaps://", strlen("coaps://"))) {
        host = const_cast<char*>(uri) + strlen("coaps://");
    }
    else if (0 == strncmp(uri, "coap://", strlen("coap://"))) {
        host = const_cast<char*>(uri) + strlen("coap://");
    }
    else {
        return openlwm2m::STS_ERR_INVALID_VALUE;
    }

    port = strrchr(host, ':');
    if (port == NULL) {
        return openlwm2m::STS_ERR_INVALID_VALUE;
    }

    if (host[0] == '[') {
        host++;
        if (*(port - 1) == ']') {
            *(port - 1) = 0;
        }
        else {
            return openlwm2m::STS_ERR_INVALID_VALUE;
        }
    }

    *port = 0;
    port++;

    struct addrinfo *res, *ainfo;
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    memset(dst, 0, sizeof(*dst));

    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family = AF_UNSPEC;

    if (getaddrinfo(host, port, &hints, &res) != 0) {
        return openlwm2m::STS_ERR;
    }

    for (ainfo = res; ainfo != NULL; ainfo = ainfo->ai_next) {
        switch (ainfo->ai_family) {
            case AF_INET6:
            case AF_INET:
                dst->size = ainfo->ai_addrlen;
                memcpy(&dst->addr.sin6, ainfo->ai_addr, dst->size);
                return openlwm2m::STS_OK;

            default:
                break;
        }
    }

    return openlwm2m::STS_ERR_NOT_EXIST;
}
