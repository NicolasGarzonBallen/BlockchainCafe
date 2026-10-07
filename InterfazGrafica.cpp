//
// Created by nicol on 1/10/2026.
//
//
// Bono: interfaz grafica con Raylib (implementacion).
//
// Se usa el estilo "modo inmediato" de Raylib: en cada cuadro se dibuja toda
// la pantalla, y cada boton se dibuja y a la vez informa si fue presionado.
// Solo se usan caracteres ASCII porque la fuente por defecto de Raylib no
// trae tildes.
//

#include "InterfazGrafica.h"

#include <cctype>
#include <ctime>
#include <string>
#include "Node.h"

namespace {

// ---------------------------------------------------------------- medidas
const int ANCHO = 1280;
const int ALTO = 760;

const float FRANJA_X = 20.0f;                       // franja donde se dibujan los bloques
const float FRANJA_ANCHO = ANCHO - 40.0f;
const float TARJETA_Y = 112.0f;
const float TARJETA_ANCHO = 210.0f;
const float TARJETA_ALTO = 150.0f;
const float TARJETA_PASO = 262.0f;                  // ancho + espacio para la doble flecha

const float PANEL_Y = 322.0f;
const float PANEL_ALTO = 386.0f;

// ---------------------------------------------------------------- colores
const Color FONDO         = {245, 241, 234, 255};   // crema
const Color CAFE_OSCURO   = { 74,  44,  29, 255};
const Color CAFE_MEDIO    = {124,  82,  52, 255};
const Color CAFE_CLARO    = {222, 205, 184, 255};
const Color BLANCO        = {255, 255, 255, 255};
const Color TEXTO         = { 40,  33,  28, 255};
const Color TEXTO_SUAVE   = {110, 100,  92, 255};
const Color AZUL          = { 33,  99, 171, 255};
const Color VERDE         = { 46, 125,  50, 255};
const Color VERDE_CLARO   = {220, 237, 214, 255};
const Color ROJO          = {183,  28,  28, 255};
const Color ROJO_CLARO    = {248, 215, 212, 255};
const Color NARANJA       = {211, 104,  16, 255};
const Color NARANJA_CLARO = {252, 233, 212, 255};
const Color DORADO        = {212, 160,  23, 255};
const Color GRIS          = {150, 150, 150, 255};
const Color GRIS_CLARO    = {250, 248, 244, 255};

const char* ETAPAS[] = {"cosecha", "beneficio", "trilla", "almacenamiento", "transporte", "exportacion"};
const int NUM_ETAPAS = 6;

// ---------------------------------------------------------------- utilidades de texto
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

bool convertirDecimal(const std::string& texto, double& valor) {
    if (texto.empty()) return false;
    try {
        size_t usados = 0;
        valor = std::stod(texto, &usados);
        return usados == texto.size();
    } catch (...) {
        return false;
    }
}

bool convertirEntero(const std::string& texto, int& valor) {
    if (texto.empty()) return false;
    try {
        size_t usados = 0;
        valor = std::stoi(texto, &usados);
        return usados == texto.size();
    } catch (...) {
        return false;
    }
}

// Peso con 2 decimales, sin depender de streams.
std::string formatoPeso(double kg) {
    long long centesimas = static_cast<long long>(kg * 100.0 + 0.5);
    std::string decimales = std::to_string(centesimas % 100);
    if (decimales.size() < 2) decimales = "0" + decimales;
    return std::to_string(centesimas / 100) + "." + decimales + " kg";
}

// Muestra solo el inicio de un hash: "005b77f8.."
std::string hashCorto(const std::string& hash, size_t largo) {
    if (hash.size() <= largo) return hash;
    return hash.substr(0, largo) + "..";
}

// Recorta un texto para que quepa en 'ancho' pixeles, terminando en "..".
std::string ajustarTexto(const std::string& texto, float ancho, int tam) {
    if (MeasureText(texto.c_str(), tam) <= ancho) return texto;
    std::string t = texto;
    while (!t.empty() && MeasureText((t + "..").c_str(), tam) > ancho) t.pop_back();
    return t + "..";
}

// ---------------------------------------------------------------- utilidades de dibujo
void texto(const std::string& t, float x, float y, int tam, Color color) {
    DrawText(t.c_str(), static_cast<int>(x), static_cast<int>(y), tam, color);
}

void textoCentrado(const std::string& t, Rectangle r, int tam, Color color) {
    float ancho = static_cast<float>(MeasureText(t.c_str(), tam));
    texto(t, r.x + (r.width - ancho) / 2.0f, r.y + (r.height - tam) / 2.0f, tam, color);
}

void textoDerecha(const std::string& t, float xDerecha, float y, int tam, Color color) {
    texto(t, xDerecha - static_cast<float>(MeasureText(t.c_str(), tam)), y, tam, color);
}

void rectangulo(float x, float y, float ancho, float alto, Color color) {
    DrawRectangleRec(Rectangle{x, y, ancho, alto}, color);
}

// Doble flecha horizontal (<-->): representa los enlaces anterior/siguiente.
void dobleFlecha(float x1, float x2, float y, Color color) {
    DrawLineEx(Vector2{x1 + 3.0f, y}, Vector2{x2 - 3.0f, y}, 2.5f, color);
    DrawLineEx(Vector2{x2 - 3.0f, y}, Vector2{x2 - 13.0f, y - 7.0f}, 2.5f, color);
    DrawLineEx(Vector2{x2 - 3.0f, y}, Vector2{x2 - 13.0f, y + 7.0f}, 2.5f, color);
    DrawLineEx(Vector2{x1 + 3.0f, y}, Vector2{x1 + 13.0f, y - 7.0f}, 2.5f, color);
    DrawLineEx(Vector2{x1 + 3.0f, y}, Vector2{x1 + 13.0f, y + 7.0f}, 2.5f, color);
}

// Linea punteada: el bloque abierto todavia no esta enlazado a la cadena.
void lineaPunteada(float x1, float x2, float y, Color color) {
    for (float x = x1 + 3.0f; x < x2 - 3.0f; x += 12.0f) {
        float fin = (x + 6.0f < x2 - 3.0f) ? x + 6.0f : x2 - 3.0f;
        DrawLineEx(Vector2{x, y}, Vector2{fin, y}, 2.0f, color);
    }
}

// Borde discontinuo para la tarjeta del bloque abierto.
void bordeDiscontinuo(Rectangle r, Color color) {
    for (float x = r.x; x < r.x + r.width; x += 14.0f) {
        float largo = (x + 8.0f < r.x + r.width) ? 8.0f : r.x + r.width - x;
        rectangulo(x, r.y, largo, 2.0f, color);
        rectangulo(x, r.y + r.height - 2.0f, largo, 2.0f, color);
    }
    for (float y = r.y; y < r.y + r.height; y += 14.0f) {
        float largo = (y + 8.0f < r.y + r.height) ? 8.0f : r.y + r.height - y;
        rectangulo(r.x, y, 2.0f, largo, color);
        rectangulo(r.x + r.width - 2.0f, y, 2.0f, largo, color);
    }
}

void tituloSeccion(const std::string& t, float x, float y) {
    texto(t, x, y, 18, CAFE_OSCURO);
}

bool contieneLote(Bloque* bloque, const std::string& lote) {
    NodoSimple<Transaccion>* actual = bloque->getTransacciones().getCabeza();
    while (actual != nullptr) {
        if (actual->getDato().getIdLote() == lote) return true;
        actual = actual->getSiguiente();
    }
    return false;
}

// Tabla de movimientos: columnas fijas dentro del panel de detalle.
void encabezadoTabla(float x, float y, float ancho) {
    rectangulo(x, y, ancho, 26.0f, CAFE_CLARO);
    texto("#", x + 12, y + 6, 15, CAFE_OSCURO);
    texto("Fecha", x + 46, y + 6, 15, CAFE_OSCURO);
    texto("Lote", x + 160, y + 6, 15, CAFE_OSCURO);
    texto("Etapa", x + 240, y + 6, 15, CAFE_OSCURO);
    texto("Responsable", x + 390, y + 6, 15, CAFE_OSCURO);
    textoDerecha("Peso", x + ancho - 12, y + 6, 15, CAFE_OSCURO);
}

void filaTabla(float x, float y, float ancho, const std::string& numero, const Transaccion& t, bool resaltar) {
    if (resaltar) rectangulo(x, y - 3, ancho, 24.0f, Color{252, 243, 207, 255});
    texto(numero, x + 12, y, 16, TEXTO);
    texto(t.getFecha(), x + 46, y, 16, TEXTO);
    texto(ajustarTexto(t.getIdLote(), 72, 16), x + 160, y, 16, TEXTO);
    texto(ajustarTexto(t.getEtapa(), 140, 16), x + 240, y, 16, TEXTO);
    texto(ajustarTexto(t.getActor(), ancho - 390 - 120, 16), x + 390, y, 16, TEXTO);
    textoDerecha(formatoPeso(t.getPesoKg()), x + ancho - 12, y, 16, TEXTO);
}

} // namespace

