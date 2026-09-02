// Lectura y validacion de los argumentos de linea de comandos.
//
// Este archivo lo enlazan las DOS versiones (secuencial y paralela), asi que
// por construccion ambas aceptan exactamente las mismas banderas con la misma
// validacion. Si el parseo estuviera duplicado, cualquier cambio en una sola
// version dejaria las mediciones de speedup comparando cosas distintas.

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "screensaver.hpp"

namespace {

// Limite arbitrario pero deliberado: la deteccion de colisiones es O(N^2), asi
// que un N absurdo no "falla", solo se congela. Es mejor rechazarlo con un
// mensaje claro que dejar al usuario esperando sin saber por que.
constexpr int kMaxElements = 100000;

// Lee el valor entero que sigue a una bandera y deja 'index' apuntando al
// valor consumido. Devuelve false si falta el valor o si el texto no es un
// entero completo: "12x" se rechaza, no se lee como 12.
bool readInt(int argc, char** argv, int& index, long long& out) {
    const char* flag = argv[index];
    if (index + 1 >= argc) {
        std::fprintf(stderr, "Error: la bandera %s necesita un valor.\n", flag);
        return false;
    }

    const char* text = argv[++index];
    char* end = nullptr;
    errno = 0;
    const long long value = std::strtoll(text, &end, 10);

    if (end == text || *end != '\0' || errno == ERANGE) {
        std::fprintf(stderr, "Error: %s espera un numero entero, se recibio '%s'.\n", flag, text);
        return false;
    }

    out = value;
    return true;
}

// Igual que readInt pero para valores con decimales.
bool readFloat(int argc, char** argv, int& index, float& out) {
    const char* flag = argv[index];
    if (index + 1 >= argc) {
        std::fprintf(stderr, "Error: la bandera %s necesita un valor.\n", flag);
        return false;
    }

    const char* text = argv[++index];
    char* end = nullptr;
    errno = 0;
    const float value = std::strtof(text, &end);

    if (end == text || *end != '\0' || errno == ERANGE) {
        std::fprintf(stderr, "Error: %s espera un numero, se recibio '%s'.\n", flag, text);
        return false;
    }

    out = value;
    return true;
}

// Comprueba que un entero quepa en el rango permitido antes de guardarlo en
// Config, que trabaja con int.
bool checkRange(const char* flag, long long value, long long low, long long high, int& out) {
    if (value < low || value > high) {
        std::fprintf(stderr, "Error: %s debe estar entre %lld y %lld, se recibio %lld.\n",
                     flag, low, high, value);
        return false;
    }
    out = static_cast<int>(value);
    return true;
}

// Valida lo que no se puede revisar bandera por bandera, porque depende de dos
// o mas valores a la vez.
bool validateConfig(const Config& config, bool sawElementCount) {
    if (!sawElementCount) {
        std::fprintf(stderr, "Error: falta la bandera obligatoria -n (cantidad de elementos).\n");
        return false;
    }
    if (config.minRadius <= 0.0f) {
        std::fprintf(stderr, "Error: -rmin debe ser mayor a 0 (se recibio %.2f).\n", config.minRadius);
        return false;
    }
    if (config.maxRadius < config.minRadius) {
        std::fprintf(stderr, "Error: -rmax (%.2f) no puede ser menor que -rmin (%.2f).\n",
                     config.maxRadius, config.minRadius);
        return false;
    }
    if (config.minSpeed < 0.0f) {
        std::fprintf(stderr, "Error: -vmin no puede ser negativo (se recibio %.2f).\n", config.minSpeed);
        return false;
    }
    if (config.maxSpeed < config.minSpeed) {
        std::fprintf(stderr, "Error: -vmax (%.2f) no puede ser menor que -vmin (%.2f).\n",
                     config.maxSpeed, config.minSpeed);
        return false;
    }
    if (config.trailFade <= 0.0f || config.trailFade > 1.0f) {
        std::fprintf(stderr, "Error: -trail debe estar entre 0 (exclusivo) y 1 (se recibio %.2f).\n",
                     config.trailFade);
        return false;
    }
    if (config.glow < 0.0f || config.glow > 1.0f) {
        std::fprintf(stderr, "Error: -glow debe estar entre 0 y 1 (se recibio %.2f).\n", config.glow);
        return false;
    }
    if (config.linkDistance < 0.0f) {
        std::fprintf(stderr, "Error: -link no puede ser negativo (se recibio %.2f).\n",
                     config.linkDistance);
        return false;
    }

    // Un elemento mas ancho que el lado corto de la ventana no cabria entre
    // los dos bordes: quedaria rebotando pegado o atravesando la pared.
    const int shortSide = config.width < config.height ? config.width : config.height;
    if (config.maxRadius * 2.0f >= static_cast<float>(shortSide)) {
        std::fprintf(stderr, "Error: -rmax (%.2f) es demasiado grande para un canvas de %dx%d.\n",
                     config.maxRadius, config.width, config.height);
        return false;
    }

    return true;
}

} // namespace

