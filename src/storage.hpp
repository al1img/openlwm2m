#ifndef OPENLWM2M_STORAGE_HPP_
#define OPENLWM2M_STORAGE_HPP_

#include <cstddef>

#include "status.hpp"

namespace openlwm2m {

class Storage {
public:
    Storage(size_t maxItems) : mMaxItems(maxItems), mSize(0), mMaxSize(0), mItems(new Item[mMaxItems]) {}
    ~Storage() { delete[] mItems; }

    void* begin(bool deleted = false)
    {
        mCurrentPos = 0;
        return next(deleted);
    }

    void* next(bool deleted = false)
    {
        for (size_t i = mCurrentPos; i < mMaxItems; i++) {
            if (deleted || !mItems[i].mDeleted) {
                mCurrentPos = i + 1;
                return mItems[i].mItem;
            }
        }

        return NULL;
    }

    Status pushItem(void* item)
    {
        for (size_t i = 0; i < mMaxItems; i++) {
            if (!mItems[i].mItem) {
                mItems[i].mItem = item;
                mMaxSize++;
                return STS_OK;
            }
        }

        return STS_ERR_MEM;
    }

    void* newItem(Status* status = NULL)
    {
        for (size_t i = 0; i < mMaxItems; i++) {
            if (mItems[i].mDeleted && mItems[i].mItem) {
                mItems[i].mDeleted = false;
                mSize++;
                return mItems[i].mItem;
            }
        }

        if (status) {
            *status = STS_ERR_MEM;
        }

        return NULL;
    }

    Status deleteItem(void* item)
    {
        for (size_t i = 0; i < mMaxItems; i++) {
            if (mItems[i].mItem == item && item) {
                mItems[i].mDeleted = true;
                mSize--;
                return STS_OK;
            }
        }

        return STS_ERR_EXIST;
    }

    size_t size() const { return mSize; }

    size_t maxSize() const { return mMaxSize; }

private:
    struct Item {
        Item() : mItem(NULL), mDeleted(true) {}
        void* mItem;
        bool mDeleted;
    };

    size_t mMaxItems;
    size_t mCurrentPos;
    size_t mSize;
    size_t mMaxSize;
    Item* mItems;
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_STORAGE_HPP_ */