#include <catch2/catch.hpp>

#include <cstring>

#include "bootstraphandler.hpp"
#include "jsonconverter.hpp"
#include "objectmanager.hpp"

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

    Object* object = objectManager->getObjectById(OBJ_LWM2M_SECURITY);
    REQUIRE(object);

    ObjectInstance* instance = object->createInstance();
    REQUIRE(instance);

    status =
        static_cast<ResourceString*>(instance->getResourceInstance(RES_LWM2M_SERVER_URI))->setValue("coap://bootstrap");
    CHECK(status == STS_OK);

    status = static_cast<ResourceBool*>(instance->getResourceInstance(RES_BOOTSTRAP_SERVER))->setValue(1);
    CHECK(status == STS_OK);

    instance = object->createInstance();
    REQUIRE(instance);

    status = static_cast<ResourceString*>(instance->getResourceInstance(RES_LWM2M_SERVER_URI))
                 ->setValue("coap://lwm2mserver");
    CHECK(status == STS_OK);

    status = static_cast<ResourceInt*>(instance->getResourceInstance(RES_SECURITY_SHORT_SERVER_ID))->setValue(5);
    CHECK(status == STS_OK);
}

TEST_CASE("test BootstrapHandler", "[BootstrapHandler]")
{
    Status status = STS_OK;
    ObjectManager objectManager;
    TestTransport transport;

    BootstrapHandler bootstrapHandler("TestClient", objectManager);

    objectManager.init();

    bootstrapHandler.bind(&transport);

    setupObjects(&objectManager);

    Status retStatus = STS_ERR;

    auto requestHandler = [&](Status status) { retStatus = status; };

    run(0);

    status = bootstrapHandler.bootstrapRequest(
        [](void* context, BootstrapHandler* handler, Status status) {
            (*static_cast<decltype(requestHandler)*>(context))(status);
        },
        &requestHandler);
    REQUIRE(status == STS_OK);

    TestSession* session = transport.getLastSession();
    REQUIRE(session);
    CHECK(session->getUri() == "coap://bootstrap");

    SECTION("Timeout")
    {
        run(60000);

        CHECK(retStatus == STS_ERR_TIMEOUT);
    }

    SECTION("Discover")
    {
        size_t size = 1024;
        char result[size];

        status = bootstrapHandler.discover(result, &size);
        CHECK(status == STS_OK);

        result[size] = '\0';

        CHECK(strcmp(result, "lwm2m=\"1.1\",</0/0>,</0/1>;ssid=5;uri=coap://lwm2mserver,</3/0>") == 0);

        status = bootstrapHandler.bootstrapFinish();
        CHECK(status == STS_OK);

        CHECK(retStatus == STS_OK);
    }

#if 0
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

#endif
}
