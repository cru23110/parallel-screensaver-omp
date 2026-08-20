// Version secuencial del screensaver.
// Punto de partida: debe quedar funcional y correcta ANTES de tocar la
// version paralela (src/paralelo). Este archivo es el baseline contra el
// que se calcula el speedup.

#include <cstdio>
#include "screensaver.hpp"

bool parseArgs(int argc, char** argv, Config& config) {
    // TODO: leer -n, -w, -h, -frames, -seed desde argv.
    // TODO: validar rangos (n > 0, width/height >= minimo, etc.) y rechazar
    // entradas invalidas con un mensaje claro (programacion defensiva).
    (void)argc;
    (void)argv;
    (void)config;
    return true;
}

int main(int argc, char** argv) {
    Config config;
    if (!parseArgs(argc, argv, config)) {
        std::fprintf(stderr, "Argumentos invalidos. Uso: %s -n <N> [-w <ancho>] [-h <alto>]\n", argv[0]);
        return 1;
    }

    // TODO: inicializar SDL/OpenGL, crear N elementos, correr el loop de
    // render + fisica, medir tiempo total de ejecucion y destruir recursos
    // correctamente al salir.

    return 0;
}
