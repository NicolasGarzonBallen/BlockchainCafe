//
// Created by nicol on 27/09/2026.
//

#include "Blockchain.h"
#include "DoubleLinkedList.cpp" // DoubleLinkedList<T> es una plantilla: su implementacion
                                // debe ser visible aqui para poder instanciarla con Bloque*
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

// Instanciacion explicita de la plantilla para T = Bloque*.
// Sin esta linea el linker no generaria el codigo de DoubleLinkedList<Bloque*>,
// porque la implementacion vive en DoubleLinkedList.cpp, un .cpp separado del
// de la clase que la usa (patron que solo funciona con instanciacion explicita
// o incluyendo el .cpp donde se necesite, como se hace arriba).
template class DoubleLinkedList<Bloque*>;

Blockchain::Blockchain(int dificultad) : dificultad(dificultad), cursorNavegacion(nullptr) {
    // El bloque genesis se crea de una vez, abierto y esperando transacciones.
    bloqueAbierto = new Bloque(0, obtenerTimestampActual());
}

Blockchain::~Blockchain() {
    delete bloqueAbierto;

    // DoubleLinkedList<Bloque*> libera sus Node<Bloque*>, pero no sabe que
    // sus T son punteros que a su vez hay que liberar -- eso corresponde
    // a Blockchain, que es quien controla el ciclo de vida de los Bloque.
    // Este es el segundo nivel de gestion de memoria que pide el enunciado.
    while (!cadena.isEmpty()) {
        Bloque* b = cadena.deleteFirst();
        delete b;
    }
}

std::string Blockchain::obtenerTimestampActual() const {
    std::time_t ahora = std::time(nullptr);
    std::string texto = std::ctime(&ahora);
    if (!texto.empty() && texto.back() == '\n') {
        texto.pop_back();
    }
    return texto;
}

void Blockchain::registrarTransaccion(std::string idLote, std::string etapa, std::string actor, double pesoKg, std::string fecha) {
    Transaccion t(idLote, etapa, actor, pesoKg, fecha);
    bloqueAbierto->agregarTransaccion(t);
}

void Blockchain::minarBloqueActual() {
    if (!bloqueAbierto->tieneTransacciones()) {
        throw std::runtime_error("No se puede minar un bloque vacio");
    }

    // hashPrevio: el hash del ultimo bloque de la cadena, o "0000" si este es el genesis.
    if (cadena.isEmpty()) {
        bloqueAbierto->setHashPrevio("0000");
    } else {
        Bloque* ultimo = cadena.getLast();
        bloqueAbierto->setHashPrevio(ultimo->getHash());
    }

    long intentos = bloqueAbierto->minar(dificultad);
    std::cout << "Bloque " << bloqueAbierto->getIndice() << " minado en " << intentos
              << " intento(s). Hash: " << bloqueAbierto->getHash() << "\n";

    cadena.addNodeLast(bloqueAbierto);

    // Se abre el siguiente bloque, vacio.
    int siguienteIndice = bloqueAbierto->getIndice() + 1;
    bloqueAbierto = new Bloque(siguienteIndice, obtenerTimestampActual());
}

int Blockchain::getDificultad() const { return dificultad; }

void Blockchain::setDificultad(int dificultad) {
    if (dificultad >= 1 && dificultad <= 6) {
        this->dificultad = dificultad;
    }
}

void Blockchain::reiniciar() {
    while (!cadena.isEmpty()) {
        delete cadena.deleteFirst();
    }
    delete bloqueAbierto;
    bloqueAbierto = new Bloque(0, obtenerTimestampActual());
    cursorNavegacion = nullptr; // el cursor apuntaba a nodos que ya no existen
}
Bloque* Blockchain::getBloqueAbierto() const { return bloqueAbierto; }
DoubleLinkedList<Bloque*>& Blockchain::getCadena() { return cadena; }

// ---------------- Opcion 3: navegar la cadena ----------------

void Blockchain::iniciarNavegacion() {
    cursorNavegacion = cadena.getHead();
}

bool Blockchain::avanzarCursor() {
    if (cursorNavegacion == nullptr) return false;
    Node<Bloque*>* siguiente = cursorNavegacion->getNext();
    if (siguiente == nullptr) return false; // ya se esta en el ultimo bloque
    cursorNavegacion = siguiente;
    return true;
}

bool Blockchain::retrocederCursor() {
    if (cursorNavegacion == nullptr) return false;
    Node<Bloque*>* anterior = cursorNavegacion->getPrevious();
    if (anterior == nullptr) return false; // ya se esta en el genesis
    cursorNavegacion = anterior;
    return true;
}

bool Blockchain::hayCursor() const {
    return cursorNavegacion != nullptr;
}

Bloque* Blockchain::getBloqueCursor() const {
    if (cursorNavegacion == nullptr) return nullptr;
    return cursorNavegacion->getInfo();
}

// ---------------- Opcion 4: validar integridad ----------------

