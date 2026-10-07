//
// Created by nicol on 27/09/2026.
//

#include "Transaccion.h"
#include <sstream>
#include <iomanip>

using namespace std;

Transaccion::Transaccion() : pesoKg(0.0) {}

Transaccion::Transaccion(string idLote, string etapa, string actor, double pesoKg, string fecha)
    : idLote(idLote), etapa(etapa), actor(actor), pesoKg(pesoKg), fecha(fecha) {}

string Transaccion::getIdLote() const { return idLote; }
string Transaccion::getEtapa() const { return etapa; }
string Transaccion::getActor() const { return actor; }
double Transaccion::getPesoKg() const { return pesoKg; }
string Transaccion::getFecha() const { return fecha; }

void Transaccion::setPesoKg(double pesoKg) { this->pesoKg = pesoKg; }
void Transaccion::setActor(string actor) { this->actor = actor; }

string Transaccion::aTexto() const {
    ostringstream oss;
    // setprecision(17): con la precision por defecto (6 cifras) un cambio pequeno
    // en el peso (18000.75 -> 18000.76) imprimiria lo mismo y el hash no lo detectaria.
    oss << idLote << "|" << etapa << "|" << actor << "|"
        << setprecision(17) << pesoKg << "|" << fecha;
    return oss.str();
}