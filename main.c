#include "9cc.h"
#include <stdio.h>
#include <stdlib.h>

Token *token;
char *user_input;
Node *code[MAX_CODE];

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  user_input = argv[1];
  token = tokenize(user_input);
  program(); // code[] に文を格納

  printf(".globl _main\n");
  printf("_main:\n");

  // プロローグ
  printf("  stp x29, x30, [sp, #-16]!\n");
  printf("  mov x29, sp\n");
  printf("  sub sp, sp, #208\n");

  for (int i = 0; code[i]; i++) {
    gen_stmt(code[i]); // 文ごとに処理
  }

  // return 0 のエピローグ
  printf("  mov x0, #0\n");
  printf("  mov sp, x29\n");
  printf("  ldp x29, x30, [sp], #16\n");
  printf("  ret\n");

  return 0;
}
