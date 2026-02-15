#include "9cc.h"
#include <stdio.h>

void gen_lval(Node *node) {
  if (node->kind != ND_LVAR)
    printf("代入の左辺値が変数じゃありません");

  printf("  mov x0, x29\n");
  printf("  sub x0, x0, #%d\n", node->offset);
  printf("  str x0, [sp, #-16]!\n");
}

void gen(Node *node) {
  switch (node->kind) {
  case ND_NUM:
    printf("  mov x0, #%d\n", node->val);
    printf("  STR x0, [sp, #-16]!\n");
    return;
  case ND_LVAR:
    gen_lval(node);
    printf("  ldr x0, [sp], #16\n");
    printf("  ldr x0, [x0]\n");
    printf("  str x0, [sp, #-16]!\n");
    return;
  case ND_ASSIGN:
    gen_lval(node);
    gen(node->rhs);
    printf("  ldr x1, [sp], #16\n");
    printf("  ldr x0, [sp], #16\n");
    printf("  str x1, [x0]\n");
    printf("  str x1, [sp, #-16]!\n");
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  LDR x1, [sp], #16\n");
  printf("  LDR x0, [sp], #16\n");

  switch (node->kind) {
  case ND_ADD:
    printf("  add x0, x0, x1\n");
    break;
  case ND_SUB:
    printf("  sub x0, x0, x1\n");
    break;
  case ND_MUL:
    printf("  mul x0, x0, x1\n");
    break;
  case ND_DIV:
    printf("  sdiv x0, x0, x1\n");
    break;
  case ND_EQ:
    printf("  cmp x0, x1\n");
    printf("  cset x0, eq\n");
    break;
  case ND_NE:
    printf("  cmp x0, x1\n");
    printf("  cset x0, ne\n");
    break;
  case ND_R:
    printf("  cmp x0, x1\n");
    printf("  cset x0, lt\n");
    break;
  case ND_L:
    printf("  cmp x0, x1\n");
    printf("  cset x0, gt\n");
    break;
  case ND_RE:
    printf("  cmp x0, x1\n");
    printf("  cset x0, le\n");
    break;
  case ND_LE:
    printf("  cmp x0, x1\n");
    printf("  cset x0, ge\n");
    break;
  }

  printf("  STR x0, [sp, #-16]!\n");
}
