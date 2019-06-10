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

Timer* Timer::createTimer(uint64_t period, bool oneShot, bool start)
{
    Params params = {period, oneShot, start};

    return mTimerStorage.createItem(INVALID_ID, params);
}

/*******************************************************************************
 * Private
 ******************************************************************************/

Timer::Timer(ItemBase* parent, uint16_t id, Params params) : ItemBase(parent, id), mParams(params)
{
}

void Timer::init()
{
}

void Timer::release()
{
}

}  // namespace openlwm2m