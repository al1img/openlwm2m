#ifndef COAPTRANSPORT_HPP_
#define COAPTRANSPORT_HPP_

#include <coap2/coap.h>

#include "interface.hpp"
#include "itembase.hpp"
#include "storage.hpp"

class CoapTransport : public openlwm2m::TransportItf {
public:
    CoapTransport();
    ~CoapTransport();

    void setClient(openlwm2m::ClientItf* client) { mClient = client; }

    void* createSession(const char* uri, openlwm2m::Status* status = NULL);
    openlwm2m::Status deleteSession(void* session);

    // Bootstrap
    openlwm2m::Status bootstrapRequest(void* session, const char* clientName, openlwm2m::DataFormat* preferredFormat,
                                       RequestHandler handler, void* context);

    // Registration
    openlwm2m::Status registrationRequest(void* session, const char* clientName, int64_t lifetime, const char* version,
                                          const char* bindingMode, bool queueMode, const char* smsNumber,
                                          const char* objects, RequestHandler handler, void* context);
    openlwm2m::Status registrationUpdate(void* session, const char* location, const int64_t* lifetime,
                                         const char* bindingMode, const char* smsNumber, const char* objects,
                                         RequestHandler handler, void* context);
    openlwm2m::Status deregistrationRequest(void* session, const char* location, RequestHandler handler, void* context);

    // Device
    void deviceSend(void* session, RequestHandler handler, void* context);

    // Reporting
    void reportingNotify(void* session, RequestHandler handler, void* context);

    void run();

private:
    static const int sReqStorageSize = 16;
    static const int sDataSize = 256;

    class Request : public openlwm2m::ItemBase {
    public:
        enum RequestType { REGISTRATION, UPDATE, DEREGISTRATION };

        Request() : openlwm2m::ItemBase(NULL) {}

        void init() {}
        void release() {}

        void set(RequestType type, RequestHandler handler, void* context)
        {
            mType = type;
            mHandler = handler;
            mContext = context;
        }

        RequestType getType() const { return mType; }
        RequestHandler getHandler() const { return mHandler; }
        void* getContext() const { return mContext; }

    private:
        RequestType mType;
        RequestHandler mHandler;
        void* mContext;
    };

    openlwm2m::ClientItf* mClient;

    coap_context_t* mContext;

    openlwm2m::Lwm2mDynamicStorage<Request> mReqStorage;

    openlwm2m::Status resolveAddress(const char* uri, coap_address_t* dst);

    openlwm2m::Status sendPdu(coap_session_t* session, coap_pdu_t* pdu, coap_optlist_t** optList, const uint8_t* data,
                              size_t dataLen);

    static void responseHandler(coap_context_t* context, coap_session_t* session, coap_pdu_t* sent,
                                coap_pdu_t* received, const coap_tid_t id);

    void onResponse(coap_session_t* session, coap_pdu_t* sent, coap_pdu_t* received, const coap_tid_t id);

    static void nackHandler(coap_context_t* context, coap_session_t* session, coap_pdu_t* sent,
                            coap_nack_reason_t reason, const coap_tid_t id);

    void onNack(coap_session_t* session, coap_pdu_t* sent, coap_nack_reason_t reason, const coap_tid_t id);

    static void putReceived(coap_context_t* context, coap_resource_t* resource, coap_session_t* session,
                            coap_pdu_t* request, coap_binary_t* token, coap_string_t* query, coap_pdu_t* response);

    void onPutReceived(coap_resource_t* resource, coap_session_t* session, coap_pdu_t* request, coap_binary_t* token,
                       coap_string_t* query, coap_pdu_t* response);

    static void getReceived(coap_context_t* context, coap_resource_t* resource, coap_session_t* session,
                            coap_pdu_t* request, coap_binary_t* token, coap_string_t* query, coap_pdu_t* response);

    void onGetReceived(coap_resource_t* resource, coap_session_t* session, coap_pdu_t* request, coap_binary_t* token,
                       coap_string_t* query, coap_pdu_t* response);

    openlwm2m::Status code2Status(uint8_t code);
    uint8_t status2Code(openlwm2m::Status status);

    openlwm2m::Status getLocationPath(coap_pdu_t* pdu, char* location, size_t size);
    openlwm2m::Status getUriPath(coap_pdu_t* pdu, char* location, size_t size);
    openlwm2m::Status getAccept(coap_pdu_t* pdu, openlwm2m::DataFormat* data);

    void insertLocation(const char* location, coap_optlist_t** optList);

    void registrationResponse(coap_pdu_t* received, Request* request);
};

#endif /* COAPTRANSPORT_HPP_ */
