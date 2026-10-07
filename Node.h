//
// Created by nicol on 27/09/2026.
//

#ifndef NODE_H
#define NODE_H

template <typename T>
class DoubleLinkedList;

template <typename T>
class Node {
    friend class DoubleLinkedList<T>;
private:
    T info;
    Node<T>* next;
    Node<T>* previous;
public:
    Node(T info) {
        this->info = info;
        next = nullptr;
        previous = nullptr;
    }

    T getInfo() const {
        return info;
    }

    Node<T> * getNext() const {
        return next;
    }

    Node<T> * getPrevious() const {
        return previous;
    }
};

#endif //NODE_H