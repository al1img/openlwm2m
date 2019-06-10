#ifndef OPENLWM2M_TIMER_HPP_
#define OPENLWM2M_TIMER_HPP_

#include "itembase.hpp"
#include "storage.hpp"

namespace openlwm2m {

class Timer {
public:
    static Status poll(uint64_t currentTimeMs, uint64_t* poolInMs);

    Timer();
    virtual ~Timer();

    void start(uint64_t period, bool oneShot = false, bool start = false);
    void stop();

private:
    static List<Timer> sTimerList;
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_TIMER_HPP_ */