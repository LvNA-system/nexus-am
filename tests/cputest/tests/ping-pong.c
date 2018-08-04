#include "trap.h"

#define KB 1024
#define MB 1024 * 1024

volatile int ball = 0;
int main() {
  int mhartid = read_const_csr(mhartid);
  if (mhartid) {
    while (1) {
      while (!ball);
      printf("Pong\n");
      ball = 0;
    }
  } else {
    while (1) {
      printf("Ping\n");
      ball = 1;
      while (ball);
    }
  }
  return 0;
}
