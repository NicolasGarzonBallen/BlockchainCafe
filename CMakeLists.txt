cmake_minimum_required(VERSION 3.16)
project(BlockchainCafe C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# Nucleo del proyecto (lo comparten la consola y la interfaz grafica).
# DoubleLinkedList.cpp no va aqui: Blockchain.cpp lo incluye e instancia la
# plantilla con Bloque*.
# ---------------------------------------------------------------------------
add_library(nucleo STATIC
        Transaccion.cpp
        Bloque.cpp
        Blockchain.cpp)
target_include_directories(nucleo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Programa de consola (el menu obligatorio).
add_executable(blockchain_consola main.cpp)
target_link_libraries(blockchain_consola PRIVATE nucleo)

# ---------------------------------------------------------------------------
# Bono: interfaz grafica con Raylib.
# Si Raylib ya esta instalado se usa; si no, CMake lo descarga de GitHub la
# primera vez (requiere internet). Para compilar solo la consola:
#   -DCON_INTERFAZ_GRAFICA=OFF
# ---------------------------------------------------------------------------
option(CON_INTERFAZ_GRAFICA "Compilar la interfaz grafica con Raylib" ON)

if (CON_INTERFAZ_GRAFICA)
    find_package(raylib QUIET)
    if (NOT raylib_FOUND)
        include(FetchContent)
        set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
        FetchContent_Declare(
                raylib
                GIT_REPOSITORY https://github.com/raysan5/raylib.git
                GIT_TAG 5.5
                GIT_SHALLOW TRUE)
        FetchContent_MakeAvailable(raylib)
    endif ()

    add_executable(blockchain_gui main_gui.cpp InterfazGrafica.cpp)
    target_link_libraries(blockchain_gui PRIVATE nucleo raylib)
endif ()