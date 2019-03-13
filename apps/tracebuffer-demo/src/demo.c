#include "demo.h"

// for now, klib does not support printf("%llx");
// can anyone solve this?
// I'm tired of handwriting a print_llx everytime
void print_llx(long long value) {
  const char hex_map[] = "0123456789abcdef";
  char *p = (char *)&value;
  for (int i = 7; i >= 0; i--) {
    printf("%c%c", hex_map[(p[i] >> 4) & 0xf], hex_map[p[i] & 0xf]);
  }
}

void trace_flowthrough_test();
void init_stack_test();
void stack_test();

#define CSR_MEPC 0x341
#define CSR_BCAUSE 0xfcd

// exception handler
extern void register_exception_handler();
extern void trace_exception_handler(void);
extern void stack_exception_handler(void);
void my_exception_handler() {
  long long bcause = read_csr(CSR_BCAUSE);
  if (bcause != 0) {
    trace_exception_handler();
    stack_exception_handler();
  } else {
    // normal exception
    printf("illegal instruction!\n");
    // advance the mepc by 2byte to skip over the illegal instruction
    // so we keep rolling
    long long mepc = read_csr(CSR_MEPC);
    write_csr(CSR_MEPC, mepc + 4);
  }
}

int main() {
  register_exception_handler();
  init_stack_test();

  // test instruction trace buffer
  trace_flowthrough_test();

  // test stack trace
  stack_test();
  return 0;
}