void printUsage(const char* programName) {
    std::fprintf(stderr,
        "\n"
        "Uso: %s -n <cantidad> [opciones]\n"
        "\n"
        "Obligatorio:\n"
        "  -n <entero>      Cantidad de elementos a dibujar (1 a %d).\n"
        "\n"
        "Ventana:\n"
        "  -w <entero>      Ancho del canvas en pixeles (minimo 640, default 640).\n"
        "  -h <entero>      Alto del canvas en pixeles (minimo 480, default 480).\n"
        "\n"
        "Corrida:\n"
        "  -frames <entero> Cuadros a dibujar antes de salir solo. 0 = seguir hasta\n"
        "                   cerrar la ventana (default). Al usar -frames se apaga el\n"
        "                   vsync, porque en ese modo se esta midiendo, no viendo.\n"
        "  -seed <entero>   Semilla pseudoaleatoria. 0 = distinta cada vez (default).\n"
        "  -t <entero>      Hilos de OpenMP. 0 = los que decida OpenMP (default). La\n"
        "                   version secuencial acepta la bandera pero la ignora.\n"
        "\n"
        "Apariencia:\n"
        "  -rmin <numero>   Radio minimo de un elemento (default 4).\n"
        "  -rmax <numero>   Radio maximo de un elemento (default 14).\n"
        "  -vmin <numero>   Rapidez inicial minima, en px/s (default 40).\n"
        "  -vmax <numero>   Rapidez inicial maxima, en px/s (default 180).\n"
        "  -sides <entero>  Lados del poligono de cada elemento, 3 a 12 (default 3).\n"
        "  -glow <numero>   Intensidad del resplandor, entre 0 y 1. Con N grande los\n"
        "                   colores se suman y saturan a blanco: bajarlo lo compensa\n"
        "                   (default 1).\n"
        "  -trail <numero>  Opacidad de la estela, entre 0 y 1. Mas alto = cola mas\n"
        "                   corta (default 0.15).\n"
        "  -link <numero>   Distancia maxima para unir dos elementos con una linea,\n"
        "                   en pixeles. 0 desactiva las lineas (default 120).\n"
        "  -noclock         No dibujar el reloj.\n"
        "  -nohud           No dibujar los contadores de N, FPS e hilos.\n"
        "\n"
        "Capturas:\n"
        "  -shot <archivo>  Guardar el ultimo cuadro como imagen BMP al salir. Sirve\n"
        "                   para dejar capturas reproducibles para el informe.\n"
        "\n"
        "  --help           Mostrar esta ayuda.\n"
        "\n"
        "Ejemplos:\n"
        "  %s -n 400 -w 1280 -h 720\n"
        "  %s -n 2000 -frames 600 -seed 42 -nohud\n"
        "\n",
        programName, kMaxElements, programName, programName);
}

