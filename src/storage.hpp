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
    friend class List;

    T* mData;
    Node* mNext;
};

template <class T>
class List {
public:
    List() : mSize(0), mBegin(NULL), mEnd(NULL) {}

    Node<T>* begin() const { return mBegin; }

    size_t size() const { return mSize; }

    void insertBegin(Node<T>* node)
    {
        if (mBegin) {
            node->mNext = mBegin;
        }
        else {
            mEnd = node;
            node->mNext = NULL;
        }

        mBegin = node;

        mSize++;
    }

    void insertAfter(Node<T>* prevNode, Node<T>* node)
    {
        node->mNext = prevNode->mNext;
        prevNode->mNext = node;

        if (!node->mNext) {
            mEnd = node;
        }

        mSize++;
    }

    void insertEnd(Node<T>* node)
    {
        if (mEnd) {
            mEnd->mNext = node;
        }
        else {
            mEnd = node;
            mBegin = node;
        }

        mSize++;
    }

    void remove(Node<T>* node)
    {
        Node<T>* prevNode = NULL;
        Node<T>* curNode = this->mBegin;

        while (curNode) {
            if (curNode == node) {
                removeNode(prevNode, curNode);
                break;
            }

            prevNode = curNode;
            curNode = curNode->mNext;
        }
    }

    Node<T>* remove(T* data)
    {
        Node<T>* prevNode = NULL;
        Node<T>* curNode = this->mBegin;

        while (curNode) {
            if (curNode->mData == data) {
                removeNode(prevNode, curNode);

                return curNode;
            }

            prevNode = curNode;
            curNode = curNode->mNext;
        }

        return NULL;
    }

private:
    size_t mSize;
    Node<T>* mBegin;
    Node<T>* mEnd;

    void removeNode(Node<T>* prevNode, Node<T>* node)
    {
        if (!prevNode) {
            mBegin = node->mNext;
        }
        else {
            prevNode->mNext = node->mNext;
        }

        if (node == mEnd) {
            mEnd = prevNode;
        }

        node->mNext = NULL;

        mSize--;
    }
};

template <class T>
class StorageBase : public List<T> {
public:
    StorageBase() {}

    ~StorageBase()
    {
        Node<T>* node = this->begin();

        while (node) {
            Node<T>* tmp = node;

            node = node->next();

            delete tmp->get();
            delete tmp;
        }
    }

    T* getItemById(uint16_t id)
    {
        Node<T>* node = this->begin();
        while (node) {
            if (node->get()->getId() == id) {
                return node->get();
            }

            node = node->next();
        }

        return NULL;
    }

protected:
    Status findNodeAndId(uint16_t* id, Node<T>** node)
    {
        uint16_t newId = 0;

        if (*id != INVALID_ID) {
            newId = *id;
        }

        *node = NULL;
        Node<T>* curNode = this->begin();

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
            curNode = curNode->next();
        }

        *id = newId;

        return STS_OK;
    }
};

template <class T, class P>
class StorageItem : public StorageBase<T> {
public:
    StorageItem(P param, size_t maxItems) : mMaxItems(maxItems)
    {
#if CONFIG_RESERVE_MEMORY
        ASSERT_MESSAGE(mMaxItems, "Unlimited instances is not supported with memory reservation");

        for (size_t i = 0; i < mMaxItems; i++) {
            T* item = new T(NULL, INVALID_ID, param);

            Node<T>* newNode = new Node<T>(item);

            mFreeList.insertEnd(newNode);
        }
#endif
    }

    ~StorageItem()
    {
#if CONFIG_RESERVE_MEMORY
        Node<T>* node = mFreeList.begin();

        while (node) {
            Node<T>* tmp = node;

            delete node->get();

            node = node->next();
            delete tmp;
        }
#endif
    }

    T* newItem(ItemBase* parent, uint16_t id, P param, Status* status = NULL)
    {
        if (mMaxItems != 0 && this->size() == mMaxItems) {
            if (status) *status = STS_ERR_NO_MEM;
            return NULL;
        }

        Node<T>* node;
        Status retStatus;

        if ((retStatus = this->findNodeAndId(&id, &node)) != STS_OK) {
            if (status) *status = retStatus;
            return NULL;
        }

#if CONFIG_RESERVE_MEMORY
        Node<T>* newNode = mFreeList.begin();

        if (!newNode) {
            if (status) *status = STS_ERR_NO_MEM;
            return NULL;
        }

        mFreeList.remove(newNode);

        newNode->get()->setParent(parent);
        newNode->get()->setId(id);
#else
        T* newItem = new T(parent, id, param);
        Node<T>* newNode = new Node<T>(newItem);
#endif

        newNode->get()->init();

        if (node) {
            this->insertAfter(node, newNode);
        }
        else {
            this->insertBegin(newNode);
        }

        return newNode->get();
    }

    Status deleteItem(T* item)
    {
        Node<T>* node = this->begin();

        while (node) {
            if (node->get() == item) {
                node->get()->release();
                this->remove(node);

#if CONFIG_RESERVE_MEMORY
                mFreeList.insertEnd(node);
#else
                delete node->get();
                delete node;
#endif

                return STS_OK;
            }

            node = node->next();
        }

        return STS_ERR_NOT_EXIST;
    }

    void clear()
    {
        Node<T>* node = this->begin();

        while (node) {
            node->get()->release();

            Node<T>* tmp = node;

            node = node->next();
            this->remove(tmp);

#if CONFIG_RESERVE_MEMORY
            mFreeList.insertEnd(tmp);
#else
            delete tmp->get();
            delete tmp;
#endif
        }
    }

    bool hasFreeItem() const { return mMaxItems == 0 || this->mSize < mMaxItems; }

private:
    size_t mMaxItems;
#if CONFIG_RESERVE_MEMORY
    List<T> mFreeList;
#endif
};

template <class T, class P>
class StorageArray : public StorageBase<T> {
public:
    StorageArray() {}

    T* newItem(ItemBase* parent, uint16_t id, P param, Status* status = NULL)
    {
        Node<T>* node;
        Status retStatus;

        if ((retStatus = this->findNodeAndId(&id, &node)) != STS_OK) {
            if (status) *status = retStatus;
            return NULL;
        }

        Node<T>* newNode = new Node<T>(new T(parent, id, param));

        if (node) {
            this->insertAfter(node, newNode);
        }
        else {
            this->insertBegin(newNode);
        }

        return newNode->get();
    }

    void init()
    {
        Node<T>* node = this->begin();

        while (node) {
            node->get()->init();
            node = node->next();
        }
    }

    void release()
    {
        Node<T>* node = this->begin();

        while (node) {
            node->get()->release();
            node = node->next();
        }
    }
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_STORAGE_HPP_ */
