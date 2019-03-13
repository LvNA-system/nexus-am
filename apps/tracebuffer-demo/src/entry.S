#if __riscv_xlen == 64
# define SLL32    sllw
# define STORE    sd
# define LOAD     ld
# define LWU      lwu
# define LOG_REGBYTES 3
#else
# define SLL32    sll
# define STORE    sw
# define LOAD     lw
# define LWU      lw
# define LOG_REGBYTES 2
#endif

#define REGBYTES (1 << LOG_REGBYTES)
#define INTEGER_CONTEXT_SIZE (32 * REGBYTES)

#define bscratch 0x7de
#define bcause 0xfcd

.option norvc
.section .text
.globl register_exception_handler
.type register_exception_handler, @function

register_exception_handler:
  # load mtvec and make sure it sticks
  la t0, exception_entry
1:csrw mtvec, t0
  csrr t1, mtvec
  bne t0, t1, 1b
  ret

# exception should be properly aligned
.align 3
exception_entry:
  # save t0 for temporary usage
  csrw bscratch, t0
  csrr t0, bcause
  bne t0, zero, tracebuf_exception

ordinary_exception:
  # ordinary interrupt/exception
  # can use all the mXXX CSR registers
  csrr t0, bscratch
  csrrw sp, mscratch, sp
  la sp, normal_exception_stack_end
  j store_gprs

tracebuf_exception:
  # ordinary interrupt/exception
  # can only write the bXXX CSR registers
  csrr t0, bscratch
  csrrw sp, bscratch, sp
  # use a separete stack
  la sp, tracebuf_exception_stack_end
  j store_gprs

store_gprs:
  addi sp, sp, -INTEGER_CONTEXT_SIZE
  # Save all of the registers.
  STORE ra, 1*REGBYTES(sp)
  STORE gp, 3*REGBYTES(sp)
  STORE tp, 4*REGBYTES(sp)
  STORE t0, 5*REGBYTES(sp)
  STORE t1, 6*REGBYTES(sp)
  STORE t2, 7*REGBYTES(sp)
  STORE s0, 8*REGBYTES(sp)
  STORE s1, 9*REGBYTES(sp)
  STORE a0,10*REGBYTES(sp)
  STORE a1,11*REGBYTES(sp)
  STORE a2,12*REGBYTES(sp)
  STORE a3,13*REGBYTES(sp)
  STORE a4,14*REGBYTES(sp)
  STORE a5,15*REGBYTES(sp)
  STORE a6,16*REGBYTES(sp)
  STORE a7,17*REGBYTES(sp)
  STORE s2,18*REGBYTES(sp)
  STORE s3,19*REGBYTES(sp)
  STORE s4,20*REGBYTES(sp)
  STORE s5,21*REGBYTES(sp)
  STORE s6,22*REGBYTES(sp)
  STORE s7,23*REGBYTES(sp)
  STORE s8,24*REGBYTES(sp)
  STORE s9,25*REGBYTES(sp)
  STORE s10,26*REGBYTES(sp)
  STORE s11,27*REGBYTES(sp)
  STORE t3,28*REGBYTES(sp)
  STORE t4,29*REGBYTES(sp)
  STORE t5,30*REGBYTES(sp)
  STORE t6,31*REGBYTES(sp)

.globl my_exception_handler
  # call exception handler
  la t0, my_exception_handler
  jalr t0

restore_gprs:
  # Restore all of the registers.
  LOAD ra, 1*REGBYTES(sp)
  LOAD gp, 3*REGBYTES(sp)
  LOAD tp, 4*REGBYTES(sp)

  # no hurry to restore t0
  # we still need to use it
  # LOAD t0, 5*REGBYTES(sp)

  LOAD t1, 6*REGBYTES(sp)
  LOAD t2, 7*REGBYTES(sp)
  LOAD s0, 8*REGBYTES(sp)
  LOAD s1, 9*REGBYTES(sp)
  LOAD a0,10*REGBYTES(sp)
  LOAD a1,11*REGBYTES(sp)
  LOAD a2,12*REGBYTES(sp)
  LOAD a3,13*REGBYTES(sp)
  LOAD a4,14*REGBYTES(sp)
  LOAD a5,15*REGBYTES(sp)
  LOAD a6,16*REGBYTES(sp)
  LOAD a7,17*REGBYTES(sp)
  LOAD s2,18*REGBYTES(sp)
  LOAD s3,19*REGBYTES(sp)
  LOAD s4,20*REGBYTES(sp)
  LOAD s5,21*REGBYTES(sp)
  LOAD s6,22*REGBYTES(sp)
  LOAD s7,23*REGBYTES(sp)
  LOAD s8,24*REGBYTES(sp)
  LOAD s9,25*REGBYTES(sp)
  LOAD s10,26*REGBYTES(sp)
  LOAD s11,27*REGBYTES(sp)
  LOAD t3,28*REGBYTES(sp)
  LOAD t4,29*REGBYTES(sp)
  LOAD t5,30*REGBYTES(sp)
  LOAD t6,31*REGBYTES(sp)

  csrr t0, bcause
  bne t0, zero, tracebuf_exception_end

normal_exception_end:
  # normal exception
  LOAD t0, 5*REGBYTES(sp)
  csrrw sp, mscratch, sp
  mret

tracebuf_exception_end:
  LOAD t0, 5*REGBYTES(sp)
  csrrw sp, bscratch, sp
  mret


.section .data
.align 3
normal_exception_stack:
  .skip 0x4000
normal_exception_stack_end:
tracebuf_exception_stack:
  .skip 0x4000
tracebuf_exception_stack_end:
