//
// Created by nicol on 27/09/2026.
//

#include "DoubleLinkedList.h"

#include <iosfwd>
#include <ostream>
#include <stdexcept>

using namespace std;

template<typename T>
DoubleLinkedList<T>::DoubleLinkedList() {
    head = nullptr;
    tail = nullptr;
    size = 0;
}

template<typename T>
bool DoubleLinkedList<T>::isEmpty() {
    return this->head == nullptr && this->tail == nullptr;
}

template<typename T>
void DoubleLinkedList<T>::addNodeFirst(T info) {
    Node<T>* newNode = new Node<T>(info);
    if (isEmpty()) {
        this->head = newNode;
        this->tail = newNode;
    }else {
        newNode->next = head;
        head->previous = newNode;
        this->head = newNode;
    }
    size ++;
}

template<typename T>
void DoubleLinkedList<T>::addNodeLast(T info) {
    // CORRECCION: faltaba el return. Sin el, en lista vacia se insertaban dos nodos.
    if (isEmpty()) {
        addNodeFirst(info);
        return;
    }

    Node<T> *newNode = new Node<T>(info);

    newNode->previous = tail;
    tail->next = newNode;
    tail = newNode;

    size++;
}

template<typename T>
void DoubleLinkedList<T>::addNodeAfterTo(Node<T> *current, T info) {
    if (current == nullptr) return;

    if (current == tail) {
        addNodeLast(info);
        return;
    };

    Node<T>* newNode = new Node<T>(info);

    newNode->next = current->next;
    newNode->previous = current;
    current->next = newNode;
    newNode->next->previous = newNode;
    size++;
}

template<typename T>
void DoubleLinkedList<T>::addNodeBeforeTo(Node<T> *current, T info) {
    if (current == nullptr) return;

    // CORRECCION: faltaba el return. Sin el, seguia y llamaba addNodeAfterTo
    // con current->previous == nullptr. Tambien sobraba el size++ del final,
    // porque addNodeFirst y addNodeAfterTo ya incrementan size.
    if (current == head) {
        addNodeFirst(info);
        return;
    }

    addNodeAfterTo(current->previous, info);
}

template<typename T>
void DoubleLinkedList<T>::addNodeSorted(T info) {
    // La lista queda ordenada de mayor a menor (head = el mayor),
    // usando el operator> del tipo T.
    if (isEmpty()) {
        addNodeFirst(info);
        return;
    }

    if (info > head->info) {
        addNodeFirst(info);
        return;
    }

    // CORRECCION: se recorre desde tail hacia head mientras el nodo actual
    // sea MENOR que info; al detenerse, aux es el primer nodo mayor o igual
    // y el nuevo nodo va justo despues de el.
    Node<T>* aux = tail;
    while (aux != head && info > aux->info) {
        aux = aux->previous;
    }
    addNodeAfterTo(aux, info);
}

template<typename T>
Node<T> * DoubleLinkedList<T>::findNode(T info) {
    Node<T>* aux = tail;
    while (aux!=nullptr) {
        if(aux->info == info) {
            return aux;
        }
        aux = aux->previous;
    }
    return nullptr;
}

// ==================== Metodos agregados ====================

template<typename T>
Node<T> * DoubleLinkedList<T>::findNodeFromHead(T info) {
    // Misma busqueda que findNode pero aprovechando el otro sentido del
    // doble enlace. Util cuando se sabe que el elemento esta cerca del inicio.
    Node<T>* aux = head;
    while (aux != nullptr) {
        if (aux->info == info) {
            return aux;
        }
        aux = aux->next;
    }
    return nullptr;
}

template<typename T>
int DoubleLinkedList<T>::getPosition(T info) {
    Node<T>* aux = head;
    int position = 1;
    while (aux != nullptr) {
        if (aux->info == info) {
            return position;
        }
        aux = aux->next;
        position++;
    }
    return -1;
}

