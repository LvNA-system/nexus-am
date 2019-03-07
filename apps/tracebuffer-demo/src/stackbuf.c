#include "demo.h"

typedef union {
  struct {
    uint64_t data;
    uint64_t addr   :40;
    uint32_t timeLo24 :24;
    uint32_t timeHi16 :16;
    uint32_t type   : 3;
    uint32_t cmd    : 5;
    uint64_t pc     :40;
  };
  uint64_t val[3];
} __attribute__((packed)) StackBufData;

void clear_stack(void) {
  write_csr(mstackbuftriggerstart, 0);
  write_csr(mstackbuftriggerend, 0);
  set_csr(mstackbufctrl, CTRL_RSTN_MASK);
  // now clear the rstn bit will form a negedge to zero stat.head
  write_csr(mstackbufctrl, 0);
  write_csr(mstackbufstat, 0);
}

void start_stack(void) {
  set_csr(mstackbufctrl, CTRL_ENABLE_MASK);
}

void stop_stack(void) {
  write_csr(mstackbuftriggerstart, 0);
  write_csr(mstackbuftriggerend, 0);
  // clear the enable bit
  clear_csr(mstackbufctrl, CTRL_ENABLE_MASK);
}

void dump_stack(void) {
  BufStat stat = {.val = read_csr(mstackbufstat)};
  BufCtrl ctrl = {.val = read_csr(mstackbufctrl)};

  int stackbuffer_head = stat.head;
  printf("stackbuf stat = %x\n", stat.val);
  printf("stackbuffer head: %d\n", stackbuffer_head);

  // if overflow, then tail = head
  int i = (stat.overflow ? stackbuffer_head : 0);

  do {
    ctrl.index = i;
    write_csr(mstackbufctrl, ctrl.val);
    StackBufData buf;
    buf.val[0] = read_csr(mstackbufdata0);
    buf.val[1] = read_csr(mstackbufdata1);
    buf.val[2] = read_csr(mstackbufdata2);

    uint64_t time = (buf.timeHi16 << 16) | buf.timeLo24;
    uint32_t cmd_char = (buf.cmd & 0x1 ? 'S' : 'L');
    if (((buf.cmd >> 3) & 0x3) == 0x1) {
      // AMO
      cmd_char = 'A';
    }
    const char* mTypeName[] = { "B", "H", "W", "D", "BU", "HU", "WU", "???" };

    printf("stackbuf[%3d]: %d", i, (uint32_t) time);
    //print_llx(time);
    printf(": pc = 0x");
    print_llx(buf.pc);
    printf(" ");
    printf("addr: 0x");
    print_llx(buf.addr);
    printf(" ");
    printf("[%c(%d) %s] data: 0x", cmd_char, buf.cmd, mTypeName[buf.type]);
    print_llx(buf.data);
    printf("\n");

    i = (i + 1) % (stat.size + 1);
  } while (i != stackbuffer_head);
}

volatile int g_dummy = 0;
int f(volatile int a, volatile int b, volatile int c) {
  if (a == 1) {
    g_dummy = b;
    return 1;
  }
  else {
    g_dummy = c;
    return f(a - 1, b - 1, c - 1) + 1;
  }
}

void stack_test() {
  // do not use trigger
  clear_stack();
  start_stack();
  f(2, 1, 0);
  stop_stack();
  dump_stack();

  // use trigger
  clear_stack();
  write_csr(mstackbuftriggerstart, (long long)f);
  write_csr(mstackbuftriggerend, (long long)printf);
//  set_csr(mstackbufctrl, CTRL_TRIGGER_MASK);
  f(2, 1, 0);
  printf("%d: should stop\n", __LINE__);
  f(2, 1, 0);
  stop_stack();
  dump_stack();

  clear_stack();
  start_stack();
  f(2, 1, 0);
  printf("%d: should stop\n", __LINE__);
  f(2, 1, 0);
  stop_stack();
  dump_stack();
}

void init_stack_test(void) {
  extern char stack, stack_end;
  write_csr(mstackbufstackbase, &stack);
  write_csr(mstackbufstackend, &stack_end);
  //write_csr(mstackbufstackbase, 0);
  //write_csr(mstackbufstackend, ~0);

  assert(sizeof(StackBufData) == 192 / 8);
}