int Blockchain::validarCadena() {
    if (cadena.isEmpty()) return -1;

    Node<Bloque*>* actual = cadena.getHead();
    std::string hashRealAnterior = "0000"; // convencion para el genesis

    while (actual != nullptr) {
        Bloque* bloque = actual->getInfo();

        // (a) el contenido actual del bloque debe seguir produciendo el hash guardado
        std::string hashReal = bloque->calcularHash();
        if (hashReal != bloque->getHash()) {
            return bloque->getIndice();
        }

        // (b) el hashPrevio guardado debe coincidir con el hash real del bloque anterior
        if (bloque->getHashPrevio() != hashRealAnterior) {
            return bloque->getIndice();
        }

        hashRealAnterior = hashReal;
        actual = actual->getNext();
    }

    return -1;
}

// ---------------- Opcion 5: trazabilidad de un lote ----------------

void Blockchain::trazabilidadLote(const std::string& idLote) {
    bool encontrada = false;
    Node<Bloque*>* actual = cadena.getHead();

    std::cout << "\nTrazabilidad del lote " << idLote << ":\n";

    // Primer eje: bloque por bloque, siguiendo la lista doble
    while (actual != nullptr) {
        Bloque* bloque = actual->getInfo();

        // Segundo eje: dentro del bloque, su lista sencilla de transacciones,
        // recorrida nodo por nodo (sin std::vector).
        NodoSimple<Transaccion>* nodoTx = bloque->getTransacciones().getCabeza();
        while (nodoTx != nullptr) {
            const Transaccion& t = nodoTx->getDato();
            if (t.getIdLote() == idLote) {
                std::cout << "  [Bloque " << bloque->getIndice() << "] " << t.getEtapa()
                          << " - actor: " << t.getActor() << ", peso: " << t.getPesoKg()
                          << " kg, fecha: " << t.getFecha() << "\n";
                encontrada = true;
            }
            nodoTx = nodoTx->getSiguiente();
        }

        actual = actual->getNext();
    }

    // El bloque abierto todavia no esta en la cadena, pero puede tener
    // transacciones del lote que aun no se han minado.
    NodoSimple<Transaccion>* nodoAbierto = bloqueAbierto->getTransacciones().getCabeza();
    while (nodoAbierto != nullptr) {
        const Transaccion& t = nodoAbierto->getDato();
        if (t.getIdLote() == idLote) {
            std::cout << "  [Bloque " << bloqueAbierto->getIndice() << ", aun no minado] "
                      << t.getEtapa() << " - actor: " << t.getActor() << ", peso: " << t.getPesoKg()
                      << " kg, fecha: " << t.getFecha() << "\n";
            encontrada = true;
        }
        nodoAbierto = nodoAbierto->getSiguiente();
    }

    if (!encontrada) {
        std::cout << "  No se encontraron transacciones para ese lote.\n";
    }
}

// ---------------- Opcion 6: simular manipulacion ----------------

Bloque* Blockchain::buscarBloquePorIndice(int indice) {
    Node<Bloque*>* actual = cadena.getHead();
    while (actual != nullptr) {
        if (actual->getInfo()->getIndice() == indice) {
            return actual->getInfo();
        }
        actual = actual->getNext();
    }
    return nullptr;
}

bool Blockchain::simularManipulacion(int indiceBloque, int posicionTransaccion, double nuevoPeso) {
    Bloque* bloque = buscarBloquePorIndice(indiceBloque);
    if (bloque == nullptr) return false;

    Transaccion* t = bloque->obtenerTransaccion(posicionTransaccion);
    if (t == nullptr) return false;

    // A proposito NO se vuelve a minar el bloque: el hash almacenado queda
    // desactualizado respecto al contenido, para que validarCadena() (opcion 4)
    // lo detecte al recalcularlo.
    t->setPesoKg(nuevoPeso);
    return true;
}

// ---------------- Bono: persistencia en archivo ----------------
//
// Formato de texto, un dato por linea (asi los textos pueden tener espacios):
//
//   CADENA_CAFE v1
//   <dificultad>
//   <cantidad de bloques sellados>
//   por cada bloque:  BLOQUE / indice / timestamp / hashPrevio / hash / nonce /
//                     cantidad de transacciones / 5 lineas por transaccion
//                     (lote, etapa, actor, pesoKg, fecha)
//   PENDIENTES / cantidad / 5 lineas por transaccion del bloque abierto
//   FIN

