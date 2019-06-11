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
    ItemBase(ItemBase* parent, uint16_t id) : mParent(parent), mId(id) {}

private:
    template <class, class>
    friend class StorageItem;

    template <class, class>
    friend class StorageArray;

    ItemBase* mParent;
    uint16_t mId;

    void setId(uint16_t id) { mId = id; }
    void setParent(ItemBase* parent) { mParent = parent; }
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_ITEMBASE_HPP_ */
