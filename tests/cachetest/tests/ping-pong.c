#include "klib.h"

volatile int ball = 0;
int main() {
  int cpu = _cpu();
  if (cpu) {
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
