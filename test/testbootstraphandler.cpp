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

    status = instance->getResourceInstance(RES_LWM2M_SERVER_URI)->setString("coap://bootstrap");
    CHECK(status == STS_OK);

    status = instance->getResourceInstance(RES_BOOTSTRAP_SERVER)->setBool(1);
    CHECK(status == STS_OK);

    instance = object->createInstance();
    REQUIRE(instance);

    status = instance->getResourceInstance(RES_LWM2M_SERVER_URI)->setString("coap://lwm2mserver");
    CHECK(status == STS_OK);

    status = instance->getResourceInstance(RES_SECURITY_SHORT_SERVER_ID)->setInt(5);
    CHECK(status == STS_OK);
}

TEST_CASE("test BootstrapHandler", "[BootstrapHandler]")
{
    Status status = STS_OK;
    ObjectManager objectManager;
    TestTransport transport;

    BootstrapHandler bootstrapHandler("TestClient", objectManager);

    // Create option resource 16 with multiple int resource 2
    Object* object = objectManager.getObjectById(OBJ_LWM2M_SECURITY);
    REQUIRE(object);

    status = object->createResourceUint(16, OP_READWRITE, false, false, 10);
    CHECK(status == STS_OK);

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

        CHECK(strcmp(result, "lwm2m=\"1.1\",</0/0>,</0/1>;ssid=5;uri=coap://lwm2mserver,</1>,</3/0>") == 0);

        status = bootstrapHandler.bootstrapFinish();
        CHECK(status == STS_OK);

        CHECK(retStatus == STS_OK);
    }

    SECTION("Read")
    {
        char data[1024];
        size_t size;
        DataFormat format = DATA_FMT_SENML_JSON;

        // Read object

        const char* obj0Data =
            "[\
{\"bn\":\"/0/0/\",\"n\":\"0\",\"vs\":\"coap://bootstrap\"},\
{\"n\":\"1\",\"vb\":true},\
{\"n\":\"2\",\"v\":0},\
{\"n\":\"3\",\"vd\":\"\"},\
{\"n\":\"4\",\"vd\":\"\"},\
{\"n\":\"5\",\"vd\":\"\"},\
{\"bn\":\"/0/1/\",\"n\":\"0\",\"vs\":\"coap://lwm2mserver\"},\
{\"n\":\"1\",\"vb\":false},{\"n\":\"2\",\"v\":0},\
{\"n\":\"3\",\"vd\":\"\"},\
{\"n\":\"4\",\"vd\":\"\"},\
{\"n\":\"5\",\"vd\":\"\"},\
{\"n\":\"10\",\"v\":5}]";

        size = sizeof(data);

        status = bootstrapHandler.read(&format, static_cast<void*>(data), &size, OBJ_LWM2M_SECURITY);
        CHECK(status == STS_OK);

        data[size] = '\0';

        printf("%s\n", data);
        printf("%s\n", obj0Data);

        CHECK(strcmp(obj0Data, data) == 0);

        // Read object instance

        const char* obj0Ins1Data =
            "[\
{\"bn\":\"/0/1/\",\"n\":\"0\",\"vs\":\"coap://lwm2mserver\"},\
{\"n\":\"1\",\"vb\":false},\
{\"n\":\"2\",\"v\":0},\
{\"n\":\"3\",\"vd\":\"\"},\
{\"n\":\"4\",\"vd\":\"\"},\
{\"n\":\"5\",\"vd\":\"\"},\
{\"n\":\"10\",\"v\":5}]";

        size = sizeof(data);

        status = bootstrapHandler.read(&format, static_cast<void*>(data), &size, OBJ_LWM2M_SECURITY, 1);
        CHECK(status == STS_OK);

        data[size] = '\0';

        printf("%s\n", data);
        printf("%s\n", obj0Ins1Data);

        CHECK(strcmp(obj0Ins1Data, data) == 0);

        status = bootstrapHandler.read(&format, static_cast<void*>(data), &size, OBJ_DEVICE);
        CHECK(status == STS_ERR_NOT_ALLOWED);

        status = bootstrapHandler.bootstrapFinish();
        CHECK(status == STS_OK);

        CHECK(retStatus == STS_OK);
    }

    SECTION("Write")
    {
        char readData[1024];
        size_t readSize;

        DataFormat format = DATA_FMT_SENML_JSON;

        // Write object

        const char* writeData0 =
            "[\
{\"bn\":\"/1/0/\",\"n\":\"0\",\"v\":5},\
{\"n\":\"1\",\"v\":300},\
{\"n\":\"6\",\"vb\":false},\
{\"n\":\"7\",\"vs\":\"U\"},\
{\"bn\":\"/1/1/\",\"n\":\"0\",\"v\":3},\
{\"n\":\"1\",\"v\":600},\
{\"n\":\"6\",\"vb\":false},\
{\"n\":\"7\",\"vs\":\"U\"}]";

        status = bootstrapHandler.write(DATA_FMT_SENML_JSON, static_cast<void*>(const_cast<char*>(writeData0)),
                                        strlen(writeData0), OBJ_LWM2M_SERVER);
        CHECK(status == STS_OK);

        readSize = sizeof(readData);

        status = bootstrapHandler.read(&format, static_cast<void*>(readData), &readSize, OBJ_LWM2M_SERVER);
        CHECK(status == STS_OK);

        readData[readSize] = '\0';

        printf("%s\n", writeData0);
        printf("%s\n", readData);

        CHECK(strcmp(writeData0, readData) == 0);

        // Write object instance

        const char* writeData1 =
            "[\
{\"bn\":\"/0/1/\",\"n\":\"0\",\"vs\":\"coap://justserver\"},\
{\"n\":\"1\",\"vb\":false},\
{\"n\":\"2\",\"v\":3},\
{\"n\":\"3\",\"vd\":\"\"},\
{\"n\":\"4\",\"vd\":\"\"},\
{\"n\":\"5\",\"vd\":\"\"},\
{\"n\":\"10\",\"v\":25}]";

        status = bootstrapHandler.write(DATA_FMT_SENML_JSON, static_cast<void*>(const_cast<char*>(writeData1)),
                                        strlen(writeData1), OBJ_LWM2M_SECURITY, 1);
        CHECK(status == STS_OK);

        readSize = sizeof(readData);

        status = bootstrapHandler.read(&format, static_cast<void*>(readData), &readSize, OBJ_LWM2M_SECURITY, 1);
        CHECK(status == STS_OK);

        readData[readSize] = '\0';

        printf("%s\n", writeData1);
        printf("%s\n", readData);

        CHECK(strcmp(writeData1, readData) == 0);

        // Write resource

        const char* writeData2 =
            "[\
{\"bn\":\"/0/0/16/\",\"n\":\"0\",\"v\":23},\
{\"n\":\"3\",\"v\":38},\
{\"n\":\"6\",\"v\":12}]";

        status = bootstrapHandler.write(DATA_FMT_SENML_JSON, static_cast<void*>(const_cast<char*>(writeData2)),
                                        strlen(writeData2), 0, 0, 16);
        CHECK(status == STS_OK);

        const char* writeData3 = "[{\"bn\":\"/0/0/16/\",\"n\":\"6\",\"v\":45}]";

        status = bootstrapHandler.write(DATA_FMT_SENML_JSON, static_cast<void*>(const_cast<char*>(writeData3)),
                                        strlen(writeData3), 0, 0, 16);
        CHECK(status == STS_OK);

        const char* writeData4 = "[{\"bn\":\"/0/0/16/\",\"n\":\"9\",\"v\":112}]";

        status = bootstrapHandler.write(DATA_FMT_SENML_JSON, static_cast<void*>(const_cast<char*>(writeData4)),
                                        strlen(writeData4), 0, 0, 16);
        CHECK(status == STS_OK);

        const char* obj0Ins1Data =
            "[\
{\"bn\":\"/0/0/\",\"n\":\"0\",\"vs\":\"coap://bootstrap\"},\
{\"n\":\"1\",\"vb\":true},\
{\"n\":\"2\",\"v\":0},\
{\"n\":\"3\",\"vd\":\"\"},\
{\"n\":\"4\",\"vd\":\"\"},\
{\"n\":\"5\",\"vd\":\"\"},\
{\"n\":\"16/0\",\"v\":23},\
{\"n\":\"16/3\",\"v\":38},\
{\"n\":\"16/6\",\"v\":45},\
{\"n\":\"16/9\",\"v\":112}]";

        readSize = sizeof(readData);

        status = bootstrapHandler.read(&format, static_cast<void*>(readData), &readSize, 0, 0);
        CHECK(status == STS_OK);

        readData[readSize] = '\0';

        printf("%s\n", obj0Ins1Data);
        printf("%s\n", readData);

        CHECK(strcmp(obj0Ins1Data, readData) == 0);

        status = bootstrapHandler.bootstrapFinish();
        CHECK(status == STS_OK);

        CHECK(retStatus == STS_OK);
    }

    SECTION("Delete")
    {
        char discover[1024];
        size_t size;

        Object* object = objectManager.getObjectById(OBJ_LWM2M_SERVER);
        REQUIRE(object);

        ObjectInstance* instance = object->createInstance();
        REQUIRE(instance);

        instance = object->createInstance();
        REQUIRE(instance);

        // Delete object instance
        status = bootstrapHandler.deleteInstance(OBJ_LWM2M_SERVER, 1);
        CHECK(status == STS_OK);

        size = sizeof(discover);

        status = bootstrapHandler.discover(discover, &size);
        CHECK(status == STS_OK);

        discover[size] = '\0';

        printf("%s\n", discover);

        CHECK(strcmp(discover, "lwm2m=\"1.1\",</0/0>,</0/1>;ssid=5;uri=coap://lwm2mserver,</1/0>,</3/0>") == 0);

        // Delete object
        status = bootstrapHandler.deleteInstance(OBJ_LWM2M_SERVER);
        CHECK(status == STS_OK);

        size = sizeof(discover);

        status = bootstrapHandler.discover(discover, &size);
        CHECK(status == STS_OK);

        discover[size] = '\0';

        printf("%s\n", discover);

        CHECK(strcmp(discover, "lwm2m=\"1.1\",</0/0>,</0/1>;ssid=5;uri=coap://lwm2mserver,</1>,</3/0>") == 0);

        // Delete all
        status = bootstrapHandler.deleteInstance();
        CHECK(status == STS_OK);

        size = sizeof(discover);

        status = bootstrapHandler.discover(discover, &size);
        CHECK(status == STS_OK);

        discover[size] = '\0';

        printf("%s\n", discover);

        CHECK(strcmp(discover, "lwm2m=\"1.1\",</0/0>,</1>,</3/0>") == 0);

        // Delete device object
        status = bootstrapHandler.deleteInstance(OBJ_DEVICE);
        CHECK(status == STS_ERR_NOT_ALLOWED);

        // Delete Bootstrap-Server Account
        status = bootstrapHandler.deleteInstance(OBJ_LWM2M_SECURITY, 0);
        CHECK(status == STS_ERR_NOT_ALLOWED);

        status = bootstrapHandler.bootstrapFinish();
        CHECK(status == STS_OK);

        CHECK(retStatus == STS_OK);
    }
}
