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
        if (std::strcmp(argv[i], "-n") == 0) {
            if (i + 1 >= argc) { std::fprintf(stderr, "Falta el valor de -n\n"); return false; }
            config.n = std::atoi(argv[++i]);
        } else if (std::strcmp(argv[i], "-w") == 0) {
            if (i + 1 >= argc) { std::fprintf(stderr, "Falta el valor de -w\n"); return false; }
            config.width = std::atoi(argv[++i]);
        } else if (std::strcmp(argv[i], "-h") == 0) {
            if (i + 1 >= argc) { std::fprintf(stderr, "Falta el valor de -h\n"); return false; }
            config.height = std::atoi(argv[++i]);
        } else if (std::strcmp(argv[i], "-frames") == 0) {
            if (i + 1 >= argc) { std::fprintf(stderr, "Falta el valor de -frames\n"); return false; }
            config.frames = std::atoi(argv[++i]);
        } else if (std::strcmp(argv[i], "-seed") == 0) {
            if (i + 1 >= argc) { std::fprintf(stderr, "Falta el valor de -seed\n"); return false; }
            config.seed = static_cast<unsigned int>(std::strtoul(argv[++i], nullptr, 10));
        } else {
            std::fprintf(stderr, "Argumento desconocido: %s\n", argv[i]);
            return false;
        }
    }

    // Programacion defensiva: valores fuera de rango se rechazan aqui en vez
    // de dejar que el programa arranque con datos sin sentido.
    if (config.n <= 0) {
        std::fprintf(stderr, "-n debe ser mayor a 0 (recibido: %d)\n", config.n);
        return false;
    }
    if (config.width < 640) {
        std::fprintf(stderr, "-w debe ser al menos 640 (recibido: %d)\n", config.width);
        return false;
    }
    if (config.height < 480) {
        std::fprintf(stderr, "-h debe ser al menos 480 (recibido: %d)\n", config.height);
        return false;
    }
    if (config.frames < 0) {
        std::fprintf(stderr, "-frames no puede ser negativo (recibido: %d)\n", config.frames);
        return false;
    }

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
