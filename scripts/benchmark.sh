#!/usr/bin/env bash
# Corre una version del screensaver varias veces y junta los tiempos en un CSV
# para la bitacora de pruebas (Anexo 3, minimo 10 mediciones por prueba).
#
# Se apoya en dos cosas del programa:
#   -frames  hace que la corrida termine sola despues de N cuadros, asi que
#            todas las repeticiones hacen exactamente el mismo trabajo.
#   -seed    fija el estado inicial, asi que la secuencial y la paralela
#            recorren la misma trayectoria y los tiempos son comparables.
#
# El programa imprime al salir una linea que empieza con "CSV," y trae los
# tiempos ya medidos por dentro. Se usa esa linea en vez de cronometrar el
# proceso desde afuera, porque asi no se cuenta el arranque de SDL ni el cierre
# de la ventana, que no tienen nada que ver con lo que se esta paralelizando.
#
# Uso: scripts/benchmark.sh <binario> <N> <cuadros> <repeticiones> <salida.csv> [hilos]
# Ej:  scripts/benchmark.sh bin/screensaver_seq 3000 300 10 resultados/seq_3000.csv
#      scripts/benchmark.sh bin/screensaver_par 3000 300 10 resultados/par_3000_8.csv 8

set -euo pipefail

BIN="${1:?Falta el path al binario}"
N_ELEM="${2:?Falta la cantidad de elementos (N)}"
FRAMES="${3:?Falta la cantidad de cuadros por corrida}"
REPS="${4:?Falta la cantidad de repeticiones}"
OUT="${5:?Falta el archivo de salida csv}"
THREADS="${6:-0}"

# Semilla fija: todas las corridas y las dos versiones arrancan igual.
SEED=42

if [ ! -x "$BIN" ]; then
    echo "No existe o no es ejecutable: $BIN" >&2
    exit 1
fi

mkdir -p "$(dirname "$OUT")"
echo "repeticion,version,n,ancho,alto,hilos,cuadros,total_s,simulacion_s,dibujo_s" > "$OUT"

for i in $(seq 1 "$REPS"); do
    # -nohud y -noclock quitan de la medicion el texto sobreimpreso, que no es
    # parte de lo que se esta comparando.
    LINE=$("$BIN" -n "$N_ELEM" -frames "$FRAMES" -seed "$SEED" -t "$THREADS" -nohud -noclock \
           | grep '^CSV,' || true)

    if [ -z "$LINE" ]; then
        echo "La corrida $i no devolvio linea CSV; se aborta." >&2
        exit 1
    fi

    # Se reemplaza el prefijo "CSV" por el numero de repeticion.
    echo "$i,${LINE#CSV,}" >> "$OUT"
    echo "Corrida $i/$REPS: $LINE"
done

echo
echo "Resultados guardados en $OUT"
