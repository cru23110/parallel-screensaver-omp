#ifndef SCREENSAVER_HPP
#define SCREENSAVER_HPP

// Estructuras y utilidades compartidas entre la version secuencial y paralela.
// Mantener este header sincronizado entre ambas implementaciones para que
// las mediciones de speedup comparen exactamente el mismo modelo de datos.

struct Config {
    int n = 100;          // Cantidad de elementos a renderizar (parametro obligatorio)
    int width = 640;       // Ancho del canvas (minimo 640)
    int height = 480;      // Alto del canvas (minimo 480)
    int frames = 0;        // 0 = correr indefinidamente hasta cerrar la ventana
    unsigned int seed = 0; // Semilla para generacion pseudoaleatoria de posiciones/colores
};

struct Element {
    float x, y;     // Posicion
    float vx, vy;   // Velocidad
    float radius;   // Radio / tamano
    unsigned char r, g, b; // Color RGB
};

// Parsea argumentos de linea de comando hacia Config.
// Debe incluir programacion defensiva: valores faltantes, negativos o no numericos.
bool parseArgs(int argc, char** argv, Config& config);

#endif // SCREENSAVER_HPP
