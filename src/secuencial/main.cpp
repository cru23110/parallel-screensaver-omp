// Screensaver, version SECUENCIAL.
//
// Es el baseline del proyecto: aqui no se usa OpenMP. Todo el trabajo de un
// cuadro (fisica de los N elementos y dibujo) lo hace un solo hilo, y los
// tiempos que imprime al salir son el denominador del speedup que se calcula
// en la Fase 3.
//
// El ciclo principal es a proposito muy corto: leer eventos, avanzar la
// simulacion, dibujar y cronometrar cada parte por separado. Todo lo demas
// esta en los modulos de include/ y src/common/.

#include <cstdio>
#include <string>

#include "metrics.hpp"
#include "render.hpp"
#include "screensaver.hpp"
#include "simulation.hpp"

namespace {

// Nombre con el que esta version aparece en el resumen y en el CSV.
constexpr const char* kVersionName = "secuencial";

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

    // La version secuencial acepta -t para tener exactamente las mismas
    // banderas que la paralela, pero corre siempre en un hilo. Se avisa para
    // que nadie interprete mal una medicion.
    if (config.threads > 1) {
        std::fprintf(stderr, "Aviso: -t no aplica en la version secuencial, se ignora.\n");
    }

    Simulation sim = createSimulation(config);

    Renderer renderer;
    if (!renderer.init(config)) return 1;

    Metrics metrics;
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

            renderer.setTitle("Screensaver secuencial - N=" + std::to_string(config.n) +
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
