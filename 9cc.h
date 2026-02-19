#ifndef NINECC_H
#define NINECC_H

#include <stdbool.h>

#define MAX_CODE 1000
#define MAX_INDEX 100

typedef struct Node Node;
typedef struct LVar LVar;

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

typedef struct Obj Obj;
struct Obj {
  Obj *next;
  char *name;

  Node *body;

  LVar *locals;
  int stack_size;
};

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
  ND_BLOCK,
  ND_FUNCALL,
} Nodekind;

struct Node {
  Nodekind kind;
  Node *lhs;
  Node *rhs;
  // if,for,while
  Node *els;
  Node *init;
  Node *inc;
  Node *cond;
  Node *then;

  char *brk_label;
  char *cont_label;
  // block
  Node **body;
  int body_len;
  int val;
  int offset;

  // function
  char *funcname;
  Node **args;
  int argc;
};

struct LVar {
  LVar *next;
  char *name;
  int len;
  int offset;
};

LVar *locals;

extern Token *token;
extern char *user_input;
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
void gen_func(Obj *fn);
void gen_expr(Node *node);
void gen_stmt(Node *node);
void error(char *loc, char *fmt, ...);
Obj *program();
bool consume(const char *op);
bool equal(Token *tok, const char *op);
void expect(const char op);
int expect_number();
bool at_eof();
Token *consume_ident();

#endif
