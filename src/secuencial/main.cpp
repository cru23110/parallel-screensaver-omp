// Version secuencial del screensaver.
// Punto de partida: debe quedar funcional y correcta ANTES de tocar la
// version paralela (src/paralelo). Este archivo es el baseline contra el
// que se calcula el speedup.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "screensaver.hpp"

bool parseArgs(int argc, char** argv, Config& config) {
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            config.n = std::atoi(argv[++i]);
        } else if (std::strcmp(argv[i], "-w") == 0 && i + 1 < argc) {
            config.width = std::atoi(argv[++i]);
        } else if (std::strcmp(argv[i], "-h") == 0 && i + 1 < argc) {
            config.height = std::atoi(argv[++i]);
        } else if (std::strcmp(argv[i], "-frames") == 0 && i + 1 < argc) {
            config.frames = std::atoi(argv[++i]);
        } else if (std::strcmp(argv[i], "-seed") == 0 && i + 1 < argc) {
            config.seed = static_cast<unsigned int>(std::strtoul(argv[++i], nullptr, 10));
        }
    }

    // TODO (siguiente tarea): validar rangos (n > 0, width/height >= minimo,
    // etc.) y rechazar entradas invalidas o flags desconocidos con un mensaje
    // claro (programacion defensiva), en vez de ignorarlos en silencio.
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
