// Screensaver, version PARALELA con OpenMP.
//
// Las tres etapas de stepSimulation() se reparten entre hilos (ver
// src/paralelo/physics.cpp): integrate() con un parallel for simple,
// resolveCollisions() con un lock por elemento, y buildLinks() con un
// buffer por hilo.

#include <cstdio>
#include <string>

#include <omp.h>

#include "metrics.hpp"
#include "render.hpp"
#include "screensaver.hpp"
#include "simulation.hpp"

namespace {

// Nombre con el que esta version aparece en el resumen y en el CSV.
constexpr const char* kVersionName = "paralelo";

// Cada cuanto se refresca el contador de FPS del titulo y del HUD. Actualizarlo
// en cada cuadro lo dejaria ilegible de tanto parpadeo.
constexpr double kFpsRefreshSeconds = 0.5;

} // namespace

int main(int argc, char** argv) {
    Config config;
    if (!parseArgs(argc, argv, config)) {
        printUsage(argv[0]);
        return 1;
    }

    // -t en 0 (default) deja que OpenMP decida cuantos hilos usar; cualquier
    // otro valor lo fija explicitamente antes de entrar al ciclo principal.
    if (config.threads > 0) {
        omp_set_num_threads(config.threads);
    }

    Simulation sim = createSimulation(config);

    Renderer renderer;
    if (!renderer.init(config)) return 1;

    Metrics metrics;
    // Hilos que OpenMP va a usar de verdad, no lo que se pidio con -t
    // (que puede ser 0 = "decida OpenMP").
    metrics.threads = omp_get_max_threads();

    Stopwatch runTimer;    // Duracion total del ciclo principal.
    Stopwatch stageTimer;  // Se reinicia en cada etapa para medirla aparte.
    Stopwatch fpsTimer;    // Ventana sobre la que se promedian los FPS mostrados.
    long long framesAtLastFpsUpdate = 0;

    while (!pollQuitRequest()) {
        // Con -frames el programa se detiene solo, que es lo que necesita el
        // script de mediciones para poder cronometrar corridas comparables.
        if (config.frames > 0 && metrics.frames >= config.frames) break;

        stageTimer.reset();
        stepSimulation(sim, config);
        metrics.physicsSeconds += stageTimer.lap();

        renderer.drawFrame(sim, config, metrics);
        metrics.renderSeconds += stageTimer.lap();

        ++metrics.frames;

        if (fpsTimer.elapsed() >= kFpsRefreshSeconds) {
            const double window = fpsTimer.lap();
            metrics.fps = static_cast<double>(metrics.frames - framesAtLastFpsUpdate) / window;
            framesAtLastFpsUpdate = metrics.frames;

            renderer.setTitle("Screensaver paralelo - N=" + std::to_string(config.n) +
                              " - FPS=" + std::to_string(static_cast<int>(metrics.fps)));
        }
    }

    metrics.totalSeconds = runTimer.elapsed();

    // La captura se toma antes de imprimir el resumen, mientras la ventana
    // todavia existe.
    if (config.screenshotPath != nullptr && renderer.saveScreenshot(config.screenshotPath)) {
        std::printf("Captura guardada en %s\n", config.screenshotPath);
    }

    printMetrics(config, metrics, kVersionName);

    // La ventana y el renderer los libera el destructor de Renderer al salir
    // de main, incluso si se salio por un error.
    return 0;
}
