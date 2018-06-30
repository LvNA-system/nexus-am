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
2:bne a0, zero, 2b

  la sp, stack_end
  tail _trm_init



handle_trap:
  # exception stack (for simulation)
  li sp, 0x100fff000
  tail bad_trap

.align 3
stack:
  .skip 0x4000
stack_end:
