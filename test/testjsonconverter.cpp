#include <catch2/catch.hpp>

#include <cstring>

#include "jsonconverter.hpp"

using namespace openlwm2m;

bool compareResourceData(DataConverter::ResourceData* data1, DataConverter::ResourceData* data2)
{
    if (!(data1->objectId == data2->objectId && data1->objectInstanceId == data2->objectInstanceId &&
          data1->resourceId == data2->resourceId && data1->resourceInstanceId == data2->resourceInstanceId)) {
        return false;
    }

    if (!(data1->dataType == data2->dataType)) {
        return false;
    }

    if (!(data1->timestamp == data2->timestamp)) {
        return false;
    }

    switch (data1->dataType) {
        case DATA_TYPE_NONE:
            return true;

        case DATA_TYPE_STRING:
            return strcmp(data1->strValue, data2->strValue) == 0;

        case DATA_TYPE_FLOAT:
            return data1->floatValue == data2->floatValue;

        case DATA_TYPE_BOOL:
            return data1->boolValue == data2->boolValue;

        case DATA_TYPE_OPAQUE:
            if (data1->opaqueValue.size != data2->opaqueValue.size) {
                return false;
            }

            for (size_t i = 0; i < data1->opaqueValue.size; i++) {
                if (data1->opaqueValue.data[i] != data2->opaqueValue.data[i]) {
                    return false;
                }
            }

            return true;

        case DATA_TYPE_OBJLINK:
            return data1->objlnkValue.objectId == data2->objlnkValue.objectId &&
                   data1->objlnkValue.objectInstanceId == data2->objlnkValue.objectInstanceId;

        default:
            return false;
    }
}

void testDecoding(JsonConverter* converter, const char* jsonData, DataConverter::ResourceData* testData1, size_t size)
{
    Status status = STS_OK;
    DataConverter::ResourceData data;

    status = converter->startDecoding("", reinterpret_cast<void*>(const_cast<char*>(jsonData)), strlen(jsonData));
    REQUIRE(status == STS_OK);

    for (size_t i = 0; i < size; i++) {
        REQUIRE(converter->nextDecoding(&data) == STS_OK);
        REQUIRE(compareResourceData(&testData1[i], &data));
    }

    REQUIRE(converter->nextDecoding(&data) == STS_ERR_NOT_EXIST);
}

