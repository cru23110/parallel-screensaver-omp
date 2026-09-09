// Fisica del screensaver, version PARALELA.
//
// Mismo algoritmo que src/secuencial/physics.cpp, repartido entre hilos con
// OpenMP en sus tres etapas:
//   1. integrate()          - mover y rotar cada elemento, rebotar en bordes. O(N)
//   2. resolveCollisions()  - choques elasticos entre elementos.              O(N^2)
//   3. buildLinks()         - lineas de conexion de la constelacion.          O(N^2)
//
// El orden importa: primero se mueve, luego se corrigen los solapamientos que
// ese movimiento produjo, y al final se miden distancias sobre las posiciones
// ya corregidas, para que las lineas coincidan con lo que se ve dibujado.

#include <cmath>

#include "simulation.hpp"

namespace {

// Por debajo de esta distancia entre centros se considera que dos elementos
// estan en el mismo punto: no hay una normal de choque bien definida, asi que
// se dejan pasar en vez de dividir entre casi cero.
constexpr float kMinSeparation = 1e-4f;

// Mueve y gira cada elemento un paso de tiempo, y lo rebota si toco un borde.
//
// Cada iteracion solo toca el elemento 'i', asi que repartirla entre hilos es
// seguro sin ningun mecanismo de sincronizacion.
void integrate(Simulation& sim, const Config& config, float dt) {
    const float width = static_cast<float>(config.width);
    const float height = static_cast<float>(config.height);
    const float twoPi = 2.0f * kPi;
    const int count = static_cast<int>(sim.elements.size());

    #pragma omp parallel for
    for (int i = 0; i < count; ++i) {
        Element& e = sim.elements[i];
        e.x += e.vx * dt;
        e.y += e.vy * dt;

        // El angulo se mantiene dentro de una vuelta. No cambia nada visual,
        // pero evita que despues de horas de animacion el float pierda
        // precision por acumular un numero enorme.
        e.angle = std::fmod(e.angle + e.angularSpeed * dt, twoPi);

        // Rebote en los bordes: se invierte la componente perpendicular a la
        // pared y se reubica el elemento justo adentro. Sin ese reajuste, uno
        // que llegara muy rapido podria quedarse atrapado invirtiendo su
        // velocidad cuadro tras cuadro sin llegar a salir.
        if (e.x - e.radius < 0.0f) {
            e.x = e.radius;
            e.vx = -e.vx;
        } else if (e.x + e.radius > width) {
            e.x = width - e.radius;
            e.vx = -e.vx;
        }

        if (e.y - e.radius < 0.0f) {
            e.y = e.radius;
            e.vy = -e.vy;
        } else if (e.y + e.radius > height) {
            e.y = height - e.radius;
            e.vy = -e.vy;
        }
    }
}

// Choque elastico entre dos circulos de igual masa.
//
// Con masas iguales, un choque elastico simplemente intercambia la componente
// de la velocidad que va a lo largo de la normal del choque; la componente
// tangencial no se toca. Se trabaja con el vector normal unitario (dx, dy)/d
// en lugar de sacar el angulo con atan2 y volver a coseno y seno: es la misma
// operacion, con menos funciones trigonometricas y sin perder precision al ir
// y volver del angulo.
//
// La actualizacion de a y b se protege con critical: dos hilos podrian tratar
// de chocar el mismo elemento con otros dos distintos al mismo tiempo.
//
// Se probo tambien con un lock por elemento en vez de este critical global,
// pensando que dejaria correr en paralelo choques sin elementos en comun. En
// la practica no gano: casi todo el costo de resolveCollisions() esta en el
// filtro de distancia (de solo lectura, ya paralelo), no en esta seccion
// critica, que solo se ejecuta en el pequeno porcentaje de pares que de
// verdad chocan. Con pocos choques reales, el critical global es igual de
// rapido y mas simple.
void collide(Element& a, Element& b) {
    const float dx = b.x - a.x;
    const float dy = b.y - a.y;
    const float distanceSquared = dx * dx + dy * dy;
    const float contactDistance = a.radius + b.radius;

    if (distanceSquared >= contactDistance * contactDistance) return;
    if (distanceSquared < kMinSeparation) return;

    #pragma omp critical
    {
        // Se vuelve a medir por si otro hilo ya movio a o b mientras
        // esperaba entrar aqui.
        const float dxLocked = b.x - a.x;
        const float dyLocked = b.y - a.y;
        const float distanceSquaredLocked = dxLocked * dxLocked + dyLocked * dyLocked;

        if (distanceSquaredLocked < contactDistance * contactDistance &&
            distanceSquaredLocked >= kMinSeparation) {
            const float distance = std::sqrt(distanceSquaredLocked);
            const float nx = dxLocked / distance;
            const float ny = dyLocked / distance;

            // Velocidad de b respecto de a, proyectada sobre la normal. Si es
            // negativa se estan acercando; si es positiva ya se estan
            // separando y volver a intercambiar velocidades los dejaria
            // pegados vibrando.
            const float approachSpeed = (b.vx - a.vx) * nx + (b.vy - a.vy) * ny;
            if (approachSpeed < 0.0f) {
                a.vx += approachSpeed * nx;
                a.vy += approachSpeed * ny;
                b.vx -= approachSpeed * nx;
                b.vy -= approachSpeed * ny;
            }

            // Se separan a partes iguales para deshacer el solapamiento que
            // dejo el paso de integracion, que da saltos discretos y no se
            // detiene justo en el punto de contacto.
            const float overlap = 0.5f * (contactDistance - distance);
            a.x -= overlap * nx;
            a.y -= overlap * ny;
            b.x += overlap * nx;
            b.y += overlap * ny;
        }
    }
}

// Revisa los N*(N-1)/2 pares posibles. Empezar en j = i + 1 evita revisar cada
// par dos veces y que un elemento choque contra si mismo.
void resolveCollisions(Simulation& sim) {
    const int count = static_cast<int>(sim.elements.size());

    // schedule(dynamic) porque las primeras iteraciones de 'i' hacen mucho
    // mas trabajo (bucle j mas grande) que las ultimas.
    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < count; ++i) {
        for (int j = i + 1; j < count; ++j) {
            collide(sim.elements[i], sim.elements[j]);
        }
    }
}

