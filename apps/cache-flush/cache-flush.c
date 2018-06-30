#include <am.h>
#include "klib.h"

#define LOOP 100
#define STEP 64
#define K  * 1024
#define M * (1024 * 1024)
#define SIZE (3 M)

#define ARRAY ((volatile char *)(void *)0x80100000)

int main() {
  uint32_t t0 = 0, t1 = 0;
  printf("loop = %d, step = %d, size = %d\n", LOOP, STEP, SIZE);
  while (1) {
    printf("press any key to stop flushing cache\n");
    while (_read_key() == _KEY_NONE) {

      int nr_loop = LOOP;
      int i;
      t0 = _uptime();

      while (nr_loop --) {
        for (i = 0; i < SIZE; i += STEP) {
          (void)ARRAY[i];
        }
      }
      t1 = _uptime();
    }

    printf("time cost: %d ms\n", t1 - t0);
    printf("press any key to restart flushing cache\n");
    while (_read_key() == _KEY_NONE);

  }
}
