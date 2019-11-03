#ifndef OPENLWM2M_TESTTRANSPORT_HPP_
#define OPENLWM2M_TESTTRANSPORT_HPP_

#include "interface.hpp"

class TestSession {
public:
    TestSession(const char* uri) : mUri(uri), mStatus(openlwm2m::STS_OK) {}

    std::string getUri() const { return mUri; }

    void registrationRequest(int64_t lifetime, const char* bindingMode, const char* smsNumber, const char* objects,
                             openlwm2m::TransportItf::RequestHandler handler, void* context)
    {
        mLifetime = lifetime;
        mBindingMode = bindingMode;
        mObjects = objects;

        if (handler) {
            handler(context, const_cast<char*>("/rd/0"), mStatus);
        }
    }

    void registrationUpdate(const int64_t* lifetime, const char* bindingMode, const char* smsNumber,
                            const char* objects, openlwm2m::TransportItf::RequestHandler handler, void* context)
    {
        if (lifetime) {
            mLifetime = *lifetime;
        }

        if (bindingMode) {
            mBindingMode = bindingMode;
        }

        if (objects) {
            mObjects = objects;
        }

        mUpdateReceived = true;

        if (handler) {
            handler(context, NULL, mStatus);
        }
    }

    void deregistrationRequest(openlwm2m::TransportItf::RequestHandler handler, void* context)
    {
        if (handler) {
            handler(context, NULL, mStatus);
        }
    }

    void bootstrapRequest(const char* clientName, openlwm2m::DataFormat* preferredFormat,
                          openlwm2m::TransportItf::RequestHandler handler, void* context)
    {
        if (handler) {
            handler(context, NULL, mStatus);
        }
    }

    void setStatus(openlwm2m::Status status) { mStatus = status; }

    int64_t getLifetime() const { return mLifetime; }
    std::string getBindingMode() const { return mBindingMode; }
    std::string getObjects() const { return mObjects; }

    bool getUpdateReceived()
    {
        bool updateReceived = mUpdateReceived;

        mUpdateReceived = false;

        return updateReceived;
    }

private:
    std::string mUri;
    openlwm2m::Status mStatus;

    int64_t mLifetime;
    std::string mBindingMode;
    std::string mObjects;

    bool mUpdateReceived;
};

class TestTransport : public openlwm2m::TransportItf {
public:
    TestTransport() : mLastSession(NULL) {}

    void setClient(openlwm2m::ClientItf* client) {}

    void* createSession(const char* uri, openlwm2m::Status* status = NULL)
    {
        mLastSession = new TestSession(uri);
        return mLastSession;
    }

    openlwm2m::Status deleteSession(void* session)
    {
        delete static_cast<TestSession*>(session);
        return openlwm2m::STS_OK;
    }

    openlwm2m::Status bootstrapRequest(void* session, const char* clientName, openlwm2m::DataFormat* preferredFormat,
                                       RequestHandler handler, void* context)
    {
        static_cast<TestSession*>(session)->bootstrapRequest(clientName, preferredFormat, handler, context);

        return openlwm2m::STS_OK;
    }

    openlwm2m::Status registrationRequest(void* session, const char* clientName, int64_t lifetime, const char* version,
                                          const char* bindingMode, bool queueMode, const char* smsNumber,
                                          const char* objects, RequestHandler handler, void* context)
    {
        static_cast<TestSession*>(session)->registrationRequest(lifetime, bindingMode, smsNumber, objects, handler,
                                                                context);

        return openlwm2m::STS_OK;
    }

    openlwm2m::Status registrationUpdate(void* session, const char* location, const int64_t* lifetime,
                                         const char* bindingMode, const char* smsNumber, const char* objects,
                                         RequestHandler handler, void* context)
    {
        static_cast<TestSession*>(session)->registrationUpdate(lifetime, bindingMode, smsNumber, objects, handler,
                                                               context);
        return openlwm2m::STS_OK;
    }

    openlwm2m::Status deregistrationRequest(void* session, const char* location, RequestHandler handler, void* context)
    {
        static_cast<TestSession*>(session)->deregistrationRequest(handler, context);

        return openlwm2m::STS_OK;
    }

    void deviceSend(void* session, RequestHandler handler, void* context) {}

    void reportingNotify(void* session, RequestHandler handler, void* context) {}

    TestSession* getLastSession() const { return mLastSession; }

private:
    TestSession* mLastSession;
};

#endif /* OPENLWM2M_TESTTRANSPORT_HPP_ */
