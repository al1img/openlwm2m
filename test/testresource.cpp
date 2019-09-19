#include <catch2/catch.hpp>

#include "resource.hpp"

using namespace openlwm2m;

TEST_CASE("test resource instance", "[resource]")
{
    

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

    DataConverter::ResourceData testData1[] = {
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

    testDecoding(&converter, "", reinterpret_cast<void*>(const_cast<char*>(jsonData1)), strlen(jsonData1), testData1,
                 sizeof(testData1) / sizeof(DataConverter::ResourceData));
    testEncoding(&converter, reinterpret_cast<void*>(const_cast<char*>(jsonData1)), strlen(jsonData1), testData1,
                 sizeof(testData1) / sizeof(DataConverter::ResourceData));

    /*******************************************************************************
     * Data 2
     ******************************************************************************/

    const char* jsonData2 =
        "[{\"bn\":\"/72/1/2\",\"bt\":25462629,\"v\":22.4,\"t\":0},\
{\"v\":22.9,\"t\":-25},\
{\"v\":24.1,\"t\":-45}]";

    DataConverter::ResourceData testData2[] = {
        {72, 1, 2, INVALID_ID, DATA_TYPE_FLOAT, 25462629, {.floatValue = 22.4}},
        {72, 1, 2, INVALID_ID, DATA_TYPE_FLOAT, 25462629 - 25, {.floatValue = 22.9}},
        {72, 1, 2, INVALID_ID, DATA_TYPE_FLOAT, 25462629 - 45, {.floatValue = 24.1}},
    };

    testDecoding(&converter, "", reinterpret_cast<void*>(const_cast<char*>(jsonData2)), strlen(jsonData2), testData2,
                 sizeof(testData2) / sizeof(DataConverter::ResourceData));
    testEncoding(&converter, reinterpret_cast<void*>(const_cast<char*>(jsonData2)), strlen(jsonData2), testData2,
                 sizeof(testData2) / sizeof(DataConverter::ResourceData));

    /*******************************************************************************
     * Data 3
     ******************************************************************************/
    // TODO: shall we support VLO in format of FFFF:FFFF (Table: 7.4.3.-4)
    const char* jsonData3 =
        "[{\"bn\":\"/65/0/0/\",\"n\":\"0\",\"vlo\":\"66:0\"},\
{\"n\":\"1\",\"vlo\":\"66:1\"},\
{\"bn\":\"/65/0/\",\"n\":\"1\",\"vs\":\"8613800755500\"},\
{\"n\":\"2\",\"v\":1},\
{\"bn\":\"/66/0/\",\"n\":\"0\",\"vs\":\"myService1\"},\
{\"n\":\"1\",\"vs\":\"Internet.15.234\"},\
{\"n\":\"2\",\"vlo\":\"67:0\"},\
{\"bn\":\"/66/1/\",\"n\":\"0\",\"vs\":\"myService2\"},\
{\"n\":\"1\",\"vs\":\"Internet.15.235\"},\
{\"n\":\"2\",\"vlo\":\"65535:65535\"},\
{\"bn\":\"/67/0/\",\"n\":\"0\",\"vs\":\"85.76.76.84\"},\
{\"n\":\"1\",\"vs\":\"85.76.255.255\"}]";

    DataConverter::ResourceData testData3[] = {
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

    testDecoding(&converter, "", reinterpret_cast<void*>(const_cast<char*>(jsonData3)), strlen(jsonData3), testData3,
                 sizeof(testData3) / sizeof(DataConverter::ResourceData));
    testEncoding(&converter, reinterpret_cast<void*>(const_cast<char*>(jsonData3)), strlen(jsonData3), testData3,
                 sizeof(testData3) / sizeof(DataConverter::ResourceData));

    /*******************************************************************************
     * Data 4
     ******************************************************************************/

    const char* jsonData4 = "[{\"bn\":\"/3/0/0\",\"vs\":\"Open Mobile Alliance\"}]";

    DataConverter::ResourceData testData4[] = {
        {3, 0, 0, INVALID_ID, DATA_TYPE_STRING, 0, {.strValue = const_cast<char*>("Open Mobile Alliance")}},
    };

    testDecoding(&converter, "", reinterpret_cast<void*>(const_cast<char*>(jsonData4)), strlen(jsonData4), testData4,
                 sizeof(testData4) / sizeof(DataConverter::ResourceData));
    testEncoding(&converter, reinterpret_cast<void*>(const_cast<char*>(jsonData4)), strlen(jsonData4), testData4,
                 sizeof(testData4) / sizeof(DataConverter::ResourceData));

    /*******************************************************************************
     * Data 5
     ******************************************************************************/

    const char* jsonData5 = "[{\"bn\":\"/3/0/\",\"n\":\"0\"},{\"n\":\"9\"},{\"bn\":\"/1/0/1\"}]";

    DataConverter::ResourceData testData5[] = {
        {3, 0, 0, INVALID_ID},
        {3, 0, 9, INVALID_ID},
        {1, 0, 1, INVALID_ID},
    };

    testDecoding(&converter, "", reinterpret_cast<void*>(const_cast<char*>(jsonData5)), strlen(jsonData5), testData5,
                 sizeof(testData5) / sizeof(DataConverter::ResourceData));
    testEncoding(&converter, reinterpret_cast<void*>(const_cast<char*>(jsonData5)), strlen(jsonData5), testData5,
                 sizeof(testData5) / sizeof(DataConverter::ResourceData));

    /*******************************************************************************
     * Data 6
     ******************************************************************************/

    const char* jsonData6 =
        "[{\"bn\":\"/3/0/\",\"n\":\"0\",\"vs\":\"Open Mobile Alliance\"},\
{\"n\":\"9\",\"v\":95},\
{\"bn\":\"/1/0/1\",\"v\":86400}]";

    DataConverter::ResourceData testData6[] = {
        {3, 0, 0, INVALID_ID, DATA_TYPE_STRING, 0, {.strValue = const_cast<char*>("Open Mobile Alliance")}},
        {3, 0, 9, INVALID_ID, DATA_TYPE_FLOAT, 0, {.floatValue = 95}},
        {1, 0, 1, INVALID_ID, DATA_TYPE_FLOAT, 0, {.floatValue = 86400}},
    };

    testDecoding(&converter, "", reinterpret_cast<void*>(const_cast<char*>(jsonData6)), strlen(jsonData6), testData6,
                 sizeof(testData6) / sizeof(DataConverter::ResourceData));
    testEncoding(&converter, reinterpret_cast<void*>(const_cast<char*>(jsonData6)), strlen(jsonData6), testData6,
                 sizeof(testData6) / sizeof(DataConverter::ResourceData));

    /*******************************************************************************
     * Data 7
     ******************************************************************************/

    const char* jsonData7 =
        "[{\"bn\":\"/3311/\",\"n\":\"0/5850\",\"vb\":false},\
{\"n\":\"1/5850\",\"vb\":false},\
{\"n\":\"2/5851\",\"v\":20},\
{\"bn\":\"/3308/0/5900\",\"v\":18}]";

    DataConverter::ResourceData testData7[] = {
        {3311, 0, 5850, INVALID_ID, DATA_TYPE_BOOL, 0, {.boolValue = 0}},
        {3311, 1, 5850, INVALID_ID, DATA_TYPE_BOOL, 0, {.boolValue = 0}},
        {3311, 2, 5851, INVALID_ID, DATA_TYPE_FLOAT, 0, {.floatValue = 20}},
        {3308, 0, 5900, INVALID_ID, DATA_TYPE_FLOAT, 0, {.floatValue = 18}},
    };

    testDecoding(&converter, "", reinterpret_cast<void*>(const_cast<char*>(jsonData7)), strlen(jsonData7), testData7,
                 sizeof(testData7) / sizeof(DataConverter::ResourceData));
    testEncoding(&converter, reinterpret_cast<void*>(const_cast<char*>(jsonData7)), strlen(jsonData7), testData7,
                 sizeof(testData7) / sizeof(DataConverter::ResourceData));
}
