#include <climits>

#include <catch2/catch.hpp>

#include "testplatform.hpp"
#include "timer.hpp"

using namespace openlwm2m;

static bool timerIsFired;

Status timerCallback(void* context)
{
    timerIsFired = true;

    return STS_OK;
}

static void run(uint64_t currentTimeMs)
{
    setCurrentTime(currentTimeMs);
    REQUIRE(Timer::run() == STS_OK);
}

TEST_CASE("test timer", "[timer]")
{
    Timer timer(0);

    timerIsFired = false;

    SECTION("test one shot")
    {
        run(0);

        timer.start(100, timerCallback, nullptr, true);

        // time 100
        run(100);
        REQUIRE(timerIsFired);
        timerIsFired = false;

        // time 500
        run(500);
        REQUIRE_FALSE(timerIsFired);
    }

    SECTION("test periodic")
    {
        run(0);

        timer.start(100, timerCallback);

        for (int i = 0; i < 10; i++) {
            run((i + 1) * 100);
            REQUIRE(timerIsFired);
            timerIsFired = false;
        }
    }
}
