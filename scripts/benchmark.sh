#!/usr/bin/env bash
# Corre una version del screensaver N veces y guarda los tiempos en un CSV
# para la bitacora de pruebas (Anexo 3, minimo 10 mediciones por prueba).
#
# Uso: scripts/benchmark.sh <binario> <N_elementos> <repeticiones> <salida.csv>
# Ej:  scripts/benchmark.sh bin/screensaver_par 1000 10 resultados/par_1000.csv

set -euo pipefail

BIN="${1:?Falta el path al binario}"
N_ELEM="${2:?Falta la cantidad de elementos (N)}"
REPS="${3:?Falta la cantidad de repeticiones}"
OUT="${4:?Falta el archivo de salida csv}"

echo "run,tiempo_segundos" > "$OUT"

for i in $(seq 1 "$REPS"); do
    START=$(date +%s.%N)
    "$BIN" -n "$N_ELEM" > /dev/null
    END=$(date +%s.%N)
    ELAPSED=$(echo "$END - $START" | bc)
    echo "$i,$ELAPSED" >> "$OUT"
    echo "Run $i/$REPS: ${ELAPSED}s"
done

echo "Resultados guardados en $OUT"
