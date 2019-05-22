#ifndef OPENLWM2M_STORAGE_HPP_
#define OPENLWM2M_STORAGE_HPP_

#include <cstddef>

#include "config.hpp"
#include "itembase.hpp"
#include "log.hpp"
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
    Lwm2mStorage(ItemBase* parent, P param, size_t maxItems)
        : mParent(parent),
          mMaxItems(maxItems),
          mSize(0),
          mStart(NULL)
#if RESERVE_MEMORY
          ,
          mFree(NULL)
#endif
    {
#if RESERVE_MEMORY
        LWM2M_ASSERT_MESSAGE(mMaxItems, "Unlimited instances is not supported with memory reservation");

        Node<T>* prevNode = NULL;

        for (size_t i = 0; i < mMaxItems; i++) {
            T* item = new T(mParent, INVALID_ID, param);
            Node<T>* newNode = new Node<T>(item);

            // Assign first node
            if (!mFree) {
                mFree = newNode;
            }

            // Assign next node
            if (prevNode) {
                prevNode->mNext = newNode;
            }

            prevNode = newNode;
        }
#endif
    }

    Lwm2mStorage(ItemBase* parent)
        : mParent(parent),
          mMaxItems(0),
          mSize(0),
          mStart(NULL)
#if RESERVE_MEMORY
          ,
          mFree(NULL)
#endif
    {
    }

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

#if RESERVE_MEMORY
        node = mFree;

        while (node) {
            Node<T>* tmp = node;

            delete node->get();

            node = node->mNext;
            delete tmp;
        }
#endif
    }

    T* newItem(uint16_t id, P param, Status* status = NULL)
    {
        if (mMaxItems != 0 && mSize == mMaxItems) {
            if (status) *status = STS_ERR_MEM;
            return NULL;
        }

        Node<T>* node;
        Status retStatus;
        if ((retStatus = findNodeAndId(&id, &node)) != STS_OK) {
            if (status) *status = retStatus;
            return NULL;
        }

        Node<T>* newNode;

#if RESERVE_MEMORY
        if (mMaxItems != 0) {
            if (!mFree) {
                if (status) *status = STS_ERR_MEM;
                return NULL;
            }

            newNode = mFree;
            mFree = newNode->mNext;
            newNode->mNext = NULL;
            newNode->get()->setId(id);
        }
        else
#endif
        {
            newNode = new Node<T>(new T(mParent, id, param));
        }

        newNode->get()->create();

        insertNode(node, newNode);

        mSize++;

        return newNode->get();
    }

    Status deleteInstance(T* item)
    {
        Node<T>* prevNode = NULL;
        Node<T>* node = mStart;

        while (node) {
            if (node->get() == item) {
                node->get()->release();

                if (prevNode) {
                    prevNode->mNext = node->mNext;
                }
                else {
                    mStart = node->mNext;
                }

#if RESERVE_MEMORY
                if (mMaxItems != 0) {
                    if (mFree) {
                        node->mNext = mFree->mNext;
                    }

                    mFree = node;
                }
                else
#endif
                {
                    delete node->get();
                    delete node;
                }

                mSize--;

                return STS_OK;
            }

            prevNode = node;
            node = node->mNext;
        }

        return STS_ERR_EXIST;
    }

    Node<T>* begin() const { return mStart; }

    T* getItemById(uint16_t id)
    {
        Node<T>* node = mStart;
        while (node) {
            if (node->get()->getId() == id) {
                return node->get();
            }
            node = node->mNext;
        }

        return NULL;
    }

    void clear()
    {
        Node<T>* node = mStart;

        while (node) {
            node->get()->release();

            Node<T>* tmp = node;
            node = node->mNext;

#if RESERVE_MEMORY
            if (mMaxItems != 0) {
                if (mFree) {
                    tmp->mNext = mFree->mNext;
                }

                mFree = tmp;
            }
            else
#endif
            {
                delete tmp->get();
                delete tmp;
            }
        }

        mStart = NULL;
    }

    size_t size() const { return mSize; }

    bool hasFreeItem() const { return mMaxItems == 0 || mSize < mMaxItems; }

private:
    ItemBase* mParent;
    size_t mMaxItems;
    size_t mSize;

    Node<T>* mStart;
#if RESERVE_MEMORY
    Node<T>* mFree;
#endif

    Status findNodeAndId(uint16_t* id, Node<T>** node)
    {
        uint16_t newId = 0;

        if (*id != INVALID_ID) {
            newId = *id;
        }

        *node = NULL;
        Node<T>* curNode = mStart;

        while (curNode) {
            if (*id != INVALID_ID && newId == curNode->get()->getId()) {
                return STS_ERR_EXIST;
            }

            if (newId < curNode->get()->getId()) {
                break;
            }

            if (*id == INVALID_ID) {
                newId++;
            }

            *node = curNode;
            curNode = curNode->mNext;
        }

        *id = newId;

        return STS_OK;
    }

    void insertNode(Node<T>* node, Node<T>* newNode)
    {
        // Empty storage
        if (!mStart) {
            mStart = newNode;
        }
        // Insert into begin
        else if (!node) {
            newNode->mNext = mStart;
            mStart = newNode;
        }
        // Insert before node
        else {
            newNode->mNext = node->mNext;
            node->mNext = newNode;
        }
    }
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_STORAGE_HPP_ */