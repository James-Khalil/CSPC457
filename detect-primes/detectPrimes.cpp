/// ============================================================================
/// Copyright (C) 2022 Pavol Federl (pfederl@ucalgary.ca)
/// All Rights Reserved. Do not distribute this file.
/// ============================================================================
///
/// You must modify this file and then submit it for grading to D2L.
///
/// You can delete all contents of this file and start from scratch if
/// you wish, as long as you implement the detect_primes() function as
/// defined in "detectPrimes.h".

#include "detectPrimes.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include<thread>


//The thread structure. Uses a similar structure to pi calc, by dividing the prime into equal chunks for each thread to handle
// the main difference is that we need a cancellation in case one thread finds the number to be not prime, which requires an atomic
struct thread {
    pthread_t id;
    int64_t start = 0;
    int64_t end = 0;
    int64_t number = 0;
    bool prime;
};

std::atomic<bool> isPrime;

// Altered code to account for multithreading changes
static bool is_prime(int64_t n, int64_t start, int64_t end)
{
  // handle trivial cases
  // technically could've been handled earlier to avoid all threads doing this test but arguably pretty trivial time loss so not important
  if (n < 2) {
    isPrime = false; 
    return false;}
  if (n <= 3) return true; // 2 and 3 are primes
  if (n % 2 == 0) {
    isPrime = false; 
    return false;} // handle multiples of 2
  if (n % 3 == 0) {
    isPrime = false; 
    return false;} // handle multiples of 3
  if (start >= n-2) return true; // case for when number of threads exceed number
  // try to divide n by every number 5 .. sqrt(n)
  //Slight change just to ensure that we are starting from a multiple of 6 + 5, so that we properly detect primes
  // this means we do some of the closer numbers several times over, but at most only 1 number should overlap between threads
  start = start - 5;
  while(start % 6 != 0){
    start--;
  }
  start = start + 5;
  for (int64_t i = start; i <= end; i+=6) {
    if (!isPrime) {
    return false;} // End it immediately, as some thread has decided this is not a prime
    if (n % i == 0) {
    isPrime = false; 
    return false;}
    if (n % (i + 2) == 0) {
    isPrime = false; 
    return false;}
  }
  // didn't find any divisors, so it must be a prime
  return true;
}

void* thread_function(void * args)
{ 
  struct thread * tm = (struct thread *) args;
  tm->prime = is_prime(tm->number,tm->start,tm->end);
  pthread_exit(NULL);
}


std::vector<int64_t>
detect_primes(const std::vector<int64_t> & nums, int n_threads)
{
  thread thread_array[n_threads]; 
  std::vector<int64_t> result;
  for (auto num : nums) {
    isPrime = true; // This should only ever be flipped true here
    //Parellize the inner loop
    for(int i = 0; i < n_threads; i++){
      thread_array[i].number = num;
      //For the first thread, we want our starting value of 5
      if(i==0){
        thread_array[i].start = 5; 
        thread_array[i].end = thread_array[i].start + (sqrt(num)/n_threads); 
      } 
      //For every thread except the first
      else{
        thread_array[i].start = thread_array[i-1].end+1; 
        thread_array[i].end = thread_array[i].start + (sqrt(num)/n_threads); 
      }
      //After setting each thread with the intial values, it runs while the next thread is set up.
      pthread_create(&thread_array[i].id, NULL,thread_function, &thread_array[i]);
    } 
    for(int i=0; i< n_threads; i++){
      pthread_join(thread_array[i].id, NULL);
    }
    if(isPrime) result.push_back(num);
  }
  return result;
}