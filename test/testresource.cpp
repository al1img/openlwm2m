#include <catch2/catch.hpp>

#include <cstring>

#include "object.hpp"

using namespace openlwm2m;

TEST_CASE("test resource info", "[resource]")
{
    Object object(2, ITF_ALL, true, true);

    object.createResourceString(2, OP_READWRITE, true, true, 100, 200);

    object.init();

    ResourceInstance* resourceInstance = object.getResourceInstance(0, 2);
    REQUIRE(resourceInstance);

    ResourceInfo& info = resourceInstance->getResource()->getInfo();

    CHECK(info.getId() == 2);
    CHECK(info.isSingle());
    CHECK(info.isMandatory());
    CHECK(info.getType() == DATA_TYPE_STRING);
    CHECK(info.getMaxInstances() == 1);
    CHECK(info.checkOperation(OP_READWRITE));
    CHECK(info.min().minUint == 0);
    CHECK(info.max().maxUint == 200);

    bool cbkIndication = false;
    ResourceInstance* cbkInstance = NULL;

    auto handler = [&](void* context, ResourceInstance* resourceInstance) {
        cbkIndication = true;
        cbkInstance = resourceInstance;
    };

    info.setValueChangedCbk(
        [](void* context, ResourceInstance* resourceInstance) {
            (*static_cast<decltype(handler)*>(context))(context, resourceInstance);
        },
        &handler);

    CHECK_FALSE(cbkIndication);

    info.valueChanged(resourceInstance);

    CHECK(cbkIndication);
    CHECK(cbkInstance == resourceInstance);

    object.release();
}

TEST_CASE("test resource", "[resource]")
{
    Status status = STS_OK;

    Object object(2, ITF_ALL, true, true);

    object.createResourceString(4, OP_READWRITE, false, false, 10);

    object.init();

    ObjectInstance* instance = object.getFirstInstance();
    REQUIRE(instance);

    Resource* resource = instance->getFirstResource();
    REQUIRE(resource);
    CHECK(resource->getId() == 4);
    CHECK(resource->getInfo().getType() == DATA_TYPE_STRING);
    CHECK_FALSE(resource->getInfo().isSingle());
    CHECK_FALSE(resource->getInfo().isMandatory());
    CHECK(resource->getInfo().getMaxInstances() == 10);

    ResourceInstance* resourceInstance = resource->getFirstInstance();
    REQUIRE_FALSE(resourceInstance);

    uint16_t ids[] = {2, 4, 5, 7, 9, 11, 23, 45, 56, 78};

    for (size_t i = 0; i < sizeof(ids) / sizeof(uint16_t); i++) {
        resourceInstance = resource->createInstance(ids[i], &status);
        REQUIRE(resourceInstance);
        CHECK(resourceInstance->getId() == ids[i]);
    }

    int i = 0;

    for (resourceInstance = resource->getFirstInstance(); resourceInstance;
         resourceInstance = resource->getNextInstance()) {
        REQUIRE(resourceInstance);
        CHECK(resourceInstance->getId() == ids[i++]);
    }

    for (size_t i = 0; i < sizeof(ids) / sizeof(uint16_t); i++) {
        status = resource->deleteInstance(ids[i]);
        CHECK(status == STS_OK);
        resourceInstance = resource->getInstanceById(ids[i]);
        CHECK_FALSE(resourceInstance);
    }

    object.release();
}

TEST_CASE("test resource string", "[resource]")
{
    Status status = STS_OK;
    bool cbkIndication = false;
    ResourceInstance* cbkInstance = NULL;

    auto handler = [&](void* context, ResourceInstance* resourceInstance) {
        cbkIndication = true;
        cbkInstance = resourceInstance;
    };

    Object object(2, ITF_ALL, true, true);

    object.createResourceString(4, OP_READWRITE, true, true, 1, 10,
                                [](void* context, ResourceInstance* resourceInstance) {
                                    (*static_cast<decltype(handler)*>(context))(context, resourceInstance);
                                },
                                &handler);

    object.init();

    ResourceString* instance = static_cast<ResourceString*>(object.getResourceInstance(0, 4));
    REQUIRE(instance);

    cbkIndication = false;
    cbkInstance = NULL;

    status = instance->setValue("Test string");
    CHECK_FALSE(status == STS_OK);

    status = instance->setValue("TestString");
    CHECK(status == STS_OK);

    CHECK(cbkIndication);
    CHECK(cbkInstance == instance);

    CHECK(strcmp(instance->getValue(), "TestString") == 0);

    object.release();
}

