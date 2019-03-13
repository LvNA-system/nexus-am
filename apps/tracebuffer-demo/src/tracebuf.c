#include "demo.h"

typedef union {
  struct {
    uint32_t instr;
    uint32_t exception : 1;
    uint32_t cause     : 5;
    uint32_t time_lo   :26;
    uint32_t time_hi   :24;
    uint64_t pc        :40;
  };
  struct {
    uint64_t lo;
    uint64_t hi;
  } val;
} TraceBufData;

int fib(volatile int a) {
  if (a <= 2) {
    // trigger invalid instruction exception, to see how it goes
    // this illegal instruction should not be traced
    asm volatile (".word 0x0");

    // test system call
    // this ecall instrution should be traced
    _trap();
    return 1;
  }
  else
    return fib(a - 1) + fib(a - 2);
}

void clear_trace(void) {
  write_csr(mtracebuftriggerstart, 0);
  write_csr(mtracebuftriggerend, 0);
  set_csr(mtracebufctrl, CTRL_RSTN_MASK);
  // now clear the rstn bit will form a negedge to zero stat.head
  write_csr(mtracebufctrl, 0);
  write_csr(mtracebufstat, 0);
}

void start_trace(void) {
  set_csr(mtracebufctrl, CTRL_ENABLE_MASK);
}

void stop_trace(void) {
  write_csr(mtracebuftriggerstart, 0);
  write_csr(mtracebuftriggerend, 0);
  // clear the enable bit
  clear_csr(mtracebufctrl, CTRL_ENABLE_MASK);
}

void dump_trace(void) {
  BufStat stat = {.val = read_csr(mtracebufstat)};
  BufCtrl ctrl = {.val = read_csr(mtracebufctrl)};

  int tracebuffer_head = stat.head;
  printf("tracebuf stat = %x\n", stat.val);
  printf("tracebuffer head: %d\n", tracebuffer_head);

  // if overflow, then tail = head
  int i = (stat.overflow ? tracebuffer_head : 0);

  do {
    ctrl.index = i;
    write_csr(mtracebufctrl, ctrl.val);
    TraceBufData data;
    data.val.lo = read_csr(mtracebufdatalo);
    data.val.hi = read_csr(mtracebufdatahi);
    printf("tracebuf[%3d]: time = %u, pc = ", i, (data.time_hi << 26)| data.time_lo);
    //printf("sizeof(data) = %d", sizeof(data));
    print_llx(data.pc);
    printf(" instr = %08x", data.instr);
    if (data.exception) {
      printf(" [cause = %d]", data.cause);
    }
    printf("\n");
    i = (i + 1) % (stat.size + 1);
  } while (i != tracebuffer_head);
}

void trace_exception_handler(void) {
  // check whether trace buffer throws exception?
  BufStat stat = {.val = read_csr(mtracebufstat)};
  BufCtrl ctrl = {.val = read_csr(mtracebufctrl)};

  int exception = stat.exception && ctrl.exception;
  if (!exception)
    return;

  // overflow should never happen, since we set the threshold small
  assert(!stat.overflow);

  // dump everything in the buffer
  dump_trace();

  stat.head = 0;
  stat.exception = 0;
  write_csr(mtracebufstat, stat.val);
}

static void set_threshold(int threshold) {
  BufCtrl ctrl = {.val = read_csr(mtracebufctrl)};
  ctrl.threshold = threshold;
  write_csr(mtracebufctrl, ctrl.val);
}

static void enable_exception(void) {
  BufCtrl ctrl = {.val = read_csr(mtracebufctrl)};
  ctrl.exception = 1;
  write_csr(mtracebufctrl, ctrl.val);
}

// window set to 0, flowthrough mode
// in this mode, we capture every instruction that retires
// if the tracebuffer overflows, we just wrap around
int trace_flowthrough_test() {
  // do not use trigger
  clear_trace();
  set_threshold(32);
  enable_exception();
  start_trace();
  int ret = fib(2);
  stop_trace();

  // overflow test
  //clear_trace();
  //start_trace();
  //ret = fib(8);
  //stop_trace();
  //dump_trace();

  // use trigger
  clear_trace();
  write_csr(mtracebuftriggerstart, (long long)fib);
  write_csr(mtracebuftriggerend, (long long)printf);
  //  set_csr(mtracebufctrl, CTRL_TRIGGER_MASK);
  ret = fib(3);
  ret = fib(3);
  stop_trace();
  dump_trace();

  write_csr(mtracebuftriggerstart, (long long)fib);
  write_csr(mtracebuftriggerend, (long long)printf);
  //  set_csr(mtracebufctrl, CTRL_TRIGGER_MASK);
  ret = fib(3);
  ret = fib(3);
  stop_trace();
  dump_trace();

  return ret;
}
