#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include <vector>

#include "screensaver.hpp"

// ---------------------------------------------------------------------------
// Estado de la animacion y operaciones que lo hacen avanzar.
//
// createSimulation() es codigo comun a las dos versiones (src/common), porque
// ambas tienen que arrancar del MISMO estado inicial para que la comparacion
// de tiempos sea justa. stepSimulation() en cambio se implementa por separado
// en cada version (src/secuencial/physics.cpp y src/paralelo/physics.cpp):
// esa es justamente la parte que la Fase 2 paraleliza.
// ---------------------------------------------------------------------------

// Dos elementos lo bastante cerca como para unirlos con una linea. Se guardan
// los indices y no punteros para que el arreglo de elementos pueda crecer o
// reordenarse sin invalidar nada.
struct Link {
    int a, b;         // Indices dentro de Simulation::elements.
    float strength;   // 1 cuando estan pegados, 0 al limite de Config::linkDistance.
};

// Estado completo de la animacion en un instante dado.
struct Simulation {
    std::vector<Element> elements;  // Los N elementos.
    std::vector<Link> links;        // Se recalcula entero en cada paso.
    float time = 0.0f;              // Segundos simulados desde el arranque.
};

// Crea los N elementos con posicion, velocidad, giro y color pseudoaleatorios,
// respetando los rangos de 'config'. Con la misma semilla devuelve siempre el
// mismo estado inicial.
Simulation createSimulation(const Config& config);

// Avanza la simulacion un paso de Config::timeStep segundos: integra el
// movimiento, rebota en los bordes, resuelve las colisiones entre elementos y
// recalcula las lineas de conexion.
void stepSimulation(Simulation& sim, const Config& config);

#endif // SIMULATION_HPP
