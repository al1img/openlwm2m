#include "timer.hpp"

#include <climits>

#include "log.hpp"
#include "platform.hpp"

#define LOG_MODULE "Timer"

namespace openlwm2m {

/*******************************************************************************
 * Public
 ******************************************************************************/

Status Timer::run()
{
    Status retStatus = STS_OK;

    uint64_t currentTimeMs = Platform::getCurrentTime();

    Node<Timer>* node = sTimerList.begin();

    while (node) {
        Status status = node->get()->processTimer(currentTimeMs);

        if (status != STS_OK && retStatus == STS_OK) {
            retStatus = status;
        }

        node = node->next();
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

void Timer::start(uint64_t periodMs, TimerCallback callback, void* context, bool oneShot)
{
    LOG_DEBUG("Start %d, period: %lu, oneshot: %d", mId, periodMs, oneShot);

    mPeriod = periodMs;
    mCallback = callback;
    mContext = context;
    mOneShot = oneShot;
    mStarted = true;
    mFireAt = Platform::getCurrentTime() + periodMs;
}

void Timer::restart()
{
    LOG_DEBUG("Restart %d", mId);

    mFireAt = Platform::getCurrentTime() + mPeriod;
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

Status Timer::processTimer(uint64_t currentTimeMs)
{
    Status status = STS_OK;

    if (!mStarted) {
        return status;
    }

    if (currentTimeMs >= mFireAt) {
        LOG_DEBUG("Timer %d fired", mId);

        if (mOneShot) {
            stop();

            status = mCallback(mContext);

            return status;
        }

        status = mCallback(mContext);

        mFireAt = currentTimeMs + mPeriod;

        LOG_DEBUG("Timer %d will fire at: %lu", mId, mFireAt);
    }

    return status;
}

}  // namespace openlwm2m
