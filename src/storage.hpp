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
    template <class>
    friend class StorageBase;

    template <class, class>
    friend class StorageItem;

    template <class, class>
    friend class StorageArray;

    T* mData;
    Node* mNext;
};

template <class T>
class StorageBase {
public:
    StorageBase() : mSize(0), mStart(NULL) {}

    virtual ~StorageBase()
    {
        Node<T>* node = mStart;

        while (node) {
            Node<T>* tmp = node;

            node = node->mNext;

            delete tmp->get();
            delete tmp;
        }
    }

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

    Node<T>* begin() const { return mStart; }

    size_t size() const { return mSize; }

protected:
    size_t mSize;
    Node<T>* mStart;

    void insertNode(Node<T>* prevNode, Node<T>* node)
    {
        // Empty storage
        if (!mStart) {
            mStart = node;
        }
        // Insert into begin
        else if (!prevNode) {
            node->mNext = mStart;
            mStart = node;
        }
        // Insert before node
        else {
            node->mNext = prevNode->mNext;
            prevNode->mNext = node;
        }

        mSize++;
    }

    void removeNode(Node<T>* prevNode, Node<T>* node)
    {
        if (prevNode) {
            prevNode->mNext = node->mNext;
        }
        else {
            mStart = node->mNext;
        }

        node->mNext = NULL;

        mSize--;
    }

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
};

template <class T, class P>
class StorageItem : public StorageBase<T> {
public:
    StorageItem(ItemBase* parent, P param, size_t maxItems)
        : mParent(parent),
          mMaxItems(maxItems)
#if RESERVE_MEMORY
          ,
          mFree(NULL)
#endif
    {
#if RESERVE_MEMORY
        ASSERT_MESSAGE(mMaxItems, "Unlimited instances is not supported with memory reservation");

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

    ~StorageItem()
    {
#if RESERVE_MEMORY
        Node<T>* node = mFree;

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
        if (mMaxItems != 0 && this->mSize == mMaxItems) {
            if (status) *status = STS_ERR_MEM;
            return NULL;
        }

        Node<T>* node;
        Status retStatus;

        if ((retStatus = this->findNodeAndId(&id, &node)) != STS_OK) {
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

        newNode->get()->init();

        this->insertNode(node, newNode);

        return newNode->get();
    }

    Status deleteInstance(T* item)
    {
        Node<T>* prevNode = NULL;
        Node<T>* node = this->mStart;

        while (node) {
            if (node->get() == item) {
                node->get()->release();

                removeNode(prevNode, node);

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

                return STS_OK;
            }

            prevNode = node;
            node = node->mNext;
        }

        return STS_ERR_EXIST;
    }

    void clear()
    {
        Node<T>* node = this->mStart;

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

        this->mStart = NULL;
        this->mSize = 0;
    }

    bool hasFreeItem() const { return mMaxItems == 0 || this->mSize < mMaxItems; }

private:
    ItemBase* mParent;
    size_t mMaxItems;
#if RESERVE_MEMORY
    Node<T>* mFree;
#endif
};

template <class T, class P>
class StorageArray : public StorageBase<T> {
public:
    StorageArray(ItemBase* parent) : mParent(parent) {}

    T* createItem(uint16_t id, P param, Status* status = NULL)
    {
        Node<T>* node;
        Status retStatus;

        if ((retStatus = this->findNodeAndId(&id, &node)) != STS_OK) {
            if (status) *status = retStatus;
            return NULL;
        }

        Node<T>* newNode = new Node<T>(new T(mParent, id, param));

        this->insertNode(node, newNode);

        return newNode->get();
    }

    void init()
    {
        Node<T>* node = this->mStart;

        while (node) {
            node->get()->init();
            node = node->mNext;
        }
    }

    void release()
    {
        Node<T>* node = this->mStart;

        while (node) {
            node->get()->release();
            node = node->mNext;
        }
    }

private:
    ItemBase* mParent;
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_STORAGE_HPP_ */