#include <iostream>
#include <list>
#include <functional>
#include <stdlib.h>
#include <cstring>
#include <pthread.h>
#include <vector>
#include <chrono>

int user_main(int argc, char **argv);

void demonstration(std::function<void()> && lambda) {
  lambda();
}

#define NUM_THREADS 4

struct ThreadData {
    int low;
    int high;
    std::function<void(int)> lambda;
    ThreadData(int low, int high, std::function<void(int)> lambda) : low(low), high(high), lambda(lambda) {}
};

struct ThreadData2D {
    int low1;
    int high1;
    int low2;
    int high2;
    std::function<void(int, int)> lambda;
    ThreadData2D(int low1, int high1, int low2, int high2, std::function<void(int, int)> lambda) : low1(low1), high1(high1), low2(low2), high2(high2), lambda(lambda) {}
};

void parallel_for(int low, int high, std::function<void(int)> &&lambda, int numThreads) {
    std::vector<pthread_t> threads(numThreads);
    int range = high - low;
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numThreads; ++i) {
        pthread_create(&threads[i], NULL, [](void* arg) -> void* {
            auto data = static_cast<ThreadData*>(arg);
            for (int j = data->low; j < data->high; ++j) {
                data->lambda(j);
            }
            delete data;
            return NULL;
        }, new ThreadData{i * range / numThreads + low, (i + 1) * range / numThreads + low, lambda});
    }
    for (auto& thread : threads) {
        pthread_join(thread, NULL);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end-start;
    std::cout << "Execution time: " << diff.count() << " s\n";
}

void parallel_for(int low, int high, std::function<void(int)> &&lambda) {
    parallel_for(low, high, std::move(lambda), NUM_THREADS);
}

void parallel_for(int low1, int high1, int low2, int high2, std::function<void(int, int)> &&lambda, int numThreads) {
    std::vector<pthread_t> threads(numThreads);
    int range1 = high1 - low1;
    int range2 = high2 - low2;
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numThreads; ++i) {
        pthread_create(&threads[i], NULL, [](void* arg) -> void* {
            auto data = static_cast<ThreadData2D*>(arg);
            for (int j = data->low1; j < data->high1; ++j) {
                for (int k = data->low2; k < data->high2; ++k) {
                    data->lambda(j, k);
                }
            }
            delete data;
            return NULL;
        }, new ThreadData2D{i * range1 / numThreads + low1, (i + 1) * range1 / numThreads + low1, low2, high2, lambda});
    }
    for (auto& thread : threads) {
        pthread_join(thread, NULL);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end-start;
    std::cout << "Execution time: " << diff.count() << " s\n";
}

int main(int argc, char **argv) {
  int x=5,y=1;
  auto lambda1 = [x, &y](void) {
    y = 5;
    std::cout<<"====== Welcome to Assignment-"<<y<<" of the CSE231(A) ======\n";
  };
  demonstration(lambda1);

  int rc = user_main(argc, argv);
 
  auto lambda2 = []() {
    std::cout<<"====== Hope you enjoyed CSE231(A) ======\n";
  };
  demonstration(lambda2);
  return rc;
}

#define main user_main