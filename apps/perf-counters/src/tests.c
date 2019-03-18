#include <am.h>
#include <klib.h>

// use this to test nLoad, nStore,
// nBranch and nBranchMisprediction perf counters
void loop(int n) {
  // use volatile to avoid being optimized out
  volatile int a;
  for (int i = 0; i < n; i++)
    for (int j = 0; j < 1024; j++)
      a += 1;
}

#define K  * 1024
#define M * (1024 * 1024)
#define SIZE (3 M)

volatile char array[SIZE]; 

// use this to test nLoad, nStore, nDCacheMiss,
// nBranch and nBranchMisprediction perf counters
void cache_flush(int n) {
  int step = SIZE / 1024;
  for (int i = 0; i < n; i++)
    for (int j = 0; j < 1024; j++)
      array[(j * step) % SIZE] += 1;
}

int direction[] = {
  1 ,  0 ,  1 ,  1 ,  0 ,  0 ,  1 ,  1 ,  0 ,  1 ,  0 ,  0 ,  0 ,  1 ,  1 ,  0 ,  1 ,  0 ,  0 ,  0 ,  0 ,  0 ,  1 ,  0 ,  1 ,  1 ,  1 ,  1 ,  1 ,  1 ,  0 ,  0 ,  0 ,  0 ,  1 ,  1 ,  0 ,  1 ,  0 ,  1 ,  1 ,  0 ,  0 ,  0 ,  1 ,  1 ,  0 ,  1 ,  1 ,  0 ,  1 ,  1 ,  1 ,  1 ,  1 ,  1 ,  1 ,  1 ,  1 ,  0 ,  1 ,  0 ,  0 ,  0 ,  1 ,  1 ,  0 ,  1 ,  1 ,  0 ,  1 ,  0 ,  1 ,  1 ,  0 ,  1 ,  0 ,  0 ,  1 ,  1 ,  1 ,  0 ,  0 ,  1 ,  0 ,  0 ,  1 ,  0 ,  0 ,  1 ,  0 ,  1 ,  0 ,  1 ,  0 ,  1 ,  0 ,  0 ,  1 ,  0 ,  1 ,  0 ,  0 ,  0 ,  0 ,  1 ,  0 ,  1 ,  1 ,  1 ,  1 ,  1 ,  0 ,  0 ,  1 ,  1 ,  1 ,  1 ,  0 ,  1 ,  1 ,  0 ,  0 ,  0 ,  0 ,  1 ,  1 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  1 ,  1 ,  1 ,  1 ,  0 ,  0 ,  1 ,  0 ,  1 ,  0 ,  1 ,  1 ,  0 ,  1 ,  1 ,  0 ,  0 ,  1 ,  1 ,  0 ,  0 ,  1 ,  0 ,  0 ,  0 ,  0 ,  0 ,  1 ,  1 ,  0 ,  0 ,  0 ,  0 ,  1 ,  1 ,  1 ,  1 ,  0 ,  0 ,  1 ,  1 ,  0 ,  0 ,  0 ,  1 ,  1 ,  1 ,  0 ,  0 ,  1 ,  0 ,  1 ,  0 ,  0 ,  1 ,  0 ,  0 ,  1 ,  1 ,  0 ,  0 ,  1 ,  0 ,  0 ,  1 ,  0 ,  1 ,  1 ,  0 ,  1 ,  0 ,  0 ,  1 ,  0 ,  1 ,  1 ,  1 ,  1 ,  0 ,  0 ,  1 ,  0 ,  1 ,  1 ,  0 ,  0 ,  1 ,  0 ,  1 ,  0 ,  1 ,  1 ,  0 ,  1 ,  0 ,  0 ,  1 ,  1 ,  1 ,  1 ,  1 ,  1 ,  1 ,  0 ,  0 ,  1 ,  0 ,  0 ,  0 ,  1 ,  1 ,  1 ,  1 ,  0 ,  0 ,  1 ,  1 ,  0 ,  0 ,  1 ,  0 ,  0};

#define NBranches sizeof(direction) / sizeof(int)

// use this to test nLoad, nStore,
// nBranch and nBranchMisprediction perf counters
void random_branch(int n) {
  volatile int a, b;
  for (int i = 0; i < n; i++)
    for (int j = 0; j < NBranches; j++)
      if (direction[j])
        a++;
      else
        b++;
}
