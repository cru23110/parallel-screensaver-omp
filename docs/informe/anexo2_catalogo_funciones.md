# Anexo 2 — Catálogo de funciones

Screensaver Paralelo con OpenMP · Proyecto #1 · Computación Paralela y Distribuida · UVG · Semestre 2, 2026

---

## Módulo: `src/common/args.cpp` (declarado en `include/screensaver.hpp`)

### `parseArgs`
| Campo | Detalle |
|---|---|
| **Firma** | `bool parseArgs(int argc, char** argv, Config& config)` |
| **Entradas** | `argc`: número de argumentos de la línea de comandos. `argv`: arreglo de cadenas con los argumentos. `config`: estructura donde se escriben los parámetros (se modifica). |
| **Salida** | `true` si todos los argumentos son válidos y `config` fue llenada; `false` si algo falta, no es numérico o está fuera de rango (imprime el motivo por `stderr`). |
| **Descripción** | Recorre `argv` y llena los campos de `Config` según las banderas recibidas (`-n`, `-w`, `-h`, `-seed`, `-t`, `-frames`, `-sides`, `-rmin`, `-rmax`, `-vmin`, `-vmax`, `-trail`, `-glow`, `-link`, `-shot`, `-noclock`, `-nohud`). Llama a `validateConfig` al final para revisar coherencia global. Desactiva `vsync` automáticamente cuando se usa `-frames`, para que la medición no quede limitada por el refresco del monitor. |

### `printUsage`
| Campo | Detalle |
|---|---|
| **Firma** | `void printUsage(const char* programName)` |
| **Entradas** | `programName`: nombre del ejecutable (normalmente `argv[0]`). |
| **Salida** | Ninguna (imprime por `stdout`). |
| **Descripción** | Imprime la lista de todas las banderas con su descripción y valor por defecto. Se llama cuando `parseArgs` devuelve `false`. |

---

## Módulo: `src/common/elements.cpp` (declarado en `include/simulation.hpp`)

### `createSimulation`
| Campo | Detalle |
|---|---|
| **Firma** | `Simulation createSimulation(const Config& config)` |
| **Entradas** | `config`: parámetros de la corrida (en especial `n`, `seed`, radios, velocidades, `width`, `height`, paleta). |
| **Salida** | Un objeto `Simulation` con los N elementos inicializados y la lista de enlaces vacía. |
| **Descripción** | Inicializa `std::mt19937` con `config.seed` (o con `std::random_device` si la semilla es 0). Para cada elemento sortea radio, posición (descontando el radio para que quede dentro del canvas), dirección y rapidez, ángulo, velocidad angular y desfase de pulso. El color se elige uniformemente de `config.palette`. Con la misma semilla, las dos versiones producen exactamente el mismo estado inicial. |

---

## Módulo: `src/common/metrics.cpp` (declarado en `include/metrics.hpp`)

### `class Stopwatch`
| Método | Descripción |
|---|---|
| `Stopwatch()` | Constructor. Registra el instante de inicio usando `std::chrono::steady_clock`. |
| `elapsed() → double` | Devuelve los segundos transcurridos desde la construcción o el último `reset()`. |
| `reset()` | Reinicia el cronómetro al instante actual. |
| `lap() → double` | Devuelve los segundos transcurridos y llama a `reset()`. |

### `printMetrics`
| Campo | Detalle |
|---|---|
| **Firma** | `void printMetrics(const Config& config, const Metrics& metrics, const char* versionName)` |
| **Entradas** | `config`: parámetros de la corrida. `metrics`: totales acumulados. `versionName`: `"secuencial"` o `"paralelo"`. |
| **Salida** | Ninguna (imprime por `stdout`). |
| **Descripción** | Imprime primero un resumen legible con tiempos totales, por cuadro y porcentajes, y al final una línea `CSV,...` que `benchmark.sh` extrae para armar los CSVs de la bitácora. |

---

## Módulo: `src/common/render.cpp` (declarado en `include/render.hpp`)

