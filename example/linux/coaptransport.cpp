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
Status CoapTransport::registrationRequest(void* session, const char* clientName, int64_t lifetime, const char* version,
                                          const char* bindingMode, bool queueMode, const char* smsNumber,
                                          const char* objects, RequestHandler handler, void* context)
{
    coap_optlist_t* optList = NULL;
    char buf[CONFIG_DEFAULT_STRING_LEN + 1];
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
    ret = snprintf(buf, sizeof(buf), "lt=%ld", lifetime);
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

    coap_pdu_t* pdu =
        coap_pdu_init(COAP_MESSAGE_CON, COAP_REQUEST_POST, coap_new_message_id(static_cast<coap_session_t*>(session)),
                      coap_session_max_pdu_size(static_cast<coap_session_t*>(session)));

    Status status = sendPdu(static_cast<coap_session_t*>(session), pdu, &optList,
                            reinterpret_cast<const uint8_t*>(objects), strlen(objects));

    if (status != STS_OK) {
        return status;
    }

    Request* request = mReqStorage.newItem(pdu->tid);

    request->mParam = (Request::Param){Request::REGISTRATION, handler, context};

    if (!request) {
        return STS_ERR_NO_MEM;
    }

    LOG_DEBUG("Send registration request, tid: %d", pdu->tid);

    return STS_OK;
}

Status CoapTransport::registrationUpdate(void* session, const char* location, const int64_t* lifetime,
                                         const char* bindingMode, const char* smsNumber, const char* objects,
                                         RequestHandler handler, void* context)
{
    coap_optlist_t* optList = NULL;
    char buf[CONFIG_DEFAULT_STRING_LEN + 1];
    int ret;

    coap_insert_optlist(&optList, coap_new_optlist(COAP_OPTION_CONTENT_TYPE,
                                                   coap_encode_var_safe(reinterpret_cast<uint8_t*>(buf), sizeof(buf),
                                                                        COAP_MEDIATYPE_APPLICATION_LINK_FORMAT),
                                                   reinterpret_cast<const uint8_t*>(buf)));

    insertLocation(location, &optList);

    // Lifetime
    if (lifetime) {
        ret = snprintf(buf, sizeof(buf), "lt=%ld", *lifetime);
        coap_insert_optlist(&optList,
                            coap_new_optlist(COAP_OPTION_URI_QUERY, ret, reinterpret_cast<const uint8_t*>(buf)));
    }

    // Binding
    if (bindingMode) {
        ret = snprintf(buf, sizeof(buf), "b=%s", bindingMode);
        coap_insert_optlist(&optList,
                            coap_new_optlist(COAP_OPTION_URI_QUERY, ret, reinterpret_cast<const uint8_t*>(buf)));
    }

    // SMS number
    if (smsNumber) {
        ret = snprintf(buf, sizeof(buf), "sms=%s", smsNumber);
        coap_insert_optlist(&optList,
                            coap_new_optlist(COAP_OPTION_URI_QUERY, ret, reinterpret_cast<const uint8_t*>(buf)));
    }

    coap_pdu_t* pdu =
        coap_pdu_init(COAP_MESSAGE_CON, COAP_REQUEST_POST, coap_new_message_id(static_cast<coap_session_t*>(session)),
                      coap_session_max_pdu_size(static_cast<coap_session_t*>(session)));

    Status status = STS_OK;

    if (objects) {
        status = sendPdu(static_cast<coap_session_t*>(session), pdu, &optList,
                         reinterpret_cast<const uint8_t*>(objects), strlen(objects));
    }
    else {
        status = sendPdu(static_cast<coap_session_t*>(session), pdu, &optList, NULL, 0);
    }

    if (status != STS_OK) {
        return status;
    }

    Request* request = mReqStorage.newItem(pdu->tid);

    request->mParam = (Request::Param){Request::REGISTRATION, handler, context};

    if (!request) {
        return STS_ERR_NO_MEM;
    }

    LOG_DEBUG("Send registration update, tid: %d", pdu->tid);

    return STS_OK;
}

