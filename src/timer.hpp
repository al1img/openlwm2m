#ifndef OPENLWM2M_TIMER_HPP_
#define OPENLWM2M_TIMER_HPP_

#include "itembase.hpp"
#include "storage.hpp"

namespace openlwm2m {

class Timer {
public:
    static Status poll(uint64_t currentTimeMs, uint64_t* poolInMs);

    typedef Status (*TimerCallback)(void* context);

    Timer();
    ~Timer();

    void start(uint64_t period, TimerCallback callback, void* context, bool oneShot = false);
    void stop();

private:
    static List<Timer> sTimerList;

    uint64_t mPeriod;
    bool mStarted;
    bool mOneShot;
    TimerCallback mCallback;
    void* mContext;
    uint64_t mFireAt;

    Status processTimer(uint64_t currentTimeMs, uint64_t* poolInMs);
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_TIMER_HPP_ */