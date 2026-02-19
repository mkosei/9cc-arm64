#include "9cc.h"
#include <_stdio.h>
#include <_string.h>
#include <alloca.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern Token *token;
extern char *user_input;
LVar *locals = NULL;
static char *brk_label;
static char *cont_label;

void error(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " ");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

int align_to(int n, int align) { return (n + align - 1) / align * align; }

void expect_keyword(const char *kw) {
  if (token->kind != TK_KEYWORD || !equal(token, kw))
    error(token->str, "'%s'ではありません", kw);

  token = token->next;
}

char *expect_ident() {
  if (token->kind != TK_IDENT)
    error(token->str, "識別子ではありません");

  char *name = strndup(token->str, token->len);
  token = token->next;
  return name;
}

static char *new_unique_name(void) {
  static int id = 0;
  char *buf = malloc(32);
  if (!buf)
    return NULL;
  snprintf(buf, 32, ".L%d", id++);
  return buf;
}

LVar *find_lvar(Token *tok) {
  for (LVar *var = locals; var; var = var->next) {
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
      return var;
  }
  return NULL;
}

Node *new_node(Nodekind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(*node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(*node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

Node *declaration() {
  expect_keyword("int");

  Token *tok = consume_ident();
  if (!tok)
    error(token->str, "変数名が必要");

  LVar *lvar = calloc(1, sizeof(LVar));
  lvar->name = tok->str;
  lvar->len = tok->len;
  lvar->next = locals;

  lvar->offset = locals ? locals->offset + 8 : 8;
  locals = lvar;

  Node *node = new_node(ND_BLOCK, NULL, NULL);
  node->body = NULL;
  node->body_len = 0;

  if (consume("=")) {
    Node *lhs = calloc(1, sizeof(Node));
    lhs->kind = ND_LVAR;
    lhs->offset = lvar->offset;

    node = new_node(ND_ASSIGN, lhs, expr());
  }

  expect(';');
  return node;
}

Node *assign() {
  Node *node = equality();
  if (consume("="))
    node = new_node(ND_ASSIGN, node, equality());
  return node;
}

Node *expr() { return assign(); }

Node *stmt() {
  if (token->kind == TK_KEYWORD && equal(token, "int"))
    return declaration();
  Node *node;
  if (token->kind == TK_KEYWORD && equal(token, "return")) {
    token = token->next;
    node = new_node(ND_RETURN, expr(), NULL);
    expect(';');
    return node;
  }

  if (token->kind == TK_KEYWORD && equal(token, "if")) {
    token = token->next;
    expect('(');
    Node *cond = expr();
    expect(')');
    Node *then = stmt();
    Node *els = NULL;
    if (token->kind == TK_KEYWORD && equal(token, "else")) {
      token = token->next;
      els = stmt();
    }
    node = new_node(ND_IF, cond, then);
    node->els = els;
    return node;
  }

  if (token->kind == TK_KEYWORD && equal(token, "while")) {
    token = token->next;
    expect('(');

    Node *cond = expr();
    expect(')');

    // char *brk = brk_label;
    // char *cont = cont_label;

    LVar *locals_backup = locals;
    Node *then = stmt();
    locals = locals_backup;

    node = new_node(ND_FOR, NULL, NULL);
    node->cond = cond;
    node->then = then;

    node->brk_label = new_unique_name();
    node->cont_label = new_unique_name();

    // brk_label = brk;
    // cont_label = cont;

    return node;
  }

  if (token->kind == TK_KEYWORD && equal(token, "for")) {
    token = token->next;
    expect('(');

    // char *brk = brk_label;
    // char *cont = cont_label;

    // LVar *locals_backup = locals;

    Node *init = NULL;
    if (!consume(";"))
      init = expr();
    expect(';');

    Node *cond = NULL;
    if (!consume(";"))
      cond = expr();
    expect(';');

    Node *inc = NULL;
    if (!consume(")"))
      inc = expr();
    expect(')');

    LVar *locals_backup = locals;
    Node *then = stmt();
    // node = new_node(ND_FOR, cond, then);
    locals = locals_backup;

    node = new_node(ND_FOR, NULL, NULL);

    node->init = init;
    node->cond = cond;
    node->inc = inc;
    node->then = then;

    node->brk_label = new_unique_name();
    node->cont_label = new_unique_name();

    // brk_label = brk;
    // cont_label = cont;
    return node;
  }

  if (equal(token, "{")) {
    token = token->next;
    Node **stmts = malloc(sizeof(Node *) * 100);
    int len = 0;

    while (!equal(token, "}")) {
      stmts[len++] = stmt();
    }
    expect('}');

    Node *node = new_node(ND_BLOCK, NULL, NULL);
    node->body = stmts;
    node->body_len = len;
    return node;
  }

  if (equal(token, ";")) {
    token = token->next;
    Node *node = new_node(ND_BLOCK, NULL, NULL);
    node->body = NULL;
    node->body_len = 0;
    return node;
  }

  node = expr();
  expect(';');
  return node;
}

Obj *function() {
  locals = NULL;
  expect_keyword("int");

  char *name = expect_ident();

  expect('(');
  expect(')');

  expect('{');

  Node **stmts = calloc(100, sizeof(Node *));
  int len = 0;

  while (!equal(token, "}"))
    stmts[len++] = stmt();

  expect('}');

  Node *body = new_node(ND_BLOCK, NULL, NULL);
  body->body = stmts;
  body->body_len = len;

  Obj *fn = calloc(1, sizeof(Obj));
  fn->name = name;
  fn->body = body;
  fn->locals = locals;
  int max = 0;

  for (LVar *var = locals; var; var = var->next)
    if (max < var->offset)
      max = var->offset;

  fn->stack_size = align_to(max, 16);
  return fn;
}

Obj *program() {
  Obj head = {};
  Obj *cur = &head;
  while (!at_eof())
    cur = cur->next = function();

  return head.next;
}

Node *equality() {
  Node *node = relational();
  for (;;) {
    if (consume("=="))
      node = new_node(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_node(ND_NE, node, relational());
    else
      return node;
  }
}

Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<="))
      node = new_node(ND_LE, node, add());
    else if (consume("<"))
      node = new_node(ND_LT, node, add());
    else if (consume(">="))
      node = new_node(ND_GE, node, add());
    else if (consume(">"))
      node = new_node(ND_GT, node, add());
    else
      return node;
  }
}

Node *add() {
  Node *node = mul();
  for (;;) {
    if (consume("+"))
      node = new_node(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
  }
}

Node *mul() {
  Node *node = unary();
  for (;;) {
    if (consume("*"))
      node = new_node(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_node(ND_DIV, node, unary());
    else
      return node;
  }
}

Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(')');
    return node;
  }

  Token *tok = consume_ident();
  if (tok) {
    if (consume("(")) {
      Node *node = calloc(1, sizeof(Node));
      node->kind = ND_FUNCALL;
      node->funcname = strndup(tok->str, tok->len);

      Node **args = malloc(sizeof(Node *) * 8);
      int argc = 0;

      if (!consume(")")) {
        args[argc++] = expr();
        while (consume(",")) {
          args[argc++] = expr();
        }
        expect(')');
      }
      node->args = args;
      node->argc = argc;
      return node;
    }

    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;

    LVar *lvar = find_lvar(tok);
    if (lvar) {
      node->offset = lvar->offset;
    } else {
      lvar = calloc(1, sizeof(LVar));
      lvar->next = locals;
      lvar->name = tok->str;
      lvar->len = tok->len;

      if (locals)
        lvar->offset = locals->offset + 8;
      else
        lvar->offset = 8;

      node->offset = lvar->offset;
      locals = lvar;
    }

    return node;
  }

  return new_node_num(expect_number());
}

Node *unary() {
  if (consume("+"))
    return primary();
  if (consume("-"))
    return new_node(ND_SUB, new_node_num(0), primary());
  return primary();
}
