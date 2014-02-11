#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>

# define MAX_THREADS         64
void *compute_pi   ( void *);

int total_hits, total_misses,
    sample_points, sample_points_per_thread, num_threads;
struct arg_to_thread {int t_seed ; int hits ;} ;

main (int argc, char *argv[]  )    {
  /* two arguments should be supplied - the number of points and the number of threads */
  if(argc != 3) {printf("wrong number of arguments") ; exit(2) ;};
  int i;
  pthread_t p_threads[MAX_THREADS];
  pthread_attr_t  attr;
  double computed_pi;
  double time_start, time_end;
  struct timeval tv;
  struct timezone tz;
  struct arg_to_thread my_arg[MAX_THREADS] ;

  pthread_attr_init (&attr);
  pthread_attr_setscope  (&attr, PTHREAD_SCOPE_SYSTEM);
  sample_points = atoi(argv[1]) ;
  num_threads = atoi(argv[2]) ;
  printf("# of points = %d and # of thread = %d\n ",sample_points, num_threads);

       gettimeofday (&tv ,   &tz);
       time_start = (double)tv.tv_sec +
                 (double)tv.tv_usec / 1000000.0;

       total_hits =0;
       sample_points_per_thread = sample_points /num_threads;
       for (i=0; i< num_threads; i++  ){
        my_arg[i].t_seed = i;
        pthread_create (&p_threads[i], &attr, compute_pi,
          (void*) &my_arg[i]);
       }
       for (i=0; i< num_threads; i++) {
        pthread_join (p_threads[i], NULL);
        total_hits += my_arg[i].hits;

       }
       computed_pi = 4.0*(double) total_hits /
        ((double) (sample_points));
       gettimeofday (&tv ,  &tz);
       time_end = (double)tv.tv_sec +
               (double)tv.tv_usec / 1000000.0;

       printf ("Computed PI = %lf\n", computed_pi);
       printf ("%lf\n", time_end - time_start);
}

void  *compute_pi (void *s) {
  struct arg_to_thread *local_arg ;
  int seed, i ;
  double rand_no_x, rand_no_y;
  int local_hits;
  local_arg = s;

  seed=  (*local_arg).t_seed;
  local_hits =0;
  for (i=0; i<sample_points_per_thread; i++) {
    rand_no_x = (double) (rand_r (&seed))/(double) RAND_MAX ;
    rand_no_y = (double) (rand_r (&seed))/(double) RAND_MAX ;
    if (((rand_no_x - 0.5) *(rand_no_x - 0.5) +
      (rand_no_y - 0.5) * (rand_no_y - 0.5)) <0.25)
    local_hits ++;
    seed *=i;
  }

  (*local_arg).hits = local_hits;
  pthread_exit (0);
}