// ======================================================================
//  Construccion y ciclo principal
// ======================================================================

InterfazGrafica::InterfazGrafica(Blockchain& blockchain, const std::string& rutaArchivo)
    : blockchain(blockchain), rutaArchivo(rutaArchivo),
      posicionSeleccionada(0), viendoAbierto(true), mostrandoRastreo(false),
      resultadoValidacion(-2), desplazamiento(0.0f), colorMensaje(CAFE_OSCURO),
      campoLote{"", 12, false}, campoActor{"", 40, false}, campoPeso{"", 10, true},
      campoFecha{fechaHoy(), 10, false}, campoRastreo{"", 12, false},
      campoMovimiento{"", 4, true}, campoPesoFalso{"", 10, true},
      campoActivo(nullptr), etapaSeleccionada(0) {

    // Se intenta cargar el historial guardado (bono de persistencia).
    ResultadoCarga carga = blockchain.cargarDesdeArchivo(rutaArchivo);
    if (carga == CARGA_OK) {
        resultadoValidacion = blockchain.validarCadena();
        std::string resumen = "Historial cargado: " + std::to_string(cantidadSellados()) +
                              " bloque(s) sellado(s) y " +
                              std::to_string(blockchain.getBloqueAbierto()->getTransacciones().getSize()) +
                              " pendiente(s).";
        if (resultadoValidacion == -1) {
            informar(resumen + " Verificacion automatica: todo en orden.", VERDE);
        } else {
            informar(resumen + " ALERTA: el bloque #" + std::to_string(resultadoValidacion) +
                     " fue alterado.", ROJO);
        }
    } else if (carga == CARGA_ARCHIVO_INVALIDO) {
        informar("El archivo de historial esta danado; se empezo un historial nuevo.", ROJO);
    } else {
        informar("Bienvenido. Registre el primer movimiento en el panel de la derecha.", CAFE_OSCURO);
    }

    if (cantidadSellados() > 0) {
        seleccionar(0);
    } else {
        viendoAbierto = true;
    }
}

