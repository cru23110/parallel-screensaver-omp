// Resumen de tiempos que se imprime al terminar la corrida.
//
// El formato es identico en las dos versiones a proposito: el script de la
// Fase 3 lee la misma salida sin importar cual binario corrio, y la linea
// "simulacion" es la que interesa para el speedup, porque es la unica parte
// que la version paralela reparte entre hilos.

#include <cstdio>

#include "metrics.hpp"

void printMetrics(const Config& config, const Metrics& metrics, const char* versionName) {
    // Con 0 cuadros no hay nada que promediar y dividir daria infinito.
    if (metrics.frames <= 0) {
        std::printf("No se dibujo ningun cuadro, no hay tiempos que reportar.\n");
        return;
    }

    const double frames = static_cast<double>(metrics.frames);
    const double otherSeconds = metrics.totalSeconds - metrics.physicsSeconds - metrics.renderSeconds;

    // Porcentajes sobre el total, para ver de un vistazo donde se va el tiempo.
    const double physicsShare = 100.0 * metrics.physicsSeconds / metrics.totalSeconds;
    const double renderShare = 100.0 * metrics.renderSeconds / metrics.totalSeconds;

    std::printf("\n");
    std::printf("=== Resumen de la corrida (%s) ===\n", versionName);
    std::printf("  elementos          : %d\n", config.n);
    std::printf("  canvas             : %dx%d\n", config.width, config.height);
    std::printf("  semilla            : %u%s\n", config.seed,
                config.seed == 0 ? " (no determinista)" : "");
    std::printf("  hilos              : %d\n", metrics.threads);
    std::printf("  cuadros            : %lld\n", metrics.frames);
    std::printf("  tiempo total       : %.4f s\n", metrics.totalSeconds);
    std::printf("  fps promedio       : %.2f\n", frames / metrics.totalSeconds);
    std::printf("  simulacion (total) : %.4f s  (%.1f%% del total)\n",
                metrics.physicsSeconds, physicsShare);
    std::printf("  simulacion (cuadro): %.4f ms\n", 1000.0 * metrics.physicsSeconds / frames);
    std::printf("  dibujo (total)     : %.4f s  (%.1f%% del total)\n",
                metrics.renderSeconds, renderShare);
    std::printf("  dibujo (cuadro)    : %.4f ms\n", 1000.0 * metrics.renderSeconds / frames);
    std::printf("  otros (eventos)    : %.4f s\n", otherSeconds);
    std::printf("\n");

    // Linea en formato CSV para que el script de mediciones la pueda tomar tal
    // cual, sin tener que interpretar el resumen de arriba.
    std::printf("CSV,%s,%d,%d,%d,%d,%lld,%.6f,%.6f,%.6f\n",
                versionName, config.n, config.width, config.height, metrics.threads,
                metrics.frames, metrics.totalSeconds, metrics.physicsSeconds,
                metrics.renderSeconds);
}
