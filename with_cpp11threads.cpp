/**
* Example C code for demonstrating the use of C++11 threads in user code
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

#include <thread>
#include <condition_variable>
#include <vector>
#include <iostream>

#include "mylib.h"


/* Implementation of a C++11 barrier. wait() can be called multiple times.
 * See e.g. http://stackoverflow.com/questions/24465533/implementing-boostbarrier-in-c11 for alternatives.
 *
 * This class will most likely be hidden inside 'mylib' if 'mylib' were a mature library and not just for demonstration purposes.
 */
class Barrier
{
public:
  explicit Barrier(int num_threads) : threads_required_(num_threads), threads_left_(num_threads), counter_(0) {}

  void wait()
  {
    int ctr = counter_;
    std::unique_lock<std::mutex> lock(mutex_);

    threads_left_ -= 1;
    if (threads_left_ == 0) // all threads arrived
    {
     ++counter_;
     threads_left_ = threads_required_;
     condition_.notify_all();
    }
    else
    {
      condition_.wait(lock, [this, ctr] { return ctr != counter_; });
    }
  }

private:
  int threads_required_;
  int threads_left_;
  int counter_;
  std::mutex              mutex_;
  std::condition_variable condition_;
};



/* Callback routine for synchronization of C++11 threads. */
void cpp11thread_sync(int tid, int tsize, void *data)
{
  Barrier *barrier = reinterpret_cast<Barrier*>(data);

  barrier->wait();
}


/* Data holder passed as data argument to std::thread() 
 * Note: In principle it should be possible to pass function arguments directly to std::thread.
 * However, all my attempts failed (C++ type system frenzy), so I resort to this pthread-like approach.
 */
typedef struct
{
  mylib_ThreadControl tcontrol;
  double *v1;
  double *v2;
  double *v3;
  int N;
} ArgumentT;

/* std::thread entry point for vector addition */
void *threaded_add(void *data)
{
  ArgumentT *args = (ArgumentT *)data;
  mylib_vector_add(args->tcontrol, args->v1, args->v2, args->v3, args->N);
}

/* std::thread entry point for dot product */
void *threaded_dot(void *data)
{
  ArgumentT *args = (ArgumentT *)data;
  mylib_vector_dot(args->tcontrol, args->v1, args->v2, args->v3, args->N);
}

/** Main program. Here is the actual usage of mylib shown. */
int main(int argc, char **argv)
{
  int N = 10, num_threads = 4;
  mylib_ThreadFactory tfactory;
  Barrier thread_barrier(num_threads);
  std::vector<mylib_ThreadControl> tcontrol(num_threads);
  std::vector<std::thread> threads(num_threads);
  std::vector<ArgumentT> args(num_threads);

  /* Create global thread manager and register C++11 thread synchronization routine.
   * The following three lines could be merged into a single line by providing a convenience routine
   * mylib_ThreadFactory_create_cpp11threads(...);
   */
  mylib_ThreadFactory_create(&tfactory);
  tfactory->sync = cpp11thread_sync;
  tfactory->sync_data = (void*)&thread_barrier;

  /* Create vectors with data. Using malloc() for the sake of uniformity with the other two examples. */
  std::vector<double> v1(N);
  std::vector<double> v2(N);
  std::vector<double> v3(N);

  /* Set entries in v1 and v2 */
  for (int i=0; i<N; ++i)
  {
    v1[i] = i;
    v2[i] = N - i;
  }

  /*
   *  First operation: Add entries.
   *  Generate a per-thread thread control, wrap arguments, and launch thread.
   */
  for (int i=0; i<num_threads; ++i)
  {
    mylib_ThreadFactory_create_control(tfactory, tcontrol.data() + i);
    tcontrol[i]->tid   = i;
    tcontrol[i]->tsize = num_threads;

    /* Note: I could not find a way of passing 'mylib_vector_add' and arguments directly to std::thread(), hence this pthread-like workaround */
    args[i].tcontrol = tcontrol[i];
    args[i].v1 = v1.data();
    args[i].v2 = v2.data();
    args[i].v3 = v3.data();
    args[i].N  = N;
    threads[i] = std::thread(threaded_add, (void*)&args[i]);
  }

  /** Wait for threads to complete. Clean up thread control object. */
  for (int i=0; i<num_threads; ++i)
  {
    threads[i].join();
    mylib_ThreadFactory_destroy_control(tfactory, tcontrol[i]);
  }

  std::cout << "Result of vector addition: ";
  for (int i = 0; i<N; ++i)
    std::cout << v3[i] << " ";
  std::cout << std::endl;



  /*
   *  Second operation: Compute dot product.
   *  Same control flow as before: Create thread control object, wrap function arguments, launch thread.
   */
  for (int i=0; i<num_threads; ++i)
  {
    mylib_ThreadFactory_create_control(tfactory, tcontrol.data() + i);
    tcontrol[i]->tid   = i;
    tcontrol[i]->tsize = num_threads;

    /* Note: I could not find a way of passing 'mylib_vector_add' and arguments directly to std::thread(), hence this pthread-like workaround */
    args[i].tcontrol = tcontrol[i];
    args[i].v1 = v1.data();
    args[i].v2 = v2.data();
    args[i].v3 = v3.data();
    args[i].N  = N;
    threads[i] = std::thread(threaded_dot, (void*)&args[i]);
  }

  /** Wait for threads to complete. Clean up thread control object. */
  for (int i=0; i<num_threads; ++i)
  {
    threads[i].join();
    mylib_ThreadFactory_destroy_control(tfactory, tcontrol[i]);
  }

  std::cout << "Result of dot product: " << v3[0] << std::endl;

  /* Clean up */
  mylib_ThreadFactory_destroy(tfactory);

  return EXIT_SUCCESS;
}

