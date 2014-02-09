/****************************************************************************
 *  Matrix vector multiplication using one thread
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

int N ;
float** a ;
float* b ;
float* x ;

main (int argc, char *argv[] )    {
/* the array size should be supplied as a command line argument */
  if(argc != 2) {printf("wrong number of arguments") ; exit(2) ;}
  N = atoi(argv[1]) ;
  printf("Array size = %d \n ", N );
  int mid = (N+1)/2;
  int i, j;
  double time_start, time_end;
  struct timeval tv;
  struct timezone tz;

/* allocate arrays dynamically */
  a = malloc(sizeof(float*)*N);
  for (i = 0; i < N; i++) {
		a[i] = malloc(sizeof(float)*N);
  }
  b = malloc(sizeof(float)*N);
  x = malloc(sizeof(float)*N);

  /* Inititialize matrix A and vector B. */
  for (i=0; i<N; i++) {
    for (j=0; j<N; j++) {
      if (j == i)                    { a[i][j] = 2.0; }
      else if (j == i-1 || j == i+1) { a[i][j] = 1.0; }
      else                           { a[i][j] = 0.01; }
    }
    b[i] = mid - abs(i-mid+1);
  }

  gettimeofday (&tv ,   &tz);
  time_start = (double)tv.tv_sec +
            (double)tv.tv_usec / 1000000.0;

    for (i=0; i<N; i++) x[i] = 0.0;
    for (i=0; i<N; i++) {
      for (j=0; j<N; j++) {
         x[i] += a[i][j] * b[j];
      }
    }

  gettimeofday (&tv ,  &tz);
  time_end = (double)tv.tv_sec +
          (double)tv.tv_usec / 1000000.0;

/* this is for checking the results */
  for (i=0; i<N; i+=N/10) { printf(" %10.6f",x[i]); }
  printf("\n");
  printf ("time = %lf\n", time_end - time_start);

}
