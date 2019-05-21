#ifndef OPENLWM2M_LWM2MBASE_HPP_
#define OPENLWM2M_LWM2MBASE_HPP_

#include <stdint.h>

namespace openlwm2m {

#define LWM2M_INVALID_ID 0xFFFF

class Lwm2mBase {
public:
    Lwm2mBase* getParent() const { return mParent; }
    uint16_t getId() const { return mId; }

protected:
    Lwm2mBase(Lwm2mBase* parent, uint16_t id) : mParent(parent), mId(id) {}

    virtual void create() {}
    virtual void release() {}

private:
    template <class, class>
    friend class Lwm2mStorage;

    Lwm2mBase* mParent;
    uint16_t mId;

    void setId(uint16_t id) { mId = id; }
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_LWM2MBASE_HPP_ */