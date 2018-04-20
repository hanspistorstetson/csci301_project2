#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    FILE* file = NULL;
    int* data = NULL;
    int i = 0;
    int j = 0;
    int swap;
    int n;
    clock_t begin = clock();
    file = fopen(argv[1], "r");
    fscanf(file, "%d", &n);

    data = (int*)malloc(n * sizeof(int));
    for (i = 0; i < n; i++) {
        fscanf(file, "%d", &(data[i]));
    }
    fclose(file);

    for (i = 0; i < n-1; i++) {
        for (j = 0; j < n - i - 1; j++) {
            if (data[j] > data[j+1]) {
                swap = data[j];
                data[j] = data[j+1];
                data[j+1] = swap;
            }
        }
    }

    file = fopen(argv[2], "w");
    fprintf(file, "%d\n", n);
    for (i = 0; i < n; i++) {
        fprintf(file, "%d\n", data[i]);
    }
    fclose(file);
    clock_t end = clock();
    double time = (double)(end-begin) / CLOCKS_PER_SEC;
    printf("Time spent: %f seconds\n", time);


    return 0;
}