// Arma la lista de pares lo bastante cercanos como para unirlos con una linea.
// Recorre los mismos pares que resolveCollisions pero solo lee posiciones, sin
// modificar nada: por eso no necesita locks, solo un buffer por hilo para no
// pisarse al escribir en sim.links.
void buildLinks(Simulation& sim, const Config& config) {
    sim.links.clear();
    if (config.linkDistance <= 0.0f) return;

    const int count = static_cast<int>(sim.elements.size());
    const float maxDistance = config.linkDistance;
    const float maxDistanceSquared = maxDistance * maxDistance;

    #pragma omp parallel
    {
        std::vector<Link> localLinks;

        #pragma omp for schedule(dynamic)
        for (int i = 0; i < count; ++i) {
            const Element& a = sim.elements[i];
            for (int j = i + 1; j < count; ++j) {
                const Element& b = sim.elements[j];
                const float dx = b.x - a.x;
                const float dy = b.y - a.y;
                const float distanceSquared = dx * dx + dy * dy;
                if (distanceSquared >= maxDistanceSquared) continue;

                // La fuerza va de 1 (pegados) a 0 (justo en el limite) y luego
                // se traduce en la opacidad de la linea, para que las
                // conexiones se desvanezcan en vez de aparecer de golpe.
                const float distance = std::sqrt(distanceSquared);
                localLinks.push_back(Link{i, j, 1.0f - distance / maxDistance});
            }
        }

        #pragma omp critical
        {
            sim.links.insert(sim.links.end(), localLinks.begin(), localLinks.end());
        }
    }
}

} // namespace

void stepSimulation(Simulation& sim, const Config& config) {
    const float dt = config.timeStep;

    integrate(sim, config, dt);
    resolveCollisions(sim);
    buildLinks(sim, config);

    sim.time += dt;
}