void InterfazGrafica::ejecutar() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);   // la ventana se puede redimensionar o maximizar
    InitWindow(ANCHO, ALTO, "Cooperativa Cafetera de Boyaca - Trazabilidad del cafe");
    SetWindowMinSize(ANCHO / 2, ALTO / 2);
    SetExitKey(KEY_NULL);   // ESC no cierra la ventana (se podria presionar escribiendo)
    SetTargetFPS(60);

    // Si la pantalla es mas pequena que la interfaz (por ejemplo 1366x768),
    // la ventana inicial se reduce para que quepa completa.
    int monitor = GetCurrentMonitor();
    int maxAncho = GetMonitorWidth(monitor) - 40;
    int maxAlto = GetMonitorHeight(monitor) - 120;   // barra de titulo y barra de tareas
    if (maxAncho > 0 && maxAlto > 0 && (ANCHO > maxAncho || ALTO > maxAlto)) {
        float e = (float)maxAncho / ANCHO < (float)maxAlto / ALTO ? (float)maxAncho / ANCHO : (float)maxAlto / ALTO;
        int ancho = static_cast<int>(ANCHO * e);
        int alto = static_cast<int>(ALTO * e);
        SetWindowSize(ancho, alto);
        SetWindowPosition((GetMonitorWidth(monitor) - ancho) / 2, (maxAlto - alto) / 2 + 30);
    }

    // Toda la interfaz se dibuja en un lienzo fijo de ANCHO x ALTO y luego se
    // escala para que quepa completo en la ventana, sea cual sea su tamano.
    RenderTexture2D lienzo = LoadRenderTexture(ANCHO, ALTO);
    SetTextureFilter(lienzo.texture, TEXTURE_FILTER_BILINEAR);

    while (!WindowShouldClose()) {
        // Escala y margenes para centrar el lienzo dentro de la ventana.
        float escalaX = static_cast<float>(GetScreenWidth()) / ANCHO;
        float escalaY = static_cast<float>(GetScreenHeight()) / ALTO;
        float escala = escalaX < escalaY ? escalaX : escalaY;
        float margenX = (GetScreenWidth() - ANCHO * escala) / 2.0f;
        float margenY = (GetScreenHeight() - ALTO * escala) / 2.0f;

        // El raton se traduce a coordenadas del lienzo, asi los botones siguen
        // funcionando aunque la ventana este escalada.
        SetMouseOffset(static_cast<int>(-margenX), static_cast<int>(-margenY));
        SetMouseScale(1.0f / escala, 1.0f / escala);

        procesarEntrada();

        BeginTextureMode(lienzo);
        ClearBackground(FONDO);
        dibujarEncabezado();
        dibujarCadena();
        dibujarBarraAcciones();
        dibujarDetalle();
        dibujarPanelDerecho();
        dibujarMensaje();
        EndTextureMode();

        BeginDrawing();
        ClearBackground(CAFE_OSCURO);
        // La altura negativa en el origen voltea la imagen (las texturas de
        // OpenGL quedan al reves respecto a la pantalla).
        DrawTexturePro(lienzo.texture,
                       Rectangle{0, 0, static_cast<float>(ANCHO), -static_cast<float>(ALTO)},
                       Rectangle{margenX, margenY, ANCHO * escala, ALTO * escala},
                       Vector2{0, 0}, 0.0f, BLANCO);
        EndDrawing();
    }

    UnloadRenderTexture(lienzo);
    CloseWindow();

    // Guardado automatico al cerrar.
    blockchain.guardarEnArchivo(rutaArchivo);
}

// ======================================================================
//  Entrada y controles
// ======================================================================

void InterfazGrafica::procesarEntrada() {
    // Un clic en cualquier parte suelta el campo activo; si el clic cae sobre
    // un campo, campo() lo vuelve a activar al dibujarlo en este mismo cuadro.
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        campoActivo = nullptr;
    }

    if (campoActivo != nullptr) {
        int c = GetCharPressed();
        while (c > 0) {
            bool permitido = (c >= 32 && c <= 126);
            if (campoActivo->soloNumeros) {
                permitido = std::isdigit(c) || c == '.';
            }
            if (permitido && static_cast<int>(campoActivo->texto.size()) < campoActivo->maxLargo) {
                campoActivo->texto.push_back(static_cast<char>(c));
            }
            c = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE) && !campoActivo->texto.empty()) {
            campoActivo->texto.pop_back();
        }
    } else {
        // Flechas del teclado: navegar la cadena (solo si no se esta escribiendo).
        if (IsKeyPressed(KEY_RIGHT)) irSiguiente();
        if (IsKeyPressed(KEY_LEFT)) irAnterior();
    }

    // Rueda del raton sobre la franja: desplazar la cadena.
    float rueda = GetMouseWheelMove();
    Vector2 raton = GetMousePosition();
    if (rueda != 0.0f && raton.y >= TARJETA_Y && raton.y <= TARJETA_Y + TARJETA_ALTO) {
        desplazamiento -= rueda * 60.0f;
    }
}

bool InterfazGrafica::boton(Rectangle r, const char* etiqueta, bool habilitado) {
    bool encima = habilitado && CheckCollisionPointRec(GetMousePosition(), r);
    Color fondo = !habilitado ? CAFE_CLARO : (encima ? CAFE_OSCURO : CAFE_MEDIO);
    DrawRectangleRec(r, fondo);
    textoCentrado(etiqueta, r, 16, habilitado ? BLANCO : TEXTO_SUAVE);
    return encima && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

void InterfazGrafica::campo(Rectangle r, const char* etiqueta, CampoTexto& c) {
    texto(etiqueta, r.x, r.y - 17, 14, TEXTO_SUAVE);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), r)) {
        campoActivo = &c;
    }
    bool activo = (campoActivo == &c);

    DrawRectangleRec(r, BLANCO);
    DrawRectangleLinesEx(r, activo ? 2.0f : 1.0f, activo ? AZUL : GRIS);

    // Si el texto no cabe, se muestra el final (lo que se esta escribiendo).
    std::string visible = c.texto;
    while (!visible.empty() && MeasureText(visible.c_str(), 16) > r.width - 16) {
        visible.erase(0, 1);
    }
    texto(visible, r.x + 7, r.y + (r.height - 16) / 2.0f, 16, TEXTO);

    // Cursor parpadeante.
    if (activo && static_cast<int>(GetTime() * 2.0) % 2 == 0) {
        float x = r.x + 8 + static_cast<float>(MeasureText(visible.c_str(), 16));
        rectangulo(x, r.y + 6, 2.0f, r.height - 12, TEXTO);
    }
}

