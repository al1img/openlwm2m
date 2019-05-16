#ifndef OPENLWM2M_LIST_HPP_
#define OPENLWM2M_LIST_HPP_

#include <cstddef>

namespace openlwm2m {

class List;

class Node {
public:
    Node(void* data) : mData(data), mPrev(NULL), mNext(NULL) {}
    void* get() { return mData; }
    Node* next() { return mNext; }

private:
    friend class List;

    void* mData;
    Node* mPrev;
    Node* mNext;
};

class List {
public:
    List() : mStart(NULL), mEnd(NULL) {}
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

    void append(void* data)
    {
        Node* newNode = new Node(data);
        if (!mStart) {
            mStart = newNode;
        }

        if (mEnd) {
            mEnd->mNext = newNode;
            newNode->mPrev = mEnd;
        }

        mEnd = newNode;
    }

private:
    Node* mStart;
    Node* mEnd;
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_LIST_HPP_ */