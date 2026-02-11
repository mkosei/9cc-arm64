.globl _main
_main:
  mov x0, #3
  STR x0, [sp, #-16]!
  mov x0, #5
  STR x0, [sp, #-16]!
  LDR x1, [sp], #16
  LDR x0, [sp], #16
  add x0, x0, x1
  STR x0, [sp, #-16]!
  mov x0, #2
  STR x0, [sp, #-16]!
  LDR x1, [sp], #16
  LDR x0, [sp], #16
  sdiv x0, x0, x1
  STR x0, [sp, #-16]!
  LDR x0, [sp], #16
  ret