Status CoapTransport::deregistrationRequest(void* session, const char* location, RequestHandler handler, void* context)
{
    return STS_OK;
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

Status CoapTransport::sendPdu(coap_session_t* session, coap_pdu_t* pdu, coap_optlist_t** optList, const uint8_t* data,
                              size_t dataLen)
{
    Status status = STS_OK;
    coap_tid_t tid = COAP_INVALID_TID;

    if (!pdu) {
        status = STS_ERR_NO_MEM;
        goto err;
    }

    if (*optList && !coap_add_optlist_pdu(pdu, optList)) {
        status = STS_ERR_NO_MEM;
        goto err;
    }

    if (data && !coap_add_data(pdu, dataLen, data)) {
        status = STS_ERR_NO_MEM;
        goto err;
    }

    coap_show_pdu(LOG_DEBUG, pdu);

    if ((tid = coap_send(session, pdu)) == COAP_INVALID_TID) {
        return STS_ERR;
    }

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
    LOG_DEBUG("Response received, tid: %d", id);

    Request* request = mReqStorage.getItemById(id);

    if (request) {
        switch (request->mParam.type) {
            case Request::REGISTRATION:
                registrationResponse(received, request);
                break;

            default:
                request->mParam.handler(request->mParam.context, NULL, code2Status(received->code));
                break;
        }

        mReqStorage.deleteItem(request);
    }
    else {
        LOG_ERROR("Unknown response received, id: %d", id);
    }
}

void CoapTransport::nackHandler(coap_context_t* context, coap_session_t* session, coap_pdu_t* sent,
                                coap_nack_reason_t reason, const coap_tid_t id)
{
    static_cast<CoapTransport*>(context->app)->onNack(session, sent, reason, id);
}

void CoapTransport::onNack(coap_session_t* session, coap_pdu_t* sent, coap_nack_reason_t reason, const coap_tid_t id)
{
    LOG_DEBUG("Nack received, tid: %d", id);

    Request* request = mReqStorage.getItemById(id);

    if (request) {
        request->mParam.handler(request->mParam.context, NULL, STS_ERR_TIMEOUT);
        mReqStorage.deleteItem(request);
    }
    else {
        LOG_ERROR("Unknown Nack received, tid: %d", id);
    }

    coap_session_connected(session);
}

Status CoapTransport::code2Status(uint8_t code)
{
    if (COAP_RESPONSE_CLASS(code) == 2) {
        return STS_OK;
    }

    switch (code) {
        case COAP_RESPONSE_CODE(400):
            return STS_ERR_INVALID_VALUE;

        case COAP_RESPONSE_CODE(403):
            return STS_ERR_NO_ACCESS;

        case COAP_RESPONSE_CODE(404):
            return STS_ERR_NOT_EXIST;

        case COAP_RESPONSE_CODE(412):
            return STS_ERR_INVALID_STATE;

        default:
            return STS_ERR;
    }
}

void CoapTransport::insertLocation(const char* location, coap_optlist_t** optList)
{
    char* begin = const_cast<char*>(location);
    char* current = begin;

    while (true) {
        if (*current == '/' || *current == '\0') {
            if (current - begin > 0) {
                coap_insert_optlist(optList, coap_new_optlist(COAP_OPTION_URI_PATH, current - begin,
                                                              reinterpret_cast<const uint8_t*>(begin)));
            }

            begin = current + 1;
        }

        if (*current == '\0') {
            break;
        }

        current++;
    }
}

void CoapTransport::registrationResponse(coap_pdu_t* received, Request* request)
{
    char location[CONFIG_DEFAULT_STRING_LEN + 1];
    coap_opt_iterator_t optIter;
    int size = 0;

    coap_opt_t* option = coap_check_option(received, COAP_OPTION_LOCATION_PATH, &optIter);

    while (option) {
        location[size++] = '/';

        if (size == CONFIG_DEFAULT_STRING_LEN || size + coap_opt_length(option) > CONFIG_DEFAULT_STRING_LEN) {
            break;
        }

        strncpy(&location[size], reinterpret_cast<const char*>(coap_opt_value(option)), coap_opt_length(option));

        size += coap_opt_length(option);

        option = coap_option_next(&optIter);
    }

    location[size] = '\0';

    request->mParam.handler(request->mParam.context, location, code2Status(received->code));
}