#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <climits>

#include "coaptransport.hpp"
#include "log.hpp"

#define LOG_MODULE "Coap"

/*******************************************************************************
 * CoapTransport
 ******************************************************************************/

CoapTransport::CoapTransport() : mReqStorage(Request::Param(), REQ_STORAGE_SIZE)
{
    coap_startup();

    mContext = coap_new_context(NULL);

    mContext->app = this;

    coap_set_log_level(LOG_DEBUG);

    coap_register_response_handler(mContext, &CoapTransport::responseHandler);
    coap_register_nack_handler(mContext, &CoapTransport::nackHandler);
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
void CoapTransport::bootstrapRequest(void* session, RequestHandler handler, void* context)
{
}

// Registration
void CoapTransport::registrationRequest(void* session, const char* clientName, uint64_t lifetime, const char* version,
                                        const char* bindingMode, bool queueMode, const char* smsNumber,
                                        const char* objects, RequestHandler handler, void* context)
{
    Request::Param param = {handler, context};

    Request* request = mReqStorage.newItem(NULL, INVALID_ID, param);
    ASSERT(request);

    coap_pdu_t* pdu = coap_pdu_init(COAP_MESSAGE_CON, COAP_REQUEST_GET, request->getId(),
                                    coap_session_max_pdu_size(static_cast<coap_session_t*>(session)));
    ASSERT(pdu);

    coap_add_option(pdu, COAP_OPTION_URI_PATH, 5, reinterpret_cast<const uint8_t*>("hello"));

    coap_tid_t tid = coap_send(static_cast<coap_session_t*>(session), pdu);

    LOG_DEBUG("Send tid: %d", tid);
}

void CoapTransport::registrationUpdate(void* session, const uint32_t* lifetime, const char* bindingMode,
                                       const char* smsNumber, const char* objects, RequestHandler handler,
                                       void* context)
{
}

void CoapTransport::registrationDeregister(void* session, RequestHandler handler, void* context)
{
}

// Device
void CoapTransport::deviceSend(void* session, RequestHandler handler, void* context)
{
}

// Reporting
void CoapTransport::reportingNotify(void* session, RequestHandler handler, void* context)
{
}

uint64_t CoapTransport::run(uint64_t timeout)
{
    if (timeout == ULONG_MAX) {
        timeout = 0;
    }

    return coap_run_once(mContext, timeout);
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

void CoapTransport::responseHandler(coap_context_t* context, coap_session_t* session, coap_pdu_t* sent,
                                    coap_pdu_t* received, const coap_tid_t id)
{
    static_cast<CoapTransport*>(context->app)->onResponse(session, sent, received, id);
}

void CoapTransport::onResponse(coap_session_t* session, coap_pdu_t* sent, coap_pdu_t* received, const coap_tid_t id)
{
    LOG_DEBUG("Response received");
}

void CoapTransport::nackHandler(coap_context_t* context, coap_session_t* session, coap_pdu_t* sent,
                                coap_nack_reason_t reason, const coap_tid_t id)
{
    static_cast<CoapTransport*>(context->app)->onNack(session, sent, reason, id);
}

void CoapTransport::onNack(coap_session_t* session, coap_pdu_t* sent, coap_nack_reason_t reason, const coap_tid_t id)
{
    LOG_DEBUG("Nack received");
}
