#include "9cc.h"
#include <_stdio.h>
#include <alloca.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern Token *token;
extern char *user_input;
LVar *locals = NULL;

void error(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " "); // pos個の空白を出力
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {
    return false;
  }
  token = token->next;
  return true;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op) {
    error(token->str, "'%c'ではありません", op);
  }
  token = token->next;
}

int expect_number() {
  if (token->kind != TK_NUM) {
    error(token->str, "数ではありません");
  }
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() { return token->kind == TK_EOF; }

int is_alnum(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') || (c == '_');
}

Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }
    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' ||
        *p == ')' || *p == ';') {
      cur = new_token(TK_RESERVED, cur, p++);
      cur->len = 1;
      continue;
    }
    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }
    if (!strncmp(p, "==", 2) || !strncmp(p, "!=", 2) || !strncmp(p, "<=", 2) ||
        !strncmp(p, ">=", 2)) {
      cur = new_token(TK_RESERVED, cur, p);
      cur->len = 2;
      p += 2;
      continue;
    }
    if (*p == '=') {
      cur = new_token(TK_RESERVED, cur, p);
      cur->len = 1;
      p++;
      continue;
    }

    if (*p == '<' || *p == '>') {
      cur = new_token(TK_RESERVED, cur, p++);
      cur->len = 1;
      continue;
    }

    if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
      cur = new_token(TK_RETURN, cur, p);
      cur->len = 6;
      p += 6;
      continue;
    }

    if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
      cur = new_token(TK_IF, cur, p);
      cur->len = 2;
      p += 2;
      continue;
    }

    if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
      cur = new_token(TK_ELSE, cur, p);
      cur->len = 4;
      p += 4;
      continue;
    }

    if (isalpha(*p)) {
      char *q = p;
      while (is_alnum(*p))
        p++;
      cur = new_token(TK_IDENT, cur, q);
      cur->len = p - q;
      continue;
    }

    error(p, "トークナイズできません");
  }
  new_token(TK_EOF, cur, p);
  return head.next;
}

Token *consume_ident() {
  if (token->kind != TK_IDENT)
    return NULL;

  Token *t = token;
  token = token->next;
  return t;
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

Node *assign() {
  Node *node = equality();
  if (consume("="))
    node = new_node(ND_ASSIGN, node, equality());
  return node;
}

Node *expr() { return assign(); }

Node *stmt() {
  Node *node;
  if (token->kind == TK_RETURN) {
    token = token->next;
    node = new_node(ND_RETURN, expr(), NULL);
    expect(';');
    return node;
  }

  if (token->kind == TK_IF) {
    token = token->next;
    Node *cond = expr();
    Node *then = stmt();
    Node *els = NULL;
    if (token->kind == TK_ELSE) {
      token = token->next;
      els = stmt();
    }
    node = new_node(ND_IF, cond, then);
    node->els = els;
    return node;
  }

  node = expr();
  expect(';');
  return node;
}

void program() {
  int i = 0;
  while (!at_eof())
    code[i++] = stmt();
  code[i] = NULL;
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
    if (consume("<"))
      node = new_node(ND_LT, node, add());
    else if (consume("<="))
      node = new_node(ND_LE, node, add());
    else if (consume(">"))
      node = new_node(ND_GT, node, add());
    else if (consume(">="))
      node = new_node(ND_GE, node, add());
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

  // Token *tok = consume_ident();
  // if (tok) {
  //   Node *node = calloc(1, sizeof(Node));
  //   node->kind = ND_LVAR;
  //   LVar *lvar = find_lvar(tok);
  //   if (lvar) {
  //     lvar->offset = node->offset;
  //   } else {
  //     lvar = calloc(1, sizeof(LVar));
  //     lvar->next = locals;
  //     lvar->name = tok->str;
  //     lvar->len = tok->len;
  //     lvar->offset = locals->offset + 8;
  //     node->offset = lvar->offset;
  //     locals = lvar;
  //   }
  //   return node;
  // }

  Token *tok = consume_ident();
  if (tok) {
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
