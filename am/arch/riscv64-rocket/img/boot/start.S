.section entry, "ax"
.globl _start
.type _start, @function

_start:
  # load mtvec and make sure it sticks
  la t0, handle_trap
  csrw mtvec, t0
  csrr t1, mtvec
1:bne t0, t1, 1b

  # only let hart 0 run
  csrr  a0,mhartid
  # load sp
  # stack size for one core: 0x4000
  li a1, 0x4000
  mul a0, a0, a1
  la sp, stack_end
  sub sp, sp, a0
  tail _trm_init

.align 3
handle_trap:
  # exception stack (for simulation)
  li sp, 0x100fff000
  tail bad_trap

# reserve stack space for at least 8 cores
stack:
  .skip 0x20000
stack_end:
