#include "timer.hpp"
#include "log.hpp"

#define LOG_MODULE "Timer"

namespace openlwm2m {

/*******************************************************************************
 * Public
 ******************************************************************************/

Status Timer::poll(uint64_t currentTimeMs, uint64_t* poolInMs)
{
    LOG_DEBUG("Poll at: %lu", currentTimeMs);

    uint64_t timerPoolInMs = 0;

    if (poolInMs && timerPoolInMs < *poolInMs) {
        *poolInMs = timerPoolInMs;
    }

    LOG_DEBUG("Poll in: %lu", timerPoolInMs);

    return STS_OK;
}

Timer::Timer()
{
}

Timer::~Timer()
{
}

/*******************************************************************************
 * Private
 ******************************************************************************/

List<Timer> Timer::sTimerList;

}  // namespace openlwm2m