#include <am.h>
#include "riscv64.h"

static unsigned long boot_time = 0;
#define HZ 100000000

static inline uint64_t gettime(void) {
  return read_csr(mcycle) / (HZ / 1000);
}

void _ioe_init() {
  boot_time = gettime();
}

// -------------------- cycles and uptime --------------------

unsigned long _uptime(){
  return gettime() - boot_time;
}

// -------------------- video --------------------


_Screen _screen = {
  .width  = 0,
  .height = 0,
};

void _draw_p(int x, int y, uint32_t p) {
}

void _draw_sync() {
}

// -------------------- keyboard --------------------

static inline int upevent(int e) { return e; }
static inline int downevent(int e) { return e | 0x8000; }

int _read_key(){
  int uart_recv();
  int key = uart_recv();
  if (key == -1) {
    return _KEY_NONE;
  }
  return key;
}

