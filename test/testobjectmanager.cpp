#include <catch2/catch.hpp>

#include <cstring>

#include "jsonconverter.hpp"
#include "objectmanager.hpp"

using namespace openlwm2m;

TEST_CASE("test bootstrap write", "[objectmanager]")
{
    Status status = STS_OK;
    ObjectManager objectManager;

    objectManager.addConverter(new JsonConverter());

    objectManager.init();

    const char* jsonData =
        "[{\"n\":\"/1/0/0\",\"v\":1},\
          {\"n\":\"/1/0/1\",\"v\":10},\
          {\"n\":\"/1/0/6\",\"vb\":true},\
          {\"n\":\"/1/0/7\",\"vs\":\"U\"}\
        ]";

    status = objectManager.write(ITF_BOOTSTRAP, DATA_FMT_SENML_JSON, "",
                                 reinterpret_cast<void*>(const_cast<char*>(jsonData)), strlen(jsonData));

    REQUIRE(status == STS_OK);
}