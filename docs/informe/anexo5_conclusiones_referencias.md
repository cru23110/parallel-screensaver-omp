# Conclusiones, Recomendaciones y Referencias

Screensaver Paralelo con OpenMP · Proyecto #1 · Computación Paralela y Distribuida · UVG · Semestre 2, 2026

---

## Conclusiones

1. **La paralelización no siempre produce el speedup esperado para N pequeño.** Las mediciones con N=1000 mostraron tiempos muy similares entre la versión secuencial y la paralela. Esto se explica porque con pocos elementos, el costo de crear y coordinar los hilos de OpenMP (overhead) es comparable al del trabajo en sí. El beneficio de la paralelización se vuelve más significativo a medida que N crece.

2. **El cuello de botella está en el render, no en la física.** Para N=1000 con 300 cuadros, el tiempo de simulación fue de aproximadamente 0.28 s mientras el de dibujo fue de 5.4 s. Esto confirma que SDL_Renderer es el componente que más tiempo consume, y que paralelizar solo la física tiene un impacto limitado en el tiempo total. Para obtener speedups significativos con N pequeño, sería necesario paralelizar el render también, lo cual requiere una librería gráfica que soporte acceso multihilo.

3. **El patrón de acceso O(N²) de `resolveCollisions` y `buildLinks` es el candidato real a paralelizar.** Con N grande, estas dos funciones dominan el tiempo de simulación y es donde la paralelización produce ganancias reales. El uso de `schedule(dynamic)` fue fundamental para equilibrar la carga entre hilos, dado que el bucle interno se hace más corto a medida que `i` crece.

4. **La protección de memoria compartida con `#pragma omp critical` es correcta pero tiene costo.** La sección crítica en `collide` garantiza que dos hilos no actualicen el mismo elemento simultáneamente, pero serializa las escrituras en los pares que colisionan. Para N grande con muchas colisiones simultáneas, este overhead puede limitar el speedup. Una alternativa más eficiente sería asignar cada elemento a un único hilo y manejar los choques entre hilos distintos con un paso de comunicación posterior.

5. **La metodología PCAM fue útil para estructurar el diseño paralelo.** Identificar la granularidad (elemento a elemento para `integrate`, par a par para las colisiones), la comunicación necesaria (escrituras compartidas en `collide`, escrituras al vector `links`), la aglomeración (a nivel de ciclo for) y el mapeo (dinámico por el desbalance inherente) permitió tomar decisiones justificadas en vez de agregar `#pragma omp parallel for` de forma ciega.

---

## Recomendaciones

- **Usar `-link 0` para deshabilitar `buildLinks` en mediciones de física pura**, ya que las líneas de conexión no forman parte de la física y añaden trabajo O(N²) que puede confundir los resultados.
- **Probar con N ≥ 5000** para que el tiempo de simulación sea mayor que el del render y el speedup sea medible de forma significativa.
- **Explorar `#pragma omp atomic`** en lugar de `#pragma omp critical` para las escrituras escalares dentro de `collide`, ya que `atomic` tiene menor overhead para operaciones simples sobre variables individuales.
- **Considerar una grilla espacial** (dividir el canvas en celdas y solo revisar pares en celdas vecinas) como optimización extra: reduciría la colisión de O(N²) a O(N) en promedio y sería paralelizable sin condiciones de carrera por diseño.
- **Medir en varias máquinas** antes de sacar conclusiones sobre el número óptimo de hilos, ya que el número de núcleos físicos disponibles varía por equipo.

---

## Referencias

1. Chapman, B., Jost, G., & Van Der Pas, R. (2008). *Using OpenMP: Portable Shared Memory Parallel Programming*. MIT Press. — Base teórica de las directivas OpenMP usadas (`parallel for`, `critical`, `schedule`).

2. OpenMP Architecture Review Board. (2021). *OpenMP Application Programming Interface, Version 5.2*. Recuperado de https://www.openmp.org/specifications/ — Especificación oficial de referencia para las directivas de sincronización y distribución de trabajo.

3. Foster, I. (1995). *Designing and Building Parallel Programs*. Addison-Wesley. Versión en línea: http://www.mcs.anl.gov/dbpp/ — Fuente del método PCAM (Partición, Comunicación, Aglomeración, Mapeo) aplicado en la Fase 2 del proyecto.
