#include "9cc.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Token *token;
char *user_input;

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  user_input = argv[1];
  token = tokenize(user_input);

  program();

  Node *node = expr();

  printf(".globl _main\n");
  printf("_main:\n");

  printf("  stp x29, x30, [sp, #-16]!\n");
  printf("  mov x29, sp\n");
  printf("  sub sp, sp, #208\n");

  for (int i = 0; code[i]; i++) {
    gen(code[i]);
    printf("  ldr x0, [sp], #16\n");
  }

  printf("  mov sp, x29\n");
  printf("  ldp x29, x30, [sp], #16\n");
  // printf("  LDR x0, [sp], #16\n");
  printf("  ret\n");
  return 0;
}
