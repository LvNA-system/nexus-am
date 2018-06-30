#include <am.h>
#include "klib.h"

#define K  * 1024
#define M * (1024 * 1024)
#define SIZE (64 K)

#define READ_MODE

uint32_t scan_read() {
  volatile uint32_t *A = _heap.start;
  int i;

  uint32_t cycle = read_cycle();
  for (i = 0; i < SIZE / sizeof(uint32_t); i ++) {
    (void)A[i];
  }
  return read_cycle() - cycle;
}

int main() {
  printf("Array size = %d\n", SIZE);
#ifdef READ_MODE
  printf("Read mode\n");

  uint32_t cycle[2];

  disable_prefetch();
  cycle[0] = scan_read();
  enable_prefetch();
  cycle[1] = scan_read();

  printf("prefetch disable cycle =\t%d\n", cycle[0]);
  printf("prefetch enable cycle =\t%d\n", cycle[1]);
#else
  uint32_t addr_lo = ((uintptr_t)A & 0xffffffffu);
  printf("Initializing array at 0x1%08x...\n", addr_lo);

  for (i = 0; i < SIZE / sizeof(uint32_t); i ++) {
    A[i] = i;
  }
  printf("Checking...\n");
  for (i = 0; i < SIZE / sizeof(uint32_t); i ++) {
    assert(A[i] == i);
  }
#endif

  printf("Pass!\n");
  return 0;
}
