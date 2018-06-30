#ifndef __ARCH_H__
#define __ARCH_H__

struct _RegSet {
};

#ifdef __cplusplus
extern "C" {
#endif

#define read_const_csr(reg) ({ unsigned long __tmp; \
    asm ("csrr %0, " #reg : "=r"(__tmp)); \
    __tmp; })

static inline void enable_prefetch() {
  asm volatile ("csrwi 0x801, 0x1");
}

static inline void disable_prefetch() {
  asm volatile ("csrwi 0x801, 0x0");
}

static inline uint64_t read_cycle() {
  return read_const_csr(mcycle);
}

#ifdef __cplusplus
}
#endif
#endif
