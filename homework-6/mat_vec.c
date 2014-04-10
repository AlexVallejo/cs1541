/****************************************************************************
 *  Matrix vector multiplication using N threads
 *****************************************************************************/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

int N, num_threads;
float** a;
float* b;
float* x;

void mm(int id) {
  int i, j, k;
  double sum;

  // compute bounds for this threads---just algebra
  int startrow = id * N / num_threads;
  int endrow = (id + 1) * (N / num_threads) - 1;

  for (i = startrow; i < endrow; i++)
    for (j = 0; j < N; j++)
      x[i] += a[i][j] * b[j];
}

void *worker(void *arg){
  int id = *((int *) arg);
  mm(id);
  return 0;
}

int main(int argc, char *argv[] ){
  // the array size and number of execution threads should be supplied as a
  // command line argument
  if (argc != 3){
    printf("wrong number of arguments");
    exit(2);
  }

  pthread_t *threads;
  N = atoi(argv[1]);
  num_threads = atoi(argv[2]);
  printf("Array size = %d \n ", N );
  int mid = (N+1)/2;
  int i, j;
  int *p;
  double time_start, time_end;
  struct timeval tv;
  struct timezone tz;

  // allocate arrays dynamically
  a = malloc(sizeof(float*)*N);
  for (i = 0; i < N; i++) {
    a[i] = malloc(sizeof(float)*N);
  }

  b = malloc(sizeof(float)*N);
  x = malloc(sizeof(float)*N);

  // Inititialize matrix A and vector B
  for (i=0; i<N; i++) {
    for (j=0; j<N; j++) {
      if (j == i)
        a[i][j] = 2.0;

      else if (j == i-1 || j == i+1)
        a[i][j] = 1.0;

      else
        a[i][j] = 0.01;
    }
    b[i] = mid - abs(i-mid+1);
  }

  gettimeofday(&tv, &tz);
  time_start = (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;

  for (i=0; i<N; i++) 
    x[i] = 0.0;

  // Allocate thread handles
  threads = (pthread_t *) malloc(num_threads * sizeof(pthread_t));

  // Create threads
  for (i = 0; i < num_threads; i++) {
    p = (int *) malloc(sizeof(int));  // yes, memory leak, don't worry for now
    *p = i;
    pthread_create(&threads[i], NULL, worker, (void *)(p));
  }

  for (i = 0; i < num_threads; i++) {
    pthread_join(threads[i], NULL);
  }

  gettimeofday(&tv, &tz);
  time_end = (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;

  // this is for checking the results
  for (i=0; i<N; i+=N/10)
    printf(" %10.6f",x[i]);
  
  printf("\ntime = %lf\n", time_end - time_start);
}
