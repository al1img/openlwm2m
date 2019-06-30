#ifndef OPENLWM2M_ITEMBASE_HPP_
#define OPENLWM2M_ITEMBASE_HPP_

#include <stdint.h>

namespace openlwm2m {

#define INVALID_ID 0xFFFF

class ItemBase {
public:
    ItemBase* getParent() const { return mParent; }
    uint16_t getId() const { return mId; }

protected:
    ItemBase(ItemBase* parent) : mParent(parent), mId(INVALID_ID) {}

private:
    template <class, class>
    friend class Lwm2mDynamicStorage;
    template <class, class>
    friend class Lwm2mStorage;

    ItemBase* mParent;
    uint16_t mId;

    void setId(uint16_t id) { mId = id; }
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_ITEMBASE_HPP_ */
