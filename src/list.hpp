#ifndef OPENLWM2M_LIST_HPP_
#define OPENLWM2M_LIST_HPP_

#include <cstddef>

namespace openlwm2m {

class List;

class Node {
public:
    Node(void* data) : mData(data), mNext(NULL) {}
    void* get() { return mData; }
    Node* next() { return mNext; }

private:
    friend class List;

    void* mData;
    Node* mNext;
};

class List {
public:
    List() : mStart(NULL), mSize(0) {}
    ~List()
    {
        Node* node = mStart;

        while (node) {
            Node* tmp = node;
            node = node->mNext;
            delete tmp;
        }
    }

    Node* begin() { return mStart; }

    void insert(void* data, Node* node = NULL)
    {
        Node* newNode = new Node(data);

        // List is empty
        if (!mStart) {
            mStart = mEnd = newNode;
        }
        // Insert at the beginning
        else if (node == mStart || !node) {
            newNode->mNext = mStart;
            mStart = newNode;
        }
        else {
            Node* curNode = mStart;
            while (curNode) {
                if (curNode->mNext == node) {
                    break;
                }
                curNode = curNode->mNext;
            }
            newNode->mNext = curNode->mNext;
            curNode->mNext = newNode;
        }

        mSize++;
    }

    void append(void* data)
    {
        Node* newNode = new Node(data);

        if (!mStart) {
            mStart = mEnd = newNode;
        }
        else {
            mEnd->mNext = newNode;
            mEnd = newNode;
        }

        mSize++;
    }

    size_t size() const { return mSize; }

private:
    Node* mStart;
    Node* mEnd;
    size_t mSize;
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_LIST_HPP_ */