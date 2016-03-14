/**
* Example C code for demonstrating the use of pthread in user code
* to interface into a simple linear algebra library.
*
* This example illustrates how a possible library interface
* with support for different threading methodologies can look like.
* In particular, 'mylib' is not restricted to either
* pthread, C++11 threads, or OpenMP, but can be used with either of the three
* (and possibly other threading approaches).
*
*
* Author: Karl Rupp
*
* License: MIT/X11 license (see file LICENSE.txt)
*/

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#include "mylib.h"

/* Helper struct for holding pthread synchronization data.
 * Data objects are passed to the generic sync() method in mylib,
 * providing a synchronization primitive similar to an OpenMP barrier.
 */
typedef struct 
{
  pthread_barrier_t *barrier;
  pthread_mutex_t   *mutex;
  unsigned char status_counter;
} PthreadBarrierData;


/* Callback routine for pthread synchronization, registered in mylib.
   This routine implements a thread barrier similar to an OpenMP barrier. */
void pthread_sync(int tid, int tsize, void *data)
{
  PthreadBarrierData *barrier_data = (PthreadBarrierData *)data;
  pthread_barrier_t *my_barrier = barrier_data->barrier;
  pthread_mutex_t   *my_mutex   = barrier_data->mutex;
  unsigned char status  = barrier_data->status_counter;

  pthread_barrier_wait(my_barrier);

  /* Reset barrier for next use */
  pthread_mutex_lock(my_mutex);
  if (barrier_data->status_counter == status)
  {
    pthread_barrier_destroy(my_barrier);
    pthread_barrier_init(my_barrier, NULL, tsize);
    barrier_data->barrier = my_barrier;
    barrier_data->status_counter += 1;
  }
  pthread_mutex_unlock(my_mutex);
}

/* Data holder passed as data argument to pthread_create() */
typedef struct
{
  mylib_ThreadControl tcontrol;
  double *v1;
  double *v2;
  double *v3;
  int N;
} ArgumentT;

/* pthread entry point */
void *threaded_add(void *data)
{
  ArgumentT *args = (ArgumentT *)data;
  mylib_vector_add(args->tcontrol, args->v1, args->v2, args->v3, args->N);
}

/* pthread entry point */
void *threaded_dot(void *data)
{
  ArgumentT *args = (ArgumentT *)data;
  mylib_vector_dot(args->tcontrol, args->v1, args->v2, args->v3, args->N);
}


/** Main program. Here is the actual usage of mylib shown. */
int main(int argc, char **argv)
{
  int i, N = 10, num_threads = 4;
  double *v1, *v2, *v3;
  mylib_ThreadFactory tfactory;
  mylib_ThreadControl *tcontrol;
  ArgumentT *args;
  pthread_t *threads;
  /* The following variables could be hidden inside mylib if needed. */
  pthread_barrier_t   barrier;
  pthread_mutex_t     mutex;
  PthreadBarrierData  barrier_data;

  /* Create global thread manager and register synchronization callback together with the required barrier and mutex.
   * The following code block could be wrapped inside a singe mylib_ThreadFactory_create_pthread(...) routine.
   */
  mylib_ThreadFactory_create(&tfactory);
  tfactory->sync = pthread_sync;
  pthread_barrier_init(&barrier, NULL, num_threads);
  pthread_mutex_init(&mutex, NULL);
  barrier_data.barrier = &barrier;
  barrier_data.mutex   = &mutex;
  barrier_data.status_counter = 1;
  tfactory->sync_data = &barrier_data;

  /* Prepare per-thread context information */
  threads  = (pthread_t *)          malloc(num_threads * sizeof(pthread_t));
  tcontrol = (mylib_ThreadControl *)malloc(num_threads * sizeof(mylib_ThreadControl));
  args     = (ArgumentT *)          malloc(num_threads * sizeof(ArgumentT));

  /* Create vectors with data. */
  v1 = malloc(sizeof(double) * N);
  v2 = malloc(sizeof(double) * N);
  v3 = malloc(sizeof(double) * N);

  /* Set entries in v1 and v2 */
  for (i=0; i<N; ++i)
  {
    v1[i] = (double)i;
    v2[i] = (double)(N - i);
  }
  
  /*
   *  First operation: Add entries. 
   *  Set up the thread control object for each thread and then call the entry point threaded_add().
   *  threaded_add is required because pthread only allows to call into functions taking one void pointer as argument.
   */
  for (i=0; i<num_threads; ++i)
  {
    mylib_ThreadFactory_create_control(tfactory, tcontrol + i);
    tcontrol[i]->tid   = i;
    tcontrol[i]->tsize = num_threads;

    args[i].tcontrol = tcontrol[i];
    args[i].v1 = v1;
    args[i].v2 = v2;
    args[i].v3 = v3;
    args[i].N  = N;
    pthread_create(threads + i, NULL, threaded_add, (void*)&args[i]);
  }

  /* Wait for all threads to finish and clean up associated thread control objects */
  for (i=0; i<num_threads; ++i)
  {
    pthread_join(threads[i], NULL);
    mylib_ThreadFactory_destroy_control(tfactory, tcontrol[i]);
  }

  printf("Result of vector addition: ");
  for (i = 0; i<N; ++i)
    printf("%g ", v3[i]);
  printf("\n");

  /*
   *  Second operation: Compute dot product
   *  Code flow as before: Create thread control object, register thread ID and thread count, pass arguments to entry point 'threaded_dot'.
   */
  for (i=0; i<num_threads; ++i)
  {
    mylib_ThreadFactory_create_control(tfactory, tcontrol + i);
    tcontrol[i]->tid   = i;
    tcontrol[i]->tsize = num_threads;

    args[i].tcontrol = tcontrol[i];
    args[i].v1 = v1;
    args[i].v2 = v2;
    args[i].v3 = v3;
    args[i].N  = N;
    pthread_create(threads + i, NULL, threaded_dot, (void*)&args[i]);
  }

  /* Wait for all threads to finish and clean up associated thread control objects */
  for (i=0; i<num_threads; ++i)
  {
    pthread_join(threads[i], NULL);
    mylib_ThreadFactory_destroy_control(tfactory, tcontrol[i]);
  }

  printf("Result of dot product: %g\n", v3[0]);

  /* Tidy up */
  free(v1);
  free(v2);
  free(v3);
  free(threads);
  free(tcontrol);
  free(args);

  pthread_barrier_destroy(&barrier);
  pthread_mutex_destroy(&mutex);

  mylib_ThreadFactory_destroy(tfactory);

  return EXIT_SUCCESS;
}

