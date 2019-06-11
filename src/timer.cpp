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

    LOG_DEBUG("Poll in: %lu", timerPoolInMs);

    return retStatus;
}

Timer::Timer() : mPeriod(0), mStarted(false), mOneShot(false), mCallback(NULL), mContext(NULL), mFireAt(0)
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
    mPeriod = period;
    mCallback = callback;
    mContext = context;
    mOneShot = oneShot;
    mStarted = true;
    mFireAt = ULONG_MAX;
}

void Timer::stop()
{
    mPeriod = 0;
    mCallback = NULL;
    mContext = NULL;
    mOneShot = false;
    mStarted = false;
    mFireAt = ULONG_MAX;
}

/*******************************************************************************
 * Private
 ******************************************************************************/

List<Timer> Timer::sTimerList;

Status Timer::processTimer(uint64_t currentTimeMs, uint64_t* poolInMs)
{
    Status status = STS_OK;

    if (mStarted) {
        if (currentTimeMs >= mFireAt) {
            status = mCallback(mContext);

            if (mOneShot) {
                stop();
            }
        }
    }

    if (mStarted) {
        mFireAt = currentTimeMs + mPeriod;

        if (poolInMs && mPeriod < *poolInMs) {
            *poolInMs = mPeriod;
        }
    }

    return status;
}

}  // namespace openlwm2m