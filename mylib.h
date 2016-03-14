

/************** Part 1: Thread Control and Management ****************/

/* Thread factory struct. In a real-world implementation this struct should not be exposed publicly, but provided as an opaque pointer. */
typedef struct
{
  /* function pointers for synchronization, etc. */
  void (*sync)(int tid, int size, void *data);                        /* thread synchronization function */
  void *sync_data;                                 /* Optional user-provided auxiliary data passed to sync */

  void *shared_data; /* pointer for exchanging data across threads */

  /* A full-fledged implementation requires a bunch of other callbacks.
   * For illustration purposes, however, we will only consider a sync() method here. */
} mylib_ThreadFactory_internal, *mylib_ThreadFactory;




/* Thread control struct. In a real-world implementation this struct is probably not exposed publicly, but provided as an opaque pointer.
 * Data members would then be manipulated via external functions, e.g. mylib_ThreadControl_set_thread_size()
 */
typedef struct
{
  /* thread layout information */
  int tid;              /* thread ID */
  int tsize;            /* total number of threads */

  mylib_ThreadFactory shared_context;

} mylib_ThreadControl_internal, *mylib_ThreadControl;

#ifdef __cplusplus
extern "C" {
#endif

/* Creates an empty ThreadFactory object. */
int mylib_ThreadFactory_create(mylib_ThreadFactory *tfactory);

/* Destroys a ThreadFactory object. */
int mylib_ThreadFactory_destroy(mylib_ThreadFactory tfactory);

/* Factory function for creating an empty ThreadControl object. */
int mylib_ThreadFactory_create_control(mylib_ThreadFactory tfactory, mylib_ThreadControl *tcontrol);

/* Factory function for creating an empty ThreadControl object. */
int mylib_ThreadFactory_destroy_control(mylib_ThreadFactory tfactory, mylib_ThreadControl tcontrol);

/* Allocates a shared buffer for all threads in tcontrol. */
int mylib_ThreadControl_malloc(mylib_ThreadControl tcontrol, int num_bytes, void **ptr);

/* Frees a shared buffer allocated for all threads in tcontrol .*/
int mylib_ThreadControl_free(mylib_ThreadControl tcontrol, void *ptr);

/* Synchronizes all threads in tcontrol (i.e. no thread proceeds before all threads have reached this point) */
int mylib_ThreadControl_sync(mylib_ThreadControl tcontrol);


/************** Part 2: Worker routines ****************/

/* Compute the sum of two vectors v1 and v2, store result in vector vresult. All vectors of length vsize. */
int mylib_vector_add(mylib_ThreadControl tcontrol, double *v1, double *v2, double *vresult, int vsize);

/* Compute the dot product of two vectors v1 and v2, store result in dotresult. v1 and v2 of length vsize. */
int mylib_vector_dot(mylib_ThreadControl tcontrol, double *v1, double *v2, double *dotresult, int vsize);

#ifdef __cplusplus
}
#endif
