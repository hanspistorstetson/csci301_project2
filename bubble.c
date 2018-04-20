
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>


void swap(int* arr, int i, int j)
{
  int t = arr[i];
  arr[i] = arr[j];
  arr[j] = t;
}


void bubblesort(int* arr, int n)
{
  int i, j;
  for (i = n-2; i >= 0; i--)
    for (j = 0; j <= i; j++)
      if (arr[j] > arr[j+1])
        swap(arr, j, j+1);
}


int* merge(int* arr1, int n1, int* arr2, int n2)
{
  int* result = (int*)malloc((n1 + n2) * sizeof(int));
  int i = 0;
  int j = 0;
  int k;
  for (k = 0; k < n1 + n2; k++) {
    if (i >= n1) {
      result[k] = arr2[j];
      j++;
    }
    else if (j >= n2) {
      result[k] = arr1[i];
      i++;
    }
    else if (arr1[i] < arr2[j]) {
      result[k] = arr1[i];
      i++;
    }
    else {
      result[k] = arr[j];
      j++;
    }
  }
  return result;
}


int main(int argc, char** argv)
{
  int n;
  int* data = NULL;
  int c, s;
  int* chunk;
  int o;
  int* other;
  int step;
  int p, id;
  MPI_Status status;
  double elapsed_time;
  FILE* file = NULL;
  int i;

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
    data = (int*)malloc(p*c * sizeof(int));
    for (i = 0; i < n; i++)
      fscanf(file, "%d", &(data[i]));
    fclose(file);
    for (i = n; i < p*c; i++)
      data[i] = 0;
  }

  MPI_Barrier(MPI_COMM_WORLD);
  elapsed_time = - MPI_Wtime();

  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

  c = n/p;
  if (n%p) {
      c++;
  }

  chunk = (int*)malloc(c * sizeof(int));
  MPI_Scatter(data, c, MPI_INT, chunk, c, MPI_INT, 0, MPI_COMM_WORLD);
  free(data);
  data = NULL;

  s = (n >= c * (id+1)) ? c : n - c * id;
  bubblesort(chunk, s);

  for (step = 1; step < p; step = 2*step) {
    // IF NOT MASTER (COMBINER) NODE
    if (id % (2*step)!=0) {
      MPI_Send(chunk, s, MPI_INT, id-step, 0, MPI_COMM_WORLD);
      break;
    }
    // IF MASTER (COMBINER) NODE
    if (id+step < p) {
      o = (n >= c * (id+2*step)) ? c * step : n - c * (id+step);
      other = (int*)malloc(o * sizeof(int));
      MPI_Recv(other, o, MPI_INT, id+step, 0, MPI_COMM_WORLD, &status);
      data = merge(chunk, s, other, o);
      free(chunk);
      free(other);
      chunk = data;
      s = s + o;
      // s is the length of the new combined array
    }
  }

  elapsed_time += MPI_Wtime();

  if (id == 0) {
    file = fopen(argv[2], "w");
    fprintf(file, "%d\n", s);
    for (i = 0; i < s; i++)
      fprintf(file, "%d\n", chunk[i]);
    fclose(file);
    printf("Bubblesorting %d numbers on %d procs took %f secs\n", n, p, elapsed_time);
  }

  MPI_Finalize();
  return 0;
}
