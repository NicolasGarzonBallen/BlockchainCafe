//
// Created by nicol on 27/09/2026.
//

#include "Bloque.h"
#include <sstream>
#include <iomanip>

Bloque::Bloque(int indice, std::string timestamp)
    : indice(indice), timestamp(timestamp), hashPrevio(""), hash(""), nonce(0) {}

void Bloque::agregarTransaccion(Transaccion t) {
    transacciones.insertarFinal(t);
}

bool Bloque::tieneTransacciones() const {
    return !transacciones.isEmpty();
}

std::string Bloque::aTexto() const {
    std::ostringstream oss;
    oss << indice << "|" << timestamp << "|";

    // Se concatena el aTexto() de cada transaccion, recorriendo la lista sencilla
    // nodo por nodo (sin pasar por std::vector). Si una transaccion cambia
    // (aunque sea un solo digito del peso), este texto cambia, y por lo tanto
    // el hash tambien -- esa es la base de la deteccion de manipulacion.
    NodoSimple<Transaccion>* actual = transacciones.getCabeza();
    while (actual != nullptr) {
        oss << actual->getDato().aTexto() << ";";
        actual = actual->getSiguiente();
    }

    oss << "|" << hashPrevio << "|" << nonce;
    return oss.str();
}

std::string Bloque::calcularHash() const {
    std::string texto = aTexto();

    // FNV-1a de 64 bits.
    unsigned long long hashValor = 14695981039346656037ULL; // offset basis
    const unsigned long long primo = 1099511628211ULL;

    for (unsigned char c : texto) {
        hashValor ^= c;
        hashValor *= primo;
    }

    // Se rellena con ceros a la izquierda hasta 16 digitos hexadecimales
    // (64 bits). Sin este relleno, std::hex nunca imprime ceros a la
    // izquierda y la condicion "el hash debe empezar en 0" jamas se cumpliria,
    // dejando a minar() en un bucle infinito.
    std::ostringstream oss;
    oss << std::hex << std::setfill('0') << std::setw(16) << hashValor;
    return oss.str();
}

long Bloque::minar(int dificultad) {
    std::string objetivo(dificultad, '0');
    long intentos = 0;

    nonce = 0;
    hash = calcularHash();
    intentos++;

    while (hash.compare(0, dificultad, objetivo) != 0) {
        nonce++;
        hash = calcularHash();
        intentos++;
    }

    return intentos;
}

Transaccion* Bloque::obtenerTransaccion(int posicion) {
    if (posicion < 1) return nullptr;

    NodoSimple<Transaccion>* actual = transacciones.getCabeza();
    int i = 1;
    while (actual != nullptr && i < posicion) {
        actual = actual->getSiguiente();
        i++;
    }

    if (actual == nullptr) return nullptr;
    return &(actual->getDatoRef());
}

int Bloque::getIndice() const { return indice; }
std::string Bloque::getTimestamp() const { return timestamp; }
std::string Bloque::getHashPrevio() const { return hashPrevio; }
void Bloque::setHashPrevio(std::string h) { hashPrevio = h; }

void Bloque::restaurarSello(std::string hashPrevio, std::string hash, unsigned long nonce) {
    this->hashPrevio = hashPrevio;
    this->hash = hash;
    this->nonce = nonce;
}
std::string Bloque::getHash() const { return hash; }
unsigned long Bloque::getNonce() const { return nonce; }
ListaSimple<Transaccion>& Bloque::getTransacciones() { return transacciones; }