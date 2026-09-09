# Cálculo de Speedup y Eficiencia

Todas las corridas usan la misma semilla (`-seed 42`) y el mismo número de
cuadros por prueba, así que la secuencial y la paralela recorren exactamente
la misma trayectoria y hacen el mismo trabajo. El speedup se calcula sobre
`simulacion_s` (la columna de física), no sobre el tiempo total, porque el
dibujo con SDL es secuencial en las dos versiones (ver README).

Las 10 mediciones de cada prueba están en su CSV correspondiente dentro de
esta misma carpeta.

## Prueba 1 — N=1000, 300 cuadros, 8 hilos

| Métrica | Valor |
|---|---|
| Promedio secuencial (T_s) — `seq_1000.csv` | 0.848929 s |
| Promedio paralelo (T_p, 8 hilos) — `par_1000_8.csv` | 0.638113 s |
| **Speedup S = T_s / T_p** | **1.330** |
| **Eficiencia E = S / 8** | **0.166 (16.6%)** |

## Prueba 2 — N=1000, 300 cuadros, 4 hilos

| Métrica | Valor |
|---|---|
| Promedio secuencial (T_s) — `seq_1000.csv` | 0.848929 s |
| Promedio paralelo (T_p, 4 hilos) — `par_1000_4.csv` | 0.654176 s |
| **Speedup S = T_s / T_p** | **1.298** |
| **Eficiencia E = S / 4** | **0.324 (32.4%)** |

Con N=1000, pasar de 4 a 8 hilos casi no mejora el speedup (1.298 → 1.330),
pero la eficiencia cae a la mitad. Con este tamaño de problema, cada hilo
adicional aporta cada vez menos trabajo real y compite más por la sección
crítica y el overhead de arrancar/repartir el `parallel for`.

## Prueba 3 — N=3000, 30 cuadros, 8 hilos

| Métrica | Valor |
|---|---|
| Promedio secuencial (T_s) — `seq_3000.csv` | 0.462285 s |
| Promedio paralelo (T_p, 8 hilos) — `par_3000_8.csv` | 0.181123 s |
| **Speedup S = T_s / T_p** | **2.552** |
| **Eficiencia E = S / 8** | **0.319 (31.9%)** |

Con N=3000 el speedup casi se duplica frente a N=1000 (2.552 vs 1.330) con
los mismos 8 hilos. Confirma lo esperado: `resolveCollisions()` y
`buildLinks()` son O(N²), así que al crecer N el trabajo paralelizable
crece mucho más rápido que el overhead fijo de OpenMP (crear el equipo de
hilos, repartir el `parallel for`, entrar/salir de la sección crítica), y el
paralelismo empieza a pagarse mejor.

## Nota sobre el entorno de medición

Las 4 pruebas se corrieron en la misma máquina y de la misma forma
(`SDL_VIDEODRIVER=dummy`, sin ventana real), para que las comparaciones
fueran justas entre sí. `seq_1000.csv` muestra más variación entre
repeticiones que las demás pruebas (las primeras 3 corridas rondan 0.6 s de
simulación y el resto sube a ~0.95-1.0 s): es ruido del sistema (otros
procesos de fondo), no del programa — por eso el enunciado pide mínimo 10
repeticiones y se promedia, en vez de usar una sola corrida.

## Nota sobre sincronía: ¿lock por elemento en vez de `critical` global?

`resolveCollisions()` protege la actualización de cada choque con un único
`#pragma omp critical` que envuelve a **todos** los pares, no solo a los que
comparten un elemento. Antes de dejarlo así se probó una alternativa más
fina: un `omp_lock_t` por elemento, tomando primero el lock del índice más
chico para no generar interbloqueos, de forma que dos choques sin ningún
elemento en común pudieran resolverse al mismo tiempo.

Se midió esa alternativa contra la versión con `critical`, con N=3000 y 8
hilos (el caso con más pares chocando a la vez), 5 corridas cada una:

| Versión | Simulación promedio (5 corridas) |
|---|---|
| `critical` global | 0.6931 s |
| Lock por elemento | 0.7215 s |

El lock por elemento **no ganó** — salió por encima del `critical` global.
La razón: casi todo el costo de `resolveCollisions()` está en el filtro de
distancia entre pares, que ya es de solo lectura y corre en paralelo sin
ningún tipo de protección; la sección que sí se protege (la actualización de
posición/velocidad) solo se ejecuta en el pequeño porcentaje de pares que de
verdad están chocando en ese cuadro. Con tan pocos choques reales, el
`critical` global casi nunca genera espera entre hilos, y el lock por
elemento solo agrega el costo de dos llamadas extra (`omp_set_lock` /
`omp_unset_lock` por elemento) sin quitar contención real. Por eso el código
final se quedó con el `critical` global: es más simple y, en este caso, igual
de rápido o más.

**Camino real para mejorar el speedup**: no es la sincronía, sino el propio
recorrido O(N²) de `resolveCollisions()` y `buildLinks()`. Reemplazarlo por
una grilla espacial (dividir el canvas en celdas y solo comparar elementos de
celdas vecinas) evitaría la enorme mayoría de comparaciones que ya se sabe
que van a dar "no chocan" — es la optimización de estructura de datos que
queda anotada como extra opcional en `PLAN.md`.
