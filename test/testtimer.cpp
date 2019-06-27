#include <climits>

#include <catch2/catch.hpp>

#include "timer.hpp"

using namespace openlwm2m;

static bool sTimerIsFired = false;

Status timerCallback(void* context)
{
    sTimerIsFired = true;

    return STS_OK;
}

TEST_CASE("test timer", "[timer]")
{
    Timer timer;
    uint64_t pollInMs = ULONG_MAX;
    uint64_t currentTimeMs = 0;

    sTimerIsFired = false;

    SECTION("test one shot")
    {
        timer.start(100, timerCallback, nullptr, true);

        timer.poll(currentTimeMs, &pollInMs);

        REQUIRE(pollInMs == 100);

        currentTimeMs += pollInMs;
        pollInMs = ULONG_MAX;

        timer.poll(currentTimeMs, &pollInMs);

        REQUIRE(sTimerIsFired);
        REQUIRE(pollInMs == ULONG_MAX);

        sTimerIsFired = false;

        currentTimeMs += 200;

        timer.poll(currentTimeMs, &pollInMs);

        REQUIRE(!sTimerIsFired);
        REQUIRE(pollInMs == ULONG_MAX);
    }
}