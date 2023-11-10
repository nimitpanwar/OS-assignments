#include <iostream>
#include <list>
#include <functional>
#include <stdlib.h>
#include <cstring>


int user_main(int argc, char **argv);

void demonstration(std::function<void()> && lambda) {
  lambda();
}


int count=0;

void parallel_for(int low, int high, std::function<void(int)> &&lambda, int numThreads){

  struct timespec startTime, endTime;
  clock_gettime(CLOCK_MONOTONIC, &startTime);


  int chunk = (high-low)/(numThreads);
  pthread_t tid[numThreads];

  struct thread_args {
    int low;
    int high;
    std::function<void(int)> lambdafunc;
    thread_args(int low, int high, std::function<void(int)> lambda) : low(low), high(high), lambdafunc(lambda) {}
  };

  for (int i = 0; i < numThreads; i++) {
    thread_args* to_pass = new thread_args((i*chunk),((i+1)*chunk),lambda);
    pthread_create(&tid[i], NULL, [](void* arg) -> void* {
        thread_args* curr_arg = (thread_args*)(arg);
        for (int j = curr_arg->low; j < curr_arg->high; j++) {
            curr_arg->lambdafunc(j);
        }
        return NULL;
        delete curr_arg;
    }, (void*)to_pass);
  }

  for (int i = 0; i < numThreads; i++) {
    pthread_join(tid[i], NULL);
  }

  clock_gettime(CLOCK_MONOTONIC, &endTime);

  double seconds = (endTime.tv_sec - startTime.tv_sec) + (endTime.tv_nsec - startTime.tv_nsec) / 1e9;

  printf("Execution Time: %f seconds\n", seconds);
  
} 



void parallel_for(int low1, int high1, int low2, int high2,std::function<void(int, int)> &&lambda, int numThreads){

  struct timespec startTime, endTime;
  clock_gettime(CLOCK_MONOTONIC, &startTime);


  pthread_t tid[numThreads];

  struct thread_args2 {
    int low1;
    int high1;
    int low2;
    int high2;
    std::function<void(int,int)> lambdafunc;
    thread_args2(int low1, int high1, int low2, int high2, std::function<void(int,int)> lambdafn) : low1(low1), high1(high1), low2(low2), high2(high2), lambdafunc(lambdafn) {}
  };

  int chunk1 = (high1- low1)/numThreads;
  int chunk2=  (high2- low2)/numThreads;

  for(int i=0;i<numThreads;i++){
    thread_args2* to_pass = new thread_args2((i*chunk1),(i+1)*chunk1,low2,high2,lambda);
    pthread_create(&tid[i], NULL, [](void* arg) -> void*{ 
      thread_args2* curr_arg = (thread_args2*)(arg);
      for(int j=curr_arg->low1;j<curr_arg->high1;j++){
        for(int k=curr_arg->low2;k<curr_arg->high2;k++){
          curr_arg->lambdafunc(j,k);
        }
      }
      delete curr_arg;
      return NULL;
    },(void*)to_pass);
  }
  
  for (int i = 0; i < numThreads; i++) {
    pthread_join(tid[i], NULL);
  } 

  clock_gettime(CLOCK_MONOTONIC, &endTime);

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


