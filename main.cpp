//
// Interfaz de consola del sistema de trazabilidad del cafe.
//
// Este archivo solo se ocupa de hablar con la persona usuaria: pide los
// datos, los valida y muestra los resultados en lenguaje claro. Toda la
// logica (cadena, hash, minado, validacion) vive en la clase Blockchain.
//
// Nota: se usan solo caracteres ASCII (sin tildes ni simbolos) para que el
// texto se vea bien en cualquier consola, incluida la de Windows.
//

#include <cctype>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include "Blockchain.h"

// Archivo donde se guarda el historial (bono de persistencia). Se crea en la
// carpeta desde la que se ejecuta el programa; la interfaz grafica usa el mismo.
const std::string ARCHIVO_HISTORIAL = "cadena_cafe.txt";

// Se lanza si la entrada de datos se cierra (Ctrl+D, Ctrl+Z o fin de archivo).
// Al propagarse hasta main, los destructores liberan toda la memoria igual.
struct EntradaCerrada {};

// ======================================================================
//  Utilidades de lectura y validacion
// ======================================================================

// Quita espacios y saltos de linea al inicio y al final.
std::string recortar(const std::string& s) {
    size_t i = 0, j = s.size();
    while (i < j && std::isspace(static_cast<unsigned char>(s[i]))) i++;
    while (j > i && std::isspace(static_cast<unsigned char>(s[j - 1]))) j--;
    return s.substr(i, j - i);
}