void InterfazGrafica::informar(const std::string& t, Color color) {
    mensaje = t;
    colorMensaje = color;
}

// ======================================================================
//  Navegacion (usa el cursor de Blockchain, igual que la opcion 3)
// ======================================================================

int InterfazGrafica::cantidadSellados() {
    return blockchain.getCadena().getSize();
}

// posicion == cantidadSellados() significa "el bloque abierto".
void InterfazGrafica::seleccionar(int posicion) {
    mostrandoRastreo = false;
    if (posicion >= cantidadSellados()) {
        viendoAbierto = true;
        posicionSeleccionada = cantidadSellados();
    } else {
        blockchain.iniciarNavegacion();
        for (int i = 0; i < posicion; i++) {
            blockchain.avanzarCursor();
        }
        viendoAbierto = false;
        posicionSeleccionada = posicion;
    }
    asegurarVisible(posicionSeleccionada);
}

void InterfazGrafica::irSiguiente() {
    mostrandoRastreo = false;
    if (viendoAbierto) {
        informar("Este es el bloque abierto: no hay un bloque siguiente.", CAFE_OSCURO);
        return;
    }
    if (blockchain.avanzarCursor()) {
        posicionSeleccionada++;
    } else {
        // Despues del ultimo bloque sellado viene el bloque abierto.
        viendoAbierto = true;
        posicionSeleccionada = cantidadSellados();
    }
    asegurarVisible(posicionSeleccionada);
}

void InterfazGrafica::irAnterior() {
    mostrandoRastreo = false;
    if (viendoAbierto) {
        if (cantidadSellados() == 0) {
            informar("Aun no hay bloques sellados.", CAFE_OSCURO);
        } else {
            seleccionar(cantidadSellados() - 1);
        }
        return;
    }
    if (blockchain.retrocederCursor()) {
        posicionSeleccionada--;
    } else {
        informar("Este es el primer bloque (genesis): no hay uno anterior.", CAFE_OSCURO);
    }
    asegurarVisible(posicionSeleccionada);
}

void InterfazGrafica::asegurarVisible(int posicion) {
    float x = posicion * TARJETA_PASO;
    if (x < desplazamiento) desplazamiento = x;
    if (x + TARJETA_ANCHO > desplazamiento + FRANJA_ANCHO) {
        desplazamiento = x + TARJETA_ANCHO - FRANJA_ANCHO;
    }
}

// ======================================================================
//  Acciones
// ======================================================================

void InterfazGrafica::registrarMovimiento() {
    std::string lote = mayusculas(recortar(campoLote.texto));
    std::string actor = recortar(campoActor.texto);
    std::string fecha = recortar(campoFecha.texto);
    double peso = 0.0;

    if (lote.empty()) { informar("Escriba el codigo del lote (ej. L07).", ROJO); return; }
    if (actor.empty()) { informar("Escriba quien es el responsable de esta etapa.", ROJO); return; }
    if (!convertirDecimal(recortar(campoPeso.texto), peso) || peso <= 0.0) {
        informar("Peso no valido: escriba un numero mayor que 0 (decimales con punto).", ROJO);
        return;
    }
    if (!fechaValida(fecha)) { informar("Fecha no valida: use el formato AAAA-MM-DD.", ROJO); return; }

    blockchain.registrarTransaccion(lote, ETAPAS[etapaSeleccionada], actor, peso, fecha);

    // Se conserva el lote (suele repetirse) y se limpian los demas datos.
    campoActor.texto.clear();
    campoPeso.texto.clear();
    seleccionar(cantidadSellados());   // mostrar el bloque abierto con el nuevo movimiento
    informar("Movimiento registrado en el bloque abierto #" +
             std::to_string(blockchain.getBloqueAbierto()->getIndice()) +
             ". Pulse 'Sellar bloque' para protegerlo.", VERDE);
}

void InterfazGrafica::sellarBloque() {
    if (!blockchain.getBloqueAbierto()->tieneTransacciones()) {
        informar("No hay movimientos pendientes: registre al menos uno antes de sellar.", ROJO);
        return;
    }
    try {
        blockchain.minarBloqueActual();
    } catch (const std::exception& e) {
        informar(std::string("No se pudo sellar: ") + e.what(), ROJO);
        return;
    }

    Bloque* sellado = blockchain.getCadena().getLast();
    resultadoValidacion = -2;   // la cadena cambio: hay que volver a verificar
    seleccionar(cantidadSellados() - 1);
    informar("Bloque #" + std::to_string(sellado->getIndice()) + " sellado en " +
             std::to_string(sellado->getNonce() + 1) + " intentos. Hash: " + sellado->getHash(), VERDE);
}

void InterfazGrafica::verificarCadena() {
    if (cantidadSellados() == 0) {
        informar("Aun no hay bloques sellados para verificar.", CAFE_OSCURO);
        return;
    }
    resultadoValidacion = blockchain.validarCadena();
    if (resultadoValidacion == -1) {
        informar("Cadena valida: ningun registro fue alterado.", VERDE);
    } else {
        informar("ALERTA: se detecto una alteracion en el bloque #" + std::to_string(resultadoValidacion) +
                 ". Ese bloque y los siguientes ya no son confiables.", ROJO);
    }
}

