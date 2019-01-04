#include <am.h>
#include <klib.h>

#define read_csr(reg) ({ unsigned long __tmp; \
    asm volatile ("csrr %0, " #reg : "=r"(__tmp)); \
    __tmp; })

#define write_csr(reg, val) ({ \
    asm volatile ("csrw " #reg ", %0" :: "rK"(val)); })

// trace buffer
// these four control registers are r/w
#define mtracebufferenable 0x7d0
#define mtracebufferhead 0x7d1
#define mtracebufferwindowsize 0x7d2
#define mtracebufferindex 0x7d3
#define mtracebuffertrigger 0x7d4

// stack trace buffer
#define mstackbufferenable 0x7d5
#define mstackbufferhead 0x7d6
#define mstackbuffertrigger 0x7d7
#define mstacktargetindex 0x7d8
#define mstackcallpcindex 0x7d9
#define mstackarg0index 0x7da
#define mstackarg1index 0x7db
#define mstackarg2index 0x7dc

// trace buffer entries are read only
#define mtracebuffercontent 0xfc0
// stack buffer entries are read only
#define mstacktargetcontent 0xfc1
#define mstackcallpccontent 0xfc2
#define mstackarg0content 0xfc3
#define mstackarg1content 0xfc4
#define mstackarg2content 0xfc5

// xlen == 64, uses long long
void csr_write(int csr, long long value) {
  switch (csr) {
    case mtracebufferenable: write_csr(0x7d0, value); break;
    case mtracebufferhead: write_csr(0x7d1, value); break;
    case mtracebufferwindowsize: write_csr(0x7d2, value); break;
    case mtracebufferindex: write_csr(0x7d3, value); break;
    case mtracebuffertrigger: write_csr(0x7d4, value); break;
    case mstackbufferenable: write_csr(0x7d5, value); break;
    case mstackbufferhead: write_csr(0x7d6, value); break;
    case mstackbuffertrigger: write_csr(0x7d7, value); break;
    case mstacktargetindex: write_csr(0x7d8, value); break;
    case mstackcallpcindex: write_csr(0x7d9, value); break;
    case mstackarg0index: write_csr(0x7da, value); break;
    case mstackarg1index: write_csr(0x7db, value); break;
    case mstackarg2index: write_csr(0x7dc, value); break;
    default: printf("invalid csr: %x\n", csr); break;
  }
}

long long csr_read(int csr) {
  long long ret = 0;
  switch (csr) {
    case mtracebufferenable: ret = read_csr(0x7d0); break;
    case mtracebufferhead: ret = read_csr(0x7d1); break;
    case mtracebufferwindowsize: ret = read_csr(0x7d2); break;
    case mtracebufferindex: ret = read_csr(0x7d3); break;
    case mtracebuffertrigger: ret = read_csr(0x7d4); break;
    case mstackbufferenable: ret = read_csr(0x7d5); break;
    case mstackbufferhead: ret = read_csr(0x7d6); break;
    case mstackbuffertrigger: ret = read_csr(0x7d7); break;
    case mstacktargetindex: ret = read_csr(0x7d8); break;
    case mstackcallpcindex: ret = read_csr(0x7d9); break;
    case mstackarg0index: ret = read_csr(0x7da); break;
    case mstackarg1index: ret = read_csr(0x7db); break;
    case mstackarg2index: ret = read_csr(0x7dc); break;
    case mtracebuffercontent: ret = read_csr(0xfc0); break;
    case mstacktargetcontent: ret = read_csr(0xfc1); break;
    case mstackcallpccontent: ret = read_csr(0xfc2); break;
    case mstackarg0content: ret = read_csr(0xfc3); break;
    case mstackarg1content: ret = read_csr(0xfc4); break;
    case mstackarg2content: ret = read_csr(0xfc5); break;
    default: break;
  }
  return ret;
}

int fib(volatile int a) {
  if (a <= 2)
    return 1;
  else
    return fib(a - 1) + fib(a - 2);
}

// for now, klib does not support printf("%llx");
// can anyone solve this?
// I'm tired of handwriting a print_llx everytime
void print_llx(long long value) {
  char* hex_map[] = {
    "0", "1", "2", "3", "4", "5", "6", "7",
    "8", "9", "a", "b", "c", "d", "e", "f"
  };
  char *p = (char *)&value;
  for (int i = 7; i >= 0; i--)
    printf("%s%s", hex_map[(p[i] >> 4) & 0xf], hex_map[p[i] & 0xf]);
}

// ********** instruction trace tests **********
void clear_trace(void) {
  csr_write(mtracebuffertrigger, 0);
  csr_write(mtracebufferenable, 0);
  csr_write(mtracebufferhead, 0);
  csr_write(mtracebufferwindowsize, 0);
}

void start_trace(void) {
  csr_write(mtracebufferenable, 1);
}

void stop_trace(void) {
  csr_write(mtracebufferenable, 0);
}

void dump_trace(void) {
  int tracebuffer_head = csr_read(mtracebufferhead);
  printf("tracebuffer head: %d\n", tracebuffer_head);
  for (int i = 0; i < tracebuffer_head; i++) {
    csr_write(mtracebufferindex, i);
    long long content = csr_read(mtracebuffercontent);
    print_llx(content);
    printf("\n");
  }
}

// window set to 0, flowthrough mode
// in this mode, we capture every instruction that retires
// if the tracebuffer overflows, we just wrap around
int trace_flowthrough_test() {
  // do not use trigger
  clear_trace();
  start_trace();
  int ret = fib(4);
  stop_trace();
  dump_trace();

  // use trigger
  clear_trace();
  csr_write(mtracebuffertrigger, (long long)fib);
  ret = fib(4);
  stop_trace();
  dump_trace();
  return ret;
}

// window set to 20
// in this mode, we capture the first n(n is the window size you set)
// instructions after trace is enabled
int trace_window_test() {
  // do not use trigger
  clear_trace();
  csr_write(mtracebufferwindowsize, 20);
  start_trace();
  int ret = fib(4);
  stop_trace();
  dump_trace();

  // use trigger
  clear_trace();
  csr_write(mtracebufferwindowsize, 20);
  csr_write(mtracebuffertrigger, (long long)fib);
  ret = fib(4);
  stop_trace();
  dump_trace();
  return ret;
}

// ********** call/return trace tests **********
void clear_stack(void) {
  csr_write(mstackbuffertrigger, 0);
  csr_write(mstackbufferenable, 0);
  csr_write(mstackbufferhead, 0);
}

void start_stack(void) {
  csr_write(mstackbufferenable, 1);
}

void stop_stack(void) {
  csr_write(mstackbufferenable, 0);
}

void dump_stack(void) {
  int stackbuffer_head = csr_read(mstackbufferhead);
  printf("stackbuffer:\n");
  for (int i = 0; i < stackbuffer_head; i++) {
    csr_write(mstacktargetindex, i);
    long long target = csr_read(mstacktargetcontent);

    csr_write(mstackcallpcindex, i);
    long long callpc = csr_read(mstackcallpccontent);

    csr_write(mstackarg0index, i);
    long long arg0 = csr_read(mstackarg0content);

    csr_write(mstackarg1index, i);
    long long arg1 = csr_read(mstackarg1content);

    csr_write(mstackarg2index, i);
    long long arg2 = csr_read(mstackarg2content);

    printf("callpc: ");
    print_llx(callpc);
    printf(" ");
    printf("target: ");
    print_llx(target);
    printf(" ");
    printf("arg0: %d arg1: %d arg2: %d\n", arg0, arg1, arg2);
  }
}

int f(volatile int a, volatile int b, volatile int c) {
  if (a == 1) {
    dump_stack();
    return 1;
  }
  else
    return f(a - 1, b - 1, c - 1) + 1;
}

int stack_test() {
  // do not use trigger
  clear_stack();
  start_stack();
  int ret = f(10, 9, 8);

  // before next run
  // do not clear stack and see whether the stack pops when function returns
  // use trigger
  csr_write(mstackbuffertrigger, (long long)f);
  ret = f(10, 9, 8);
  stop_stack();
  return ret;
}

int main() {
  // test instruction trace buffer
  trace_flowthrough_test();
  trace_window_test();

  // test call/return trace
  stack_test();
  return 0;
}
