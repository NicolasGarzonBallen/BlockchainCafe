//
// Created by nicol on 27/09/2026.
//

#ifndef BLOQUE_H
#define BLOQUE_H

#include <string>
#include "ListaSimple.h"
#include "Transaccion.h"

// El "nodo" conceptual de la cadena. Ojo: aqui NO se guardan punteros
// anterior/siguiente -- esos ya los da gratis el Node<T> de tu
// DoubleLinkedList cuando la cadena se declara como DoubleLinkedList<Bloque*>.
// Bloque solo se preocupa de sus propios datos.
class Bloque {
private:
    int indice;
    std::string timestamp;
    std::string hashPrevio;
    std::string hash;
    unsigned long nonce;
    ListaSimple<Transaccion> transacciones;

public:
    Bloque(int indice, std::string timestamp);

    void agregarTransaccion(Transaccion t);
    bool tieneTransacciones() const;

    // Devuelve un puntero a la transaccion en esa posicion (1-based) para poder
    // editarla en el sitio, o nullptr si la posicion no existe. Se usa en la
    // opcion 6 (simular manipulacion): se cambia el dato pero NO se vuelve a
    // minar el bloque, para que validarCadena() detecte la inconsistencia.
    Transaccion* obtenerTransaccion(int posicion);

    // Texto plano sobre el que se calcula el hash: indice + timestamp + datos + hashPrevio + nonce
    std::string aTexto() const;

    // Aplica la funcion hash (FNV-1a, 64 bits) sobre aTexto() y la devuelve en hexadecimal.
    // Es un hash didactico: cumple con ser determinista y con el efecto avalancha,
    // pero no tiene las garantias criptograficas de SHA-256 (no es resistente
    // a colisiones ni a ataques de fuerza bruta dirigidos).
    std::string calcularHash() const;

    // Prueba de trabajo: ajusta el nonce por fuerza bruta hasta que calcularHash()
    // empiece con 'dificultad' ceros. Devuelve la cantidad de intentos realizados.
    long minar(int dificultad);

    int getIndice() const;
    std::string getTimestamp() const;
    std::string getHashPrevio() const;
    void setHashPrevio(std::string h);

    // Persistencia: restaura el sello (hashPrevio, hash y nonce) leido desde el
    // archivo, SIN volver a minar. Si alguien edito el archivo a mano, el hash
    // guardado ya no coincidira con calcularHash() y validarCadena() lo detecta.
    void restaurarSello(std::string hashPrevio, std::string hash, unsigned long nonce);
    std::string getHash() const;
    unsigned long getNonce() const;
    ListaSimple<Transaccion>& getTransacciones();
};

#endif //BLOQUE_H