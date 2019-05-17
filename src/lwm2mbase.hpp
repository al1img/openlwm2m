#ifndef OPENLWM2M_LWM2MBASE_HPP_
#define OPENLWM2M_LWM2MBASE_HPP_

#include <stdint.h>

namespace openlwm2m {

class Lwm2mBase {
public:
    Lwm2mBase* getParent() const { return mParent; }
    uint16_t getId() const { return mId; }

protected:
    Lwm2mBase(Lwm2mBase* parent, uint16_t id) : mParent(parent), mId(id) {}

private:
    Lwm2mBase* mParent;
    uint16_t mId;
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_LWM2MBASE_HPP_ */