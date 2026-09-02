// Screensaver, version PARALELA con OpenMP.
//
// PENDIENTE - Fase 2 del plan (ver PLAN.md). Esta version se arma copiando
// src/secuencial/main.cpp y src/secuencial/physics.cpp a esta carpeta y
// paralelizando unicamente physics.cpp:
//
//   1. Copiar los dos archivos tal cual y comprobar que compila y corre igual
//      que la secuencial (todavia sin ninguna directiva de OpenMP).
//   2. Aplicar PCAM sobre stepSimulation():
//      - integrate()         : un elemento por iteracion, sin dependencias
//                              entre ellas. Es el reparto directo.
//      - resolveCollisions() : cada par escribe en DOS elementos, asi que un
//                              reparto ingenuo tiene carrera de datos. Aqui va
//                              el mecanismo de proteccion de memoria compartida.
//      - buildLinks()        : solo lee posiciones, pero todos los hilos
//                              escriben en el mismo vector de salida.
//   3. Reemplazar el cronometro por omp_get_wtime() y reportar los hilos reales
//      con omp_get_max_threads(), respetando la bandera -t.
//
// Las banderas y la validacion NO se copian: parseArgs vive en
// src/common/args.cpp y lo enlazan las dos versiones, para que sea imposible
// que se desincronicen y las mediciones dejen de comparar lo mismo.

#include <cstdio>

int main() {
    std::fprintf(stderr,
                 "La version paralela todavia no esta implementada.\n"
                 "Corresponde a la Fase 2 del plan; por ahora use bin/screensaver_seq.\n");
    return 1;
}
