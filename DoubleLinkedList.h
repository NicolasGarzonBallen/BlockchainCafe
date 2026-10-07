//
// Created by nicol on 27/09/2026.
//

#ifndef DOUBLELINKEDLIST_H
#define DOUBLELINKEDLIST_H
#include "Node.h"


template <typename T>
class DoubleLinkedList {
private:
  Node<T>* head;
  Node<T>* tail;
  int size;
public:
  DoubleLinkedList();
  ~DoubleLinkedList();


  bool isEmpty();

  void addNodeFirst(T info);

  void addNodeLast(T info);

  void addNodeAfterTo(Node<T>* current, T info);

  void addNodeBeforeTo(Node<T>* current, T info);

  void addNodeSorted(T info);

  Node<T>* findNode(T info);

  // ---------------- Metodos agregados ----------------

  // Busqueda
  Node<T>* findNodeFromHead(T info);   // busca desde head (la original busca desde tail)
  int getPosition(T info);             // posicion 1-based, -1 si no existe
  Node<T>* getNodeAt(int position);    // nodo en una posicion, nullptr si no existe

  // Eliminacion
  T deleteNode(Node<T>* current);      // elimina un nodo dado y devuelve su info
  bool deleteByInfo(T info);           // busca por info y elimina
  T deleteFirst();
  T deleteLast();
  void clear();                        // elimina todos los nodos

  // Consulta
  int getSize();
  T getFirst();
  T getLast();
  void reverse();                      // invierte la lista intercambiando next/previous

  Node<T> * getHead() const {
    return head;
  }

  Node<T> * getTail() const {
    return tail;
  }
};



#endif //DOUBLELINKEDLIST_H