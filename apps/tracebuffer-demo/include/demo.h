#ifndef __DEMO_H__
#define __DEMO_H__

#include <am.h>
#include <riscv64.h>
#include <klib.h>

#define str(x) #x

#define set_csr(reg, val) \
    asm volatile ("csrsi %0, %1" :: "i"(reg), "rK"(val))

#define clear_csr(reg, val) \
    asm volatile ("csrci %0, %1" :: "i"(reg), "rK"(val))

#undef read_csr
#undef write_csr

#define read_csr(reg) ({ \
    long long val; \
    asm volatile ("csrr %0, %1" : "=r"(val) : "i"(reg)); \
    val; })

#define write_csr(reg, val) \
  asm volatile ("csrw %0, %1" :: "i"(reg), "rK"(val))

enum {
  // trace buffer
  // these two control registers are r/w
  mtracebufctrl = 0x7d0,
  mtracebufstat,
  mtracebuftriggerstart,
  mtracebuftriggerend,

  // stack buffer
  mstackbufctrl = 0x7d8,
  mstackbufstat,
  mstackbuftriggerstart,
  mstackbuftriggerend,
  mstackbufstackbase,
  mstackbufstackend,

  // trace buffer entries are read only
  mtracebufdatalo = 0xfc0,
  mtracebufdatahi,

  // stack buffer entries are read only
  mstackbufdata0 = 0xfc8,
  mstackbufdata1,
  mstackbufdata2,
};

#define CTRL_ENABLE_MASK 0x1
#define CTRL_EXCEPTION_MASK 0x2
#define CTRL_RSTN_MASK 0x4

typedef union {
  struct {
    uint32_t enable    : 1;
    uint32_t exception : 1;
    uint32_t rstn      : 1;
    uint32_t pad0      : 5;
    uint32_t threshold : 8;
    uint8_t  index        ;
  };
  uint64_t val;
} BufCtrl;

typedef union {
  struct {
    uint32_t overflow :1;
    uint32_t pad0     :7;
    uint8_t  size       ;
    uint8_t  head       ;
  };
  uint64_t val;
} BufStat;

void print_llx(long long value);

#endif
