//
// Created by nicol on 1/10/2026.
//

//
// Bono: interfaz grafica con Raylib.
//
// Dibuja la cadena de bloques (bloques enlazados con doble flecha y sus
// transacciones) y permite navegarla, ademas de registrar, sellar, verificar,
// rastrear un lote, probar una alteracion y guardar el historial.
//
// La interfaz no reimplementa nada de la logica: todo pasa por Blockchain,
// y la navegacion usa el mismo cursor (iniciarNavegacion / avanzarCursor /
// retrocederCursor) que la opcion 3 del menu de consola.
//

#ifndef INTERFAZGRAFICA_H
#define INTERFAZGRAFICA_H

#include <string>
#include "raylib.h"
#include "Blockchain.h"

// Una caja de texto editable del formulario.
struct CampoTexto {
    std::string texto;
    int maxLargo;
    bool soloNumeros;   // si es true, solo acepta digitos y punto decimal
};

class InterfazGrafica {
private:
    Blockchain& blockchain;
    std::string rutaArchivo;

    // ----- estado de la vista -----
    int posicionSeleccionada;   // posicion del bloque que se esta viendo (0 = genesis)
    bool viendoAbierto;         // true si se esta viendo el bloque abierto (sin sellar)
    bool mostrandoRastreo;      // true si el panel de detalle muestra el recorrido de un lote
    std::string loteRastreado;  // lote resaltado en la cadena (vacio = ninguno)
    int resultadoValidacion;    // -2 = sin verificar, -1 = cadena valida, >=0 = bloque alterado
    float desplazamiento;       // desplazamiento horizontal de la franja de bloques

    std::string mensaje;
    Color colorMensaje;

    // ----- formulario -----
    CampoTexto campoLote;
    CampoTexto campoActor;
    CampoTexto campoPeso;
    CampoTexto campoFecha;
    CampoTexto campoRastreo;
    CampoTexto campoMovimiento;
    CampoTexto campoPesoFalso;
    CampoTexto* campoActivo;
    int etapaSeleccionada;

    // ----- entrada y controles -----
    void procesarEntrada();
    bool boton(Rectangle r, const char* texto, bool habilitado = true);
    void campo(Rectangle r, const char* etiqueta, CampoTexto& c);
    void informar(const std::string& texto, Color color);

    // ----- navegacion -----
    int cantidadSellados();
    void seleccionar(int posicion);
    void irSiguiente();
    void irAnterior();
    void asegurarVisible(int posicion);

    // ----- acciones -----
    void registrarMovimiento();
    void sellarBloque();
    void verificarCadena();
    void guardarHistorial();
    void rastrearLote();
    void alterarDato();

    // ----- dibujo -----
    void dibujarEncabezado();
    void dibujarCadena();
    void dibujarTarjeta(Bloque* bloque, float x, bool seleccionada);
    void dibujarTarjetaAbierta(float x, bool seleccionada);
    void dibujarBarraAcciones();
    void dibujarDetalle();
    void dibujarDetalleBloque(Bloque* bloque, Rectangle panel);
    void dibujarDetalleAbierto(Rectangle panel);
    void dibujarRastreo(Rectangle panel);
    void dibujarPanelDerecho();
    void dibujarMensaje();

public:
    InterfazGrafica(Blockchain& blockchain, const std::string& rutaArchivo);

    // Abre la ventana y corre el ciclo principal hasta que se cierre.
    // Al cerrar, guarda el historial automaticamente.
    void ejecutar();
};

#endif //INTERFAZGRAFICA_H
