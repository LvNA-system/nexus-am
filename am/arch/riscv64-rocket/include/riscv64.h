#ifndef __RISCV64_H__
#define __RISCV64_H__

#define MAX_HARTS 2

#define read_csr(reg) ({ unsigned long __tmp; \
    asm volatile ("csrr %0, " #reg : "=r"(__tmp)); \
    __tmp; })

#define write_csr(reg, val) ({ \
    asm volatile ("csrw " #reg ", %0" :: "rK"(val)); })


static inline void enable_prefetch() {
  asm volatile ("csrwi 0x801, 0x1");
}

static inline void disable_prefetch() {
  asm volatile ("csrwi 0x801, 0x0");
}

static inline uint64_t read_cycle() {
  return read_csr(mcycle);
}


#endif
