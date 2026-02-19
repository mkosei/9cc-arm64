#include "9cc.h"
#include <stdio.h>
#include <stdlib.h>

Token *token;
char *user_input;

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  user_input = argv[1];
  token = tokenize(user_input);
  Obj *prog = program();

  printf(".globl _main\n");
  printf("_main:\n");

  // プロローグ
  printf("  stp x29, x30, [sp, #-16]!\n");
  printf("  mov x29, sp\n");
  printf("  sub sp, sp, #208\n");

  for (Obj *fn = prog; fn; fn = fn->next)
    gen_func(fn);

  // return 0 のエピローグ
  printf("  mov x0, #0\n");
  printf("  mov sp, x29\n");
  printf("  ldp x29, x30, [sp], #16\n");
  printf("  ret\n");

  return 0;
}
