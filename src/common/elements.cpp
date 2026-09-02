// Creacion del estado inicial de la animacion.
//
// Vive en src/common (y no duplicado en cada version) porque la secuencial y
// la paralela tienen que arrancar EXACTAMENTE del mismo estado para que la
// comparacion de tiempos sea justa. Con la misma semilla, las dos recorren la
// misma trayectoria y deben terminar en la misma posicion.

#include <cmath>
#include <random>

#include "simulation.hpp"

namespace {

// Cuantas lineas de conexion se esperan por elemento. Es solo una reserva
// inicial para que el primer cuadro no tenga que pedir memoria varias veces;
// si se queda corta el vector crece solo.
constexpr int kExpectedLinksPerElement = 4;

} // namespace

Simulation createSimulation(const Config& config) {
    Simulation sim;
    sim.elements.reserve(static_cast<std::size_t>(config.n));

    // Semilla 0 significa "distinta cada vez". Cualquier otro valor hace la
    // corrida reproducible, que es lo que se necesita para medir.
    std::mt19937 rng(config.seed != 0 ? config.seed : std::random_device{}());

    std::uniform_real_distribution<float> radiusDist(config.minRadius, config.maxRadius);
    std::uniform_real_distribution<float> speedDist(config.minSpeed, config.maxSpeed);
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * kPi);
    std::uniform_real_distribution<float> spinDist(-config.maxAngularSpeed, config.maxAngularSpeed);
    std::uniform_int_distribution<int> colorDist(0, config.paletteCount - 1);

    for (int i = 0; i < config.n; ++i) {
        Element element;

        element.radius = radiusDist(rng);

        // La posicion se sortea ya descontando el radio, para que ningun
        // elemento nazca a medias fuera del canvas.
        std::uniform_real_distribution<float> xDist(element.radius,
                                                    static_cast<float>(config.width) - element.radius);
        std::uniform_real_distribution<float> yDist(element.radius,
                                                    static_cast<float>(config.height) - element.radius);
        element.x = xDist(rng);
        element.y = yDist(rng);

        // La velocidad se arma como direccion + rapidez en vez de sortear vx y
        // vy por separado: asi todos los elementos arrancan con una rapidez
        // dentro del rango pedido, sin importar hacia donde vayan.
        const float direction = angleDist(rng);
        const float speed = speedDist(rng);
        element.vx = speed * std::cos(direction);
        element.vy = speed * std::sin(direction);

        element.angle = angleDist(rng);
        element.angularSpeed = spinDist(rng);

        // Desfase propio para que el pulso de cada elemento no vaya sincronizado
        // con el de los demas.
        element.phase = angleDist(rng);

        element.color = config.palette[colorDist(rng)];

        sim.elements.push_back(element);
    }

    sim.links.reserve(static_cast<std::size_t>(config.n) * kExpectedLinksPerElement);
    return sim;
}
