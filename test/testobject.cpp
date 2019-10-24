#include <catch2/catch.hpp>

#include <cstring>

#include "jsonconverter.hpp"
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

    status = object.createExecutableResource(testData[5].id, true);
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

struct TestData {
    const char* writeData;
    const char* readData;
    Status status;
};

void testReadWrite(Object* object, TestData* testData, size_t size, bool checkOperation, bool ignoreMissing,
                   bool replace)
{
    Status status = STS_OK;
    JsonConverter writeConverter, readConverter;
    char readBuffer[1500];

    for (size_t i = 0; i < size; i++) {
        if (testData[i].writeData != NULL) {
            status = writeConverter.startDecoding(reinterpret_cast<void*>(const_cast<char*>(testData[i].writeData)),
                                                  strlen(testData[i].writeData));
            CHECK(status == STS_OK);

            status = object->write(&writeConverter);
            CHECK(status == testData[i].status);
        }

        if (testData[i].readData != NULL) {
            status = readConverter.startEncoding(readBuffer, sizeof(readBuffer) - 1);
            REQUIRE(status == STS_OK);

            status = object->read(&readConverter);
            CHECK(status == STS_OK);

            size_t size;
            status = readConverter.finishEncoding(&size);
            REQUIRE(status == STS_OK);
            readBuffer[size] = '\0';

            printf("%s\n", readBuffer);
            printf("%s\n", testData[i].readData);

            CHECK(strcmp(testData[i].readData, readBuffer) == 0);
        }
    }
}

