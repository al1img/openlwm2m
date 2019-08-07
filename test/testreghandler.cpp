#include <catch2/catch.hpp>

#include <cstring>

#include "jsonconverter.hpp"
#include "objectmanager.hpp"
#include "reghandler.hpp"

using namespace openlwm2m;

class TestSession {
public:
    TestSession(const char* uri) : mUri(uri) {}

    std::string getUri() const { return mUri; }

    void registrationRequest(int64_t lifetime, const char* bindingMode, const char* smsNumber, const char* objects,
                             TransportItf::RequestHandler handler, void* context)
    {
        mLifetime = lifetime;
        mBindingMode = bindingMode;
        mObjects = objects;

        if (handler) {
            handler(context, mStatus);
        }
    }

    void registrationUpdate(const int64_t* lifetime, const char* bindingMode, const char* smsNumber,
                            const char* objects, TransportItf::RequestHandler handler, void* context)
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
            handler(context, mStatus);
        }
    }

    void deregistrationRequest(TransportItf::RequestHandler handler, void* context)
    {
        if (handler) {
            handler(context, mStatus);
        }
    }

    void setStatus(Status status) { mStatus = status; }

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
    Status mStatus;

    int64_t mLifetime;
    std::string mBindingMode;
    std::string mObjects;

    bool mUpdateReceived;
};

class TestTransport : public TransportItf {
public:
    TestTransport() : mLastSession(NULL) {}

    void* createSession(const char* uri, Status* status = NULL)
    {
        mLastSession = new TestSession(uri);
        return mLastSession;
    }

    Status deleteSession(void* session)
    {
        delete static_cast<TestSession*>(session);
        return STS_OK;
    }

    void bootstrapRequest(void* session, RequestHandler handler, void* context) {}

    Status registrationRequest(void* session, const char* clientName, int64_t lifetime, const char* version,
                               const char* bindingMode, bool queueMode, const char* smsNumber, const char* objects,
                               RequestHandler handler, void* context)
    {
        static_cast<TestSession*>(session)->registrationRequest(lifetime, bindingMode, smsNumber, objects, handler,
                                                                context);

        return STS_OK;
    }

    Status registrationUpdate(void* session, const int64_t* lifetime, const char* bindingMode, const char* smsNumber,
                              const char* objects, RequestHandler handler, void* context)
    {
        static_cast<TestSession*>(session)->registrationUpdate(lifetime, bindingMode, smsNumber, objects, handler,
                                                               context);
        return STS_OK;
    }

    Status deregistrationRequest(void* session, RequestHandler handler, void* context)
    {
        static_cast<TestSession*>(session)->deregistrationRequest(handler, context);

        return STS_OK;
    }

    void deviceSend(void* session, RequestHandler handler, void* context) {}

    void reportingNotify(void* session, RequestHandler handler, void* context) {}

    TestSession* getLastSession() const { return mLastSession; }

private:
    TestSession* mLastSession;
};

void pollRequest()
{
}

void setupObjects(ObjectManager* objectManager)
{
    Status status = STS_OK;

    const char* security1 =
        "\
[\
{\"bn\":\"/0/0/\",\"n\":\"0\",\"vs\":\"coap://test\"},\
{\"n\":\"10\",\"v\":1}\
]\
";

    status = objectManager->write(ITF_BOOTSTRAP, DATA_FMT_SENML_JSON, "/0",
                                  reinterpret_cast<void*>(const_cast<char*>(security1)), strlen(security1));
    REQUIRE(status == STS_OK);

    const char* server1 =
        "\
[\
{\"bn\":\"/1/0/\",\"n\":\"0\",\"v\":1},\
{\"n\":\"1\",\"v\":30},\
{\"n\":\"7\",\"vs\":\"U\"}\
]\
";

    status = objectManager->write(ITF_BOOTSTRAP, DATA_FMT_SENML_JSON, "/1",
                                  reinterpret_cast<void*>(const_cast<char*>(server1)), strlen(server1));
    REQUIRE(status == STS_OK);
}

TEST_CASE("test reghandler", "[reghandler]")
{
    Status status = STS_OK;
    ObjectManager objectManager;
    TestTransport transport;

    objectManager.addConverter(new JsonConverter());

    RegHandler regHandler(NULL, (RegHandler::Params){objectManager, "TestClient", false, pollRequest});

    objectManager.init();

    setupObjects(&objectManager);

    SECTION("Success registration")
    {
        regHandler.setId(1);

        regHandler.init();

        status = regHandler.bind(&transport);
        REQUIRE(status == STS_OK);

        TestSession* session = transport.getLastSession();
        REQUIRE(session);
        CHECK(session->getUri() == "coap://test");

        session->setStatus(STS_OK);

        status = regHandler.registration([](void* context, Status status) { REQUIRE(status == STS_OK); });
        REQUIRE(status == STS_OK);

        status = Timer::poll(0);
        REQUIRE(status == STS_OK);

        CHECK(session->getLifetime() == 30);
        CHECK(session->getBindingMode() == "U");
        CHECK(session->getObjects() == "<1/0>,<3/0>");

        status = Timer::poll(0);
        REQUIRE(status == STS_OK);

        status = Timer::poll(20000);
        REQUIRE(status == STS_OK);

        CHECK_FALSE(session->getUpdateReceived());

        status = Timer::poll(30000);
        REQUIRE(status == STS_OK);

        status = Timer::poll(30000);
        REQUIRE(status == STS_OK);

        CHECK(session->getUpdateReceived());

        status = Timer::poll(60000);
        REQUIRE(status == STS_OK);

        status = Timer::poll(60000);
        REQUIRE(status == STS_OK);

        CHECK(session->getUpdateReceived());

        status = regHandler.deregistration([](void* context, Status status) { REQUIRE(status == STS_OK); });
        REQUIRE(status == STS_OK);

        CHECK(regHandler.getState() == RegHandler::STATE_DEREGISTERED);
    }

    regHandler.release();
}
