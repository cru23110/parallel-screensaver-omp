# Screensaver Paralelo con OpenMP

Proyecto #1 del curso **Computación Paralela y Distribuida** (Universidad del Valle de Guatemala, Semestre 2, 2026 — Docente: Marlon Fuentes).

Implementación de un screensaver que genera **N** elementos animados con física/trigonometría (movimiento, rebotes, colisiones), primero en una versión **secuencial** y luego en una o más versiones **paralelas** con OpenMP, comparando desempeño mediante speedup y eficiencia.

## Integrantes

- [ ] Nombre 1
- [ ] Nombre 2
- [ ] Nombre 3

## Requisitos del proyecto (checklist)

- [ ] Código de autoría propia en C/C++, comentado.
- [ ] Historial de commits que refleje trabajo distribuido en el tiempo (no todo de última hora).
- [ ] Uso de OpenMP.
- [ ] Versión secuencial y al menos una versión paralela.
- [ ] Cálculo de speedup y eficiencia por versión (mínimo 10 mediciones por prueba).
- [ ] Screensaver: recibe N por parámetro, varios colores pseudoaleatorios, canvas mínimo 640x480, con movimiento y física/trigonometría.
- [ ] Programación defensiva en el ingreso de datos.
- [ ] `readme.md` de uso (este archivo).
- [ ] Sin variables hard-coded: todo parametrizado por argumentos de línea de comandos.
- [ ] Mecanismos de protección de memoria compartida / sincronía (críticas, reducciones, barreras según aplique).
- [ ] Informe (carátula, índice, introducción, antecedentes, cuerpo, citas, conclusiones, apéndices, ≥3 referencias bibliográficas).
- [ ] Anexo 1: diagrama de flujo.
- [ ] Anexo 2: catálogo de funciones.
- [ ] Anexo 3: bitácora de pruebas.
- [ ] (Opcional, extra hasta 20%) Documentar cualquier optimización adicional con speedups que la respalden.

El enunciado completo está en [`docs/enunciado.pdf`](docs/enunciado.pdf).

## Estructura del repositorio

```
.
├── include/               Headers compartidos entre versión secuencial y paralela
│   └── screensaver.hpp
├── src/
│   ├── secuencial/        Versión secuencial (baseline para el speedup)
│   │   └── main.cpp
│   └── paralelo/          Versión(es) paralela(s) con OpenMP
│       └── main.cpp
├── scripts/
│   └── benchmark.sh       Corre N repeticiones de un binario y guarda tiempos en CSV
├── resultados/            CSVs y capturas de las mediciones (Anexo 3)
├── docs/
│   ├── enunciado.pdf
│   ├── diagramas/         Diagrama de flujo (Anexo 1)
│   └── informe/           Informe final en PDF
├── Makefile
└── README.md
```

## Requisitos previos

- Compilador con soporte C++17 y OpenMP (`g++` en Linux/macOS con GCC; en macOS con `clang` de Apple hay que instalar `libomp` o usar `brew install gcc`).
- [SDL2](https://www.libsdl.org/) para el renderizado (`brew install sdl2` / `apt install libsdl2-dev`).
- [SDL2_ttf](https://github.com/libsdl-org/SDL_ttf) para dibujar el reloj en pantalla (`brew install sdl2_ttf` / `apt install libsdl2-ttf-dev`). Es una dependencia extra solo para esa parte del diseño visual — si complica la compilación en alguna máquina, se puede desactivar con `Config::showClock` y quitarla del build.

## Compilación

```bash
make            # compila ambas versiones en bin/
make secuencial # solo la versión secuencial
make paralelo   # solo la versión paralela
make clean      # limpia binarios
```

## Uso

```bash
bin/screensaver_seq -n 500
bin/screensaver_par -n 500
```

| Flag | Descripción | Default |
|------|-------------|---------|
| `-n` | Cantidad de elementos a renderizar (**requerido**) | — |
| `-w` | Ancho del canvas | 640 |
| `-h` | Alto del canvas | 480 |
| `-seed` | Semilla para generación pseudoaleatoria | aleatoria |

> Los flags reales se definen en `parseArgs` (`src/secuencial/main.cpp` y `src/paralelo/main.cpp`) — mantenerlos idénticos en ambas versiones para que las pruebas comparen lo mismo.

## Medición de speedup

```bash
scripts/benchmark.sh bin/screensaver_seq 1000 10 resultados/seq_1000.csv
scripts/benchmark.sh bin/screensaver_par 1000 10 resultados/par_1000.csv
```

Con ambos CSV (mínimo 10 corridas cada uno) se calcula speedup = tiempo_secuencial / tiempo_paralelo y eficiencia = speedup / número_de_hilos, usando promedio o máximo de los tiempos según el enunciado.

## Flujo de trabajo sugerido

1. Diseñar y dejar **funcional** la versión secuencial (`src/secuencial`) antes de paralelizar nada.
2. Copiar esa lógica a `src/paralelo` y aplicar PCAM: identificar qué se puede particionar, dónde hay dependencias/datos compartidos, y paralelizar de forma incremental con OpenMP.
3. Medir speedup después de cada cambio significativo (no solo al final) y guardar los resultados en `resultados/`.
4. Documentar en el informe el razonamiento de cada mejora, sobre todo si se apunta a los criterios diferenciadores (extra).

### Convenciones de trabajo

- Commits pequeños y frecuentes, en inglés o español pero consistentes, describiendo el *por qué* del cambio.
- Una rama por feature/optimización (`feature/colisiones`, `perf/reduccion-atomica`, etc.), merge a `main` vía PR cuando compile y corra.
- Mantener el repositorio **privado** durante el desarrollo y hacerlo público solo el día de la entrega, según lo pide el enunciado.

## Licencia

Proyecto académico — Universidad del Valle de Guatemala. Uso restringido al curso Computación Paralela y Distribuida.
