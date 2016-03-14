/**
* Example C code for demonstrating the use of OpenMP in user code
* to interface into a simple linear algebra library.
*
* This example illustrates how a possible library interface
* with support for different threading methodologies can look like.
* In particular, 'mylib' is not restricted to either
* pthread, C++11 threads, or OpenMP, but can be used with either of the three
* (and possibly other threading approaches).*
* Author: Karl Rupp
*
* License: MIT/X11 license (see file LICENSE.txt)
*/

#include <omp.h>
#include <stdlib.h>
#include <stdio.h>

#include "mylib.h"

/* Routine for OpenMP synchronization */
void openmp_sync(int tid, int tsize, void *data)
{
 #pragma omp barrier
}

/* Helper routine for inserting OpenMP thread identification into ThreadControl object.
   Also works if compiled without OpenMP (i.e. single-threaded execution). */
void threads_init(mylib_ThreadFactory tfactory, mylib_ThreadControl *tcontrol)
{
  mylib_ThreadFactory_create_control(tfactory, tcontrol);

#ifdef _OPENMP
  (*tcontrol)->tid   = omp_get_thread_num();
  (*tcontrol)->tsize = omp_get_num_threads();
#else
  (*tcontrol)->tid   = 0;
  (*tcontrol)->tsize = 1;
#endif
}

/** Main program. Here is the actual usage of mylib with OpenMP shown. */
int main(int argc, char **argv)
{
  int i, N = 10;
  double *v1, *v2, *v3;
  mylib_ThreadFactory tfactory;

  /* Create global thread manager and register OpenMP synchronization routine. */
  mylib_ThreadFactory_create(&tfactory);
  tfactory->sync = openmp_sync;

  /* Create vectors with data. */
  v1 = malloc(sizeof(double) * N);
  v2 = malloc(sizeof(double) * N);
  v3 = malloc(sizeof(double) * N);

  /* Set entries in v1 and v2. */
  #pragma omp parallel for
  for (i=0; i<N; ++i)
  {
    v1[i] = (double)i;
    v2[i] = (double)(N - i);
  }

  
  /*
   *  First operation: Add entries
   *  Spawns a parallel region, then passes thread identification (id, thread count) to one thread control object per thread.
   */
  #pragma omp parallel
  {
    mylib_ThreadControl tcontrol;
    threads_init(tfactory, &tcontrol);

    mylib_vector_add(tcontrol, v1, v2, v3, N);

    mylib_ThreadFactory_destroy_control(tfactory, tcontrol);
  }

  printf("Result of vector addition: ");
  for (i = 0; i<N; ++i)
    printf("%g ", v3[i]);
  printf("\n");

  /*
   *  Second operation: Compute dot product
   *  Control flow as before.
   */
  #pragma omp parallel
  {
    mylib_ThreadControl tcontrol;
    threads_init(tfactory, &tcontrol);

    mylib_vector_dot(tcontrol, v1, v2, v3, N);

    mylib_ThreadFactory_destroy_control(tfactory, tcontrol);
  }

  printf("Result of dot product: %g\n", v3[0]);

  /* Free buffers */
  free(v1);
  free(v2);
  free(v3);

  mylib_ThreadFactory_destroy(tfactory);

  return EXIT_SUCCESS;
}

