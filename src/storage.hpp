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
            mEnd = node;
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
class Lwm2mStorage : public List<T> {
public:
    Lwm2mStorage() {}
    ~Lwm2mStorage()
    {
        Node<T>* node = this->begin();
        while (node) {
            Node<T>* tmp = node;

            node = node->next();

            delete tmp->get();
            delete tmp;
        }
    }

    Status addItem(T* item)
    {
        uint16_t id = item->getId();

        Node<T>* node;
        Status retStatus;

        if ((retStatus = this->findNodeAndId(&id, &node)) != STS_OK) {
            return retStatus;
        }

        Node<T>* newNode = new Node<T>(item);

        newNode->get()->setId(id);

        if (node) {
            this->insertAfter(node, newNode);
        }
        else {
            this->insertBegin(newNode);
        }

        return STS_OK;
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

    T* getFirstItem()
    {
        mCurrentNode = this->begin();

        if (mCurrentNode) {
            return mCurrentNode->get();
        }

        return NULL;
    }

    T* getNextItem()
    {
        if (mCurrentNode) {
            mCurrentNode = mCurrentNode->next();

            if (mCurrentNode) {
                return mCurrentNode->get();
            }
        }

        return NULL;
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

private:
    Node<T>* mCurrentNode;
};

template <class T, class P>
class Lwm2mDynamicStorage : public Lwm2mStorage<T> {
public:
    Lwm2mDynamicStorage(ItemBase* parent, P param, size_t maxItems, T*(newFunc)(ItemBase*, P) = NULL)
        : mMaxItems(maxItems)
    {
        for (size_t i = 0; i < this->mMaxItems; i++) {
            T* item;

            if (newFunc) {
                item = newFunc(parent, param);
            }
            else {
                item = new T(parent, param);
            }

            Node<T>* newNode = new Node<T>(item);

            mFreeList.insertEnd(newNode);
        }
    }

    ~Lwm2mDynamicStorage()
    {
        Node<T>* node = mFreeList.begin();

        while (node) {
            Node<T>* tmp = node;

            delete node->get();

            node = node->next();
            delete tmp;
        }
    }

    T* newItem(uint16_t id, Status* status = NULL)
    {
        if (this->mMaxItems != 0 && this->size() == this->mMaxItems) {
            if (status) *status = STS_ERR_NO_MEM;
            return NULL;
        }

        Node<T>* node;
        Status retStatus;

        if ((retStatus = this->findNodeAndId(&id, &node)) != STS_OK) {
            if (status) *status = retStatus;
            return NULL;
        }

        Node<T>* newNode = mFreeList.begin();

        if (!newNode) {
            if (status) *status = STS_ERR_NO_MEM;
            return NULL;
        }

        mFreeList.remove(newNode);

        newNode->get()->setId(id);
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

                mFreeList.insertEnd(node);

                return STS_OK;
            }

            node = node->next();
        }

        return STS_ERR_NOT_FOUND;
    }

    void clear()
    {
        Node<T>* node = this->begin();

        while (node) {
            node->get()->release();

            Node<T>* tmp = node;

            node = node->next();
            this->remove(tmp);

            mFreeList.insertEnd(tmp);
        }
    }

    bool hasFreeItem() const { return mMaxItems == 0 || this->mSize < mMaxItems; }

private:
    size_t mMaxItems;
    List<T> mFreeList;
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_STORAGE_HPP_ */
