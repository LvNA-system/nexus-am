#include <am.h>
#include <klib.h>

#define KB * 1024
#define ASIZE (2 KB)

static volatile int a[ASIZE];

int main(){
  int id = _cpu();
  printf("Initializing CPU%d...\n", id);

  int i;
  srand(id);
  srand(rand());
  for (i = 0; i < ASIZE; i ++) {
    a[i] = id;
  }

  int nr_test = 1;

  while (1) {
    printf("Testing for the %dth time...\n", nr_test);
    int temp;
    for (i = 0; i < ASIZE; i ++) {
      temp = a[i];
      a[i] = temp;
      assert(a[i] == id);
    }

    volatile int j;
    int delay = rand() % 32;
    printf("Delay %d loop...\n", delay);
    for (j = 0; j < delay; j ++);

    nr_test ++;
  }
  return 0;
}