void InterfazGrafica::guardarHistorial() {
    if (blockchain.guardarEnArchivo(rutaArchivo)) {
        informar("Historial guardado en " + rutaArchivo + ".", VERDE);
    } else {
        informar("No se pudo guardar el historial en " + rutaArchivo + ".", ROJO);
    }
}

void InterfazGrafica::rastrearLote() {
    std::string lote = mayusculas(recortar(campoRastreo.texto));
    if (lote.empty()) {
        informar("Escriba el codigo del lote que desea rastrear.", ROJO);
        return;
    }
    loteRastreado = lote;
    mostrandoRastreo = true;
    informar("Mostrando el recorrido del lote " + lote + ". Sus bloques se marcan en dorado.", CAFE_OSCURO);
}

void InterfazGrafica::alterarDato() {
    if (viendoAbierto || cantidadSellados() == 0) {
        informar("Seleccione primero un bloque SELLADO en la cadena.", ROJO);
        return;
    }
    Bloque* bloque = blockchain.getBloqueCursor();
    int cantidad = bloque->getTransacciones().getSize();
    int posicion = 0;
    double peso = 0.0;

    if (!convertirEntero(recortar(campoMovimiento.texto), posicion) || posicion < 1 || posicion > cantidad) {
        informar("El numero de movimiento debe estar entre 1 y " + std::to_string(cantidad) + ".", ROJO);
        return;
    }
    if (!convertirDecimal(recortar(campoPesoFalso.texto), peso) || peso <= 0.0) {
        informar("Peso falso no valido: escriba un numero mayor que 0.", ROJO);
        return;
    }

    blockchain.simularManipulacion(bloque->getIndice(), posicion, peso);
    resultadoValidacion = -2;
    mostrandoRastreo = false;
    informar("Dato alterado en el bloque #" + std::to_string(bloque->getIndice()) +
             " SIN volver a sellarlo. Pulse 'Verificar cadena'.", NARANJA);
}

// ======================================================================
//  Dibujo
// ======================================================================

void InterfazGrafica::dibujarEncabezado() {
    rectangulo(0, 0, ANCHO, 70, CAFE_OSCURO);
    texto("COOPERATIVA CAFETERA DE BOYACA", 20, 12, 24, BLANCO);
    texto("Sistema de trazabilidad del cafe", 20, 42, 16, CAFE_CLARO);

    std::string estado = "Bloques sellados: " + std::to_string(cantidadSellados()) +
                         "      Pendientes de sellar: " +
                         std::to_string(blockchain.getBloqueAbierto()->getTransacciones().getSize());
    textoDerecha(estado, ANCHO - 20.0f, 14, 16, BLANCO);

    // Nivel de seguridad (dificultad), ajustable con - y +.
    int nivel = blockchain.getDificultad();
    textoDerecha("Nivel de seguridad: " + std::to_string(nivel), ANCHO - 96.0f, 43, 16, CAFE_CLARO);
    if (boton(Rectangle{ANCHO - 86.0f, 38, 30, 24}, "-", nivel > 1)) {
        blockchain.setDificultad(nivel - 1);
        informar("Nivel de seguridad: " + std::to_string(nivel - 1) + " (aplica a los proximos sellos).", CAFE_OSCURO);
    }
    if (boton(Rectangle{ANCHO - 50.0f, 38, 30, 24}, "+", nivel < 6)) {
        blockchain.setDificultad(nivel + 1);
        informar("Nivel de seguridad: " + std::to_string(nivel + 1) + " (aplica a los proximos sellos).", CAFE_OSCURO);
    }
}

void InterfazGrafica::dibujarCadena() {
    texto("CADENA DE BLOQUES", FRANJA_X, 84, 18, CAFE_OSCURO);
    texto("clic en un bloque para verlo  -  flechas del teclado o rueda del raton para moverse",
          FRANJA_X + 200, 87, 14, TEXTO_SUAVE);

    int sellados = cantidadSellados();
    float contenido = (sellados + 1) * TARJETA_PASO - (TARJETA_PASO - TARJETA_ANCHO);
    float maximo = contenido > FRANJA_ANCHO ? contenido - FRANJA_ANCHO : 0.0f;
    if (desplazamiento < 0.0f) desplazamiento = 0.0f;
    if (desplazamiento > maximo) desplazamiento = maximo;

    bool clic = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    Vector2 raton = GetMousePosition();
    bool ratonEnFranja = raton.x >= FRANJA_X && raton.x <= FRANJA_X + FRANJA_ANCHO;

    BeginScissorMode(static_cast<int>(FRANJA_X), static_cast<int>(TARJETA_Y - 6),
                     static_cast<int>(FRANJA_ANCHO), static_cast<int>(TARJETA_ALTO + 12));

    // Recorrido de la lista doble, nodo por nodo, con los enlaces de Node<T>.
    Node<Bloque*>* nodo = blockchain.getCadena().getHead();
    int posicion = 0;
    int clicEn = -1;
    while (nodo != nullptr) {
        float x = FRANJA_X + posicion * TARJETA_PASO - desplazamiento;
        if (posicion > 0) {
            dobleFlecha(x - (TARJETA_PASO - TARJETA_ANCHO), x, TARJETA_Y + TARJETA_ALTO / 2.0f, CAFE_OSCURO);
        }
        dibujarTarjeta(nodo->getInfo(), x, !viendoAbierto && posicion == posicionSeleccionada);
        if (clic && ratonEnFranja && CheckCollisionPointRec(raton, Rectangle{x, TARJETA_Y, TARJETA_ANCHO, TARJETA_ALTO})) {
            clicEn = posicion;
        }
        nodo = nodo->getNext();
        posicion++;
    }

    // Bloque abierto al final (aun no forma parte de la lista doble).
    float x = FRANJA_X + posicion * TARJETA_PASO - desplazamiento;
    if (posicion > 0) {
        lineaPunteada(x - (TARJETA_PASO - TARJETA_ANCHO), x, TARJETA_Y + TARJETA_ALTO / 2.0f, GRIS);
    }
    dibujarTarjetaAbierta(x, viendoAbierto);
    if (clic && ratonEnFranja && CheckCollisionPointRec(raton, Rectangle{x, TARJETA_Y, TARJETA_ANCHO, TARJETA_ALTO})) {
        clicEn = posicion;
    }

    EndScissorMode();

    if (clicEn >= 0) {
        seleccionar(clicEn);
    }
}

