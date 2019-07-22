#ifndef OPENLWM2M_ITEMBASE_HPP_
#define OPENLWM2M_ITEMBASE_HPP_

#include <stdint.h>

namespace openlwm2m {

#define INVALID_ID 0xFFFF

class ItemBase {
public:
    ItemBase* getParent() const { return mParent; }
    uint16_t getId() const { return mId; }
    void setId(uint16_t id) { mId = id; }

protected:
    ItemBase(ItemBase* parent) : mParent(parent), mId(INVALID_ID) {}

private:
    ItemBase* mParent;
    uint16_t mId;
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_ITEMBASE_HPP_ */
