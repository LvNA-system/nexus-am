#include <am.h>
#include <klib.h>
#include "riscv64-rocket.h"

#define read_csr(reg) ({ unsigned long __tmp; \
    asm volatile ("csrr %0, " #reg : "=r"(__tmp)); \
    __tmp; })

void bad_trap()
{
  extern void uart_init();
  // reinitialize uart, make sure it's in correct state
  // so that we can print
  uart_init();
  printf("machine mode: unhandlable trap %d @ epc = %lx\n", read_csr(mcause), read_csr(mepc));
  _halt(1);
}

