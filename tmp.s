.globl _main
_main:
  mov x0, #2
  STR x0, [sp, #-16]!
  mov x0, #2
  STR x0, [sp, #-16]!
  LDR x1, [sp], #16
  LDR x0, [sp], #16
  cmp x0, x1
  cset x0, ge
  STR x0, [sp, #-16]!
  LDR x0, [sp], #16
  ret
