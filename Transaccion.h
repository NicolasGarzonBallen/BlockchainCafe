//
// Created by nicol on 27/09/2026.
//

#ifndef TRANSACCION_H
#define TRANSACCION_H

#include <string>

// Un evento de custodia: cosecha, beneficio, trilla, transporte, exportacion...
// Es un dato puro (sin punteros), pensado para viajar por valor dentro de
// la lista sencilla de cada bloque.

using namespace std;

class Transaccion {
private:
    string idLote;
    string etapa;
    string actor;
    double pesoKg;
    string fecha;

public:
    Transaccion();
    Transaccion(string idLote, string etapa, string actor, double pesoKg, string fecha);

    string getIdLote() const;
    string getEtapa() const;
    string getActor() const;
    double getPesoKg() const;
    std::string getFecha() const;

    // Usados en la opcion 6 (simular manipulacion)
    void setPesoKg(double pesoKg);
    void setActor(string actor);

    // Concatena todos los campos en un solo texto. Esto es lo que entra,
    // junto con el de las demas transacciones del bloque, al calculo del hash.
    string aTexto() const;
};

#endif //TRANSACCION_H