### `pollQuitRequest`
| Campo | Detalle |
|---|---|
| **Firma** | `bool pollQuitRequest()` |
| **Entradas** | Ninguna. |
| **Salida** | `true` si el usuario pidió cerrar (botón de ventana, Escape o Q). |
| **Descripción** | Consume todos los eventos pendientes de SDL sin bloquear. |

### `class Renderer`

| Método | Firma | Descripción |
|---|---|---|
| `init` | `bool init(const Config& config)` | Inicializa SDL2, abre la ventana y crea el renderer acelerado. Devuelve `false` si algo falla. |
| `drawFrame` | `void drawFrame(const Simulation& sim, const Config& config, const Metrics& metrics)` | Dibuja un cuadro completo llamando internamente a `drawTrail`, `drawLinks`, `drawElements`, `drawClock` y `drawHud`. |
| `setTitle` | `void setTitle(const std::string& title)` | Actualiza el título de la ventana. |
| `saveScreenshot` | `bool saveScreenshot(const char* path) const` | Guarda el contenido actual en un archivo BMP. Devuelve `false` si falla. |
| `drawTrail` | `void drawTrail(const Config& config)` | Dibuja un rectángulo negro semitransparente sobre todo el canvas para el efecto de estela. Opacidad controlada por `config.trailFade`. |
| `drawLinks` | `void drawLinks(const Simulation& sim)` | Traza una línea entre cada par de `sim.links` con opacidad proporcional a `Link::strength`. |
| `drawElements` | `void drawElements(const Simulation& sim, const Config& config)` | Dibuja cada elemento como polígono regular de `config.coreSides` lados con radio pulsante y glow. |
| `drawClock` | `void drawClock(const Config& config)` | Muestra la hora actual en texto sobre un panel semitransparente si `config.showClock` es `true`. |
| `drawHud` | `void drawHud(const Config& config, const Metrics& metrics)` | Muestra los contadores de N, FPS e hilos si `config.showHud` es `true`. |

---

## Módulo: `src/secuencial/physics.cpp` y `src/paralelo/physics.cpp` (declarado en `include/simulation.hpp`)

### `stepSimulation`
| Campo | Detalle |
|---|---|
| **Firma** | `void stepSimulation(Simulation& sim, const Config& config)` |
| **Entradas** | `sim`: estado actual (se modifica). `config`: parámetros (en especial `timeStep` y `linkDistance`). |
| **Salida** | Ninguna (`sim` queda con posiciones, velocidades y enlaces actualizados). |
| **Descripción** | Avanza la simulación un paso llamando a `integrate`, luego `resolveCollisions`, luego `buildLinks`. El orden importa: primero se mueve, luego se corrigen solapamientos, y por último se calculan las distancias sobre posiciones ya corregidas. |

### `integrate` *(interna)*
| Campo | Detalle |
|---|---|
| **Descripción** | Mueve cada elemento con `posición += velocidad * dt`, acumula la rotación y rebota en los bordes. Cada elemento es completamente independiente. En la versión paralela usa `#pragma omp parallel for` sin sincronización adicional. |

### `collide` *(interna)*
| Campo | Detalle |
|---|---|
| **Firma** | `void collide(Element& a, Element& b)` |
| **Descripción** | Resuelve el choque elástico entre dos círculos de igual masa intercambiando la componente de velocidad sobre la normal de contacto y separando los centros para deshacer el solapamiento. En la versión paralela, la sección de escritura está protegida con `#pragma omp critical`. |

### `resolveCollisions` *(interna)*
| Campo | Detalle |
|---|---|
| **Descripción** | Recorre los N·(N-1)/2 pares y llama a `collide`. En la versión paralela, el bucle exterior usa `#pragma omp parallel for schedule(dynamic)` para equilibrar la carga (las primeras filas de `i` tienen más trabajo). |

### `buildLinks` *(interna)*
| Campo | Detalle |
|---|---|
| **Descripción** | Vacía `sim.links` y lo rellena con los pares de elementos cuya distancia es menor que `config.linkDistance`. En la versión paralela, cada hilo acumula en un vector local y los fusiona en `sim.links` con `#pragma omp critical` al final. |
