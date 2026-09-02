#ifndef SCREENSAVER_HPP
#define SCREENSAVER_HPP

#include <cstdint>

// ---------------------------------------------------------------------------
// Tipos y configuracion compartidos entre la version secuencial y la paralela.
//
// Todo lo que se pueda ajustar vive en Config (nada de constantes sueltas
// dentro de los loops), y Config se llena unicamente desde la linea de
// comandos. Las dos versiones enlazan el MISMO parseArgs (src/common/args.cpp),
// asi que por construccion aceptan exactamente las mismas banderas: si no
// fuera asi, las mediciones de speedup no compararian lo mismo.
//
// Las reglas de fisica y el concepto visual (rebote, colision elastica,
// rotacion, pulso, estelas y lineas de conexion) estan descritos en PLAN.md,
// seccion 1.
// ---------------------------------------------------------------------------

// Pi en precision simple: la simulacion trabaja en float, no hace falta doble.
inline constexpr float kPi = 3.14159265358979323846f;

// Color RGB de 8 bits por canal. La transparencia no se guarda aqui porque
// depende del efecto con el que se dibuje (nucleo, halo o linea).
struct Color {
    std::uint8_t r, g, b;
};

// Paleta "neon": pocos tonos pero muy saturados, elegidos para verse
// brillantes sobre fondo negro con mezcla aditiva. Vive fuera del loop de
// dibujo para poder cambiarla sin tocar el render.
inline constexpr Color kNeonPalette[] = {
    {   0, 255, 255 }, // cian
    { 255,  40, 170 }, // magenta
    { 170,  85, 255 }, // violeta
    {  60, 255, 130 }, // verde menta
    { 255, 205,  50 }, // ambar
    {  70, 140, 255 }, // azul electrico
};
inline constexpr int kNeonPaletteCount =
    static_cast<int>(sizeof(kNeonPalette) / sizeof(kNeonPalette[0]));

// Parametros de una corrida completa. Los defaults son los que se usan cuando
// la bandera correspondiente no aparece en la linea de comandos.
struct Config {
    // --- Parametros principales (banderas de linea de comandos) ---
    int n = 0;              // Cantidad de elementos. Obligatorio, sin default util.
    int width = 640;        // Ancho del canvas en pixeles (minimo 640).
    int height = 480;       // Alto del canvas en pixeles (minimo 480).
    int frames = 0;         // 0 = correr hasta que el usuario cierre la ventana.
    unsigned int seed = 0;  // 0 = semilla no determinista (reloj del sistema).
    int threads = 0;        // 0 = dejar que OpenMP decida. Solo lo usa la version paralela.

    // --- Apariencia y fisica ---
    float minRadius = 4.0f;         // Radio minimo de un elemento (pixeles).
    float maxRadius = 14.0f;        // Radio maximo de un elemento (pixeles).
    float minSpeed = 40.0f;         // Rapidez inicial minima (pixeles por segundo).
    float maxSpeed = 180.0f;        // Rapidez inicial maxima (pixeles por segundo).
    float maxAngularSpeed = 2.5f;   // Giro propio maximo, en radianes por segundo.
    float trailFade = 0.15f;        // Opacidad del velo negro que deja la estela (0-1).
    float linkDistance = 120.0f;    // Distancia maxima para unir dos elementos con una linea.
    float pulseSpeed = 2.0f;        // Velocidad del pulso de tamano, en radianes por segundo.
    float pulseAmount = 0.20f;      // Amplitud del pulso como fraccion del radio.
    int coreSides = 3;              // Lados del poligono de cada elemento (3 = triangulo).
    float glow = 1.0f;              // Intensidad del resplandor, de 0 a 1. Como los
                                    // elementos se dibujan con mezcla aditiva, con
                                    // muchos elementos juntos la imagen se satura a
                                    // blanco; bajar este valor lo compensa.

    // Ruta donde guardar una imagen del ultimo cuadro antes de salir, o
    // nullptr para no guardar nada. Apunta directo al texto de argv, que vive
    // hasta que termina el programa.
    const char* screenshotPath = nullptr;

    // --- Sobreimpresos (no afectan la fisica ni la paralelizacion) ---
    bool showClock = true;          // Reloj con la hora actual, arriba al centro.
    bool showHud = true;            // Contadores de N, FPS e hilos, abajo a la izquierda.

    // --- Parametros de la corrida ---
    // Paso de tiempo fijo. Se usa un valor constante en vez del tiempo real
    // entre frames para que la secuencial y la paralela recorran exactamente
    // la misma trayectoria y los tiempos sean comparables entre corridas.
    float timeStep = 1.0f / 60.0f;

    // Sincronizacion vertical. Se apaga sola cuando se pide -frames, porque en
    // ese modo interesa medir cuanto cuesta el trabajo, no esperar al monitor.
    bool vsync = true;

    // --- Paleta ---
    const Color* palette = kNeonPalette;
    int paletteCount = kNeonPaletteCount;
};

// Un elemento animado del screensaver. Para la fisica es un circulo de radio
// 'radius'; el angulo y la fase solo afectan como se dibuja.
struct Element {
    float x, y;           // Posicion del centro, en pixeles.
    float vx, vy;         // Velocidad, en pixeles por segundo.
    float radius;         // Radio base, antes de aplicarle el pulso.
    float angle;          // Rotacion propia acumulada, en radianes.
    float angularSpeed;   // Velocidad de giro propia, en radianes por segundo.
    float phase;          // Desfase del pulso, para que no pulsen todos igual.
    Color color;          // Color tomado de la paleta.
};

// Llena 'config' con las banderas recibidas. Devuelve false (y explica que
// estuvo mal por stderr) si falta un valor, si no es numerico o si esta fuera
// de rango. Definida en src/common/args.cpp.
bool parseArgs(int argc, char** argv, Config& config);

// Imprime la ayuda de uso. 'programName' normalmente es argv[0].
void printUsage(const char* programName);

#endif // SCREENSAVER_HPP
