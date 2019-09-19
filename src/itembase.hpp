#ifndef OPENLWM2M_ITEMBASE_HPP_
#define OPENLWM2M_ITEMBASE_HPP_

#include <stdint.h>
#include <cstddef>

namespace openlwm2m {

#define INVALID_ID 0xFFFF

enum ItemMandatory { MANDATORY, OPTIONAL };

enum ItemInstance { SINGLE, MULTIPLE };

class ItemBase {
public:
    ItemBase* getParent() const { return mParent; }
    uint16_t getId() const { return mId; }
    void setId(uint16_t id) { mId = id; }

protected:
    ItemBase(ItemBase* parent = NULL, uint16_t id = INVALID_ID) : mParent(parent), mId(id), mCreated(false) {}

private:
    ItemBase* mParent;
    uint16_t mId;
    bool mCreated;
};

#if 0

class ItemNode {
public:
    ItemNode(ItemBase* data) : mData(data), mNext(NULL) {}
    ItemBase* get() const { return mData; }
    ItemNode* next() const { return mNext; }

private:
    friend class ItemList;

    ItemBase* mData;
    ItemNode* mNext;
};

class ItemList {
public:
    ItemList() : mSize(0), mBegin(NULL), mEnd(NULL) {}

    ItemNode* begin() const { return mBegin; }

    size_t size() const { return mSize; }

    void insertBegin(ItemNode* node)
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

    void insertAfter(ItemNode* prevNode, ItemNode* node)
    {
        node->mNext = prevNode->mNext;
        prevNode->mNext = node;

        if (!node->mNext) {
            mEnd = node;
        }

        mSize++;
    }

    void insertEnd(ItemNode* node)
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

    void remove(ItemNode* node)
    {
        ItemNode* prevNode = NULL;
        ItemNode* curNode = this->mBegin;

        while (curNode) {
            if (curNode == node) {
                removeNode(prevNode, curNode);
                break;
            }

            prevNode = curNode;
            curNode = curNode->mNext;
        }
    }

    ItemNode* remove(ItemBase* data)
    {
        ItemNode* prevNode = NULL;
        ItemNode* curNode = this->mBegin;

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
    ItemNode* mBegin;
    ItemNode* mEnd;

    void removeNode(ItemNode* prevNode, ItemNode* node)
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

class ItemStorage : public ItemList {
public:
    ItemStorage() {}
    ~ItemStorage()
    {
        ItemNode* node = this->begin();
        while (node) {
            ItemNode* tmp = node;

            node = node->next();

            delete tmp->get();
            delete tmp;
        }
    }

    T* newItem(ItemBase* parent, uint16_t id, P param, Status* status = NULL)
    {
        if (mMaxItems > 0 && this->size() == mMaxItems) {
            if (status) *status = STS_ERR_NO_MEM;
            return NULL;
        }

        Node<T>* node;
        Status retStatus;

        if ((retStatus = this->findNodeAndId(&id, &node)) != STS_OK) {
            if (status) *status = retStatus;
            return NULL;
        }

        T* newItem;

        if (mNewFunc) {
            newItem = mNewFunc(parent, param);
        }
        else {
            newItem = new T(parent, param);
        }

        Node<T>* newNode = new Node<T>(newItem);

        newNode->get()->setId(id);

        if (node) {
            this->insertAfter(node, newNode);
        }
        else {
            this->insertBegin(newNode);
        }

        return newNode->get();
    }

    Status addItem(T* item)
    {
        if (mMaxItems > 0 && this->size() == mMaxItems) {
            return STS_ERR_NO_MEM;
        }

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

    bool hasFreeItem() const { return mMaxItems == 0 || this->mSize < mMaxItems; }

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
class Lwm2mDynamicStorage : public Lwm2mStorage<T, P> {
public:
    Lwm2mDynamicStorage(ItemBase* parent, P param, size_t maxItems, T*(newFunc)(ItemBase*, P) = NULL)
        : Lwm2mStorage<T, P>(maxItems, newFunc)
#if !CONFIG_RESERVE_MEMORY
          ,
          mParent(parent),
          mParam(param)
#endif
    {
#if CONFIG_RESERVE_MEMORY
        ASSERT_MESSAGE(this->mMaxItems, "Unlimited instances is not supported with memory reservation");

        for (size_t i = 0; i < this->mMaxItems; i++) {
            T* item;

            if (this->mNewFunc) {
                item = this->mNewFunc(parent, param);
            }
            else {
                item = new T(parent, param);
            }

            Node<T>* newNode = new Node<T>(item);

            mFreeList.insertEnd(newNode);
        }
#endif
    }

    ~Lwm2mDynamicStorage()
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

#if CONFIG_RESERVE_MEMORY
        Node<T>* newNode = mFreeList.begin();

        if (!newNode) {
            if (status) *status = STS_ERR_NO_MEM;
            return NULL;
        }

        mFreeList.remove(newNode);
#else
        T* newItem;

        if (this->mNewFunc) {
            item = this->mNewFunc(parent, param);
        }
        else {
            newItem = new T(mParent, mParam);
        }

        Node<T>* newNode = new Node<T>(newItem);
#endif

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

#if CONFIG_RESERVE_MEMORY
            mFreeList.insertEnd(tmp);
#else
            delete tmp->get();
            delete tmp;
#endif
        }
    }

private:
    size_t mMaxItems;

#if CONFIG_RESERVE_MEMORY
    List<T> mFreeList;
#else
    ItemBase* mParent;
    P mParam;
#endif
};

#endif
}  // namespace openlwm2m

#endif /* OPENLWM2M_ITEMBASE_HPP_ */