TEST_CASE("test json converter", "[jsonconverter]")
{
    JsonConverter converter;

    /*******************************************************************************
     * Data 1
     ******************************************************************************/

    const char* jsonData1 =
        "[{\"bn\":\"/3/0/\",\"n\":\"0\",\"vs\":\"Open Mobile Alliance\"},\
          {\"n\":\"1\",\"vs\":\"Lightweight M2M Client\"},\
          {\"n\":\"2\",\"vs\":\"345000123\"},\
          {\"n\":\"3\",\"vs\":\"1.0\"},\
          {\"n\":\"6/0\",\"v\":1},\
          {\"n\":\"6/1\",\"v\":5},\
          {\"n\":\"7/0\",\"v\":3800},\
          {\"n\":\"7/1\",\"v\":5000},\
          {\"n\":\"8/0\",\"v\":125},\
          {\"n\":\"8/1\",\"v\":900},\
          {\"n\":\"9\",\"v\":100},\
          {\"n\":\"10\",\"v\":15},\
          {\"n\":\"11/0\",\"v\":0},\
          {\"n\":\"13\",\"v\":1367491215},\
          {\"n\":\"14\",\"vs\":\"+02:00\"},\
          {\"n\":\"16\",\"vs\":\"U\"}]";

    DataConverter::ResourceData testData1[16] = {
        {3, 0, 0, INVALID_ID, DATA_TYPE_STRING, 0, {.strValue = const_cast<char*>("Open Mobile Alliance")}},
        {3, 0, 1, INVALID_ID, DATA_TYPE_STRING, 0, {.strValue = const_cast<char*>("Lightweight M2M Client")}},
        {3, 0, 2, INVALID_ID, DATA_TYPE_STRING, 0, {.strValue = const_cast<char*>("345000123")}},
        {3, 0, 3, INVALID_ID, DATA_TYPE_STRING, 0, {.strValue = const_cast<char*>("1.0")}},
        {3, 0, 6, 0, DATA_TYPE_FLOAT, 0, {.floatValue = 1.0}},
        {3, 0, 6, 1, DATA_TYPE_FLOAT, 0, {.floatValue = 5}},
        {3, 0, 7, 0, DATA_TYPE_FLOAT, 0, {.floatValue = 3800}},
        {3, 0, 7, 1, DATA_TYPE_FLOAT, 0, {.floatValue = 5000}},
        {3, 0, 8, 0, DATA_TYPE_FLOAT, 0, {.floatValue = 125}},
        {3, 0, 8, 1, DATA_TYPE_FLOAT, 0, {.floatValue = 900}},
        {3, 0, 9, INVALID_ID, DATA_TYPE_FLOAT, 0, {.floatValue = 100}},
        {3, 0, 10, INVALID_ID, DATA_TYPE_FLOAT, 0, {.floatValue = 15}},
        {3, 0, 11, 0, DATA_TYPE_FLOAT, 0, {.floatValue = 0}},
        {3, 0, 13, INVALID_ID, DATA_TYPE_FLOAT, 0, {.floatValue = 1367491215}},
        {3, 0, 14, INVALID_ID, DATA_TYPE_STRING, 0, {.strValue = const_cast<char*>("+02:00")}},
        {3, 0, 16, INVALID_ID, DATA_TYPE_STRING, 0, {.strValue = const_cast<char*>("U")}},
    };

    testDecoding(&converter, jsonData1, testData1, sizeof(testData1) / sizeof(DataConverter::ResourceData));

    /*******************************************************************************
     * Data 2
     ******************************************************************************/

    const char* jsonData2 =
        "[{\"bn\":\"/72/\",\"bt\":25462634,\"n\":\"1/2\",\"v\":22.4,\"t\":-5},\
          {\"n\":\"1/2\",\"v\":22.9,\"t\":-30},\
          {\"n\":\"1/2\",\"v\":24.1,\"t\":-50}]";

    DataConverter::ResourceData testData2[3] = {
        {72, 1, 2, INVALID_ID, DATA_TYPE_FLOAT, 25462634 - 5, {.floatValue = 22.4}},
        {72, 1, 2, INVALID_ID, DATA_TYPE_FLOAT, 25462634 - 30, {.floatValue = 22.9}},
        {72, 1, 2, INVALID_ID, DATA_TYPE_FLOAT, 25462634 - 50, {.floatValue = 24.1}},
    };

    testDecoding(&converter, jsonData2, testData2, sizeof(testData2) / sizeof(DataConverter::ResourceData));

    /*******************************************************************************
     * Data 3
     ******************************************************************************/
    // TODO: shall we support VLO in format of FFFF:FFFF (Table: 7.4.3.-4)
    const char* jsonData3 =
        "[{\"bn\":\"/\",\"n\":\"65/0/0/0\",\"vlo\":\"66:0\"},\
          {\"n\":\"65/0/0/1\",\"vlo\":\"66:1\"},\
          {\"n\":\"65/0/1\",\"vs\":\"8613800755500\"},\
          {\"n\":\"65/0/2\",\"v\":1},\
          {\"n\":\"66/0/0\",\"vs\":\"myService1\"},\
          {\"n\":\"66/0/1\",\"vs\":\"Internet.15.234\"},\
          {\"n\":\"66/0/2\",\"vlo\":\"67:0\"},\
          {\"n\":\"66/1/0\",\"vs\":\"myService2\"},\
          {\"n\":\"66/1/1\",\"vs\":\"Internet.15.235\"},\
          {\"n\":\"66/1/2\",\"vlo\":\"65535:65535\"},\
          {\"n\":\"67/0/0\",\"vs\":\"85.76.76.84\"},\
          {\"n\":\"67/0/1\",\"vs\":\"85.76.255.255\"}]";

    DataConverter::ResourceData testData3[12] = {
        {65, 0, 0, 0, DATA_TYPE_OBJLINK, 0, {.objlnkValue = {66, 0}}},
        {65, 0, 0, 1, DATA_TYPE_OBJLINK, 0, {.objlnkValue = {66, 1}}},
        {65, 0, 1, INVALID_ID, DATA_TYPE_STRING, 0, {.strValue = const_cast<char*>("8613800755500")}},
        {65, 0, 2, INVALID_ID, DATA_TYPE_FLOAT, 0, {.floatValue = 1}},
        {66, 0, 0, INVALID_ID, DATA_TYPE_STRING, 0, {.strValue = const_cast<char*>("myService1")}},
        {66, 0, 1, INVALID_ID, DATA_TYPE_STRING, 0, {.strValue = const_cast<char*>("Internet.15.234")}},
        {66, 0, 2, INVALID_ID, DATA_TYPE_OBJLINK, 0, {.objlnkValue = {67, 0}}},
        {66, 1, 0, INVALID_ID, DATA_TYPE_STRING, 0, {.strValue = const_cast<char*>("myService2")}},
        {66, 1, 1, INVALID_ID, DATA_TYPE_STRING, 0, {.strValue = const_cast<char*>("Internet.15.235")}},
        {66, 1, 2, INVALID_ID, DATA_TYPE_OBJLINK, 0, {.objlnkValue = {0xFFFF, 0xFFFF}}},
        {67, 0, 0, INVALID_ID, DATA_TYPE_STRING, 0, {.strValue = const_cast<char*>("85.76.76.84")}},
        {67, 0, 1, INVALID_ID, DATA_TYPE_STRING, 0, {.strValue = const_cast<char*>("85.76.255.255")}},
    };

    testDecoding(&converter, jsonData3, testData3, sizeof(testData3) / sizeof(DataConverter::ResourceData));

    /*******************************************************************************
     * Data 4
     ******************************************************************************/

    const char* jsonData4 = "[{\"bn\":\"/3/0/0\",\"vs\":\"Open Mobile Alliance\"}]";

    DataConverter::ResourceData testData4[1] = {
        {3, 0, 0, INVALID_ID, DATA_TYPE_STRING, 0, {.strValue = const_cast<char*>("Open Mobile Alliance")}},
    };

    testDecoding(&converter, jsonData4, testData4, sizeof(testData4) / sizeof(DataConverter::ResourceData));

    /*******************************************************************************
     * Data 5
     ******************************************************************************/

    const char* jsonData5 = "[{\"n\":\"/3/0/0\"},{\"n\":\"/3/0/9\"},{\"n\":\"/1/0/1\"}]";

    DataConverter::ResourceData testData5[3] = {
        {3, 0, 0, INVALID_ID},
        {3, 0, 9, INVALID_ID},
        {1, 0, 1, INVALID_ID},
    };

    testDecoding(&converter, jsonData5, testData5, sizeof(testData5) / sizeof(DataConverter::ResourceData));

    /*******************************************************************************
     * Data 6
     ******************************************************************************/

    const char* jsonData6 =
        "[{\"n\":\"/3/0/0\", \"vs\":\"Open Mobile Alliance\"},\
          {\"n\":\"/3/0/9\", \"v\":95},\
          {\"n\":\"/1/0/1\", \"v\":86400}]";

    DataConverter::ResourceData testData6[3] = {
        {3, 0, 0, INVALID_ID, DATA_TYPE_STRING, 0, {.strValue = const_cast<char*>("Open Mobile Alliance")}},
        {3, 0, 9, INVALID_ID, DATA_TYPE_FLOAT, 0, {.floatValue = 95}},
        {1, 0, 1, INVALID_ID, DATA_TYPE_FLOAT, 0, {.floatValue = 86400}},
    };

    testDecoding(&converter, jsonData6, testData6, sizeof(testData6) / sizeof(DataConverter::ResourceData));

    /*******************************************************************************
     * Data 7
     ******************************************************************************/

    const char* jsonData7 =
        "[{\"n\":\"/3311/0/5850\", \"vb\":false},\
          {\"n\":\"/3311/1/5850\", \"vb\":false},\
          {\"n\":\"/3311/2/5851\", \"v\":20},\
          {\"n\":\"/3308/0/5900\", \"v\":18}]";

    DataConverter::ResourceData testData7[4] = {
        {3311, 0, 5850, INVALID_ID, DATA_TYPE_BOOL, 0, {.boolValue = 0}},
        {3311, 1, 5850, INVALID_ID, DATA_TYPE_BOOL, 0, {.boolValue = 0}},
        {3311, 2, 5851, INVALID_ID, DATA_TYPE_FLOAT, 0, {.floatValue = 20}},
        {3308, 0, 5900, INVALID_ID, DATA_TYPE_FLOAT, 0, {.floatValue = 18}},
    };

    testDecoding(&converter, jsonData7, testData7, sizeof(testData7) / sizeof(DataConverter::ResourceData));
}