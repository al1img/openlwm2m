#ifndef OPENLWM2M_LIST_HPP_
#define OPENLWM2M_LIST_HPP_

namespace openlwm2m {

template <class T>
class List;

template <class T>
class Node {
public:
    Node(T data) : mData(data), mPrev(nullptr), mNext(nullptr) {}
    T get() { return mData; }
    Node* next() { return mNext; }

private:
    friend class List<T>;

    T mData;
    Node* mPrev;
    Node* mNext;
};

template <class T>
class List {
public:
    List() : mStart(nullptr), mEnd(nullptr) {}
    ~List()
    {
        Node<T>* node = mStart;

        while (node != nullptr) {
            Node<T>* tmp = node;
            node = node->mNext;
            delete tmp;
        }
    }

    Node<T>* begin() { return mStart; }

    void append(T data)
    {
        Node<T>* newNode = new Node<T>(data);

        mEnd = mEnd->mNext = newNode;
    }

private:
    Node<T>* mStart;
    Node<T>* mEnd;
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_LIST_HPP_ */