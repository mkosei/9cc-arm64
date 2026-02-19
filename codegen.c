#include "9cc.h"
#include <stdio.h>
#include <stdlib.h>

static int label_idx = 0;
static int count = 0;

void gen_func(Obj *fn) {
  printf(".globl %s\n", fn->name);
  printf("%s:\n", fn->name);

  printf("  stp x29, x30, [sp, #-16]!\n");
  printf("  mov x29, sp\n");
  printf("  sub sp, sp, #%d\n", fn->stack_size);

  gen_stmt(fn->body);

  printf("  mov sp, x29\n");
  printf("  ldp x29, x30, [sp], #16\n");
  printf("  ret\n");
}

void gen_lval(Node *node) {
  if (node->kind != ND_LVAR) {
    fprintf(stderr, "代入の左辺値が変数ではありません\n");
    exit(1);
  }

  printf("  mov x0, x29\n");
  printf("  sub x0, x0, #%d\n", node->offset);
  printf("  str x0, [sp, #-16]!\n");
}

void gen_stmt(Node *node) {
  switch (node->kind) {
  case ND_RETURN:
    gen_expr(node->lhs);
    printf("  ldr x0, [sp], #16\n");
    printf("  mov sp, x29\n");
    printf("  ldp x29, x30, [sp], #16\n");
    printf("  ret\n");
    return;
  case ND_IF: {
    int idx = label_idx++;
    gen_expr(node->lhs);
    printf("  ldr x0, [sp], #16\n");
    printf("  cmp x0, #0\n");
    if (node->els) {
      printf("  beq Lelse%d\n", idx);
      gen_stmt(node->rhs);
      printf("  b Lend%d\n", idx);
      printf("Lelse%d:\n", idx);
      gen_stmt(node->els);
    } else {
      printf("  beq Lend%d\n", idx);
      gen_stmt(node->rhs);
    }
    printf("Lend%d:\n", idx);
    return;
  }

  case ND_FOR: {
    int c = count++;
    if (node->init) {
      gen_stmt(node->init);
    }
    printf("Lbegin%d:\n", c);
    if (node->cond) {
      gen_expr(node->cond);
      printf("  ldr x0, [sp], #16\n");
      printf("  cmp x0, #0\n");
      printf("  beq %s\n", node->brk_label);
    }
    gen_stmt(node->then);
    printf("%s:\n", node->cont_label);

    if (node->inc) {
      gen_expr(node->inc);
    }
    printf("  b Lbegin%d\n", c);
    printf("%s:\n", node->brk_label);
    return;
  }
  case ND_BLOCK:
    for (int i = 0; i < node->body_len; i++) {
      gen_stmt(node->body[i]);
    }
    return;

  default:
    gen_expr(node);
    printf("  add sp, sp, #16\n");
    return;
  }
}

void gen_expr(Node *node) {
  switch (node->kind) {
  case ND_NUM:
    printf("  mov x0, #%d\n", node->val);
    printf("  str x0, [sp, #-16]!\n");
    return;

  case ND_FUNCALL:
    for (int i = node->argc - 1; i >= 0; i--)
      gen_expr(node->args[i]);

    for (int i = 0; i < node->argc; i++) {
      printf("  ldr x%d, [sp], #16\n", i);
    }

    printf("  bl %s\n", node->funcname);

    // 戻り値 push
    printf("  str x0, [sp, #-16]!\n");
    return;

  case ND_LVAR:
    gen_lval(node);
    printf("  ldr x0, [sp], #16\n");
    printf("  ldr x0, [x0]\n");
    printf("  str x0, [sp, #-16]!\n");
    return;

  case ND_ASSIGN:
    gen_expr(node->rhs);
    gen_lval(node->lhs);

    printf("  ldr x0, [sp], #16\n");
    printf("  ldr x1, [sp], #16\n");
    printf("  str x1, [x0]\n");
    printf("  str x1, [sp, #-16]!\n");
    return;

  default:
    gen_expr(node->lhs);
    gen_expr(node->rhs);

    printf("  ldr x1, [sp], #16\n");
    printf("  ldr x0, [sp], #16\n");
  }

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
  }

  printf("  str x0, [sp, #-16]!\n");
}
