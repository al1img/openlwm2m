#include "timer.hpp"

#include <climits>

#include "log.hpp"

#define LOG_MODULE "Timer"

namespace openlwm2m {

/*******************************************************************************
 * Public
 ******************************************************************************/

Status Timer::poll(uint64_t currentTimeMs, uint64_t* poolInMs)
{
    LOG_DEBUG("Poll, current time: %lu", currentTimeMs);

    uint64_t timerPoolInMs = ULONG_MAX;
    Status retStatus = STS_OK;

    Node<Timer>* node = sTimerList.begin();

    while (node) {
        Status status = node->get()->processTimer(currentTimeMs, &timerPoolInMs);

        if (status != STS_OK && retStatus == STS_OK) {
            retStatus = status;
        }

        node = node->next();
    }

    if (poolInMs && timerPoolInMs < *poolInMs) {
        *poolInMs = timerPoolInMs;
    }

    return retStatus;
}

Timer::Timer(uint16_t id)
    : mId(id), mPeriod(0), mStarted(false), mOneShot(false), mCallback(NULL), mContext(NULL), mFireAt(0)
{
    Node<Timer>* node = new Node<Timer>(this);

    sTimerList.insertEnd(node);
}

Timer::~Timer()
{
    Node<Timer>* node = sTimerList.remove(this);

    delete node;
}

void Timer::start(uint64_t period, TimerCallback callback, void* context, bool oneShot)
{
    LOG_DEBUG("Start %d, period: %lu, oneshot: %d", mId, period, oneShot);

    mPeriod = period;
    mCallback = callback;
    mContext = context;
    mOneShot = oneShot;
    mStarted = true;
    mFireAt = 0;
}

void Timer::stop()
{
    LOG_DEBUG("Stop %d", mId);

    mStarted = false;
}

/*******************************************************************************
 * Private
 ******************************************************************************/

List<Timer> Timer::sTimerList;

Status Timer::processTimer(uint64_t currentTimeMs, uint64_t* poolInMs)
{
    Status status = STS_OK;

    if (!mStarted) {
        return status;
    }

    // Initialize just started timer
    if (mFireAt == 0) {
        mFireAt = currentTimeMs + mPeriod;

        LOG_DEBUG("Timer %d will fire at: %lu", mId, mFireAt);
    }

    if (currentTimeMs >= mFireAt) {
        LOG_DEBUG("Timer %d fired", mId);

        status = mCallback(mContext);

        if (mOneShot) {
            stop();

            return status;
        }

        mFireAt = currentTimeMs + mPeriod;

        LOG_DEBUG("Timer %d will fire at: %lu", mId, mFireAt);
    }

    uint64_t poolIn = mFireAt - currentTimeMs;

    if (poolInMs && poolIn < *poolInMs) {
        *poolInMs = poolIn;
    }

    return status;
}

}  // namespace openlwm2m