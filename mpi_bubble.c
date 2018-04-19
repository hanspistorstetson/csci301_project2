#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void swap(int* v, int i, int j) {
    int t = v[i];
    v[i] = v[j];
    v[j] = t;
}

void bubblesort(int* v, int n) {
    int i, j;
    for (int i = n-2; i >= 0; i--) {
        for (j = 0; j <= i; j++) {
            if (v[j] > v[j+1]) {
                swap(v, j, j+1);
            }
        }
    }
}

int* merge(int* v1, int n1, int* v2, int n2) {
    int* result = (int*) malloc((n1+n2) * sizeof(int));
    int i = 0;
    int j = 0;
    int k;

    for (k = 0; k < n1 + n2; k++) {
        if (i >= n1) {
            result[k] = v2[j];
            j++;
        } else if (j >= n2) {
            result[k] = v1[i];
            i++;
        } else {
            result[k] = v2[j];
            j++;
        }
    }
    return result;
}

int main (int argc, char** argv) {
    int n, c, s, o, p, id, i, step;
    int* data = NULL;
    int* chunk;
    int* other;
    MPI_Status status;
    double elapsed_time;
    FILE* file = NULL;

    if (argc != 3) {
        fprintf(stderr, "Usage: mpirun -np <num_procs> %s <in_file> <out_file>\n", argv[0]);
        exit(1);
    }
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    if (id == 0) {
        file = fopen(argv[1], "r");
        fscanf(file, "%d", &n);
        c = n/p;
        if (n%p) {
            c++;
        }
        data = (int *) malloc(p*c * sizeof(int));
        for (i = 0; i < n ; i++) {
            fscanf(file, "%d", &(data[i]));
        }
        fclose(file);
        for (i = n; i < p*c; i++) {
            data[i] = 0;
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    elapsed_time = MPI_Wtime();

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    c = n/p;
    if (n%p) {
        c++;
    }
    
    chunk = (int*) malloc(c * sizeof(int));
    MPI_Scatter(data, c, MPI_INT, chunk, c, MPI_INT, 0, MPI_COMM_WORLD);
    free(data);
    data = NULL;

    s = (n >= c * (id + 1)) ? c : n - c * id;
    bubblesort(chunk, s);

    for (step = 1; step < p; step = step * 2) {
        if (id % (2*step) != 0) {
            MPI_Send(chunk, s, MPI_INT, id - step, 0, MPI_COMM_WORLD);
            break;
        }
        if (id + step < p) {
            o = (n >= c * (id+ 2 * step)) ? c * step : n - c * (id + step);

            other = (int*) malloc(o * sizeof(int));
            MPI_Recv(other, o, MPI_INT, id + step, 0, MPI_COMM_WORLD, &status);
            data = merge(chunk, s, other, o);
            free(chunk);
            free(other);
            chunk = data;
            s = s + o;
        }
    }

    elapsed_time += MPI_Wtime();

    if (id == 0) {
        file = fopen(argv[2], "w");
        fprintf(file, "%d\n", s);
        for (int i = 0; i < s; i++) {
            fprintf(file, "%d\n", chunk[i]);
        }
        fclose(file);
        printf("Bubblesort %d ints on %d procs: %f secs\n", n, p, elapsed_time);
    }
    MPI_Finalize();
    return 0;
}

