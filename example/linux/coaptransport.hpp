#ifndef COAPTRANSPORT_HPP_
#define COAPTRANSPORT_HPP_

#include <coap2/coap.h>

#include "interface.hpp"
#include "itembase.hpp"
#include "storage.hpp"

#define REQ_STORAGE_SIZE 16

class CoapTransport : public openlwm2m::TransportItf {
public:
    CoapTransport();
    ~CoapTransport();

    void* createSession(const char* uri, openlwm2m::Status* status = NULL);
    openlwm2m::Status deleteSession(void* session);

    // Bootstrap
    void bootstrapRequest(void* session, RequestHandler handler, void* context);

    // Registration
    void registrationRequest(void* session, const char* clientName, uint64_t lifetime, const char* version,
                             const char* bindingMode, bool queueMode, const char* smsNumber, const char* objects,
                             RequestHandler handler, void* context);
    void registrationUpdate(void* session, const uint32_t* lifetime, const char* bindingMode, const char* smsNumber,
                            const char* objects, RequestHandler handler, void* context);
    void registrationDeregister(void* session, RequestHandler handler, void* context);

    // Device
    void deviceSend(void* session, RequestHandler handler, void* context);

    // Reporting
    void reportingNotify(void* session, RequestHandler handler, void* context);

    uint64_t run(uint64_t timeout);

private:
    struct Request : public openlwm2m::ItemBase {
        struct Param {
            RequestHandler handler;
            void* context;
        };

        Request(ItemBase* parent, Param param) : openlwm2m::ItemBase(parent), mParam(param) {}

        void init() {}
        void release() {}

        Param mParam;
    };

    coap_context_t* mContext;

    openlwm2m::Lwm2mDynamicStorage<Request, Request::Param> mReqStorage;

    openlwm2m::Status resolveAddress(const char* uri, coap_address_t* dst);

    static void responseHandler(coap_context_t* context, coap_session_t* session, coap_pdu_t* sent,
                                coap_pdu_t* received, const coap_tid_t id);

    void onResponse(coap_session_t* session, coap_pdu_t* sent, coap_pdu_t* received, const coap_tid_t id);

    static void nackHandler(coap_context_t* context, coap_session_t* session, coap_pdu_t* sent,
                            coap_nack_reason_t reason, const coap_tid_t id);

    void onNack(coap_session_t* session, coap_pdu_t* sent, coap_nack_reason_t reason, const coap_tid_t id);
};

#endif /* COAPTRANSPORT_HPP_ */