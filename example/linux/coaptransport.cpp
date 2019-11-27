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

CoapTransport::CoapTransport() : mClient(NULL)
{
    coap_startup();

    mContext = coap_new_context(NULL);

    mContext->app = this;

    coap_set_log_level(LOG_DEBUG);

    coap_resource_t* resource = coap_resource_unknown_init(&CoapTransport::putReceived);
    coap_register_handler(resource, COAP_REQUEST_GET, &CoapTransport::getReceived);
    coap_add_resource(mContext, resource);

    coap_register_response_handler(mContext, &CoapTransport::responseHandler);
    coap_register_nack_handler(mContext, &CoapTransport::nackHandler);

    for (size_t i = 0; i < sReqStorageSize; i++) {
        mReqStorage.pushItem(new Request());
    }
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
Status CoapTransport::bootstrapRequest(void* session, const char* clientName, openlwm2m::DataFormat* preferredFormat,
                                       RequestHandler handler, void* context)
{
    return STS_OK;
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
        if (ret < 0 || static_cast<size_t>(ret) >= sizeof(buf)) {
            return STS_ERR_NO_MEM;
        }

        coap_insert_optlist(&optList,
                            coap_new_optlist(COAP_OPTION_URI_QUERY, ret, reinterpret_cast<const uint8_t*>(buf)));
    }

    // Version
    ret = snprintf(buf, sizeof(buf), "lwm2m=%s", version);
    if (ret < 0 || static_cast<size_t>(ret) >= sizeof(buf)) {
        return STS_ERR_NO_MEM;
    }

    coap_insert_optlist(&optList, coap_new_optlist(COAP_OPTION_URI_QUERY, ret, reinterpret_cast<const uint8_t*>(buf)));

    // Lifetime
    ret = snprintf(buf, sizeof(buf), "lt=%ld", lifetime);
    if (ret < 0 || static_cast<size_t>(ret) >= sizeof(buf)) {
        return STS_ERR_NO_MEM;
    }

    coap_insert_optlist(&optList, coap_new_optlist(COAP_OPTION_URI_QUERY, ret, reinterpret_cast<const uint8_t*>(buf)));

    // Binding
    ret = snprintf(buf, sizeof(buf), "b=%s", bindingMode);
    if (ret < 0 || static_cast<size_t>(ret) >= sizeof(buf)) {
        return STS_ERR_NO_MEM;
    }

    coap_insert_optlist(&optList, coap_new_optlist(COAP_OPTION_URI_QUERY, ret, reinterpret_cast<const uint8_t*>(buf)));

    // Queue mode
    if (queueMode) {
        coap_insert_optlist(&optList,
                            coap_new_optlist(COAP_OPTION_URI_QUERY, 1, reinterpret_cast<const uint8_t*>("Q")));
    }

    // SMS number
    if (smsNumber) {
        ret = snprintf(buf, sizeof(buf), "sms=%s", smsNumber);
        if (ret < 0 || static_cast<size_t>(ret) >= sizeof(buf)) {
            return STS_ERR_NO_MEM;
        }

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

    Request* request = mReqStorage.allocateItem(pdu->tid);

    if (!request) {
        return STS_ERR_NO_MEM;
    }

    request->set(Request::REGISTRATION, handler, context);

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
        if (ret < 0 || static_cast<size_t>(ret) >= sizeof(buf)) {
            return STS_ERR_NO_MEM;
        }

        coap_insert_optlist(&optList,
                            coap_new_optlist(COAP_OPTION_URI_QUERY, ret, reinterpret_cast<const uint8_t*>(buf)));
    }

    // Binding
    if (bindingMode) {
        ret = snprintf(buf, sizeof(buf), "b=%s", bindingMode);
        if (ret < 0 || static_cast<size_t>(ret) >= sizeof(buf)) {
            return STS_ERR_NO_MEM;
        }

        coap_insert_optlist(&optList,
                            coap_new_optlist(COAP_OPTION_URI_QUERY, ret, reinterpret_cast<const uint8_t*>(buf)));
    }

    // SMS number
    if (smsNumber) {
        ret = snprintf(buf, sizeof(buf), "sms=%s", smsNumber);
        if (ret < 0 || static_cast<size_t>(ret) >= sizeof(buf)) {
            return STS_ERR_NO_MEM;
        }

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

    Request* request = mReqStorage.allocateItem(pdu->tid);

    if (!request) {
        return STS_ERR_NO_MEM;
    }

    request->set(Request::REGISTRATION, handler, context);

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

    return STS_ERR_NOT_FOUND;
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
        switch (request->getType()) {
            case Request::REGISTRATION:
                registrationResponse(received, request);
                break;

            default:
                request->getHandler()(request->getContext(), NULL, code2Status(received->code));
                break;
        }

        mReqStorage.deallocateItem(request);
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
        request->getHandler()(request->getContext(), NULL, STS_ERR_TIMEOUT);
        mReqStorage.deallocateItem(request);
    }
    else {
        LOG_ERROR("Unknown Nack received, tid: %d", id);
    }

    coap_session_connected(session);
}

void CoapTransport::getReceived(coap_context_t* context, coap_resource_t* resource, coap_session_t* session,
                                coap_pdu_t* request, coap_binary_t* token, coap_string_t* query, coap_pdu_t* response)
{
    static_cast<CoapTransport*>(context->app)->onGetReceived(resource, session, request, token, query, response);
}

