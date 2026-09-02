#ifndef METRICS_HPP
#define METRICS_HPP

#include <chrono>

#include "screensaver.hpp"

// ---------------------------------------------------------------------------
// Medicion de tiempos. Se separa el costo de la simulacion del costo del
// dibujo porque lo unico que la Fase 2 paraleliza es la simulacion: mezclar
// los dos numeros escondería el speedup real detras del tiempo de la GPU.
// ---------------------------------------------------------------------------

// Cronometro monotonico (no se ve afectado si cambia la hora del sistema).
class Stopwatch {
public:
    Stopwatch() : start_(Clock::now()) {}

    // Segundos transcurridos desde la construccion o el ultimo reset().
    double elapsed() const {
        return std::chrono::duration<double>(Clock::now() - start_).count();
    }

    void reset() { start_ = Clock::now(); }

    // Devuelve los segundos transcurridos y reinicia la cuenta.
    double lap() {
        const double seconds = elapsed();
        reset();
        return seconds;
    }

private:
    using Clock = std::chrono::steady_clock;
    Clock::time_point start_;
};

// Totales de una corrida. Los tiempos estan en segundos.
struct Metrics {
    long long frames = 0;       // Cuadros dibujados.
    double totalSeconds = 0.0;  // Duracion completa del loop principal.
    double physicsSeconds = 0.0;// Suma del tiempo dentro de stepSimulation().
    double renderSeconds = 0.0; // Suma del tiempo dentro de drawFrame().
    double fps = 0.0;           // Promedio suavizado, solo para mostrar en pantalla.
    int threads = 1;            // Hilos usados (1 en la version secuencial).
};

// Imprime el resumen final por stdout. El formato es identico en las dos
// versiones para que el script de la Fase 3 pueda leer ambos igual.
void printMetrics(const Config& config, const Metrics& metrics, const char* versionName);

#endif // METRICS_HPP
