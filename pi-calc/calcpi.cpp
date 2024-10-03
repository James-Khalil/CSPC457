// ======================================================================
// You must modify this file and then submit it for grading to D2L.
// ======================================================================
//
// count_pixels() counts the number of pixels that fall into a circle
// using the algorithm explained here:
//
// https://en.wikipedia.org/wiki/Approximations_of_%CF%80
//
// count_pixels() takes 2 paramters:
//  r         =  the radius of the circle
//  n_threads =  the number of threads you should create
//
// Currently the function ignores the n_threads parameter. Your job is to
// parallelize the function so that it uses n_threads threads to do
// the computation.

#include "calcpi.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include<thread>

//The thread structure. 
struct thread {
    pthread_t id;
    int start = 0;
    int end = 0;
    int radius = 0;
    uint64_t pixel_count = 0;
};

// Similar function to single threading version provided, tweaked for changes by
// multithreading
void* thread_function(void * args)
{
  struct thread * tm = (struct thread *) args;
  double radius = tm->radius;
  double rsq = radius * radius;
  uint64_t count = 0;
  for(double x = tm->start; x <= tm->end ; x++){
    for(double y = 0 ; y <= radius ; y ++){
      if( x*x + y*y <= rsq) count ++;
    }
  }
  tm->pixel_count = count;
  pthread_exit(NULL);
}


uint64_t count_pixels(int r, int n_threads)
{
  thread thread_array[n_threads]; 

  // Divides the radius counting into equal chunks and runs each thread 
  for(int i = 0; i < n_threads; i++){
    thread_array[i].radius = r;

    if(i==0){
      thread_array[i].start = 1;
      thread_array[i].end = (r/n_threads);
    }
    else{
      thread_array[i].start = thread_array[i-1].end + 1;
      thread_array[i].end = thread_array[i].start + (r/n_threads);
    }
    pthread_create(&thread_array[i].id, NULL,thread_function, &thread_array[i]);
  }
  uint64_t count = 0;
  // Sums all threads
  for(int i=0; i< n_threads; i++){
    pthread_join(thread_array[i].id, NULL);
    count = count + thread_array[i].pixel_count;
  }
  return count * 4 + 1;
}