void CoapTransport::onGetReceived(coap_resource_t* resource, coap_session_t* session, coap_pdu_t* request,
                                  coap_binary_t* token, coap_string_t* query, coap_pdu_t* response)
{
    char uri[CONFIG_DEFAULT_STRING_LEN + 1];
    uint8_t data[sDataSize];
    DataFormat reqFormat = DATA_FMT_ANY, outFormat = DATA_FMT_TEXT;
    Status status = STS_OK;

    response->code = COAP_RESPONSE_CODE(205);

    if ((status = getAccept(request, &reqFormat)) != STS_OK) {
        response->code = status2Code(status);
    }
    else if ((status = getUriPath(request, uri, sizeof(uri))) != STS_OK) {
        response->code = status2Code(status);
    }

    LOG_DEBUG("GET received, uri: %s, format: %d", uri, reqFormat);

    size_t size = sDataSize;

    if (mClient && status == STS_OK) {
        if (reqFormat != DATA_FMT_CORE) {
            if ((status = mClient->deviceRead(uri, reqFormat, data, &size, &outFormat)) != STS_OK) {
                response->code = status2Code(status);
                size = 0;
            }
        }
        else {
            // TODO: discover
        }
    }

    coap_add_option(response, COAP_OPTION_CONTENT_TYPE,
                    coap_encode_var_safe(reinterpret_cast<uint8_t*>(uri), sizeof(uri), outFormat),
                    reinterpret_cast<uint8_t*>(uri));

    coap_add_data(response, size, data);

    //    coap_add_data_blocked_response(resource, session, request, response, token, outFormat, -1, 0, NULL);
}

void CoapTransport::putReceived(coap_context_t* context, coap_resource_t* resource, coap_session_t* session,
                                coap_pdu_t* request, coap_binary_t* token, coap_string_t* query, coap_pdu_t* response)
{
    static_cast<CoapTransport*>(context->app)->onPutReceived(resource, session, request, token, query, response);
}

void CoapTransport::onPutReceived(coap_resource_t* resource, coap_session_t* session, coap_pdu_t* request,
                                  coap_binary_t* token, coap_string_t* query, coap_pdu_t* response)
{
    response->code = COAP_RESPONSE_CODE(205);

    coap_add_data_blocked_response(resource, session, request, response, token, COAP_MEDIATYPE_TEXT_PLAIN, -1, 0, NULL);
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
            return STS_ERR_NOT_FOUND;

        case COAP_RESPONSE_CODE(412):
            return STS_ERR_NOT_ALLOWED;

        default:
            return STS_ERR;
    }
}

uint8_t CoapTransport::status2Code(Status status)
{
    switch (status) {
        case STS_ERR_NO_ACCESS:
            return COAP_RESPONSE_CODE(401);

        case STS_ERR_NOT_FOUND:
            return COAP_RESPONSE_CODE(404);

        case STS_ERR_NOT_ALLOWED:
            return COAP_RESPONSE_CODE(405);

        case STS_ERR_INVALID_VALUE:
            return COAP_RESPONSE_CODE(406);

        case STS_ERR_NO_MEM:
            return COAP_RESPONSE_CODE(413);

        case STS_ERR_FORMAT:
            return COAP_RESPONSE_CODE(415);

        default:
            return COAP_RESPONSE_CODE(400);
    }
}

Status CoapTransport::getAccept(coap_pdu_t* pdu, DataFormat* data)
{
    coap_opt_iterator_t optIter;
    coap_opt_t* option = coap_check_option(pdu, COAP_OPTION_ACCEPT, &optIter);

    *data = DATA_FMT_ANY;

    if (option) {
        switch (coap_opt_length(option)) {
            case 1:
                *data = static_cast<DataFormat>(*coap_opt_value(option));
                break;

            case 2:
                *data =
                    static_cast<DataFormat>(*reinterpret_cast<uint16_t*>(const_cast<uint8_t*>(coap_opt_value(option))));
                break;

            default:
                return STS_ERR_INVALID_VALUE;
        }
    }

    return STS_OK;
}

Status CoapTransport::getLocationPath(coap_pdu_t* pdu, char* location, size_t size)
{
    size_t curSize = 0;
    coap_opt_iterator_t optIter;
    coap_opt_t* option = coap_check_option(pdu, COAP_OPTION_LOCATION_PATH, &optIter);

    location[curSize] = '\0';

    while (option) {
        location[curSize++] = '/';

        if (curSize == size || curSize + coap_opt_length(option) + 1 > size) {
            return STS_ERR_NO_MEM;
        }

        strncpy(&location[curSize], reinterpret_cast<const char*>(coap_opt_value(option)), coap_opt_length(option));

        curSize += coap_opt_length(option);

        option = coap_option_next(&optIter);
    }

    location[curSize] = '\0';

    return STS_OK;
}

Status CoapTransport::getUriPath(coap_pdu_t* pdu, char* uri, size_t size)
{
    size_t curSize = 0;
    coap_opt_iterator_t optIter;
    coap_opt_t* option = coap_check_option(pdu, COAP_OPTION_URI_PATH, &optIter);

    uri[curSize] = '\0';

    while (option) {
        uri[curSize++] = '/';

        if (curSize == size || curSize + coap_opt_length(option) + 1 > size) {
            return STS_ERR_NO_MEM;
        }

        strncpy(&uri[curSize], reinterpret_cast<const char*>(coap_opt_value(option)), coap_opt_length(option));

        curSize += coap_opt_length(option);

        option = coap_option_next(&optIter);
    }

    uri[curSize] = '\0';

    return STS_OK;
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
    Status status = code2Status(received->code);
    char location[CONFIG_DEFAULT_STRING_LEN + 1];

    if (status == STS_OK) {
        status = getLocationPath(received, location, sizeof(location));
    }

    request->getHandler()(request->getContext(), location, status);
}
