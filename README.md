# Screensaver Paralelo con OpenMP

Proyecto #1 del curso **Computación Paralela y Distribuida** (Universidad del Valle de Guatemala, Semestre 2, 2026 — Docente: Marlon Fuentes).

Screensaver que genera **N** elementos animados con física y trigonometría (movimiento, rebotes en los bordes, colisiones elásticas, rotación propia y pulso), primero en una versión **secuencial** y luego en una o más versiones **paralelas** con OpenMP, comparando desempeño mediante speedup y eficiencia.

## Integrantes

- [ ] Erick
- [ ] Fabián
- [ ] Juan

## Qué hace el screensaver

Cada elemento es un polígono neón (triángulo por defecto) que se mueve sobre un fondo negro:

- **Rebote en los bordes**: al tocar una pared se invierte la componente perpendicular de la velocidad y se reubica el elemento justo adentro.
- **Colisión elástica entre elementos**: se detecta por distancia entre centros y se resuelve intercambiando la componente de la velocidad a lo largo de la normal del choque (masas iguales), separando además el solapamiento.
- **Rotación y pulso**: cada elemento gira sobre sí mismo con su propia velocidad angular y su tamaño late con un seno, con una fase distinta por elemento.
- **Estelas**: en vez de borrar la pantalla, cada cuadro pinta encima un velo negro semitransparente, lo que deja cola de movimiento.
- **Constelación**: se traza una línea entre cada par de elementos más cercanos que un umbral, con opacidad proporcional a la cercanía. Es un segundo recorrido O(N²) sobre los mismos pares que las colisiones, pero de solo lectura.
- **Reloj y HUD**: la hora y los contadores de N, FPS e hilos se dibujan con una fuente de mapa de bits 5x7 incluida en el propio programa, sin dependencias extra.

## Estado del proyecto

| Fase | Estado |
|------|--------|
| Fase 1 — versión secuencial funcional | Hecha |
| Fase 2 — versión paralela con OpenMP | Pendiente |
| Fase 3 — mediciones de speedup y eficiencia | Pendiente |
| Fase 4 — informe y anexos | Pendiente |

## Requisitos del proyecto (checklist)

- [x] Código de autoría propia en C/C++, comentado.
- [ ] Historial de commits que refleje trabajo distribuido en el tiempo (no todo de última hora).
- [ ] Uso de OpenMP.
- [ ] Versión secuencial y al menos una versión paralela.
- [ ] Cálculo de speedup y eficiencia por versión (mínimo 10 mediciones por prueba).
- [x] Screensaver: recibe N por parámetro, varios colores pseudoaleatorios, canvas mínimo 640x480, con movimiento y física/trigonometría.
- [x] Programación defensiva en el ingreso de datos.
- [x] `readme.md` de uso (este archivo).
- [x] Sin variables hard-coded: todo parametrizado por argumentos de línea de comandos.
- [ ] Mecanismos de protección de memoria compartida / sincronía.
- [ ] Informe (carátula, índice, introducción, antecedentes, cuerpo, citas, conclusiones, apéndices, ≥3 referencias bibliográficas).
- [ ] Anexo 1: diagrama de flujo.
- [ ] Anexo 2: catálogo de funciones.
- [ ] Anexo 3: bitácora de pruebas.
- [ ] (Opcional, extra hasta 20%) Documentar cualquier optimización adicional con speedups que la respalden.

El enunciado completo está en [`docs/enunciado.pdf`](docs/enunciado.pdf).

## Estructura del repositorio

```
.
├── include/                 Headers compartidos por las dos versiones
│   ├── screensaver.hpp        Config, Element, paleta y parseo de argumentos
│   ├── simulation.hpp         Estado de la animación y su avance
│   ├── render.hpp             Capa de dibujo con SDL2
│   ├── text.hpp               Dibujo de texto con la fuente incluida
│   └── metrics.hpp            Cronómetro y resumen de tiempos
├── src/
│   ├── common/              Código idéntico en ambas versiones
│   │   ├── args.cpp           Lectura y validación de argumentos
│   │   ├── elements.cpp       Creación del estado inicial
│   │   ├── render.cpp         Dibujo (estelas, constelación, glow, reloj, HUD)
│   │   ├── text.cpp           Fuente de mapa de bits 5x7
│   │   └── metrics.cpp        Impresión del resumen y de la línea CSV
│   ├── secuencial/          Versión secuencial (baseline del speedup)
│   │   ├── physics.cpp        Movimiento, rebotes, colisiones y constelación
│   │   └── main.cpp           Ciclo principal y medición
│   └── paralelo/            Versión(es) paralela(s) con OpenMP (Fase 2)
│       └── main.cpp
├── scripts/
│   └── benchmark.sh         Corre N repeticiones y junta los tiempos en un CSV
├── resultados/              CSVs y capturas de las mediciones (Anexo 3)
├── docs/
│   ├── enunciado.pdf
│   ├── diagramas/           Diagrama de flujo (Anexo 1)
│   └── informe/             Informe final en PDF
├── Makefile
└── README.md
```