TEST_CASE("test resource int", "[resource]")
{
    Status status = STS_OK;
    bool cbkIndication = false;
    ResourceInstance* cbkInstance = NULL;

    auto handler = [&](void* context, ResourceInstance* resourceInstance) {
        cbkIndication = true;
        cbkInstance = resourceInstance;
    };

    Object object(2, ITF_ALL, true, true);

    object.createResourceInt(4, OP_READWRITE, true, true, 1, -50, 50,
                             [](void* context, ResourceInstance* resourceInstance) {
                                 (*static_cast<decltype(handler)*>(context))(context, resourceInstance);
                             },
                             &handler);

    object.init();

    ResourceInt* instance = static_cast<ResourceInt*>(object.getResourceInstance(0, 4));
    REQUIRE(instance);

    cbkIndication = false;
    cbkInstance = NULL;

    status = instance->setValue(-100);
    CHECK_FALSE(status == STS_OK);

    status = instance->setValue(24);
    CHECK(status == STS_OK);

    CHECK(cbkIndication);
    CHECK(cbkInstance == instance);

    CHECK(instance->getValue() == 24);

    object.release();
}

TEST_CASE("test resource uint", "[resource]")
{
    Status status = STS_OK;
    bool cbkIndication = false;
    ResourceInstance* cbkInstance = NULL;

    auto handler = [&](void* context, ResourceInstance* resourceInstance) {
        cbkIndication = true;
        cbkInstance = resourceInstance;
    };

    Object object(2, ITF_ALL, true, true);

    object.createResourceUint(4, OP_READWRITE, true, true, 1, 20, 50,
                              [](void* context, ResourceInstance* resourceInstance) {
                                  (*static_cast<decltype(handler)*>(context))(context, resourceInstance);
                              },
                              &handler);

    object.init();

    ResourceUint* instance = static_cast<ResourceUint*>(object.getResourceInstance(0, 4));
    REQUIRE(instance);

    cbkIndication = false;
    cbkInstance = NULL;

    status = instance->setValue(10);
    CHECK_FALSE(status == STS_OK);

    status = instance->setValue(24);
    CHECK(status == STS_OK);

    CHECK(cbkIndication);
    CHECK(cbkInstance == instance);

    CHECK(instance->getValue() == 24);

    object.release();
}

TEST_CASE("test resource bool", "[resource]")
{
    Status status = STS_OK;
    bool cbkIndication = false;
    ResourceInstance* cbkInstance = NULL;

    auto handler = [&](void* context, ResourceInstance* resourceInstance) {
        cbkIndication = true;
        cbkInstance = resourceInstance;
    };

    Object object(2, ITF_ALL, true, true);

    object.createResourceBool(4, OP_READWRITE, true, true, 1,
                              [](void* context, ResourceInstance* resourceInstance) {
                                  (*static_cast<decltype(handler)*>(context))(context, resourceInstance);
                              },
                              &handler);

    object.init();

    ResourceBool* instance = static_cast<ResourceBool*>(object.getResourceInstance(0, 4));
    REQUIRE(instance);

    cbkIndication = false;
    cbkInstance = NULL;

    status = instance->setValue(1);
    CHECK(status == STS_OK);
    CHECK(instance->getValue() == 1);

    CHECK(cbkIndication);
    CHECK(cbkInstance == instance);

    cbkIndication = false;
    cbkInstance = NULL;

    status = instance->setValue(0);
    CHECK(status == STS_OK);
    CHECK(instance->getValue() == 0);

    CHECK(cbkIndication);
    CHECK(cbkInstance == instance);

    object.release();
}