template<typename T>
Node<T> * DoubleLinkedList<T>::getNodeAt(int position) {
    if (position < 1 || position > size) return nullptr;

    // Se recorre por el lado mas cercano: si la posicion esta en la primera
    // mitad se entra por head, si esta en la segunda mitad se entra por tail.
    // Esta es justamente la ventaja de la lista doblemente enlazada.
    if (position <= size / 2) {
        Node<T>* aux = head;
        for (int i = 1; i < position; i++) {
            aux = aux->next;
        }
        return aux;
    } else {
        Node<T>* aux = tail;
        for (int i = size; i > position; i--) {
            aux = aux->previous;
        }
        return aux;
    }
}

template<typename T>
T DoubleLinkedList<T>::deleteNode(Node<T> *current) {
    if (current == nullptr) {
        throw invalid_argument("El nodo a eliminar es nulo");
    }

    T info = current->info;

    // Caso 1: es el unico nodo de la lista
    if (current == head && current == tail) {
        head = nullptr;
        tail = nullptr;
    }
    // Caso 2: es la cabeza
    else if (current == head) {
        head = current->next;
        head->previous = nullptr;
    }
    // Caso 3: es la cola
    else if (current == tail) {
        tail = current->previous;
        tail->next = nullptr;
    }
    // Caso 4: esta en el medio. Aqui se ve la ventaja del doble enlace:
    // no hace falta recorrer buscando el anterior, ya lo tenemos.
    else {
        current->previous->next = current->next;
        current->next->previous = current->previous;
    }

    delete current;
    size--;
    return info;
}

template<typename T>
bool DoubleLinkedList<T>::deleteByInfo(T info) {
    Node<T>* target = findNode(info);
    if (target == nullptr) return false;

    deleteNode(target);
    return true;
}

template<typename T>
T DoubleLinkedList<T>::deleteFirst() {
    if (isEmpty()) {
        throw out_of_range("La lista esta vacia");
    }
    return deleteNode(head);
}

template<typename T>
T DoubleLinkedList<T>::deleteLast() {
    if (isEmpty()) {
        throw out_of_range("La lista esta vacia");
    }
    return deleteNode(tail);
}

template<typename T>
void DoubleLinkedList<T>::clear() {
    Node<T>* aux = head;
    while (aux != nullptr) {
        Node<T>* nextNode = aux->next;
        delete aux;
        aux = nextNode;
    }
    head = nullptr;
    tail = nullptr;
    size = 0;
}

template<typename T>
int DoubleLinkedList<T>::getSize() {
    return this->size;
}

template<typename T>
T DoubleLinkedList<T>::getFirst() {
    if (isEmpty()) {
        throw out_of_range("La lista esta vacia");
    }
    return head->info;
}

template<typename T>
T DoubleLinkedList<T>::getLast() {
    if (isEmpty()) {
        throw out_of_range("La lista esta vacia");
    }
    return tail->info;
}

template<typename T>
void DoubleLinkedList<T>::reverse() {
    if (isEmpty()) return;

    Node<T>* aux = head;
    while (aux != nullptr) {
        // Se intercambian los dos punteros de cada nodo
        Node<T>* temp = aux->next;
        aux->next = aux->previous;
        aux->previous = temp;

        // Se avanza usando el puntero original guardado en temp
        aux = temp;
    }

    // Finalmente se intercambian head y tail
    Node<T>* temp = head;
    head = tail;
    tail = temp;
}

template <typename T>
ostream& operator<<(ostream& os, DoubleLinkedList<T>* list) {
    Node<T>* aux = list->getHead();
    while (aux!=nullptr) {
        os << aux->getInfo()<<"\n";
        aux = aux->getNext();
    }
    return os;
};

template<typename T>
DoubleLinkedList<T>::~DoubleLinkedList() {
    // El destructor libera todos los nodos para no dejar fugas de memoria.
    clear();
}