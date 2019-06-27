#include <climits>

#include <catch2/catch.hpp>

#include "timer.hpp"

using namespace openlwm2m;

static bool timer1IsFired = false;
static bool timer2IsFired = false;
static bool timer3IsFired = false;

Status timer1Callback(void* context)
{
    timer1IsFired = true;

    return STS_OK;
}

Status timer2Callback(void* context)
{
    timer2IsFired = true;

    return STS_OK;
}

Status timer3Callback(void* context)
{
    timer3IsFired = true;

    return STS_OK;
}

TEST_CASE("test timer", "[timer]")
{
    Timer timer1(1), timer2(2), timer3(3);
    uint64_t pollInMs = ULONG_MAX;
    uint64_t currentTimeMs = 0;

    timer1IsFired = false;
    timer2IsFired = false;
    timer3IsFired = false;

    SECTION("test one shot")
    {
        timer1.start(100, timer1Callback, nullptr, true);
        timer2.start(200, timer2Callback, nullptr, true);
        timer3.start(300, timer3Callback, nullptr, true);

        Timer::poll(currentTimeMs, &pollInMs);

        REQUIRE(pollInMs == 100);

        currentTimeMs += pollInMs;
        pollInMs = ULONG_MAX;

        Timer::poll(currentTimeMs, &pollInMs);

        REQUIRE(pollInMs == 100);
        REQUIRE((timer1IsFired && !timer2IsFired && !timer3IsFired));

        timer1IsFired = false;

        currentTimeMs += pollInMs;
        pollInMs = ULONG_MAX;

        Timer::poll(currentTimeMs, &pollInMs);

        REQUIRE(pollInMs == 100);
        REQUIRE((!timer1IsFired && timer2IsFired && !timer3IsFired));

        timer2IsFired = false;

        currentTimeMs += pollInMs;
        pollInMs = ULONG_MAX;

        Timer::poll(currentTimeMs, &pollInMs);

        REQUIRE(pollInMs == ULONG_MAX);
        REQUIRE((!timer1IsFired && !timer2IsFired && timer3IsFired));

        timer3IsFired = false;

        currentTimeMs += 1000;

        REQUIRE(pollInMs == ULONG_MAX);
        REQUIRE((!timer1IsFired && !timer2IsFired && !timer3IsFired));
    }
}