**Por qué `src/common/`**: `parseArgs` y la creación de los elementos las enlazan las dos versiones. Así es imposible que las banderas o el estado inicial se desincronicen entre la secuencial y la paralela, que es justo lo que arruinaría la comparación de speedup. Lo único que se implementa por separado en cada versión es `stepSimulation()` (en `physics.cpp`), porque esa es la parte que la Fase 2 reparte entre hilos.

## Requisitos previos

- Compilador con soporte C++17 y OpenMP (`g++`; en macOS con `clang` de Apple hay que instalar `libomp` o usar `brew install gcc`).
- [SDL2](https://www.libsdl.org/) 2.0.18 o más nuevo, por `SDL_RenderGeometry` (`apt install libsdl2-dev` / `brew install sdl2` / `pacman -S mingw-w64-x86_64-SDL2` en MSYS2).

No hace falta SDL2_ttf: el reloj y el HUD usan una fuente de mapa de bits incluida en `src/common/text.cpp`.

## Compilación

```bash
make            # compila la versión secuencial en bin/
make secuencial # lo mismo, explícito
make paralelo   # versión paralela (disponible a partir de la Fase 2)
make clean      # limpia binarios
```

Si `pkg-config` y `sdl2-config` no están disponibles (por ejemplo, con SDL2 descomprimido a mano en Windows), se le pasan las banderas al `make`:

```bash
make SDL_FLAGS="-IC:/SDL2/include/SDL2 -LC:/SDL2/lib -lmingw32 -lSDL2main -lSDL2"
```

## Uso

```bash
bin/screensaver_seq -n 400 -w 1280 -h 720
```

Se cierra con el botón de la ventana, `Esc` o `Q`.

| Flag | Descripción | Default |
|------|-------------|---------|
| `-n` | Cantidad de elementos (**obligatorio**, 1 a 100000) | — |
| `-w` | Ancho del canvas (mínimo 640) | 640 |
| `-h` | Alto del canvas (mínimo 480) | 480 |
| `-frames` | Cuadros a dibujar antes de salir solo. 0 = hasta cerrar la ventana. Al usarlo se apaga el vsync | 0 |
| `-seed` | Semilla pseudoaleatoria. 0 = distinta cada vez | 0 |
| `-t` | Hilos de OpenMP. 0 = los que decida OpenMP. La secuencial la acepta pero la ignora | 0 |
| `-rmin` / `-rmax` | Radio mínimo / máximo de un elemento | 4 / 14 |
| `-vmin` / `-vmax` | Rapidez inicial mínima / máxima, en px/s | 40 / 180 |
| `-sides` | Lados del polígono de cada elemento (3 a 12) | 3 |
| `-trail` | Opacidad de la estela, entre 0 y 1. Más alto = cola más corta | 0.15 |
| `-link` | Distancia máxima para unir dos elementos con una línea. 0 la desactiva | 120 |
| `-shot` | Guardar el último cuadro como BMP al salir | — |
| `-noclock` / `-nohud` | No dibujar el reloj / los contadores | — |
| `--help` | Mostrar la ayuda completa | — |

Las banderas son **idénticas en las dos versiones** porque comparten el mismo `parseArgs`.

## Medición de speedup

Al terminar, el programa imprime un resumen con el tiempo total, el tiempo de simulación y el tiempo de dibujo por separado, más una línea `CSV,...` lista para procesar:

```
=== Resumen de la corrida (secuencial) ===
  elementos          : 3000
  cuadros            : 120
  tiempo total       : 3.8172 s
  simulacion (total) : 0.9380 s  (24.6% del total)
  dibujo (total)     : 2.8743 s  (75.3% del total)

CSV,secuencial,3000,1280,720,1,120,3.817187,0.937997,2.874342
```

**El speedup se calcula sobre la columna de simulación**, no sobre el tiempo total: el dibujo con SDL es secuencial en las dos versiones (`SDL_Renderer` no se puede usar desde varios hilos), así que meterlo en la cuenta escondería la mejora real detrás del tiempo de la GPU.

```bash
scripts/benchmark.sh bin/screensaver_seq 3000 300 10 resultados/seq_3000.csv
scripts/benchmark.sh bin/screensaver_par 3000 300 10 resultados/par_3000_8.csv 8
```

Con ambos CSV (mínimo 10 corridas cada uno) se calcula `speedup = simulacion_secuencial / simulacion_paralela` y `eficiencia = speedup / hilos`, usando promedio o máximo según el enunciado.

Para que la comparación sea válida, las dos corridas usan la **misma semilla** y el **mismo número de cuadros**, así que recorren exactamente la misma trayectoria y hacen exactamente el mismo trabajo.

## Convenciones de trabajo

- Commits pequeños y frecuentes, con mensajes en español (`tipo: descripción corta`), repartidos a lo largo de todo el período.
- Mantener el repositorio **privado** durante el desarrollo y hacerlo público solo el día de la entrega, según lo pide el enunciado.

## Licencia

Proyecto académico — Universidad del Valle de Guatemala. Uso restringido al curso Computación Paralela y Distribuida.
