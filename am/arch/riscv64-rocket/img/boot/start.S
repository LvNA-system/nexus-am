.section entry, "ax"
.globl _start
.type _start, @function

_start:
  j here
.align 2
magic_num:
  .word 0xdeadbeef
  # dtb will be put here
  # 8K should be enough
  .skip 0x2000
here:
  #csrwi 0x800, 1
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
  # la sp, exception_stack
  mv s0, ra
  call irq_handle
  mv ra, s0
  mret

.align 3
.globl stack
.globl stack_end
stack:
  .skip 0x4000
stack_end:
exception_stack:
  .skip 0x4000
exception_stack_end:
