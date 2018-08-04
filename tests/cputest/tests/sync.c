#include "trap.h"

#define KB 1024
#define MB 1024 * 1024

volatile int arr[8] = {0};
int main() {
  int mhartid = read_const_csr(mhartid);
  int steps = 100;
  for (int i = 0; i < steps; i++) {
    arr[mhartid] += mhartid;
  }
  nemu_assert(arr[mhartid] == mhartid * steps);
  return 0;
}
