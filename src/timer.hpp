#ifndef OPENLWM2M_TIMER_HPP_
#define OPENLWM2M_TIMER_HPP_

#include "itembase.hpp"
#include "storage.hpp"

namespace openlwm2m {

class Timer {
public:
    /**
     * Run all timers.
     *
     * @retval Status.
     */
    static Status run();

    /**
     * Timer callback.
     */
    typedef Status (*TimerCallback)(void* context);

    /**
     * Timer constructor.
     *
     * @param[in] id Timer ID.
     */
    Timer(uint16_t id);

    /**
     * Timer destructor.
     */
    ~Timer();

    /**
     * Sets timer ID.
     *
     * @param[in] id Timer ID.
     */
    void setId(uint16_t id) { mId = id; }

    /**
     * Starts the timer.
     *
     * @param[in] periodMs Timer period in msec.
     * @param[in] callback Callback which will be called when the timer is fired.
     * @param[in] context  Context which will be passed to the callback.
     * @param[in] oneShot  If true, the timer will be fired only once.
     */
    void start(uint64_t periodMs, TimerCallback callback, void* context = NULL, bool oneShot = false);

    /**
     * Restarts the timer.
     */
    void restart();

    /**
     * Stops the timer.
     */
    void stop();

private:
    static List<Timer> sTimerList;

    uint16_t mId;
    uint64_t mPeriod;
    bool mStarted;
    bool mOneShot;
    TimerCallback mCallback;
    void* mContext;
    uint64_t mFireAt;

    Status processTimer(uint64_t currentTimeMs);
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_TIMER_HPP_ */