// Screensaver, version PARALELA con OpenMP.
//
// PENDIENTE - Fase 2, paso 2 (ver PLAN.md): todavia no hay ninguna directiva
// de OpenMP aqui. Este archivo es el resultado del paso 1: una copia de
// src/secuencial/main.cpp que compila y corre igual, para confirmar que el
// punto de partida es correcto antes de repartir nada entre hilos.
//
// El paso 2 reparte stepSimulation() (en physics.cpp, tambien copiado tal
// cual por ahora) entre hilos siguiendo PCAM: integrate() es el reparto
// directo (un elemento por iteracion, sin dependencias entre ellas),
// resolveCollisions() necesita proteccion porque cada par escribe en dos
// elementos, y buildLinks() necesita que los hilos no se pisen al escribir en
// el mismo vector de salida.

#include <cstdio>
#include <string>

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

    Simulation sim = createSimulation(config);

    Renderer renderer;
    if (!renderer.init(config)) return 1;

    Metrics metrics;
    // Todavia no hay ninguna directiva de OpenMP (eso es el paso 2 de esta
    // fase), asi que en la practica esto sigue corriendo en un solo hilo sin
    // importar lo que pida -t. Cuando se agregue el reparto real, esto pasa a
    // ser omp_get_max_threads().
    metrics.threads = 1;

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