TEST_CASE("test object write", "[object]")
{
    Status status = STS_OK;

    Object object(2, ITF_ALL, false, false, 5);

    status = object.createResourceString(0, OP_READWRITE, true, true);
    REQUIRE(status == STS_OK);

    status = object.createResourceInt(1, OP_READWRITE, true, true);
    REQUIRE(status == STS_OK);

    status = object.createResourceFloat(2, OP_READ, true, true);
    REQUIRE(status == STS_OK);

    status = object.createResourceBool(3, OP_READWRITE, true, false);
    REQUIRE(status == STS_OK);

    status = object.createResourceUint(4, OP_READWRITE, false, false, 10);
    REQUIRE(status == STS_OK);

    status = object.createExecutableResource(5, true);
    REQUIRE(status == STS_OK);

    object.init();

    SECTION("write update")
    {
        TestData testData[] = {
            {// Write resources 0, 1
             "[\
{\"bn\":\"/2/0/\",\"n\":\"0\",\"vs\":\"test string\"},\
{\"n\":\"1\",\"v\":10}\
]",
             "[\
{\"bn\":\"/2/0/\",\"n\":\"0\",\"vs\":\"test string\"},\
{\"n\":\"1\",\"v\":10},\
{\"n\":\"2\",\"v\":0}\
]",
             STS_OK},
            {// Write resources 2, 3
             "[\
{\"bn\":\"/2/0/\",\"n\":\"2\",\"v\":3.2},\
{\"n\":\"3\",\"vb\":true}\
]",
             "[\
{\"bn\":\"/2/0/\",\"n\":\"0\",\"vs\":\"test string\"},\
{\"n\":\"1\",\"v\":10},\
{\"n\":\"2\",\"v\":3.2},\
{\"n\":\"3\",\"vb\":true}\
]",
             STS_OK},
            {// Write resources 4, instances 2, 5, 8
             "[\
{\"bn\":\"/2/0/\",\"n\":\"4/2\",\"v\":45},\
{\"n\":\"4/5\",\"v\":89},\
{\"n\":\"4/8\",\"v\":21}\
]",
             "[\
{\"bn\":\"/2/0/\",\"n\":\"0\",\"vs\":\"test string\"},\
{\"n\":\"1\",\"v\":10},\
{\"n\":\"2\",\"v\":3.2},\
{\"n\":\"3\",\"vb\":true},\
{\"n\":\"4/2\",\"v\":45},\
{\"n\":\"4/5\",\"v\":89},\
{\"n\":\"4/8\",\"v\":21}\
]",
             STS_OK},
            {// Write resources 4, instances 4, 12, 0
             "[\
{\"bn\":\"/2/0/\",\"n\":\"4/4\",\"v\":456},\
{\"n\":\"4/12\",\"v\":928},\
{\"n\":\"4/0\",\"v\":234}\
]",
             "[\
{\"bn\":\"/2/0/\",\"n\":\"0\",\"vs\":\"test string\"},\
{\"n\":\"1\",\"v\":10},\
{\"n\":\"2\",\"v\":3.2},\
{\"n\":\"3\",\"vb\":true},\
{\"n\":\"4/0\",\"v\":234},\
{\"n\":\"4/2\",\"v\":45},\
{\"n\":\"4/4\",\"v\":456},\
{\"n\":\"4/5\",\"v\":89},\
{\"n\":\"4/8\",\"v\":21},\
{\"n\":\"4/12\",\"v\":928}\
]",
             STS_OK},
            {// Try write executable resource 5
             "[\
{\"bn\":\"/2/0/\",\"n\":\"5\",\"v\":1023}\
]",
             "[\
{\"bn\":\"/2/0/\",\"n\":\"0\",\"vs\":\"test string\"},\
{\"n\":\"1\",\"v\":10},\
{\"n\":\"2\",\"v\":3.2},\
{\"n\":\"3\",\"vb\":true},\
{\"n\":\"4/0\",\"v\":234},\
{\"n\":\"4/2\",\"v\":45},\
{\"n\":\"4/4\",\"v\":456},\
{\"n\":\"4/5\",\"v\":89},\
{\"n\":\"4/8\",\"v\":21},\
{\"n\":\"4/12\",\"v\":928}\
]",
             STS_ERR_NOT_ALLOWED},
            {// Write missing resource 6
             "[\
{\"bn\":\"/2/0/\",\"n\":\"6\",\"v\":324.5}\
]",
             "[\
{\"bn\":\"/2/0/\",\"n\":\"0\",\"vs\":\"test string\"},\
{\"n\":\"1\",\"v\":10},\
{\"n\":\"2\",\"v\":3.2},\
{\"n\":\"3\",\"vb\":true},\
{\"n\":\"4/0\",\"v\":234},\
{\"n\":\"4/2\",\"v\":45},\
{\"n\":\"4/4\",\"v\":456},\
{\"n\":\"4/5\",\"v\":89},\
{\"n\":\"4/8\",\"v\":21},\
{\"n\":\"4/12\",\"v\":928}\
]",
             STS_OK},
            {// Write instance 1
             "[\
{\"bn\":\"/2/1/\",\"n\":\"0\",\"vs\":\"simple string\"},\
{\"n\":\"1\",\"v\":19},\
{\"n\":\"2\",\"v\":6.8},\
{\"n\":\"3\",\"vb\":false},\
{\"n\":\"4/1\",\"v\":10234},\
{\"n\":\"4/3\",\"v\":596},\
{\"n\":\"4/7\",\"v\":12},\
{\"n\":\"4/9\",\"v\":2355},\
{\"n\":\"4/24\",\"v\":22004},\
{\"n\":\"4/120\",\"v\":3450}\
]",
             "[\
{\"bn\":\"/2/0/\",\"n\":\"0\",\"vs\":\"test string\"},\
{\"n\":\"1\",\"v\":10},\
{\"n\":\"2\",\"v\":3.2},\
{\"n\":\"3\",\"vb\":true},\
{\"n\":\"4/0\",\"v\":234},\
{\"n\":\"4/2\",\"v\":45},\
{\"n\":\"4/4\",\"v\":456},\
{\"n\":\"4/5\",\"v\":89},\
{\"n\":\"4/8\",\"v\":21},\
{\"n\":\"4/12\",\"v\":928},\
{\"bn\":\"/2/1/\",\"n\":\"0\",\"vs\":\"simple string\"},\
{\"n\":\"1\",\"v\":19},\
{\"n\":\"2\",\"v\":6.8},\
{\"n\":\"3\",\"vb\":false},\
{\"n\":\"4/1\",\"v\":10234},\
{\"n\":\"4/3\",\"v\":596},\
{\"n\":\"4/7\",\"v\":12},\
{\"n\":\"4/9\",\"v\":2355},\
{\"n\":\"4/24\",\"v\":22004},\
{\"n\":\"4/120\",\"v\":3450}\
]",
             STS_OK},
        };

        testReadWrite(&object, testData, sizeof(testData) / sizeof(TestData), false, true, false);
    }

    object.release();
}