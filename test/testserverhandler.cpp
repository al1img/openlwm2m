#include <catch2/catch.hpp>

#include <cstring>

#include "jsonconverter.hpp"
#include "objectmanager.hpp"
#include "serverhandler.hpp"

#include "testplatform.hpp"

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
            handler(context, const_cast<char*>("/rd/0"), mStatus);
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
            handler(context, NULL, mStatus);
        }
    }

    void deregistrationRequest(TransportItf::RequestHandler handler, void* context)
    {
        if (handler) {
            handler(context, NULL, mStatus);
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

    void setClient(ClientItf* client) {}

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

    Status registrationUpdate(void* session, const char* location, const int64_t* lifetime, const char* bindingMode,
                              const char* smsNumber, const char* objects, RequestHandler handler, void* context)
    {
        static_cast<TestSession*>(session)->registrationUpdate(lifetime, bindingMode, smsNumber, objects, handler,
                                                               context);
        return STS_OK;
    }

    Status deregistrationRequest(void* session, const char* location, RequestHandler handler, void* context)
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

static void run(uint64_t currentTimeMs)
{
    setCurrentTime(currentTimeMs);
    REQUIRE(Timer::run() == STS_OK);
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

    status = objectManager->bootstrapWrite(DATA_FMT_SENML_JSON, reinterpret_cast<void*>(const_cast<char*>(security1)),
                                           strlen(security1), 0);
    REQUIRE(status == STS_OK);

    const char* server1 =
        "\
[\
{\"bn\":\"/1/0/\",\"n\":\"0\",\"v\":1},\
{\"n\":\"1\",\"v\":30},\
{\"n\":\"7\",\"vs\":\"U\"},\
{\"n\":\"19\",\"v\":60},\
{\"n\":\"20\",\"v\":3}\
]\
";

    status = objectManager->bootstrapWrite(DATA_FMT_SENML_JSON, reinterpret_cast<void*>(const_cast<char*>(server1)),
                                           strlen(server1), 1);
    REQUIRE(status == STS_OK);
}

TEST_CASE("test ServerHandler", "[ServerHandler]")
{
    Status status = STS_OK;
    ObjectManager objectManager;
    TestTransport transport;

    ServerHandler serverHandler("TestClient", false, objectManager);

    objectManager.init();

    setupObjects(&objectManager);

    SECTION("Success registration")
    {
        run(0);

        serverHandler.setId(1);

        serverHandler.init();

        status = serverHandler.bind(&transport);
        REQUIRE(status == STS_OK);

        TestSession* session = transport.getLastSession();
        REQUIRE(session);
        CHECK(session->getUri() == "coap://test");

        session->setStatus(STS_OK);

        status = serverHandler.registration();
        REQUIRE(status == STS_OK);

        run(0);

        CHECK(session->getLifetime() == 30);
        CHECK(session->getBindingMode() == "U");
        CHECK(session->getObjects() == "</>;rt=\"oma.lwm2m\";ct=110,<1/0>,<3/0>");

        run(10000);

        CHECK_FALSE(session->getUpdateReceived());

        run(30000);

        CHECK(session->getUpdateReceived());

        run(60000);

        CHECK(session->getUpdateReceived());

        // Check reregistration

        session->setStatus(STS_ERR);

        run(90000);

        session->setStatus(STS_OK);

        run(100000);

        run(160000);

        status = serverHandler.deregistration(
            [](void* context, ServerHandler* handler, Status status) { REQUIRE(status == STS_OK); });
        REQUIRE(status == STS_OK);

        CHECK(serverHandler.getState() == ServerHandler::STATE_DEREGISTERED);
    }

    SECTION("Failed unordered registration")
    {
        run(0);

        serverHandler.setId(1);

        serverHandler.init();

        status = serverHandler.bind(&transport);
        REQUIRE(status == STS_OK);

        TestSession* session = transport.getLastSession();
        REQUIRE(session);
        CHECK(session->getUri() == "coap://test");

        session->setStatus(STS_ERR);

        Status registrationStatus = STS_OK;

        auto handler = [&registrationStatus](void* context, Status status) { registrationStatus = status; };

        status = serverHandler.registration(true,
                                            [](void* context, ServerHandler* handle, Status status) {
                                                (*static_cast<decltype(handler)*>(context))(context, status);
                                            },
                                            &handler);

        REQUIRE(status == STS_OK);

        run(0);

        CHECK_FALSE(registrationStatus == STS_OK);

        CHECK(serverHandler.getState() == ServerHandler::STATE_DEREGISTERED);
    }

    SECTION("Failed registration with retry")
    {
        run(0);

        serverHandler.setId(1);

        serverHandler.init();

        status = serverHandler.bind(&transport);
        REQUIRE(status == STS_OK);

        TestSession* session = transport.getLastSession();
        REQUIRE(session);
        CHECK(session->getUri() == "coap://test");

        session->setStatus(STS_ERR);

        Status registrationStatus = STS_OK;

        auto inner = [&registrationStatus](void* context, ServerHandler* handler, Status status) {
            registrationStatus = status;
        };

        status = serverHandler.registration(false,
                                            [](void* context, ServerHandler* handler, Status status) {
                                                (*static_cast<decltype(inner)*>(context))(context, handler, status);
                                            },
                                            &inner);

        REQUIRE(status == STS_OK);

        run(0);

        run(60000);

        CHECK(registrationStatus == STS_OK);

        run(120000);

        CHECK(registrationStatus == STS_OK);

        run(180000);

        CHECK_FALSE(registrationStatus == STS_OK);

        CHECK(serverHandler.getState() == ServerHandler::STATE_DEREGISTERED);
    }

    serverHandler.release();
}
