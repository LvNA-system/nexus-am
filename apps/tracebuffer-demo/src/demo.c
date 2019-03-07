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

int main() {
  init_stack_test();

  // test instruction trace buffer
  trace_flowthrough_test();

  // test stack trace
  stack_test();
  return 0;
}
