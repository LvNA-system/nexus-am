#include <am.h>
#include <klib.h>
#include "riscv64.h"

extern char _end;
extern int main();
extern void uart_init();
extern void uart_send(uint8_t data);

_Area _heap = {
  .start = &_end,
};

void _putc(char ch) {
  uart_send(ch);
  if (ch == '\n') {
    uart_send('\r');
  }
}

void _halt(int code) {
  printf("Exited (%d)\n", code);
  uint64_t cycle = read_csr(mcycle);
  printf("cycle = {%u,%u}\n", cycle >> 32, cycle & 0xffffffff);
  while (1);
}

volatile int wait_core_0_init = 1;

void _trm_init() {
  uart_init();
  if (!read_csr(mhartid)) {
    int mem_size = 0x10000000;
    _heap.end = (void *)0x100000000 + mem_size;
    wait_core_0_init = 0;
  }
  while (read_csr(mhartid) && wait_core_0_init);
  int ret = main();
  _halt(ret);
}
