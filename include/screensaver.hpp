#ifndef SCREENSAVER_HPP
#define SCREENSAVER_HPP

// Estructuras y utilidades compartidas entre la version secuencial y paralela.
// Mantener este header sincronizado entre ambas implementaciones para que
// las mediciones de speedup comparen exactamente el mismo modelo de datos.
//
// Reglas de fisica/colision y diseno visual (rebote, colision elastica,
// rotacion/pulso, estelas, lineas de conexion): ver PLAN.md, seccion 1.

struct Config {
    int n = 100;          // Cantidad de elementos a renderizar (parametro obligatorio)
    int width = 640;       // Ancho del canvas (minimo 640)
    int height = 480;      // Alto del canvas (minimo 480)
    int frames = 0;        // 0 = correr indefinidamente hasta cerrar la ventana
    unsigned int seed = 0; // Semilla para generacion pseudoaleatoria de posiciones/colores

    float trailFade = 0.15f;       // Opacidad del rectangulo de estela por frame (0-1)
    float connectDistance = 120.f; // Distancia maxima para dibujar linea de conexion
    float pulseSpeed = 2.0f;       // Velocidad del pulso de tamano/brillo (rad/s)
};

struct Element {
    float x, y;     // Posicion
    float vx, vy;   // Velocidad
    float radius;   // Radio / tamano base (antes de aplicar el pulso)
    unsigned char r, g, b; // Color RGB

    float angle;        // Angulo de rotacion propio (rad)
    float angularVel;    // Velocidad angular (rad/s)
    float phase;         // Offset de fase para el pulso de sin(), evita que todos pulsen igual
};

// Parsea argumentos de linea de comando hacia Config.
// Debe incluir programacion defensiva: valores faltantes, negativos o no numericos.
bool parseArgs(int argc, char** argv, Config& config);

#endif // SCREENSAVER_HPP
