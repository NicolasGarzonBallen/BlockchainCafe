//
// Created by nicol on 27/09/2026.
//

#ifndef LISTASIMPLE_H
#define LISTASIMPLE_H

template <typename T>
class ListaSimple;

// Nodo de una lista enlazada sencilla generica: guarda un dato y el
// puntero al siguiente (no hay "previous", a diferencia del Node<T>
// que ya usan para la cadena de bloques).
template <typename T>
class NodoSimple {
    friend class ListaSimple<T>;
private:
    T dato;
    NodoSimple<T>* siguiente;
public:
    NodoSimple(T dato) : dato(dato), siguiente(nullptr) {}
    T getDato() const { return dato; }
    // Referencia (no copia) al dato guardado. Se usa cuando hace falta
    // modificar el contenido de un nodo sin recorrer toda la lista de nuevo
    // (por ejemplo, para simular una manipulacion sobre una transaccion ya guardada).
    T& getDatoRef() { return dato; }
    NodoSimple<T>* getSiguiente() const { return siguiente; }
};

// Lista enlazada sencilla generica. Un Bloque guarda una
// ListaSimple<Transaccion>, pero la clase en si no sabe nada de
// Transaccion: cualquier tipo T sirve, cumpliendo con volverla generica.
template <typename T>
class ListaSimple {
private:
    NodoSimple<T>* cabeza;
    NodoSimple<T>* cola;
    int n;
public:
    ListaSimple() : cabeza(nullptr), cola(nullptr), n(0) {}

    // Copiar la lista duplicaria los punteros y provocaria un doble delete.
    ListaSimple(const ListaSimple&) = delete;
    ListaSimple& operator=(const ListaSimple&) = delete;

    // El destructor libera todos los nodos. Cuando el destructor de Bloque
    // se ejecute, el de su ListaSimple<Transaccion> se llama automaticamente
    // (es un atributo por valor, no un puntero) -- ese es el primer nivel
    // de gestion de memoria que pide el enunciado.
    ~ListaSimple() {
        NodoSimple<T>* aux = cabeza;
        while (aux != nullptr) {
            NodoSimple<T>* siguiente = aux->siguiente;
            delete aux;
            aux = siguiente;
        }
    }

    bool isEmpty() const { return cabeza == nullptr; }

    int getSize() const { return n; }

    void insertarFinal(T dato) {
        NodoSimple<T>* nuevo = new NodoSimple<T>(dato);
        if (isEmpty()) {
            cabeza = nuevo;
            cola = nuevo;
        } else {
            cola->siguiente = nuevo;
            cola = nuevo;
        }
        n++;
    }

    // Punto de entrada para recorrer la lista "a mano" desde afuera:
    // NodoSimple<T>* actual = lista.getCabeza();
    // while (actual != nullptr) { ...usar actual->getDato()...; actual = actual->getSiguiente(); }
    // Se prefiere esto a devolver un std::vector para no depender de
    // contenedores de la STL, ni siquiera como copia de conveniencia.
    NodoSimple<T>* getCabeza() const { return cabeza; }
};

#endif //LISTASIMPLE_H