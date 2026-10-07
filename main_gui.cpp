//
// Created by nicol on 1/10/2026.
//

//
// Bono: punto de entrada de la interfaz grafica (Raylib).
//
// Usa el mismo archivo de historial que la version de consola, asi que lo que
// se registre en una se ve en la otra.
//

#include "Blockchain.h"
#include "InterfazGrafica.h"

int main() {
    // Nivel de seguridad inicial = 2. Si existe un historial guardado, se usa
    // el nivel guardado en el archivo. Se puede cambiar con los botones - y +.
    Blockchain blockchain(2);

    // La interfaz carga el historial al crearse y lo guarda al cerrarse.
    InterfazGrafica interfaz(blockchain, "cadena_cafe.txt");
    interfaz.ejecutar();

    // Al salir de main, el destructor de Blockchain libera toda la memoria.
    return 0;
}