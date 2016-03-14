
#include <stdlib.h>
#include <stdio.h>

#include "mylib.h"


/************** Part 1: Thread Control and Management ****************/


/* Creates an empty ThreadFactory object. */
int mylib_ThreadFactory_create(mylib_ThreadFactory *tfactory)
{
  *tfactory = (mylib_ThreadFactory)malloc(sizeof(mylib_ThreadFactory_internal));
}

/* Destroys a ThreadFactory object. */
int mylib_ThreadFactory_destroy(mylib_ThreadFactory tfactory)
{
  free(tfactory);
}


/* Factory function for creating an empty ThreadControl object. */
int mylib_ThreadFactory_create_control(mylib_ThreadFactory tfactory, mylib_ThreadControl *tcontrol)
{
  mylib_ThreadControl new_tcontrol = (mylib_ThreadControl)malloc(sizeof(mylib_ThreadControl_internal));

  /* Fill with default parameters */
  new_tcontrol->tid   = 0;
  new_tcontrol->tsize = 0;
  new_tcontrol->shared_context = tfactory;

  *tcontrol = new_tcontrol;
}

/* Factory function for creating an empty ThreadControl object. */
int mylib_ThreadFactory_destroy_control(mylib_ThreadFactory tfactory, mylib_ThreadControl tcontrol)
{
  free(tcontrol);
}


/* Allocates a shared buffer for all threads in tcontrol. */
int mylib_ThreadControl_malloc(mylib_ThreadControl tcontrol, int num_bytes, void **ptr)
{
  mylib_ThreadControl_sync(tcontrol);

  if (tcontrol->tid == 0)
    tcontrol->shared_context->shared_data = malloc(num_bytes);

  mylib_ThreadControl_sync(tcontrol);

  *ptr = tcontrol->shared_context->shared_data;
}

/* Frees a shared buffer allocated for all threads in tcontrol .*/
int mylib_ThreadControl_free(mylib_ThreadControl tcontrol, void *ptr)
{
  mylib_ThreadControl_sync(tcontrol);

  if (tcontrol->tid == 0)
    free(ptr);
}

/* Synchronizes all threads in tcontrol (i.e. no thread proceeds before all threads have reached this point) */
int mylib_ThreadControl_sync(mylib_ThreadControl tcontrol)
{
  tcontrol->shared_context->sync(tcontrol->tid, tcontrol->tsize, tcontrol->shared_context->sync_data);
}

/************** Part 2: Worker routines ****************/



/* Compute the sum of two vectors v1 and v2, store result in vector vresult. All vectors of length vsize. */
int mylib_vector_add(mylib_ThreadControl tcontrol, double *v1, double *v2, double *vresult, int vsize)
{
  /* Compute indices to split work equally over threads */
  int i;
  int elements_per_thread = (vsize - 1) / tcontrol->tsize + 1;
  int begin_index = tcontrol->tid * elements_per_thread;
  int end_index   = (tcontrol->tid + 1) * elements_per_thread;

  if (end_index > vsize)
    end_index = vsize;

  /* Do the work */
  for (i = begin_index; i < end_index; ++i)
    vresult[i] = v1[i] + v2[i];
}

/* Compute the dot product of two vectors v1 and v2, store result in dotresult. v1 and v2 of length vsize. */
int mylib_vector_dot(mylib_ThreadControl tcontrol, double *v1, double *v2, double *dotresult, int vsize)
{
  /* Prepare thread-local data structures */
  double *thread_results = NULL;

  mylib_ThreadControl_malloc(tcontrol, tcontrol->tsize * sizeof(double), (void**)&thread_results);

  thread_results[tcontrol->tid] = 0;

  /* Compute indices to split work equally over threads */
  int i;
  int elements_per_thread = (vsize - 1) / tcontrol->tsize + 1;
  int begin_index = tcontrol->tid * elements_per_thread;
  int end_index   = (tcontrol->tid + 1) * elements_per_thread;

  if (end_index > vsize)
    end_index = vsize;

  /* Compute partial results for each thread */
  for (i = begin_index; i < end_index; ++i)
    thread_results[tcontrol->tid] += v1[i] * v2[i];

  mylib_ThreadControl_sync(tcontrol);

  /* Use first thread to sum up intermediate results */
  if (tcontrol->tid == 0)
  {
    for (i = 1; i < tcontrol->tsize; ++i)
      thread_results[0] += thread_results[i];

    *dotresult = thread_results[0];
  }

  /* Sync again to make sure 'dotresult' is valid whenever any of the threads returns from the function */
  mylib_ThreadControl_sync(tcontrol);

  mylib_ThreadControl_free(tcontrol, thread_results);

}


