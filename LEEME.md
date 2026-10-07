# Blockchain de trazabilidad del cafe (Programacion III - UPTC)

## Ejecutables

| Programa | Que es |
|---|---|
| `blockchain_consola` | Menu de consola (obligatorio) con las 7 opciones del enunciado. |
| `blockchain_gui` | Bono: interfaz grafica con Raylib. |

Ambos comparten el historial `cadena_cafe.txt` (bono de persistencia), que se
crea en la carpeta desde donde se ejecuta el programa.

## Compilar

Con CLion: abrir la carpeta del proyecto (usa `CMakeLists.txt`) y elegir el
objetivo `blockchain_consola` o `blockchain_gui`.

Por terminal:

    cmake -S . -B build
    cmake --build build
    ./build/blockchain_consola
    ./build/blockchain_gui

Si Raylib no esta instalado, CMake lo descarga de GitHub la primera vez
(requiere internet). En Linux/WSL antes hay que instalar sus dependencias:

    sudo apt install build-essential git libasound2-dev libx11-dev libxrandr-dev \
        libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev \
        libwayland-dev libxkbcommon-dev

Para compilar solo la consola: `cmake -S . -B build -DCON_INTERFAZ_GRAFICA=OFF`

## Persistencia

- Consola: al iniciar pregunta si se desea continuar el historial guardado; al
  salir (opcion 7) lo guarda automaticamente.
- Interfaz grafica: carga el historial al abrir y lo guarda al cerrar (o con el
  boton "Guardar historial").
- Al cargar se restaura cada bloque con su hash y nonce originales, sin volver
  a minar, y se verifica la cadena: si alguien edita el archivo a mano, el
  sistema lo detecta.

## Prueba de memoria con Valgrind

    valgrind --leak-check=full ./build/blockchain_consola < datos_prueba.txt

`datos_prueba.txt` funciona exista o no `cadena_cafe.txt`.
La prueba de memoria se hace sobre la consola: con la interfaz grafica,
Valgrind tambien reporta memoria de los controladores de video (OpenGL/X11)
que no pertenece al proyecto.