void InterfazGrafica::dibujarTarjeta(Bloque* bloque, float x, bool seleccionada) {
    Rectangle r = {x, TARJETA_Y, TARJETA_ANCHO, TARJETA_ALTO};
    int indice = bloque->getIndice();

    // Color segun el ultimo resultado de la verificacion.
    Color cabecera = CAFE_MEDIO;
    Color cuerpo = BLANCO;
    std::string estado;
    if (resultadoValidacion == -1) {
        cabecera = VERDE; cuerpo = VERDE_CLARO; estado = "OK";
    } else if (resultadoValidacion >= 0) {
        if (indice < resultadoValidacion) {
            cabecera = VERDE; cuerpo = VERDE_CLARO; estado = "OK";
        } else if (indice == resultadoValidacion) {
            cabecera = ROJO; cuerpo = ROJO_CLARO; estado = "ALTERADO";
        } else {
            cabecera = NARANJA; cuerpo = NARANJA_CLARO; estado = "NO CONFIABLE";
        }
    }

    DrawRectangleRec(r, cuerpo);
    rectangulo(x, TARJETA_Y, TARJETA_ANCHO, 30, cabecera);
    texto("BLOQUE #" + std::to_string(indice), x + 10, TARJETA_Y + 7, 18, BLANCO);
    if (!estado.empty()) {
        textoDerecha(estado, x + TARJETA_ANCHO - 10, TARJETA_Y + TARJETA_ALTO - 26, 13, cabecera);
    }

    float y = TARJETA_Y + 40;
    texto("Hash:   " + hashCorto(bloque->getHash(), 10), x + 10, y, 14, TEXTO);
    texto("Previo: " + hashCorto(bloque->getHashPrevio(), 10), x + 10, y + 22, 14, TEXTO);
    texto("Nonce:  " + std::to_string(bloque->getNonce()), x + 10, y + 44, 14, TEXTO);
    texto("Movimientos: " + std::to_string(bloque->getTransacciones().getSize()), x + 10, y + 66, 14, TEXTO);

    // Marca dorada si contiene el lote que se esta rastreando.
    if (!loteRastreado.empty() && contieneLote(bloque, loteRastreado)) {
        rectangulo(x, TARJETA_Y + TARJETA_ALTO - 8, TARJETA_ANCHO, 8, DORADO);
    }

    DrawRectangleLinesEx(r, seleccionada ? 4.0f : 1.5f, seleccionada ? AZUL : CAFE_MEDIO);
}

void InterfazGrafica::dibujarTarjetaAbierta(float x, bool seleccionada) {
    Bloque* abierto = blockchain.getBloqueAbierto();
    Rectangle r = {x, TARJETA_Y, TARJETA_ANCHO, TARJETA_ALTO};

    DrawRectangleRec(r, GRIS_CLARO);
    rectangulo(x, TARJETA_Y, TARJETA_ANCHO, 30, GRIS);
    texto("BLOQUE #" + std::to_string(abierto->getIndice()), x + 10, TARJETA_Y + 7, 18, BLANCO);
    textoDerecha("ABIERTO", x + TARJETA_ANCHO - 8, TARJETA_Y + 10, 12, BLANCO);

    float y = TARJETA_Y + 40;
    texto("Aun sin sellar", x + 10, y, 14, TEXTO_SUAVE);
    texto("Pendientes: " + std::to_string(abierto->getTransacciones().getSize()), x + 10, y + 22, 14, TEXTO);
    texto("Registre movimientos y", x + 10, y + 52, 14, TEXTO_SUAVE);
    texto("pulse 'Sellar bloque'", x + 10, y + 70, 14, TEXTO_SUAVE);

    if (!loteRastreado.empty() && contieneLote(abierto, loteRastreado)) {
        rectangulo(x, TARJETA_Y + TARJETA_ALTO - 8, TARJETA_ANCHO, 8, DORADO);
    }

    if (seleccionada) {
        DrawRectangleLinesEx(r, 4.0f, AZUL);
    } else {
        bordeDiscontinuo(r, GRIS);
    }
}

void InterfazGrafica::dibujarBarraAcciones() {
    float y = 276;
    if (boton(Rectangle{20, y, 130, 34}, "< Anterior")) irAnterior();
    if (boton(Rectangle{160, y, 130, 34}, "Siguiente >")) irSiguiente();

    bool hayPendientes = blockchain.getBloqueAbierto()->tieneTransacciones();
    if (boton(Rectangle{ANCHO - 620.0f, y, 220, 34}, "Sellar bloque (minar)", hayPendientes)) sellarBloque();
    if (boton(Rectangle{ANCHO - 390.0f, y, 180, 34}, "Verificar cadena", cantidadSellados() > 0)) verificarCadena();
    if (boton(Rectangle{ANCHO - 200.0f, y, 180, 34}, "Guardar historial")) guardarHistorial();
}

void InterfazGrafica::dibujarDetalle() {
    Rectangle panel = {20, PANEL_Y, 780, PANEL_ALTO};
    DrawRectangleRec(panel, BLANCO);
    DrawRectangleLinesEx(panel, 1.0f, CAFE_CLARO);

    if (mostrandoRastreo) {
        dibujarRastreo(panel);
    } else if (viendoAbierto || cantidadSellados() == 0) {
        dibujarDetalleAbierto(panel);
    } else {
        dibujarDetalleBloque(blockchain.getBloqueCursor(), panel);
    }
}

