# Cálculo de Speedup y Eficiencia — N=1000, 300 cuadros, semilla 42

## Datos brutos (tiempo de simulación en segundos)

| Repetición | Secuencial (1 hilo) | Paralelo (8 hilos) |
|---|---|---|
| 1 | 0.281509 | 0.263713 |
| 2 | 0.280372 | 0.271342 |
| 3 | 0.280916 | 0.263895 |
| 4 | 0.283781 | 0.266064 |
| 5 | 0.282319 | 0.257250 |
| 6 | 0.279597 | 0.263890 |
| 7 | 0.280980 | 0.273723 |
| 8 | 0.279935 | 0.263567 |
| 9 | 0.282446 | 0.275202 |
| 10 | 0.279466 | 0.270746 |

## Promedios

- **Promedio secuencial (T_s):** 0.281132 s
- **Promedio paralelo (T_p, 8 hilos):** 0.266939 s

## Speedup y Eficiencia

| Métrica | Valor |
|---|---|
| **Speedup S = T_s / T_p** | 0.281132 / 0.266939 = **1.053** |
| **Eficiencia E = S / p** | 1.053 / 8 = **0.132 (13.2%)** |

## Interpretación

Con N=1000 el speedup es de apenas 1.05x. La eficiencia del 13.2% confirma que
con este tamaño de problema, el tiempo de sincronización entre hilos (overhead de
OpenMP y las secciones críticas en `collide`) consume la mayor parte del tiempo
ganado. El render sigue siendo el cuello de botella real: ~5.4 s de los ~5.75 s
totales, es decir el 94% del tiempo total, no está paralelizado.

**Nota:** Con N=3000 se espera un speedup mayor sobre el tiempo de simulación
pura, ya que O(N²) crece más rápido que el overhead de coordinación.
