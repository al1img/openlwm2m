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

bool sPoolRequested = false;

void pollRequest()
{
    sPoolRequested = true;
}

void setCurrentTime(uint64_t time)
{
    REQUIRE(Timer::poll(time) == STS_OK);
    if (sPoolRequested) {
        REQUIRE(Timer::poll(time) == STS_OK);
        sPoolRequested = false;
    }
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
{\"n\":\"7\",\"vs\":\"U\"},\
{\"n\":\"19\",\"v\":60},\
{\"n\":\"20\",\"v\":3}\
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

        status = regHandler.registration(false);
        REQUIRE(status == STS_OK);

        setCurrentTime(0);

        CHECK(session->getLifetime() == 30);
        CHECK(session->getBindingMode() == "U");
        CHECK(session->getObjects() == "<1/0>,<3/0>");

        setCurrentTime(20000);

        CHECK_FALSE(session->getUpdateReceived());

        setCurrentTime(30000);

        CHECK(session->getUpdateReceived());

        setCurrentTime(60000);

        CHECK(session->getUpdateReceived());

        // Check reregistration

        session->setStatus(STS_ERR);

        setCurrentTime(90000);

        session->setStatus(STS_OK);

        setCurrentTime(100000);

        setCurrentTime(160000);

        status = regHandler.deregistration([](void* context, Status status) { REQUIRE(status == STS_OK); });
        REQUIRE(status == STS_OK);

        CHECK(regHandler.getState() == RegHandler::STATE_DEREGISTERED);
    }

    SECTION("Failed registration without retry")
    {
        regHandler.setId(1);

        regHandler.init();

        status = regHandler.bind(&transport);
        REQUIRE(status == STS_OK);

        TestSession* session = transport.getLastSession();
        REQUIRE(session);
        CHECK(session->getUri() == "coap://test");

        session->setStatus(STS_ERR);

        Status registrationStatus = STS_OK;

        auto handler = [&registrationStatus](void* context, Status status) { registrationStatus = status; };

        status = regHandler.registration(
            false, [](void* context, Status status) { (*static_cast<decltype(handler)*>(context))(context, status); },
            &handler);

        REQUIRE(status == STS_OK);

        setCurrentTime(0);

        CHECK_FALSE(registrationStatus == STS_OK);

        CHECK(regHandler.getState() == RegHandler::STATE_DEREGISTERED);
    }

    SECTION("Failed registration with retry")
    {
        regHandler.setId(1);

        regHandler.init();

        status = regHandler.bind(&transport);
        REQUIRE(status == STS_OK);

        TestSession* session = transport.getLastSession();
        REQUIRE(session);
        CHECK(session->getUri() == "coap://test");

        session->setStatus(STS_ERR);

        Status registrationStatus = STS_OK;

        auto handler = [&registrationStatus](void* context, Status status) { registrationStatus = status; };

        status = regHandler.registration(
            true, [](void* context, Status status) { (*static_cast<decltype(handler)*>(context))(context, status); },
            &handler);

        REQUIRE(status == STS_OK);

        setCurrentTime(0);

        setCurrentTime(60000);

        CHECK(registrationStatus == STS_OK);

        setCurrentTime(1200000);

        CHECK(registrationStatus == STS_OK);

        setCurrentTime(1800000);

        CHECK_FALSE(registrationStatus == STS_OK);

        CHECK(regHandler.getState() == RegHandler::STATE_DEREGISTERED);
    }

    regHandler.release();
}
