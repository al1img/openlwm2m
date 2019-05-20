#ifndef OPENLWM2M_STORAGE_HPP_
#define OPENLWM2M_STORAGE_HPP_

#include <cstddef>

#include "config.hpp"
#include "log.hpp"
#include "lwm2mbase.hpp"
#include "status.hpp"

namespace openlwm2m {

template <class T>
class Node {
public:
    Node(T* data) : mData(data), mNext(NULL) {}
    T* get() const { return mData; }
    Node* next() const { return mNext; }

private:
    template <class, class>
    friend class Lwm2mStorage;

    T* mData;
    Node* mNext;
};

template <class T, class P>
class Lwm2mStorage {
public:
    Lwm2mStorage(Lwm2mBase* parent, P param, size_t maxItems)
        : mParent(parent), mMaxItems(maxItems), mSize(0), mStart(NULL)
    {
#if RESERVE_MEMORY
        LWM2M_ASSERT_MESSAGE(mMaxItems, "Unlimited instances is not supported with memory reservation");
#endif
    }

    Lwm2mStorage(Lwm2mBase* parent) : mParent(parent), mMaxItems(0), mSize(0), mStart(NULL) {}

    ~Lwm2mStorage()
    {
        Node<T>* node = mStart;

        while (node) {
            Node<T>* tmp = node;

            node->get()->release();
            delete node->get();

            node = node->mNext;
            delete tmp;
        }
    }

    T* newItem(uint16_t id, P param, Status* status = NULL)
    {
        if (mMaxItems != 0 && mSize == mMaxItems) {
            if (status) *status = STS_ERR_MEM;
            return NULL;
        }

        uint16_t newId = 0;

        if (id != LWM2M_INVALID_ID) {
            newId = id;
        }

        Node<T>* prevNode = NULL;
        Node<T>* node = begin();

        while (node) {
            if (id != LWM2M_INVALID_ID && newId == node->get()->getId()) {
                if (status) *status = STS_ERR_EXIST;
                return NULL;
            }

            if (newId < node->get()->getId()) {
                break;
            }

            if (id == LWM2M_INVALID_ID) {
                newId++;
            }

            prevNode = node;
            node = node->next();
        }

        T* item = new T(mParent, newId, param);
        item->create();

        Node<T>* newNode = new Node<T>(item);

        // Empty storage
        if (!mStart) {
            mStart = newNode;
        }
        // Insert into begin
        else if (!prevNode) {
            newNode->mNext = mStart;
            mStart = newNode;
        }
        // Insert before node
        else {
            newNode->mNext = prevNode->mNext;
            prevNode->mNext = newNode;
        }

        mSize++;

        return item;
    }

    Status deleteInstance(T* item)
    {
        Node<T>* prevNode = NULL;
        Node<T>* node = begin();

        while (node) {
            if (node->get() == item) {
                node->get()->release();
                delete node->get();

                if (prevNode) {
                    prevNode->mNext = node->mNext;
                }
                else {
                    mStart = node->mNext;
                }

                delete node;

                mSize--;
            }

            prevNode = node;
            node = node->next();
        }

        return STS_ERR_EXIST;
    }

    Node<T>* begin() const { return mStart; }

    T* getItemById(uint16_t id)
    {
        Node<T>* node = begin();
        while (node) {
            if (node->get()->getId() == id) {
                return node->get();
            }
            node = node->next();
        }

        return NULL;
    }

    size_t size() const { return mSize; }
    bool hasFreeItem() const { return mMaxItems == 0 || mSize < mMaxItems; }

private:
    Lwm2mBase* mParent;
    size_t mMaxItems;
    size_t mSize;

    Node<T>* mStart;
};  // namespace openlwm2m

}  // namespace openlwm2m

#endif /* OPENLWM2M_STORAGE_HPP_ */