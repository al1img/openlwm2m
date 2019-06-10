#ifndef OPENLWM2M_TIMER_HPP_
#define OPENLWM2M_TIMER_HPP_

#include "itembase.hpp"
#include "storage.hpp"

namespace openlwm2m {

class Timer : public ItemBase {
public:
    static Status poll(uint64_t currentTimeMs, uint64_t* poolInMs);

    static Timer* createTimer(uint64_t period, bool oneShot = false, bool start = false);

    void start(bool oneShot = false);
    void stop();

private:
    struct Params {
        bool started;
        bool oneShot;
        uint64_t period;
    };

    friend class StorageBase<Timer>;
    friend class StorageArray<Timer, Params>;

    static StorageArray<Timer, Params> mTimerStorage;

    Params mParams;
    uint64_t mArmedAt;

    Timer(ItemBase* parent, uint16_t id, Params params);

    void init();
    void release();
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_TIMER_HPP_ */