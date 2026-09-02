#ifndef RENDER_HPP
#define RENDER_HPP

#include <SDL.h>

#include <string>
#include <vector>

#include "metrics.hpp"
#include "simulation.hpp"

// ---------------------------------------------------------------------------
// Capa de dibujo. Es identica para las dos versiones: el render se deja
// secuencial a proposito, porque SDL_Renderer no es seguro de usar desde
// varios hilos. Lo que se puede repartir entre hilos es armar los vertices,
// y eso vive en la simulacion, no aqui.
// ---------------------------------------------------------------------------

// Consume los eventos pendientes de SDL y devuelve true si el usuario pidio
// cerrar (boton de la ventana, Escape o Q).
bool pollQuitRequest();

// Ventana + renderer + buffers de geometria. La construccion que puede fallar
// se hace en init() y no en el constructor, para poder explicar el error y
// salir ordenadamente. El destructor libera todo aunque se salga por error.
class Renderer {
public:
    Renderer() = default;
    ~Renderer();

    // No tiene sentido copiar un dueno de recursos de SDL.
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    // Inicializa SDL y abre la ventana. Devuelve false y explica el motivo por
    // stderr si algo falla.
    bool init(const Config& config);

    // Dibuja un cuadro completo: estela, lineas de conexion, elementos y
    // sobreimpresos (reloj y HUD).
    void drawFrame(const Simulation& sim, const Config& config, const Metrics& metrics);

    void setTitle(const std::string& title);

    // Guarda el contenido actual de la ventana en un archivo BMP. Sirve para
    // dejar capturas reproducibles del screensaver sin depender de una
    // herramienta externa. Devuelve false y explica el motivo si falla.
    bool saveScreenshot(const char* path) const;

private:
    void drawTrail(const Config& config);
    void drawLinks(const Simulation& sim);
    void drawElements(const Simulation& sim, const Config& config);
    void drawClock(const Config& config);
    void drawHud(const Config& config, const Metrics& metrics);

    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;

    // Buffers reutilizados entre cuadros: se vacian con clear() para conservar
    // la capacidad y no pedir memoria 60 veces por segundo.
    std::vector<SDL_Vertex> vertices_;
    std::vector<int> indices_;
};

#endif // RENDER_HPP
