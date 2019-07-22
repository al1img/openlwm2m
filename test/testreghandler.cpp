#include <catch2/catch.hpp>

#include "jsonconverter.hpp"
#include "objectmanager.hpp"
#include "reghandler.hpp"

using namespace openlwm2m;

void pollRequest()
{
}

TEST_CASE("test reg handler", "[reghandler]")
{
    Status status = STS_OK;
    ObjectManager objectManager;

    objectManager.addConverter(new JsonConverter());

    Object* object =
        objectManager.createObject(OBJ_LWM2M_SERVER, Object::MULTIPLE, CONFIG_NUM_SERVERS, Object::MANDATORY, ITF_ALL);
    REQUIRE(object != NULL);

    status = object->createResourceInt(RES_SHORT_SERVER_ID, ResourceDesc::OP_READ, ResourceDesc::SINGLE, 0,
                                       ResourceDesc::MANDATORY, 1, 65535);
    REQUIRE(status == STS_OK);

    status = objectManager.write(ITF_BOOTSTRAP, DATA_FMT_SENML_JSON, "/1/0",
                                 reinterpret_cast<void*>(const_cast<char*>("{}")), 0);

    RegHandler regHandler(NULL, (RegHandler::Params){objectManager, "TestClient", false, pollRequest});

    regHandler.setId(0);

    regHandler.init();

    regHandler.release();
}