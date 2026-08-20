// Version paralela del screensaver con OpenMP.
// Partir de una copia funcional de src/secuencial y paralelizar de forma
// incremental (PCAM: Particionar, Comunicar, Aglomerar, Mapear), midiendo
// speedup en cada iteracion antes de seguir optimizando.

#include <cstdio>
#include "screensaver.hpp"

#ifdef _OPENMP
#include <omp.h>
#endif

bool parseArgs(int argc, char** argv, Config& config) {
    // TODO: mismo parsing/validacion que la version secuencial.
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

    // TODO: paralelizar la actualizacion de fisica de los N elementos con
    // OpenMP (#pragma omp parallel for), protegiendo cualquier recurso
    // compartido (colisiones, acumuladores) con secciones criticas o
    // reducciones segun corresponda.

    return 0;
}
