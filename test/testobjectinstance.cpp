#include <catch2/catch.hpp>

#include "object.hpp"
#include "objectinstance.hpp"

using namespace openlwm2m;

TEST_CASE("single instance mandatory object", "[objectinstance]")
{
    Status status = STS_OK;

    Object object(2, ITF_ALL, true, true);

    status = object.createResourceString(2, OP_READWRITE, true, true);
    CHECK(status == STS_OK);

    status = object.createResourceInt(3, OP_READWRITE, true, true);
    CHECK(status == STS_OK);

    status = object.createResourceUint(4, OP_READWRITE, true, true);
    CHECK(status == STS_OK);

    status = object.createResourceBool(5, OP_READWRITE, true, true);
    CHECK(status == STS_OK);

    status = object.createResourceOpaque(6, OP_READWRITE, true, true);
    CHECK(status == STS_OK);

    status = object.createResourceNone(7, OP_EXECUTE, true, true);
    CHECK(status == STS_OK);

    object.init();
}
