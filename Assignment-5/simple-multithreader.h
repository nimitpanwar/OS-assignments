#include <iostream>
#include <list>
#include <functional>
#include <stdlib.h>
#include <cstring>
#include <errno.h>

int user_main(int argc, char **argv);

void demonstration(std::function<void()> && lambda) {
  lambda();
}

// Structure to hold arguments for each thread
struct thread_args1 {
  int low;
  int high;
  std::function<void(int)> lambdafunc;
  thread_args1(int low, int high, std::function<void(int)> lambda) : low(low), high(high), lambdafunc(lambda) {}
};

// Function to be called by each thread
void* vec_thread_func(void* arg){
  thread_args1* curr_arg = (thread_args1*)(arg);
  // Execute the lambda function for the range of integers
  for (int j = curr_arg->low; j < curr_arg->high; j++) {
      curr_arg->lambdafunc(j);
  }
  delete curr_arg; // Free the memory allocated for thread arguments
  return NULL;
}


// Function to execute a lambda function in parallel for a range of integers
void parallel_for(int low, int high, std::function<void(int)> &&lambda, int numThreads){
    // Get the start time
    struct timespec startTime, endTime;
    if (clock_gettime(CLOCK_MONOTONIC, &startTime) != 0) {
        perror("Failed to get start time");
    return;
  }

  // Calculate the chunk size for each thread
  int chunk = (high-low)/(numThreads);
  pthread_t tid[numThreads];

  // Creating Threads
  for (int i = 0; i < numThreads; i++) {
    // Allocate memory for thread arguments
    thread_args1* to_pass = new thread_args1((i*(high-low)/(numThreads))+low,((i+1)*(high-low)/(numThreads))+low,lambda);
    if (pthread_create(&tid[i], NULL, vec_thread_func, (void*)to_pass) != 0) {        //Create a new thread
      perror("Failed to create thread");
      delete to_pass;
      return;
    }
  }

  // Wait for all threads to finish
  for (int i = 0; i < numThreads; i++) {
    if (pthread_join(tid[i], NULL) != 0) {
      perror("Failed to join thread");
      return;
    }
  }

  // Get the end time for calculation of execution time
  if (clock_gettime(CLOCK_MONOTONIC, &endTime) != 0) {
    perror("Failed to get end time");
    return;
  }
  // Calculate and print the execution time
  double seconds = (endTime.tv_sec - startTime.tv_sec) + (endTime.tv_nsec - startTime.tv_nsec) / 1e9;
  printf("Execution Time: %f seconds\n", seconds);
  
} 

// Structure to hold arguments for each thread
struct thread_args2 {
  int low1;
  int high1;
  int low2;
  int high2;
  std::function<void(int,int)> lambdafunc;
  thread_args2(int low1, int high1, int low2, int high2, std::function<void(int,int)> lambdafn) : low1(low1), high1(high1), low2(low2), high2(high2), lambdafunc(lambdafn) {}
};


// Function to be called by each thread
void* matrix_thread_func(void* arg){
  thread_args2* curr_arg = (thread_args2*)(arg);
  // Execute the lambda function for the two ranges of integers
  for(int j=curr_arg->low1;j<curr_arg->high1;j++){
    for(int k=curr_arg->low2;k<curr_arg->high2;k++){
      curr_arg->lambdafunc(j,k);
    }
  }
  delete curr_arg; // Free the memory allocated for thread arguments
  return NULL;
}

//Similar to the above function, but for two ranges of integers.
void parallel_for(int low1, int high1, int low2, int high2,std::function<void(int, int)> &&lambda, int numThreads){

// Get the start time
struct timespec startTime, endTime;
  if (clock_gettime(CLOCK_MONOTONIC, &startTime) != 0) {
    perror("Failed to get start time");
    return;
  }

  pthread_t tid[numThreads]; // Array to hold thread IDs

  // Create threads
  for(int i=0;i<numThreads;i++){
    // Allocate memory for thread arguments
    thread_args2* to_pass = new thread_args2(i * (high1-low1) / numThreads , (i + 1) * (high1-low1) / numThreads ,low2,high2,lambda);
    // Create a new thread
    if (pthread_create(&tid[i], NULL,matrix_thread_func,(void*)to_pass) != 0) {
      perror("Failed to create thread");
      delete to_pass;
      return;
    }
  }
  // Wait for all threads to finish
  for (int i = 0; i < numThreads; i++) {
    if (pthread_join(tid[i], NULL) != 0) {
      perror("Failed to join thread");
      return;
    }
  } 

  if (clock_gettime(CLOCK_MONOTONIC, &endTime) != 0) {
    perror("Failed to get end time");
    return;
  }
  // Calculate and print the execution time
  double seconds = (endTime.tv_sec - startTime.tv_sec) + (endTime.tv_nsec - startTime.tv_nsec) / 1e9;
  printf("Execution Time: %f seconds\n", seconds);
}

int main(int argc, char **argv) {
  int x=5,y=1;
  auto lambda1 = [x, &y](void) {
    y = 5;
    std::cout<<"====== Welcome to Assignment-"<<y<<" of the CSE231(A) ======\n";
  };

  demonstration(lambda1); 
  auto lambda2 = []() {
    std::cout<<"====== Hope you enjoyed CSE231(A) ======\n";
  };

  int rc = user_main(argc, argv);
  demonstration(lambda2);
  return rc;
}

#define main user_main