void InterfazGrafica::dibujarDetalleBloque(Bloque* bloque, Rectangle panel) {
    float x = panel.x + 16;
    float y = panel.y + 12;
    std::string titulo = "BLOQUE #" + std::to_string(bloque->getIndice());
    if (bloque->getIndice() == 0) titulo += "  (genesis)";
    tituloSeccion(titulo, x, y);
    textoDerecha("Sellado: " + bloque->getTimestamp(), panel.x + panel.width - 16, y + 3, 14, TEXTO_SUAVE);

    texto("Hash:", x, y + 30, 15, TEXTO_SUAVE);
    texto(bloque->getHash(), x + 150, y + 30, 15, TEXTO);
    texto("Hash anterior:", x, y + 52, 15, TEXTO_SUAVE);
    texto(bloque->getHashPrevio(), x + 150, y + 52, 15, TEXTO);
    texto("Nonce:", x, y + 74, 15, TEXTO_SUAVE);
    texto(std::to_string(bloque->getNonce()) + "   (" + std::to_string(bloque->getNonce() + 1) +
          " intentos para sellarlo)", x + 150, y + 74, 15, TEXTO);

    // Segundo eje: la lista sencilla de transacciones del bloque.
    float tablaY = y + 104;
    encabezadoTabla(panel.x + 8, tablaY, panel.width - 16);
    float filaY = tablaY + 34;
    float limite = panel.y + panel.height - 28;
    int numero = 1;
    int ocultos = 0;
    NodoSimple<Transaccion>* actual = bloque->getTransacciones().getCabeza();
    while (actual != nullptr) {
        if (filaY <= limite) {
            const Transaccion& t = actual->getDato();
            bool resaltar = !loteRastreado.empty() && t.getIdLote() == loteRastreado;
            filaTabla(panel.x + 8, filaY, panel.width - 16, std::to_string(numero), t, resaltar);
            filaY += 26;
        } else {
            ocultos++;
        }
        numero++;
        actual = actual->getSiguiente();
    }
    if (ocultos > 0) {
        texto("... y " + std::to_string(ocultos) + " movimiento(s) mas", x, filaY, 14, TEXTO_SUAVE);
    }
}

void InterfazGrafica::dibujarDetalleAbierto(Rectangle panel) {
    Bloque* abierto = blockchain.getBloqueAbierto();
    float x = panel.x + 16;
    float y = panel.y + 12;
    tituloSeccion("BLOQUE #" + std::to_string(abierto->getIndice()) + "  (abierto, sin sellar)", x, y);
    texto("Estos movimientos todavia no estan protegidos. Cuando termine de registrar,", x, y + 30, 15, TEXTO_SUAVE);
    texto("pulse 'Sellar bloque' para agregarlos de forma definitiva a la cadena.", x, y + 50, 15, TEXTO_SUAVE);

    float tablaY = y + 84;
    encabezadoTabla(panel.x + 8, tablaY, panel.width - 16);
    float filaY = tablaY + 34;
    float limite = panel.y + panel.height - 28;

    NodoSimple<Transaccion>* actual = abierto->getTransacciones().getCabeza();
    if (actual == nullptr) {
        texto("(sin movimientos pendientes)", x, filaY, 16, TEXTO_SUAVE);
        return;
    }
    int numero = 1;
    int ocultos = 0;
    while (actual != nullptr) {
        if (filaY <= limite) {
            filaTabla(panel.x + 8, filaY, panel.width - 16, std::to_string(numero), actual->getDato(), false);
            filaY += 26;
        } else {
            ocultos++;
        }
        numero++;
        actual = actual->getSiguiente();
    }
    if (ocultos > 0) {
        texto("... y " + std::to_string(ocultos) + " movimiento(s) mas", x, filaY, 14, TEXTO_SUAVE);
    }
}

void InterfazGrafica::dibujarRastreo(Rectangle panel) {
    float x = panel.x + 16;
    float y = panel.y + 12;
    tituloSeccion("RECORRIDO DEL LOTE " + loteRastreado, x, y);
    texto("Se recorren los dos ejes: bloque por bloque (lista doble) y, dentro de cada",
          x, y + 30, 15, TEXTO_SUAVE);
    texto("bloque, su lista de movimientos (lista sencilla).", x, y + 50, 15, TEXTO_SUAVE);

    float tablaY = y + 84;
    float ancho = panel.width - 16;
    rectangulo(panel.x + 8, tablaY, ancho, 26.0f, CAFE_CLARO);
    texto("Bloque", panel.x + 20, tablaY + 6, 15, CAFE_OSCURO);
    texto("Fecha", panel.x + 150, tablaY + 6, 15, CAFE_OSCURO);
    texto("Etapa", panel.x + 270, tablaY + 6, 15, CAFE_OSCURO);
    texto("Responsable", panel.x + 420, tablaY + 6, 15, CAFE_OSCURO);
    textoDerecha("Peso", panel.x + 8 + ancho - 12, tablaY + 6, 15, CAFE_OSCURO);

    float filaY = tablaY + 34;
    float limite = panel.y + panel.height - 28;
    int encontrados = 0;
    int ocultos = 0;

    // Primer eje: bloques sellados; despues, el bloque abierto.
    Node<Bloque*>* nodo = blockchain.getCadena().getHead();
    Bloque* bloque = (nodo != nullptr) ? nodo->getInfo() : blockchain.getBloqueAbierto();
    while (bloque != nullptr) {
        bool esAbierto = (bloque == blockchain.getBloqueAbierto());

        // Segundo eje: movimientos del bloque.
        NodoSimple<Transaccion>* actual = bloque->getTransacciones().getCabeza();
        while (actual != nullptr) {
            const Transaccion& t = actual->getDato();
            if (t.getIdLote() == loteRastreado) {
                encontrados++;
                if (filaY <= limite) {
                    std::string etiqueta = "#" + std::to_string(bloque->getIndice());
                    if (esAbierto) etiqueta += " (abierto)";
                    texto(etiqueta, panel.x + 20, filaY, 16, esAbierto ? TEXTO_SUAVE : TEXTO);
                    texto(t.getFecha(), panel.x + 150, filaY, 16, TEXTO);
                    texto(ajustarTexto(t.getEtapa(), 140, 16), panel.x + 270, filaY, 16, TEXTO);
                    texto(ajustarTexto(t.getActor(), 210, 16), panel.x + 420, filaY, 16, TEXTO);
                    textoDerecha(formatoPeso(t.getPesoKg()), panel.x + 8 + ancho - 12, filaY, 16, TEXTO);
                    filaY += 26;
                } else {
                    ocultos++;
                }
            }
            actual = actual->getSiguiente();
        }

        // Avanzar al siguiente bloque de la cadena, o al abierto al final.
        if (esAbierto) {
            bloque = nullptr;
        } else {
            nodo = nodo->getNext();
            bloque = (nodo != nullptr) ? nodo->getInfo() : blockchain.getBloqueAbierto();
        }
    }

    if (encontrados == 0) {
        texto("No hay movimientos registrados para este lote.", x, filaY, 16, TEXTO_SUAVE);
    } else if (ocultos > 0) {
        texto("... y " + std::to_string(ocultos) + " movimiento(s) mas", x, filaY, 14, TEXTO_SUAVE);
    }
}