bool parseArgs(int argc, char** argv, Config& config) {
    // -n no tiene un default razonable: se exige explicitamente, y para eso hay
    // que recordar si la bandera aparecio o no.
    bool sawElementCount = false;
    bool sawFrames = false;

    for (int i = 1; i < argc; ++i) {
        const char* flag = argv[i];
        long long whole = 0;

        if (std::strcmp(flag, "--help") == 0 || std::strcmp(flag, "-help") == 0) {
            printUsage(argv[0]);
            std::exit(0);
        } else if (std::strcmp(flag, "-n") == 0) {
            if (!readInt(argc, argv, i, whole)) return false;
            if (!checkRange(flag, whole, 1, kMaxElements, config.n)) return false;
            sawElementCount = true;
        } else if (std::strcmp(flag, "-w") == 0) {
            if (!readInt(argc, argv, i, whole)) return false;
            if (!checkRange(flag, whole, 640, 16384, config.width)) return false;
        } else if (std::strcmp(flag, "-h") == 0) {
            if (!readInt(argc, argv, i, whole)) return false;
            if (!checkRange(flag, whole, 480, 16384, config.height)) return false;
        } else if (std::strcmp(flag, "-frames") == 0) {
            if (!readInt(argc, argv, i, whole)) return false;
            if (!checkRange(flag, whole, 0, 100000000, config.frames)) return false;
            sawFrames = true;
        } else if (std::strcmp(flag, "-seed") == 0) {
            if (!readInt(argc, argv, i, whole)) return false;
            if (whole < 0 || whole > 4294967295LL) {
                std::fprintf(stderr, "Error: -seed debe estar entre 0 y 4294967295.\n");
                return false;
            }
            config.seed = static_cast<unsigned int>(whole);
        } else if (std::strcmp(flag, "-t") == 0) {
            if (!readInt(argc, argv, i, whole)) return false;
            if (!checkRange(flag, whole, 0, 1024, config.threads)) return false;
        } else if (std::strcmp(flag, "-sides") == 0) {
            if (!readInt(argc, argv, i, whole)) return false;
            if (!checkRange(flag, whole, 3, 12, config.coreSides)) return false;
        } else if (std::strcmp(flag, "-rmin") == 0) {
            if (!readFloat(argc, argv, i, config.minRadius)) return false;
        } else if (std::strcmp(flag, "-rmax") == 0) {
            if (!readFloat(argc, argv, i, config.maxRadius)) return false;
        } else if (std::strcmp(flag, "-vmin") == 0) {
            if (!readFloat(argc, argv, i, config.minSpeed)) return false;
        } else if (std::strcmp(flag, "-vmax") == 0) {
            if (!readFloat(argc, argv, i, config.maxSpeed)) return false;
        } else if (std::strcmp(flag, "-trail") == 0) {
            if (!readFloat(argc, argv, i, config.trailFade)) return false;
        } else if (std::strcmp(flag, "-glow") == 0) {
            if (!readFloat(argc, argv, i, config.glow)) return false;
        } else if (std::strcmp(flag, "-link") == 0) {
            if (!readFloat(argc, argv, i, config.linkDistance)) return false;
        } else if (std::strcmp(flag, "-shot") == 0) {
            if (i + 1 >= argc) {
                std::fprintf(stderr, "Error: la bandera -shot necesita un nombre de archivo.\n");
                return false;
            }
            config.screenshotPath = argv[++i];
        } else if (std::strcmp(flag, "-noclock") == 0) {
            config.showClock = false;
        } else if (std::strcmp(flag, "-nohud") == 0) {
            config.showHud = false;
        } else {
            std::fprintf(stderr, "Error: bandera desconocida '%s'.\n", flag);
            return false;
        }
    }

    if (!validateConfig(config, sawElementCount)) return false;

    // En modo medicion no se espera al monitor: el vsync taparia cualquier
    // diferencia de rendimiento detras de los 16.6 ms del refresco.
    if (sawFrames && config.frames > 0) config.vsync = false;

    return true;
}
