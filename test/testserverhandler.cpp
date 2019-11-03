#include <catch2/catch.hpp>

#include <cstring>

#include "jsonconverter.hpp"
#include "objectmanager.hpp"
#include "serverhandler.hpp"

#include "testplatform.hpp"
#include "testtransport.hpp"

using namespace openlwm2m;

static void run(uint64_t currentTimeMs)
{
    setCurrentTime(currentTimeMs);
    REQUIRE(Timer::run() == STS_OK);
}

static void setupObjects(ObjectManager* objectManager)
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

        status = serverHandler.registration(
            false, [](void* context, ServerHandler* handler, Status status) { CHECK(status == STS_OK); });
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
            [](void* context, ServerHandler* handler, Status status) { CHECK(status == STS_OK); });
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