std::string mayusculas(std::string s) {
    for (char& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
}

// Lee una linea completa (admite espacios, ej. "Finca La Esperanza").
std::string leerLinea(const std::string& etiqueta) {
    std::cout << etiqueta;
    std::string linea;
    if (!std::getline(std::cin, linea)) throw EntradaCerrada();
    return recortar(linea);
}

// Repite la pregunta hasta que se escriba algo.
std::string leerTextoObligatorio(const std::string& etiqueta) {
    while (true) {
        std::string texto = leerLinea(etiqueta);
        if (!texto.empty()) return texto;
        std::cout << "  [!] Este dato es obligatorio. Intente de nuevo.\n";
    }
}

// Lee un numero validando que sea numero completo y este dentro del rango.
// Si se pasa 'defecto', presionar Enter devuelve ese valor.
template <typename N>
N leerNumero(const std::string& etiqueta, N minimo, N maximo,
             const std::string& pista, const N* defecto = nullptr) {
    while (true) {
        std::string texto = leerLinea(etiqueta);
        if (texto.empty() && defecto != nullptr) return *defecto;

        std::istringstream iss(texto);
        N valor = N();
        if (!texto.empty() && (iss >> valor) && (iss >> std::ws).eof() &&
            valor >= minimo && valor <= maximo) {
            return valor;
        }
        std::cout << "  [!] " << pista << "\n";
    }
}

// Pregunta de si/no. Solo "S" o "SI" cuentan como si.
bool confirmar(const std::string& pregunta) {
    std::string r = mayusculas(leerLinea(pregunta));
    return r == "S" || r == "SI";
}

// Fecha de hoy en formato AAAA-MM-DD.
std::string fechaHoy() {
    std::time_t ahora = std::time(nullptr);
    char buffer[16];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", std::localtime(&ahora));
    return buffer;
}

bool fechaValida(const std::string& f) {
    if (f.size() != 10 || f[4] != '-' || f[7] != '-') return false;
    for (int i = 0; i < 10; i++) {
        if (i == 4 || i == 7) continue;
        if (!std::isdigit(static_cast<unsigned char>(f[i]))) return false;
    }
    int mes = std::stoi(f.substr(5, 2));
    int dia = std::stoi(f.substr(8, 2));
    return mes >= 1 && mes <= 12 && dia >= 1 && dia <= 31;
}

std::string leerFecha() {
    std::string hoy = fechaHoy();
    while (true) {
        std::string texto = leerLinea("Fecha del movimiento (AAAA-MM-DD) [Enter = hoy, " + hoy + "]: ");
        if (texto.empty()) return hoy;
        if (fechaValida(texto)) return texto;
        std::cout << "  [!] Formato no valido. Ejemplo: 2026-01-10\n";
    }
}

// Muestra las etapas habituales para que no haya que escribirlas (y equivocarse).
std::string leerEtapa() {
    std::cout << "Etapa del proceso:\n"
              << "   1. Cosecha\n"
              << "   2. Beneficio\n"
              << "   3. Trilla\n"
              << "   4. Almacenamiento\n"
              << "   5. Transporte\n"
              << "   6. Exportacion\n"
              << "   7. Otra (escribir el nombre)\n";
    int opcion = leerNumero<int>("Elija la etapa (1-7): ", 1, 7, "Escriba un numero entre 1 y 7.");
    switch (opcion) {
        case 1: return "cosecha";
        case 2: return "beneficio";
        case 3: return "trilla";
        case 4: return "almacenamiento";
        case 5: return "transporte";
        case 6: return "exportacion";
        default: return leerTextoObligatorio("Nombre de la etapa: ");
    }
}

// ======================================================================
//  Utilidades de presentacion
// ======================================================================

void linea(char c = '-') {
    std::cout << std::string(64, c) << "\n";
}

void titulo(const std::string& texto) {
    std::cout << "\n";
    linea('=');
    std::cout << "  " << texto << "\n";
    linea('=');
}

void pausar() {
    std::cout << "\nPresione Enter para volver al menu... ";
    std::string basura;
    std::getline(std::cin, basura);
}

// Lista los movimientos de un bloque, recorriendo su lista sencilla nodo por nodo.
void imprimirMovimientos(Bloque* b) {
    NodoSimple<Transaccion>* actual = b->getTransacciones().getCabeza();
    if (actual == nullptr) {
        std::cout << "   (sin movimientos)\n";
        return;
    }
    int i = 1;
    while (actual != nullptr) {
        const Transaccion& t = actual->getDato();
        std::cout << "   " << i << ". " << t.getFecha() << " | Lote " << t.getIdLote()
                  << " | " << t.getEtapa() << " | " << t.getActor()
                  << " | " << t.getPesoKg() << " kg\n";
        i++;
        actual = actual->getSiguiente();
    }
}

void imprimirBloque(Bloque* b, int total) {
    std::cout << "\n";
    linea();
    std::cout << " BLOQUE #" << b->getIndice() << "   (bloque " << b->getIndice() + 1
              << " de " << total << ")";
    if (b->getIndice() == 0) std::cout << "   - bloque inicial";
    std::cout << "\n";
    linea();
    std::cout << " Sellado el:                  " << b->getTimestamp() << "\n";
    std::cout << " Huella digital (hash):       " << b->getHash() << "\n";
    std::cout << " Huella del bloque anterior:  " << b->getHashPrevio();
    if (b->getIndice() == 0) std::cout << "  (no tiene anterior)";
    std::cout << "\n";
    std::cout << " Clave de sellado (nonce):    " << b->getNonce() << "\n\n";
    std::cout << " Movimientos registrados en este bloque:\n";
    imprimirMovimientos(b);
}

// ======================================================================
//  Pantallas
// ======================================================================

void mostrarBienvenida() {
    linea('=');
    std::cout << "   COOPERATIVA CAFETERA DE BOYACA\n";
    std::cout << "   Sistema de trazabilidad del cafe\n";
    linea('=');
    std::cout << "\n";
    std::cout << " Este sistema guarda el recorrido de cada lote de cafe, desde la\n";
    std::cout << " cosecha hasta la exportacion. Cada movimiento queda registrado y\n";
    std::cout << " protegido: si alguien intenta cambiar un dato ya guardado, el\n";
    std::cout << " sistema lo detecta. El historial se guarda automaticamente al salir.\n\n";
}

int pedirNivelSeguridad() {
    std::cout << "Antes de comenzar, elija el NIVEL DE SEGURIDAD (de 1 a 6).\n";
    std::cout << "Un nivel mas alto hace mas dificil alterar el historial, pero el\n";
    std::cout << "sellado de cada bloque tarda mas. Si no esta seguro, use 2.\n\n";
    int porDefecto = 2;
    return leerNumero<int>("Nivel de seguridad [Enter = 2]: ", 1, 6,
                           "Escriba un numero entre 1 y 6.", &porDefecto);
}

void mostrarMenu(Blockchain& bc) {
    int pendientes = bc.getBloqueAbierto()->getTransacciones().getSize();
    int sellados = bc.getCadena().getSize();

    titulo("MENU PRINCIPAL");
    std::cout << "  Bloques sellados: " << sellados
              << "   |   Movimientos pendientes de sellar: " << pendientes << "\n";
    linea();
    std::cout << "  1. Registrar un movimiento del cafe\n";
    std::cout << "  2. Sellar los movimientos pendientes (minar bloque)\n";
    std::cout << "  3. Ver el historial de bloques (navegar la cadena)\n";
    std::cout << "  4. Verificar que ningun registro fue alterado (validar)\n";
    std::cout << "  5. Consultar el recorrido de un lote de cafe (trazabilidad)\n";
    std::cout << "  6. Prueba de seguridad: alterar un dato (simular manipulacion)\n";
    std::cout << "  7. Salir (el historial se guarda automaticamente)\n";
    linea();
    std::cout << "  Paso a paso: registre movimientos (1), luego selle (2) y por ultimo\n";
    std::cout << "  consulte o verifique (3, 4 o 5).\n\n";
}

// ---------------------------------------------------------------- opcion 1
void accionRegistrar(Blockchain& bc) {
    titulo("REGISTRAR UN MOVIMIENTO DEL CAFE");
    std::cout << "Complete los datos del movimiento.\n\n";

    // El codigo de lote se guarda en mayusculas para que "l07" y "L07" sean el mismo lote.
    std::string lote = mayusculas(leerTextoObligatorio("Codigo del lote de cafe (ej. L07): "));
    std::string etapa = leerEtapa();
    std::string actor = leerTextoObligatorio(
        "Responsable de esta etapa (finca, cooperativa, transportador...)\n"
        "  Ej. Finca La Esperanza: ");
    double peso = leerNumero<double>(
        "Peso registrado en kg (decimales con punto, ej. 120.5): ", 0.01, 10000000.0,
        "Escriba un peso mayor que 0 (decimales con punto, ej. 120.5).");
    std::string fecha = leerFecha();

    bc.registrarTransaccion(lote, etapa, actor, peso, fecha);

    std::cout << "\n[OK] Movimiento registrado: lote " << lote << " | " << etapa << " | "
              << actor << " | " << peso << " kg | " << fecha << "\n";
    std::cout << "     Queda pendiente de sellar en el bloque #"
              << bc.getBloqueAbierto()->getIndice() << ". Cuando termine de registrar,\n"
              << "     use la opcion 2 para sellarlo de forma definitiva.\n";
}

// ---------------------------------------------------------------- opcion 2
void accionSellar(Blockchain& bc) {
    titulo("SELLAR LOS MOVIMIENTOS PENDIENTES");

    Bloque* abierto = bc.getBloqueAbierto();
    int pendientes = abierto->getTransacciones().getSize();
    int indice = abierto->getIndice();

    if (pendientes == 0) {
        std::cout << "[!] No hay movimientos pendientes de sellar.\n";
        std::cout << "    Primero registre al menos un movimiento (opcion 1).\n";
        return;
    }

    std::cout << "Se sellaran " << pendientes << " movimiento(s) en el bloque #" << indice << ".\n";
    std::cout << "Una vez sellado, ya no se podra modificar sin que el sistema lo detecte.\n";
    std::cout << "Sellando, por favor espere...\n\n";
    std::cout << "Detalle tecnico -> ";

    try {
        bc.minarBloqueActual();
        std::cout << "\n[OK] Bloque #" << indice << " sellado y guardado en el historial.\n";
    } catch (const std::exception& e) {
        std::cout << "\n[!] No se pudo sellar: " << e.what() << "\n";
    }
}

// ---------------------------------------------------------------- opcion 3
void accionNavegar(Blockchain& bc) {
    titulo("HISTORIAL DE BLOQUES");

    if (bc.getCadena().isEmpty()) {
        std::cout << "[!] Aun no hay bloques sellados.\n";
        std::cout << "    Registre movimientos (opcion 1) y luego selle (opcion 2).\n";
        return;
    }

    int total = bc.getCadena().getSize();
    bc.iniciarNavegacion();

    while (true) {
        imprimirBloque(bc.getBloqueCursor(), total);

        std::string r = mayusculas(leerLinea(
            "\n  [S] Siguiente bloque    [A] Bloque anterior    [Q] Volver al menu: "));

        if (r == "S") {
            if (!bc.avanzarCursor()) {
                std::cout << "\n  [!] Este es el ultimo bloque, no hay uno siguiente.\n";
            }
        } else if (r == "A") {
            if (!bc.retrocederCursor()) {
                std::cout << "\n  [!] Este es el primer bloque, no hay uno anterior.\n";
            }
        } else if (r == "Q") {
            return;
        } else {
            std::cout << "\n  [!] Opcion no valida. Escriba S, A o Q.\n";
        }
    }
}

// ---------------------------------------------------------------- opcion 4
void accionVerificar(Blockchain& bc) {
    titulo("VERIFICAR LA INTEGRIDAD DE LOS REGISTROS");

    if (bc.getCadena().isEmpty()) {
        std::cout << "[!] Aun no hay bloques sellados para verificar.\n";
        return;
    }

    std::cout << "Revisando " << bc.getCadena().getSize()
              << " bloque(s), desde el primero hasta el ultimo...\n\n";

    int resultado = bc.validarCadena();
    if (resultado == -1) {
        std::cout << "[OK] Todo en orden: ningun registro fue alterado.\n";
    } else {
        std::cout << "[ALERTA] Se detecto una alteracion en el bloque #" << resultado << ".\n";
        std::cout << "         Los datos de ese bloque (y de los siguientes) ya no son confiables.\n";
    }
}

// ---------------------------------------------------------------- opcion 5
void accionTrazabilidad(Blockchain& bc) {
    titulo("RECORRIDO DE UN LOTE DE CAFE");
    std::string lote = mayusculas(leerTextoObligatorio("Codigo del lote a consultar (ej. L07): "));
    bc.trazabilidadLote(lote);
}

// ---------------------------------------------------------------- opcion 6
void accionProbarAlteracion(Blockchain& bc) {
    titulo("PRUEBA DE SEGURIDAD: ALTERAR UN DATO");

    if (bc.getCadena().isEmpty()) {
        std::cout << "[!] Aun no hay bloques sellados para hacer la prueba.\n";
        std::cout << "    Registre movimientos (opcion 1) y luego selle (opcion 2).\n";
        return;
    }

    std::cout << "Esta prueba cambia A PROPOSITO el peso de un movimiento ya sellado,\n";
    std::cout << "sin volver a sellar el bloque, para comprobar que el sistema detecta\n";
    std::cout << "la alteracion. Sirve para demostrar que los registros no se pueden\n";
    std::cout << "modificar sin dejar rastro.\n\n";

    if (!confirmar("Desea continuar con la prueba? (S/N): ")) {
        std::cout << "\nPrueba cancelada. No se modifico ningun dato.\n";
        return;
    }

    int total = bc.getCadena().getSize();
    int indice = leerNumero<int>(
        "\nNumero del bloque a alterar (0 a " + std::to_string(total - 1) + "): ", 0, total - 1,
        "Escriba un numero de bloque entre 0 y " + std::to_string(total - 1) + ".");

    Bloque* bloque = bc.buscarBloquePorIndice(indice);
    std::cout << "\nMovimientos del bloque #" << indice << ":\n";
    imprimirMovimientos(bloque);

    int cantidad = bloque->getTransacciones().getSize();
    int posicion = leerNumero<int>(
        "\nNumero del movimiento a alterar (1 a " + std::to_string(cantidad) + "): ", 1, cantidad,
        "Escriba un numero entre 1 y " + std::to_string(cantidad) + ".");
    double nuevoPeso = leerNumero<double>(
        "Peso falso que se escribira, en kg (ej. 900): ", 0.01, 10000000.0,
        "Escriba un peso mayor que 0 (decimales con punto, ej. 900.5).");

    if (bc.simularManipulacion(indice, posicion, nuevoPeso)) {
        std::cout << "\n[!] Se altero el movimiento " << posicion << " del bloque #" << indice
                  << " SIN volver a sellarlo.\n";
        std::cout << "    Ahora use la opcion 4 para ver como el sistema detecta el cambio.\n";
    } else {
        std::cout << "\n[!] No se encontro ese bloque o ese movimiento. No se altero nada.\n";
    }
}

// ---------------------------------------------------------------- opcion 7
void avisarPendientesAlSalir(Blockchain& bc) {
    int pendientes = bc.getBloqueAbierto()->getTransacciones().getSize();
    if (pendientes > 0) {
        std::cout << "\n[i] Tiene " << pendientes << " movimiento(s) sin sellar. Se guardaran como\n"
                  << "    pendientes y podra sellarlos la proxima vez que abra el sistema.\n";
    }
}

// ---------------------------------------------------------------- persistencia
bool existeArchivo(const std::string& ruta) {
    std::ifstream f(ruta);
    return f.good();
}

void cargarHistorial(Blockchain& bc) {
    ResultadoCarga r = bc.cargarDesdeArchivo(ARCHIVO_HISTORIAL);
    if (r != CARGA_OK) {
        std::cout << "\n[!] El archivo " << ARCHIVO_HISTORIAL << " esta danado o incompleto.\n";
        std::cout << "    Se empezara un historial nuevo.\n\n";
        bc.setDificultad(pedirNivelSeguridad());
        return;
    }

    std::cout << "\n[OK] Historial cargado: " << bc.getCadena().getSize() << " bloque(s) sellado(s) y "
              << bc.getBloqueAbierto()->getTransacciones().getSize()
              << " movimiento(s) pendiente(s). Nivel de seguridad: " << bc.getDificultad() << ".\n";

    // Se verifica de una vez: si alguien edito el archivo a mano, se nota aqui.
    int resultado = bc.validarCadena();
    if (resultado == -1) {
        std::cout << "     Verificacion automatica: todo en orden.\n";
    } else {
        std::cout << "[ALERTA] Verificacion automatica: el historial guardado tiene una\n"
                  << "         alteracion en el bloque #" << resultado << ".\n";
    }
}

void guardarHistorial(Blockchain& bc) {
    if (bc.guardarEnArchivo(ARCHIVO_HISTORIAL)) {
        std::cout << "\n[OK] Historial guardado en " << ARCHIVO_HISTORIAL << " ("
                  << bc.getCadena().getSize() << " bloque(s) sellado(s), "
                  << bc.getBloqueAbierto()->getTransacciones().getSize() << " pendiente(s)).\n";
    } else {
        std::cout << "\n[!] No se pudo guardar el historial en " << ARCHIVO_HISTORIAL << ".\n";
    }
}

// ======================================================================
//  Programa principal
// ======================================================================

void ejecutarMenu(Blockchain& bc) {
    bool salir = false;
    while (!salir) {
        mostrarMenu(bc);
        int opcion = leerNumero<int>("Elija una opcion (1-7): ", 1, 7,
                                     "Opcion no valida. Escriba un numero del 1 al 7.");
        switch (opcion) {
            case 1: accionRegistrar(bc); break;
            case 2: accionSellar(bc); pausar(); break;
            case 3: accionNavegar(bc); break;
            case 4: accionVerificar(bc); pausar(); break;
            case 5: accionTrazabilidad(bc); pausar(); break;
            case 6: accionProbarAlteracion(bc); pausar(); break;
            case 7: avisarPendientesAlSalir(bc); salir = true; break;
        }
    }
}

int main() {
    // Los pesos se muestran siempre con 2 decimales (120.50 kg).
    std::cout << std::fixed << std::setprecision(2);

    mostrarBienvenida();

    try {
        bool cargar = false;
        if (existeArchivo(ARCHIVO_HISTORIAL)) {
            std::cout << "Se encontro un historial guardado (" << ARCHIVO_HISTORIAL << ").\n";
            cargar = confirmar("Desea continuar con ese historial? (S/N) [Enter = No]: ");
            if (!cargar) {
                std::cout << "Se empezara un historial nuevo; al salir reemplazara al guardado.\n\n";
            }
        }

        int nivel = cargar ? 2 : pedirNivelSeguridad(); // si se carga, el nivel sale del archivo

        // Al terminar este bloque, el destructor de Blockchain libera cada Bloque
        // y, con el, su lista de transacciones.
        Blockchain blockchain(nivel);
        if (cargar) {
            cargarHistorial(blockchain);
        }

        try {
            ejecutarMenu(blockchain);
        } catch (const EntradaCerrada&) {
            std::cout << "\n\nSe cerro la entrada de datos.\n";
        }

        guardarHistorial(blockchain);
        std::cout << "\nGracias por usar el sistema de trazabilidad. Hasta pronto.\n";
    } catch (const EntradaCerrada&) {
        std::cout << "\n\nSe cerro la entrada de datos. Hasta pronto.\n";
    }

    return 0;
}