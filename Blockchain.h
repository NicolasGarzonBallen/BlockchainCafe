//
// Created by nicol on 27/09/2026.
//

#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include <string>
#include "DoubleLinkedList.h"
#include "Node.h"
#include "Bloque.h"

// Resultado de cargar la cadena desde un archivo (bono de persistencia).
enum ResultadoCarga {
    CARGA_OK,               // se cargo el historial completo
    CARGA_SIN_ARCHIVO,      // el archivo no existe (primera ejecucion)
    CARGA_ARCHIVO_INVALIDO  // el archivo existe pero esta danado o incompleto
};

// La cadena es, literalmente, tu DoubleLinkedList<T> ya hecha en clase,
// instanciada con T = Bloque*. Blockchain no reimplementa el enlace doble:
// solo orquesta el "bloque abierto" (el que aun no se ha minado) y usa
// addNodeLast / getLast / deleteFirst / isEmpty de la lista que ya tienes.
class Blockchain {
private:
    DoubleLinkedList<Bloque*> cadena;
    int dificultad;
    Bloque* bloqueAbierto;

    // Cursor de navegacion (opcion 3): un puntero a Node<Bloque*>, no a Bloque*,
    // porque es el Node el que sabe moverse con getNext()/getPrevious().
    Node<Bloque*>* cursorNavegacion;

    std::string obtenerTimestampActual() const;

    // Libera todos los bloques y deja la cadena vacia con un bloque abierto nuevo.
    void reiniciar();

public:
    Blockchain(int dificultad);
    ~Blockchain();

    // Opcion 1 del menu: registra una transaccion en el bloque en construccion
    void registrarTransaccion(std::string idLote, std::string etapa, std::string actor, double pesoKg, std::string fecha);

    // Opcion 2 del menu: mina (cierra) el bloque abierto, lo enlaza a la cadena
    // e informa cuantos intentos (nonces) necesito. Abre un bloque nuevo y vacio.
    void minarBloqueActual();

    int getDificultad() const;
    void setDificultad(int dificultad);   // acepta valores de 1 a 6
    Bloque* getBloqueAbierto() const;
    DoubleLinkedList<Bloque*>& getCadena();

    // Opcion 3: navegar la cadena con un cursor que usa el doble enlace de Node<T>
    void iniciarNavegacion();
    bool avanzarCursor();
    bool retrocederCursor();
    bool hayCursor() const;
    Bloque* getBloqueCursor() const;

    // Nota: no se marcan "const" porque tu DoubleLinkedList<T> (isEmpty(), getFirst(),
    // getLast()) tampoco lo esta -- se respeta la clase tal como la entregaron en clase.

    // Opcion 4: recorre la cadena desde el genesis, recalcula cada hash y
    // verifica el encadenamiento. Devuelve -1 si es valida, o el indice del
    // primer bloque comprometido.
    int validarCadena();

    // Opcion 5: recorre los dos ejes (la cadena y, dentro de cada bloque, su
    // lista de transacciones) e imprime la historia de vida del lote.
    void trazabilidadLote(const std::string& idLote);

    // Opcion 6: busca un bloque ya minado por indice y altera una de sus
    // transacciones sin volver a minar, para demostrar la deteccion en la opcion 4.
    Bloque* buscarBloquePorIndice(int indice);
    bool simularManipulacion(int indiceBloque, int posicionTransaccion, double nuevoPeso);

    // Bono de persistencia: guarda y carga la cadena completa (bloques sellados,
    // con su hash y nonce originales, y los movimientos pendientes) en un archivo
    // de texto. Al cargar NO se vuelve a minar: se restaura tal cual se guardo.
    bool guardarEnArchivo(const std::string& ruta);
    ResultadoCarga cargarDesdeArchivo(const std::string& ruta);
};

#endif //BLOCKCHAIN_H