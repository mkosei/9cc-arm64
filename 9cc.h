#ifndef NINECC_H
#define NINECC_H

#include <stdbool.h>

#define MAX_CODE 1000
#define MAX_INDEX 100

typedef enum {
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_EQ,
  ND_NE,
  ND_LT,
  ND_GT,
  ND_LE,
  ND_GE,
  ND_NUM,
  ND_ASSIGN,
  ND_LVAR,
  ND_RETURN,
  ND_IF,
  ND_FOR,
} Nodekind;

typedef struct Node Node;

struct Node {
  Nodekind kind;
  Node *lhs;
  Node *rhs;
  // if,for,while
  Node *els;
  Node *init;
  Node *inc;
  Node *brk_label;
  Node *cont_label;
  //
  int val;
  int offset;
};

typedef enum {
  TK_RESERVED,
  TK_IDENT,
  TK_NUM,
  TK_EOF,
  TK_KEYWORD,
} TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind;
  Token *next;
  int val;
  char *str;
  int len;
};

typedef struct LVar LVar;

struct LVar {
  LVar *next;
  char *name;
  int len;
  int offset;
};

LVar *locals;

extern Token *token;
extern char *user_input;
extern Node *code[MAX_CODE];
extern Token tokens[MAX_INDEX];

Node *stmt(void);
Node *expr(void);
Node *assign(void);
Node *equality(void);
Node *relational(void);
Node *add(void);
Node *mul(void);
Node *primary(void);
Node *unary(void);

Token *tokenize(char *p);
void gen(Node *node);
void error(char *loc, char *fmt, ...);
void program();

#endif
