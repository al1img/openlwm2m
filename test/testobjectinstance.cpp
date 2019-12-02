#include <catch2/catch.hpp>

#include <cstring>

#include "jsonconverter.hpp"
#include "object.hpp"

using namespace openlwm2m;

TEST_CASE("test object instance", "[objectInstance]")
{
    Status status = STS_OK;

    Object object(4, true, true);

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

    ObjectInstance* objectInstance = object.getInstanceById(0);
    REQUIRE(objectInstance);

    Resource* resource = NULL;

    resource = objectInstance->getResourceById(0);
    CHECK(resource);

    resource = objectInstance->getResourceById(1);
    CHECK(resource);

    resource = objectInstance->getResourceById(2);
    CHECK(resource);

    resource = objectInstance->getResourceById(3);
    CHECK(resource);

    resource = objectInstance->getResourceById(4);
    CHECK(resource);

    resource = objectInstance->getResourceById(5);
    CHECK(resource);

    resource = objectInstance->getResourceById(6);
    CHECK_FALSE(resource);

    uint16_t id = 0;

    for (resource = objectInstance->getFirstResource(); resource; resource = objectInstance->getNextResource()) {
        CHECK(resource->getId() == id++);
    }

    CHECK(id == 6);

    ResourceInstance* resourceInstance = NULL;

    resourceInstance = objectInstance->getResourceInstance(0);
    CHECK(resourceInstance);

    resourceInstance = objectInstance->getResourceInstance(1);
    CHECK(resourceInstance);

    resourceInstance = objectInstance->getResourceInstance(2);
    CHECK(resourceInstance);

    resourceInstance = objectInstance->getResourceInstance(0, 5);
    CHECK_FALSE(resourceInstance);

    object.release();
}

struct TestData {
    const char* writeData;
    const char* readData;
    Status status;
};

void testReadWrite(ObjectInstance* objectInstance, TestData* testData, size_t size, bool checkOperation,
                   bool ignoreMissing, bool replace)
{
    Status status = STS_OK;
    JsonConverter writeConverter, readConverter;
    char readBuffer[1500];

    for (size_t i = 0; i < size; i++) {
        if (testData[i].writeData != NULL) {
            if (replace) {
                objectInstance->release();
                objectInstance->init();
            }

            status = writeConverter.startDecoding(reinterpret_cast<void*>(const_cast<char*>(testData[i].writeData)),
                                                  strlen(testData[i].writeData));
            CHECK(status == STS_OK);

            status = objectInstance->write(&writeConverter, checkOperation, ignoreMissing);
            CHECK(status == testData[i].status);
        }

        if (testData[i].readData != NULL) {
            status = readConverter.startEncoding(readBuffer, sizeof(readBuffer) - 1);
            REQUIRE(status == STS_OK);

            status = objectInstance->read(&readConverter, false);
            CHECK(status == STS_OK);

            size_t size;
            status = readConverter.finishEncoding(&size);
            REQUIRE(status == STS_OK);
            readBuffer[size] = '\0';

            printf("Index: %zu\n", i);
            printf("Read: %s\n", readBuffer);
            printf("Test: %s\n", testData[i].readData);

            CHECK(strcmp(testData[i].readData, readBuffer) == 0);
        }
    }
}

