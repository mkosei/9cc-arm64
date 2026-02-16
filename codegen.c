#include "9cc.h"
#include <stdio.h>
#include <stdlib.h>

void gen_lval(Node *node) {
  if (node->kind != ND_LVAR) {
    fprintf(stderr, "代入の左辺値が変数じゃありません\n");
    exit(1);
  }

  printf("  mov x0, x29\n");
  printf("  sub x0, x0, #%d\n", node->offset);
  printf("  str x0, [sp, #-16]!\n");
}

void gen(Node *node) {

  switch (node->kind) {

  case ND_RETURN:
    gen(node->lhs);
    printf("  ldr x0, [sp], #16\n");
    printf("  mov sp, x29\n");
    printf("  ldp x29, x30, [sp], #16\n");
    printf("  ret\n");
    return;

    printf("  mov x0, #%d\n", node->val);
    printf("  str x0, [sp, #-16]!\n");
    return;

  case ND_NUM:
    printf("  mov x0, #%d\n", node->val);
    printf("  str x0, [sp, #-16]!\n");
    return;

  case ND_LVAR:
    gen_lval(node);
    printf("  ldr x0, [sp], #16\n");
    printf("  ldr x0, [x0]\n");
    printf("  str x0, [sp, #-16]!\n");
    return;

  case ND_ASSIGN:
    gen_lval(node->lhs); 
    gen(node->rhs);
    printf("  ldr x1, [sp], #16\n");
    printf("  ldr x0, [sp], #16\n");
    printf("  str x1, [x0]\n");
    printf("  str x1, [sp, #-16]!\n");
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  ldr x1, [sp], #16\n");
  printf("  ldr x0, [sp], #16\n");

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

  case ND_LT:
    printf("  cmp x0, x1\n");
    printf("  cset x0, lt\n");
    break;

  case ND_LE:
    printf("  cmp x0, x1\n");
    printf("  cset x0, le\n");
    break;

  case ND_GT:
    printf("  cmp x0, x1\n");
    printf("  cset x0, gt\n");
    break;

  case ND_GE:
    printf("  cmp x0, x1\n");
    printf("  cset x0, ge\n");
    break;

  default:
    fprintf(stderr, "未対応のノード種別\n");
    exit(1);
  }

  printf("  str x0, [sp, #-16]!\n");
}

