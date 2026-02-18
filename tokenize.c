#include "9cc.h"
#include <_stdio.h>
#include <alloca.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool consume(const char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len)) {
    return false;
  }
  token = token->next;
  return true;
}

bool equal(Token *tok, const char *op) {

  if (!tok)
    return false;

  int len = strlen(op);
  return tok->len == len && strncmp(tok->str, op, len) == 0;
}

void expect(const char op) {
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
        *p == ')' || *p == ';' || *p == '{' || *p == '}') {
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
      cur = new_token(TK_KEYWORD, cur, p);
      cur->len = 6;
      p += 6;
      continue;
    }

    if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
      cur = new_token(TK_KEYWORD, cur, p);
      cur->len = 2;
      p += 2;
      continue;
    }

    if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
      cur = new_token(TK_KEYWORD, cur, p);
      cur->len = 4;
      p += 4;
      continue;
    }

    if (strncmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
      cur = new_token(TK_KEYWORD, cur, p);
      cur->len = 5;
      p += 5;
      continue;
    }

    if (strncmp(p, "for", 3) == 0 && !is_alnum(p[3])) {
      cur = new_token(TK_KEYWORD, cur, p);
      cur->len = 3;
      p += 3;
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
