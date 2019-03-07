#include <am.h>
#include <klib.h>
#include "riscv64.h"

void irq_handle()
{
  uint64_t mcause = read_csr(mcause);
  uint64_t mepc = read_csr(mepc);
  switch (mcause) {
    case 11:
      // ecall for testing, just return
      write_csr(mepc, mepc + 4);
      return;
    case 16:
      // tracebuf or stackbuf majority exception
      return;
  }
  extern void uart_init();
  // reinitialize uart, make sure it's in correct state
  // so that we can print
  uart_init();
  printf("machine mode: unhandlable trap %d @ epc = %x\n", mcause, mepc);
  _halt(1);
}

void _trap() {
  asm volatile("ecall");
}
