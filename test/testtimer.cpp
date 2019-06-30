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
    uint64_t pollTimeMs = ULONG_MAX;
    uint64_t currentTimeMs = 0;

    timer1IsFired = false;
    timer2IsFired = false;
    timer3IsFired = false;

    SECTION("test one shot")
    {
        timer1.start(100, timer1Callback, nullptr, true);
        timer2.start(200, timer2Callback, nullptr, true);
        timer3.start(300, timer3Callback, nullptr, true);

        // time 0
        Timer::poll(currentTimeMs, &pollTimeMs);

        REQUIRE(pollTimeMs == currentTimeMs + 100);

        currentTimeMs = pollTimeMs;
        pollTimeMs = ULONG_MAX;

        // time 100
        Timer::poll(currentTimeMs, &pollTimeMs);

        REQUIRE(pollTimeMs == currentTimeMs + 100);
        REQUIRE((timer1IsFired && !timer2IsFired && !timer3IsFired));

        timer1IsFired = false;

        currentTimeMs = pollTimeMs;
        pollTimeMs = ULONG_MAX;

        // time 200
        Timer::poll(currentTimeMs, &pollTimeMs);

        REQUIRE(pollTimeMs == currentTimeMs + 100);
        REQUIRE((!timer1IsFired && timer2IsFired && !timer3IsFired));

        timer2IsFired = false;

        currentTimeMs = pollTimeMs;
        pollTimeMs = ULONG_MAX;

        // time 300
        Timer::poll(currentTimeMs, &pollTimeMs);

        REQUIRE(pollTimeMs == ULONG_MAX);
        REQUIRE((!timer1IsFired && !timer2IsFired && timer3IsFired));

        timer3IsFired = false;

        currentTimeMs += 1000;
        pollTimeMs = ULONG_MAX;

        Timer::poll(currentTimeMs, &pollTimeMs);

        // time 1300
        REQUIRE(pollTimeMs == ULONG_MAX);
        REQUIRE((!timer1IsFired && !timer2IsFired && !timer3IsFired));
    }

    SECTION("test periodic")
    {
        timer1.start(100, timer1Callback);
        timer2.start(200, timer2Callback);
        timer3.start(300, timer3Callback);

        // time 0
        Timer::poll(currentTimeMs, &pollTimeMs);

        REQUIRE(pollTimeMs == currentTimeMs + 100);

        currentTimeMs = pollTimeMs;
        pollTimeMs = ULONG_MAX;

        for (int i = 0; i < 5; i++) {
            // time 100: timer1
            Timer::poll(currentTimeMs, &pollTimeMs);

            REQUIRE(pollTimeMs == currentTimeMs + 100);
            REQUIRE((timer1IsFired && !timer2IsFired && !timer3IsFired));

            timer1IsFired = false;

            currentTimeMs = pollTimeMs;
            pollTimeMs = ULONG_MAX;

            // time 200 timer1, timer2
            Timer::poll(currentTimeMs, &pollTimeMs);

            REQUIRE(pollTimeMs == currentTimeMs + 100);
            REQUIRE((timer1IsFired && timer2IsFired && !timer3IsFired));

            timer1IsFired = false;
            timer2IsFired = false;

            currentTimeMs = pollTimeMs;
            pollTimeMs = ULONG_MAX;

            // time 300 timer1, timer3
            Timer::poll(currentTimeMs, &pollTimeMs);

            REQUIRE(pollTimeMs == currentTimeMs + 100);
            REQUIRE((timer1IsFired && !timer2IsFired && timer3IsFired));

            timer1IsFired = false;
            timer3IsFired = false;

            currentTimeMs = pollTimeMs;
            pollTimeMs = ULONG_MAX;

            // time 400 timer 1, timer2
            Timer::poll(currentTimeMs, &pollTimeMs);

            REQUIRE(pollTimeMs == currentTimeMs + 100);
            REQUIRE((timer1IsFired && timer2IsFired && !timer3IsFired));

            timer1IsFired = false;
            timer2IsFired = false;

            currentTimeMs = pollTimeMs;
            pollTimeMs = ULONG_MAX;

            // time 500 timer 1
            Timer::poll(currentTimeMs, &pollTimeMs);

            REQUIRE(pollTimeMs == currentTimeMs + 100);
            REQUIRE((timer1IsFired && !timer2IsFired && !timer3IsFired));

            timer1IsFired = false;

            currentTimeMs = pollTimeMs;
            pollTimeMs = ULONG_MAX;

            // time 600 timer 1, timer2, timer3
            Timer::poll(currentTimeMs, &pollTimeMs);

            REQUIRE(pollTimeMs == currentTimeMs + 100);
            REQUIRE((timer1IsFired && timer2IsFired && timer3IsFired));

            timer1IsFired = false;
            timer2IsFired = false;
            timer3IsFired = false;

            currentTimeMs = pollTimeMs;
            pollTimeMs = ULONG_MAX;
        }
    }
}
