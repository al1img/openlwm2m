#include <catch2/catch.hpp>

#include "object.hpp"

using namespace openlwm2m;

TEST_CASE("test single instance mandatory object", "[object]")
{
    Object object(2, ITF_ALL, true, true);

    CHECK(object.getId() == 2);
    CHECK(object.checkInterface(ITF_REGISTER));
    CHECK(object.checkInterface(ITF_BOOTSTRAP));
    CHECK(object.checkInterface(ITF_DEVICE));
    CHECK(object.checkInterface(ITF_REPORTTING));

    object.init();

    // Should be 1 instance with id 0
    ObjectInstance* instance = object.getFirstInstance();
    REQUIRE(instance);
    CHECK(instance->getId() == 0);

    object.release();
}

TEST_CASE("test single instance non mandatory object", "[object]")
{
    Status status = STS_OK;

    Object object(5, ITF_BOOTSTRAP, true, false);

    CHECK(object.getId() == 5);
    CHECK_FALSE(object.checkInterface(ITF_REGISTER));
    CHECK(object.checkInterface(ITF_BOOTSTRAP));

    object.init();

    // Should be no instances
    ObjectInstance* instance = object.getFirstInstance();
    REQUIRE_FALSE(instance);

    instance = object.createInstance(34, &status);
    CHECK(status == STS_OK);
    REQUIRE(instance);
    CHECK(instance->getId() == 34);
    CHECK(Object::isInstanceChanged());

    object.release();
}

TEST_CASE("test multiple instance object", "[object]")
{
    Status status = STS_OK;

    Object object(13, ITF_BOOTSTRAP, false, false, 10);

    CHECK(object.getId() == 13);
    CHECK(object.checkInterface(ITF_CLIENT));

    object.init();

    // Should be no instances
    ObjectInstance* instance = object.getFirstInstance();
    REQUIRE_FALSE(instance);

    for (int i = 0; i < 10; i++) {
        instance = object.createInstance(i + 4, &status);
        CHECK(status == STS_OK);
        REQUIRE(instance);
    }

    for (int i = 0; i < 10; i++) {
        instance = object.getInstanceById(i + 4);
        REQUIRE(instance);
        CHECK(instance->getId() == i + 4);
    }

    int i = 0;

    for (instance = object.getFirstInstance(); instance; instance = object.getNextInstance()) {
        REQUIRE(instance);
        CHECK(instance->getId() == i++ + 4);
    }

    status = object.deleteInstance(10);
    CHECK(status == STS_OK);
    instance = object.getInstanceById(10);
    CHECK_FALSE(instance);

    status = object.deleteInstance(6);
    CHECK(status == STS_OK);
    instance = object.getInstanceById(6);
    CHECK_FALSE(instance);

    status = object.deleteInstance(12);
    CHECK(status == STS_OK);
    instance = object.getInstanceById(12);
    CHECK_FALSE(instance);

    object.release();
}

TEST_CASE("test create resources", "[object]")
{
    Status status = STS_OK;

    struct TestData {
        uint16_t id;
        Operation operations;
        DataType dataType;
    };

    TestData testData[] = {{2, OP_READWRITE, DATA_TYPE_STRING}, {3, OP_READWRITE, DATA_TYPE_INT},
                           {4, OP_READWRITE, DATA_TYPE_UINT},   {5, OP_READWRITE, DATA_TYPE_BOOL},
                           {6, OP_READWRITE, DATA_TYPE_OPAQUE}, {7, OP_EXECUTE, DATA_TYPE_NONE}};

    Object object(2, ITF_ALL, true, true);

    status = object.createResourceString(testData[0].id, testData[0].operations, true, true);
    CHECK(status == STS_OK);

    status = object.createResourceInt(testData[1].id, testData[1].operations, true, true);
    CHECK(status == STS_OK);

    status = object.createResourceUint(testData[2].id, testData[2].operations, true, true);
    CHECK(status == STS_OK);

    status = object.createResourceBool(testData[3].id, testData[3].operations, true, true);
    CHECK(status == STS_OK);

    status = object.createResourceOpaque(testData[4].id, testData[4].operations, true, true);
    CHECK(status == STS_OK);

    status = object.createResourceNone(testData[5].id, testData[5].operations, true, true);
    CHECK(status == STS_OK);

    object.init();

    ObjectInstance* instance = object.getFirstInstance();
    REQUIRE(instance);

    for (size_t i = 0; i < sizeof(testData) / sizeof(TestData); i++) {
        Resource* resource = instance->getResourceById(testData[i].id);
        CHECK(resource->getInfo().checkOperation(testData[i].operations));
        CHECK(resource->getInfo().getType() == testData[i].dataType);
    }

    int i = 0;

    for (Resource* resource = instance->getFirstResource(); resource; resource = instance->getNextResource()) {
        CHECK(resource->getInfo().checkOperation(testData[i].operations));
        CHECK(resource->getInfo().getType() == testData[i].dataType);
        i++;
    }

    ResourceInstance* resourceInstance = object.getResourceInstance(0, testData[0].id);
    REQUIRE(resourceInstance);
    CHECK(resourceInstance->getResource()->getInfo().checkOperation(testData[0].operations));
    CHECK(resourceInstance->getResource()->getInfo().getType() == testData[0].dataType);

    status = object.createResourceString(8, OP_READ, true, true);
    CHECK_FALSE(status == STS_OK);

    object.release();
}