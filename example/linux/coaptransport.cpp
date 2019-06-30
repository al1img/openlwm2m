#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <climits>
#include <cstdio>

#include "coaptransport.hpp"
#include "log.hpp"
#include "utils.hpp"

#define LOG_MODULE "Coap"

using namespace openlwm2m;

/*******************************************************************************
 * CoapTransport
 ******************************************************************************/

CoapTransport::CoapTransport() : mReqStorage(NULL, Request::Param(), REQ_STORAGE_SIZE)
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

void* CoapTransport::createSession(const char* uri, Status* status)
{
    coap_address_t dst;

    Status retStatus = resolveAddress(uri, &dst);
    if (retStatus != STS_OK) {
        if (status) *status = retStatus;
        return NULL;
    }

    return coap_new_client_session(mContext, NULL, &dst, COAP_PROTO_UDP);
}

Status CoapTransport::deleteSession(void* session)
{
    coap_session_release(static_cast<coap_session_t*>(session));

    return STS_OK;
}

// Bootstrap
void CoapTransport::bootstrapRequest(void* session, RequestHandler handler, void* context)
{
}

// Registration
Status CoapTransport::registrationRequest(void* session, const char* clientName, uint64_t lifetime, const char* version,
                                          const char* bindingMode, bool queueMode, const char* smsNumber,
                                          const char* objects, RequestHandler handler, void* context)
{
    coap_optlist_t* optList = NULL;
    char buf[CONFIG_DEFAULT_STRING_LEN];
    int ret;

    coap_insert_optlist(&optList, coap_new_optlist(COAP_OPTION_CONTENT_TYPE,
                                                   coap_encode_var_safe(reinterpret_cast<uint8_t*>(buf), sizeof(buf),
                                                                        COAP_MEDIATYPE_APPLICATION_LINK_FORMAT),
                                                   reinterpret_cast<const uint8_t*>(buf)));

    coap_insert_optlist(&optList, coap_new_optlist(COAP_OPTION_URI_PATH, 2, reinterpret_cast<const uint8_t*>("rd")));

    // Client name
    if (clientName) {
        ret = snprintf(buf, sizeof(buf), "ep=%s", clientName);
        coap_insert_optlist(&optList,
                            coap_new_optlist(COAP_OPTION_URI_QUERY, ret, reinterpret_cast<const uint8_t*>(buf)));
    }

    // Version
    ret = snprintf(buf, sizeof(buf), "lwm2m=%s", version);
    coap_insert_optlist(&optList, coap_new_optlist(COAP_OPTION_URI_QUERY, ret, reinterpret_cast<const uint8_t*>(buf)));

    // Lifetime
    ret = snprintf(buf, sizeof(buf), "lt=%lu", lifetime);
    coap_insert_optlist(&optList, coap_new_optlist(COAP_OPTION_URI_QUERY, ret, reinterpret_cast<const uint8_t*>(buf)));

    // Binding
    ret = snprintf(buf, sizeof(buf), "b=%s", bindingMode);
    coap_insert_optlist(&optList, coap_new_optlist(COAP_OPTION_URI_QUERY, ret, reinterpret_cast<const uint8_t*>(buf)));

    // Queue mode
    if (queueMode) {
        coap_insert_optlist(&optList,
                            coap_new_optlist(COAP_OPTION_URI_QUERY, 1, reinterpret_cast<const uint8_t*>("Q")));
    }

    // SMS number
    if (smsNumber) {
        ret = snprintf(buf, sizeof(buf), "sms=%s", smsNumber);
        coap_insert_optlist(&optList,
                            coap_new_optlist(COAP_OPTION_URI_QUERY, ret, reinterpret_cast<const uint8_t*>(buf)));
    }

    Request::Param param = {handler, context};
    Request* request = mReqStorage.newItem(INVALID_ID);

    if (!request) {
        return STS_ERR_NO_MEM;
    }

    uint16_t token = request->getId();

    Status status = sendPdu(static_cast<coap_session_t*>(session), COAP_MESSAGE_CON, COAP_REQUEST_POST, token, &optList,
                            reinterpret_cast<const uint8_t*>(objects), strlen(objects));

    if (status != STS_OK) {
        mReqStorage.deleteItem(request);
        return status;
    }

    return STS_OK;
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

void CoapTransport::run()
{
    coap_run_once(mContext, COAP_RESOURCE_CHECK_TIME * 1000);
}

/*******************************************************************************
 * Private
 ******************************************************************************/

Status CoapTransport::resolveAddress(const char* uri, coap_address_t* dst)
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
        return STS_ERR_INVALID_VALUE;
    }

    port = strrchr(host, ':');
    if (port == NULL) {
        return STS_ERR_INVALID_VALUE;
    }

    if (host[0] == '[') {
        host++;
        if (*(port - 1) == ']') {
            *(port - 1) = 0;
        }
        else {
            return STS_ERR_INVALID_VALUE;
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
        return STS_ERR;
    }

    for (ainfo = res; ainfo != NULL; ainfo = ainfo->ai_next) {
        switch (ainfo->ai_family) {
            case AF_INET6:
            case AF_INET:
                dst->size = ainfo->ai_addrlen;
                memcpy(&dst->addr.sin6, ainfo->ai_addr, dst->size);
                return STS_OK;

            default:
                break;
        }
    }

    return STS_ERR_NOT_EXIST;
}

Status CoapTransport::sendPdu(coap_session_t* session, uint8_t type, uint8_t code, uint16_t token,
                              coap_optlist_t** optList, const uint8_t* data, size_t dataLen)
{
    Status status = STS_OK;

    coap_pdu_t* pdu = coap_pdu_init(type, code, coap_new_message_id(session), coap_session_max_pdu_size(session));

    if (!pdu) {
        status = STS_ERR_NO_MEM;
        goto err;
    }

    if (!coap_add_token(pdu, sizeof(token), reinterpret_cast<const uint8_t*>(&token))) {
        status = STS_ERR_NO_MEM;
        goto err;
    }

    if (!coap_add_optlist_pdu(pdu, optList)) {
        status = STS_ERR_NO_MEM;
        goto err;
    }

    if (!coap_add_data(pdu, dataLen, data)) {
        status = STS_ERR_NO_MEM;
        goto err;
    }

    coap_show_pdu(LOG_DEBUG, pdu);

    coap_send(session, pdu);

    return STS_OK;

err:
    if (pdu) {
        coap_delete_pdu(pdu);
    }

    return status;
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
    LOG_DEBUG("Nack received, tid: %d, %d", id, sent->tid);

    Request* request = mReqStorage.getItemById(sent->tid);

    if (request) {
        request->mParam.handler(request->mParam.context, STS_ERR);
    }
    else {
        LOG_ERROR("Unknown response received, tid: %d", sent->tid);
    }
}