void InterfazGrafica::dibujarPanelDerecho() {
    Rectangle panel = {820, PANEL_Y, 440, PANEL_ALTO};
    DrawRectangleRec(panel, BLANCO);
    DrawRectangleLinesEx(panel, 1.0f, CAFE_CLARO);
    float x = panel.x + 14;

    // ----- Registrar un movimiento -----
    tituloSeccion("Registrar un movimiento", x, PANEL_Y + 10);
    campo(Rectangle{x, PANEL_Y + 56, 120, 30}, "Lote", campoLote);

    // Selector de etapa: [<] etapa [>]
    texto("Etapa", x + 134, PANEL_Y + 39, 14, TEXTO_SUAVE);
    if (boton(Rectangle{x + 134, PANEL_Y + 56, 30, 30}, "<")) {
        etapaSeleccionada = (etapaSeleccionada + NUM_ETAPAS - 1) % NUM_ETAPAS;
    }
    Rectangle cajaEtapa = {x + 168, PANEL_Y + 56, 210, 30};
    DrawRectangleRec(cajaEtapa, BLANCO);
    DrawRectangleLinesEx(cajaEtapa, 1.0f, GRIS);
    textoCentrado(ETAPAS[etapaSeleccionada], cajaEtapa, 16, TEXTO);
    if (boton(Rectangle{x + 382, PANEL_Y + 56, 30, 30}, ">")) {
        etapaSeleccionada = (etapaSeleccionada + 1) % NUM_ETAPAS;
    }

    campo(Rectangle{x, PANEL_Y + 110, 412, 30}, "Responsable (ej. Finca La Esperanza)", campoActor);
    campo(Rectangle{x, PANEL_Y + 164, 130, 30}, "Peso (kg)", campoPeso);
    campo(Rectangle{x + 140, PANEL_Y + 164, 130, 30}, "Fecha (AAAA-MM-DD)", campoFecha);
    if (boton(Rectangle{x + 280, PANEL_Y + 164, 132, 30}, "Registrar")) registrarMovimiento();

    DrawLineEx(Vector2{panel.x + 10, PANEL_Y + 208}, Vector2{panel.x + panel.width - 10, PANEL_Y + 208}, 1.0f, CAFE_CLARO);

    // ----- Rastrear un lote -----
    tituloSeccion("Rastrear un lote", x, PANEL_Y + 218);
    campo(Rectangle{x, PANEL_Y + 262, 150, 30}, "Codigo del lote", campoRastreo);
    if (boton(Rectangle{x + 160, PANEL_Y + 262, 120, 30}, "Rastrear")) rastrearLote();
    if (boton(Rectangle{x + 290, PANEL_Y + 262, 122, 30}, "Quitar", !loteRastreado.empty())) {
        loteRastreado.clear();
        mostrandoRastreo = false;
        informar("Rastreo quitado.", CAFE_OSCURO);
    }

    DrawLineEx(Vector2{panel.x + 10, PANEL_Y + 304}, Vector2{panel.x + panel.width - 10, PANEL_Y + 304}, 1.0f, CAFE_CLARO);

    // ----- Prueba de seguridad -----
    tituloSeccion("Prueba de seguridad", x, PANEL_Y + 312);
    texto("(bloque seleccionado)", x + 222, PANEL_Y + 316, 13, TEXTO_SUAVE);
    campo(Rectangle{x, PANEL_Y + 350, 120, 28}, "Movimiento #", campoMovimiento);
    campo(Rectangle{x + 130, PANEL_Y + 350, 130, 28}, "Peso falso (kg)", campoPesoFalso);
    if (boton(Rectangle{x + 270, PANEL_Y + 350, 142, 28}, "Alterar dato")) alterarDato();
}

void InterfazGrafica::dibujarMensaje() {
    rectangulo(0, ALTO - 44.0f, ANCHO, 44, BLANCO);
    rectangulo(0, ALTO - 44.0f, 6, 44, colorMensaje);
    texto(ajustarTexto(mensaje, ANCHO - 40.0f, 17), 20, ALTO - 31.0f, 17, colorMensaje);
}