namespace {

const char* ENCABEZADO = "CADENA_CAFE v1";

void escribirLista(std::ofstream& out, ListaSimple<Transaccion>& lista) {
    out << lista.getSize() << "\n";
    NodoSimple<Transaccion>* actual = lista.getCabeza();
    while (actual != nullptr) {
        const Transaccion& t = actual->getDato();
        // setprecision(17): el peso se recupera exactamente igual al leerlo,
        // de lo contrario el hash recalculado no coincidiria con el guardado.
        out << t.getIdLote() << "\n" << t.getEtapa() << "\n" << t.getActor() << "\n"
            << std::setprecision(17) << t.getPesoKg() << "\n" << t.getFecha() << "\n";
        actual = actual->getSiguiente();
    }
}

// Lee una linea; si el archivo se acaba antes de tiempo, el formato es invalido.
std::string leerLineaArchivo(std::ifstream& in) {
    std::string linea;
    if (!std::getline(in, linea)) {
        throw std::runtime_error("archivo incompleto");
    }
    if (!linea.empty() && linea.back() == '\r') {
        linea.pop_back(); // archivo editado en Windows
    }
    return linea;
}

long long leerEnteroArchivo(std::ifstream& in) {
    std::string linea = leerLineaArchivo(in);
    size_t usados = 0;
    long long valor = std::stoll(linea, &usados); // lanza si no es un numero
    if (usados != linea.size()) {
        throw std::runtime_error("numero invalido");
    }
    return valor;
}

double leerDecimalArchivo(std::ifstream& in) {
    std::string linea = leerLineaArchivo(in);
    size_t usados = 0;
    double valor = std::stod(linea, &usados);
    if (usados != linea.size()) {
        throw std::runtime_error("decimal invalido");
    }
    return valor;
}

void esperarEtiqueta(std::ifstream& in, const std::string& etiqueta) {
    if (leerLineaArchivo(in) != etiqueta) {
        throw std::runtime_error("se esperaba " + etiqueta);
    }
}

void leerTransacciones(std::ifstream& in, Bloque* bloque) {
    long long cantidad = leerEnteroArchivo(in);
    if (cantidad < 0) {
        throw std::runtime_error("cantidad negativa");
    }
    for (long long i = 0; i < cantidad; i++) {
        // Cada campo se lee en su propia variable, en orden: el orden de
        // evaluacion de los argumentos de una funcion no esta garantizado.
        std::string lote = leerLineaArchivo(in);
        std::string etapa = leerLineaArchivo(in);
        std::string actor = leerLineaArchivo(in);
        double peso = leerDecimalArchivo(in);
        std::string fecha = leerLineaArchivo(in);
        bloque->agregarTransaccion(Transaccion(lote, etapa, actor, peso, fecha));
    }
}

} // namespace

bool Blockchain::guardarEnArchivo(const std::string& ruta) {
    std::ofstream out(ruta);
    if (!out) {
        return false;
    }

    out << ENCABEZADO << "\n" << dificultad << "\n" << cadena.getSize() << "\n";

    Node<Bloque*>* actual = cadena.getHead();
    while (actual != nullptr) {
        Bloque* b = actual->getInfo();
        out << "BLOQUE\n" << b->getIndice() << "\n" << b->getTimestamp() << "\n"
            << b->getHashPrevio() << "\n" << b->getHash() << "\n" << b->getNonce() << "\n";
        escribirLista(out, b->getTransacciones());
        actual = actual->getNext();
    }

    out << "PENDIENTES\n";
    escribirLista(out, bloqueAbierto->getTransacciones());
    out << "FIN\n";

    out.close();
    return !out.fail();
}

ResultadoCarga Blockchain::cargarDesdeArchivo(const std::string& ruta) {
    std::ifstream in(ruta);
    if (!in) {
        return CARGA_SIN_ARCHIVO;
    }

    reiniciar(); // se parte de una cadena vacia

    try {
        esperarEtiqueta(in, ENCABEZADO);

        long long nivel = leerEnteroArchivo(in);
        if (nivel < 1 || nivel > 6) {
            throw std::runtime_error("dificultad fuera de rango");
        }

        long long cantidad = leerEnteroArchivo(in);
        if (cantidad < 0) {
            throw std::runtime_error("cantidad negativa");
        }

        for (long long i = 0; i < cantidad; i++) {
            esperarEtiqueta(in, "BLOQUE");
            long long indice = leerEnteroArchivo(in);
            std::string timestamp = leerLineaArchivo(in);
            std::string hashPrevio = leerLineaArchivo(in);
            std::string hash = leerLineaArchivo(in);
            long long nonce = leerEnteroArchivo(in);
            if (indice < 0 || nonce < 0) {
                throw std::runtime_error("valor negativo");
            }

            // Se enlaza de inmediato: si algo falla mas adelante, reiniciar()
            // lo libera junto con el resto (sin fugas de memoria).
            Bloque* bloque = new Bloque(static_cast<int>(indice), timestamp);
            cadena.addNodeLast(bloque);
            leerTransacciones(in, bloque);
            bloque->restaurarSello(hashPrevio, hash, static_cast<unsigned long>(nonce));
        }

        // El bloque abierto continua la numeracion despues del ultimo sellado.
        esperarEtiqueta(in, "PENDIENTES");
        int siguiente = cadena.isEmpty() ? 0 : cadena.getLast()->getIndice() + 1;
        delete bloqueAbierto;
        bloqueAbierto = nullptr;
        bloqueAbierto = new Bloque(siguiente, obtenerTimestampActual());
        leerTransacciones(in, bloqueAbierto);

        esperarEtiqueta(in, "FIN");
        dificultad = static_cast<int>(nivel);
    } catch (const std::exception&) {
        reiniciar();
        return CARGA_ARCHIVO_INVALIDO;
    }

    return CARGA_OK;
}