TEST_CASE("test object instance write", "[objectInstance]")
{
    Status status = STS_OK;

    Object object(4, true, true);

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
{\"bn\":\"/4/0/\",\"n\":\"0\",\"vs\":\"test string\"},\
{\"n\":\"1\",\"v\":10}\
]",
             "[\
{\"bn\":\"/4/0/\",\"n\":\"0\",\"vs\":\"test string\"},\
{\"n\":\"1\",\"v\":10},\
{\"n\":\"2\",\"v\":0}\
]",
             STS_OK},
            {// Write read only resources 2
             "[\
{\"n\":\"/4/0/2\",\"v\":10.14}\
]",
             "[\
{\"bn\":\"/4/0/\",\"n\":\"0\",\"vs\":\"test string\"},\
{\"n\":\"1\",\"v\":10},\
{\"n\":\"2\",\"v\":0}\
]",
             STS_ERR_NOT_ALLOWED},
            {// Write resources 3
             "[\
{\"n\":\"/4/0/3\",\"vb\":true}\
]",
             "[\
{\"bn\":\"/4/0/\",\"n\":\"0\",\"vs\":\"test string\"},\
{\"n\":\"1\",\"v\":10},\
{\"n\":\"2\",\"v\":0},\
{\"n\":\"3\",\"vb\":true}\
]",
             STS_OK},
            {// Write resources 4, instances 2, 5, 8
             "[\
{\"bn\":\"/4/0/\",\"n\":\"4/2\",\"v\":45},\
{\"n\":\"4/5\",\"v\":89},\
{\"n\":\"4/8\",\"v\":21}\
]",
             "[\
{\"bn\":\"/4/0/\",\"n\":\"0\",\"vs\":\"test string\"},\
{\"n\":\"1\",\"v\":10},\
{\"n\":\"2\",\"v\":0},\
{\"n\":\"3\",\"vb\":true},\
{\"n\":\"4/2\",\"v\":45},\
{\"n\":\"4/5\",\"v\":89},\
{\"n\":\"4/8\",\"v\":21}\
]",
             STS_OK},
            {// Write resources 4, instances 4, 12, 0
             "[\
{\"bn\":\"/4/0/\",\"n\":\"4/4\",\"v\":456},\
{\"n\":\"4/12\",\"v\":928},\
{\"n\":\"4/0\",\"v\":234}\
]",
             "[\
{\"bn\":\"/4/0/\",\"n\":\"0\",\"vs\":\"test string\"},\
{\"n\":\"1\",\"v\":10},\
{\"n\":\"2\",\"v\":0},\
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
{\"bn\":\"/4/0/\",\"n\":\"5\",\"v\":1023}\
]",
             "[\
{\"bn\":\"/4/0/\",\"n\":\"0\",\"vs\":\"test string\"},\
{\"n\":\"1\",\"v\":10},\
{\"n\":\"2\",\"v\":0},\
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
{\"bn\":\"/4/0/\",\"n\":\"6\",\"v\":324.5}\
]",
             "[\
{\"bn\":\"/4/0/\",\"n\":\"0\",\"vs\":\"test string\"},\
{\"n\":\"1\",\"v\":10},\
{\"n\":\"2\",\"v\":0},\
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
{\"bn\":\"/4/1/\",\"n\":\"0\",\"vs\":\"simple string\"}\
]",
             "[\
{\"bn\":\"/4/0/\",\"n\":\"0\",\"vs\":\"test string\"},\
{\"n\":\"1\",\"v\":10},\
{\"n\":\"2\",\"v\":0},\
{\"n\":\"3\",\"vb\":true},\
{\"n\":\"4/0\",\"v\":234},\
{\"n\":\"4/2\",\"v\":45},\
{\"n\":\"4/4\",\"v\":456},\
{\"n\":\"4/5\",\"v\":89},\
{\"n\":\"4/8\",\"v\":21},\
{\"n\":\"4/12\",\"v\":928}\
]",
             STS_ERR_FORMAT},
        };

        testReadWrite(object.getFirstInstance(), testData, sizeof(testData) / sizeof(TestData), true, true, false);
    }

    SECTION("write replace")
    {
        TestData testData[] = {
            {// Write resources 0, 1
             "[\
{\"bn\":\"/4/0/\",\"n\":\"0\",\"vs\":\"test string\"},\
{\"n\":\"1\",\"v\":10}\
]",
             "[\
{\"bn\":\"/4/0/\",\"n\":\"0\",\"vs\":\"test string\"},\
{\"n\":\"1\",\"v\":10},\
{\"n\":\"2\",\"v\":0}\
]",
             STS_OK},
            {// Write resources 3
             "[\
{\"n\":\"/4/0/3\",\"vb\":true}\
]",
             "[\
{\"bn\":\"/4/0/\",\"n\":\"0\",\"vs\":\"\"},\
{\"n\":\"1\",\"v\":0},\
{\"n\":\"2\",\"v\":0},\
{\"n\":\"3\",\"vb\":true}\
]",
             STS_OK},
            {// Write resources 4, instances 2, 5, 8
             "[\
{\"bn\":\"/4/0/\",\"n\":\"4/2\",\"v\":45},\
{\"n\":\"4/5\",\"v\":89},\
{\"n\":\"4/8\",\"v\":21}\
]",
             "[\
{\"bn\":\"/4/0/\",\"n\":\"0\",\"vs\":\"\"},\
{\"n\":\"1\",\"v\":0},\
{\"n\":\"2\",\"v\":0},\
{\"n\":\"4/2\",\"v\":45},\
{\"n\":\"4/5\",\"v\":89},\
{\"n\":\"4/8\",\"v\":21}\
]",
             STS_OK},
            {// Write resources 4, instances 4, 12, 0
             "[\
{\"bn\":\"/4/0/\",\"n\":\"4/4\",\"v\":456},\
{\"n\":\"4/12\",\"v\":928},\
{\"n\":\"4/0\",\"v\":234}\
]",
             "[\
{\"bn\":\"/4/0/\",\"n\":\"0\",\"vs\":\"\"},\
{\"n\":\"1\",\"v\":0},\
{\"n\":\"2\",\"v\":0},\
{\"n\":\"4/0\",\"v\":234},\
{\"n\":\"4/4\",\"v\":456},\
{\"n\":\"4/12\",\"v\":928}\
]",
             STS_OK},
        };

        testReadWrite(object.getFirstInstance(), testData, sizeof(testData) / sizeof(TestData), true, true, true);
    }

    object.release();
}
