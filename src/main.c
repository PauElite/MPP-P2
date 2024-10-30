#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <mpi.h>

#include "../include/io.h"

extern double aplicar_mh(const double *, int, int, int, int, double, int *);

static double mseconds() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec * 1000 + t.tv_usec / 1000;
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv); // Inicializar entorno MPI

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Check Number of Input Args
    if (argc < 5) {
        if (rank == 0) {
            fprintf(stderr, "Ayuda:\n");
            fprintf(stderr, "  mpirun -np <n_procesos> ./programa n m nGen tamPob mRate\n");
        }
        MPI_Finalize();
        return (EXIT_FAILURE);
    }

    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    int n_gen = atoi(argv[3]);
    int tam_pob = atoi(argv[4]);
    double m_rate = atof(argv[5]);

    // Check that 'm' is less than 'n'
    assert(m < n);

    // Generate matrix D with distance values among elements
    double *d = read_distances(n);

    // Allocate memory for output data
    int *sol = (int *)malloc(m * sizeof(int));

    #ifdef TIME
        MPI_Barrier(MPI_COMM_WORLD);
        double ti = mseconds();
    #endif

    // Call Metaheuristic
    double value = aplicar_mh(d, n, m, n_gen, tam_pob / size, m_rate, sol);

    // Gather results at root process
    double global_value;
    MPI_Reduce(&value, &global_value, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        #ifdef TIME
            double tf = mseconds();
            printf("Execution Time: %.2lf sec\n", (tf - ti) / 1000);
        #endif
        printf("Best Fitness Value: %.2lf\n", global_value);
    }

    // Free Allocated Memory
    free(sol);
    free(d);

    MPI_Finalize(); // Finalizar entorno MPI

    return (EXIT_SUCCESS);
}
