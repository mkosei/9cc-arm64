#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_EQ,  //==
  ND_NE,  //!=
  ND_L,  // >
  ND_R,  // <
  ND_LE,  // >=
  ND_RE, // <=
  ND_NUM,
} Nodekind;

typedef struct  Node Node;

Node *expr();
Node *mul();
Node *primary();
Node *unary();
Node *equality();
Node *relational();
Node *add();

struct Node
{
  Nodekind kind;
  Node *lhs;
  Node *rhs;
  int val;
};



typedef enum {
  TK_RESERVED, // 記号
  TK_NUM,      // 整数トークン
  TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

struct Token
{
  TokenKind kind;
  Token *next;
  int val;
  char *str;
  int len;
};

Token *token;

char *user_input;

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
  if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len)) {
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

bool at_eof() {
  return token->kind == TK_EOF;
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

  while(*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }
    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')') {
      cur = new_token(TK_RESERVED, cur, p++);    
      cur->len = 1;
      continue;
    }
    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }
    if (!strncmp(p, "==", 2) ||
    !strncmp(p, "!=", 2) ||
    !strncmp(p, "<=", 2) ||
    !strncmp(p, ">=", 2)) {
    cur = new_token(TK_RESERVED, cur, p);
    cur->len = 2;
    p += 2;
    continue;
  }
  if (*p == '<' || *p == '>') {
    cur = new_token(TK_RESERVED, cur, p++);
    cur->len = 1;
    continue;
  }
    error(p, "トークナイズできません");
  }
  new_token(TK_EOF, cur, p);
  return head.next;
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


Node *expr() {
  return equality();
  // Node *node = mul();
  // for (;;) {
  //   if (consume('+'))
  //     node = new_node(ND_ADD, node, mul());
  //   else if (consume('-'))
  //     node = new_node(ND_SUB, node, mul());
  //   else
  //     return node;
  // }
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
      node = new_node(ND_R, node, add());
    else if (consume("<=")) {
      node = new_node(ND_RE, node, add());     
    }
    else if (consume(">")) {
      node = new_node(ND_L, node, add());
    }
    else if (consume(">=")) {
      node = new_node(ND_LE, node, add());
    }
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
  return new_node_num(expect_number());
}

Node *unary() {
  if (consume("+"))
    return primary();
  if (consume("-"))
    return new_node(ND_SUB, new_node_num(0), primary());
  return primary();
}


void gen(Node *node) {
  if (node->kind == ND_NUM) {
    printf("  mov x0, #%d\n", node->val);
    printf("  STR x0, [sp, #-16]!\n");
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  LDR x1, [sp], #16\n");
  printf("  LDR x0, [sp], #16\n");


  // switch (node->kind) {
  //   case ND_ADD:
  //     printf("  add x0, x0, #%d\n");
  //     break;
  //   case ND_SUB:
  //     printf("  sub x0, x0, #%d\n"); 
  //     break;
  //   case ND_MUL:
  //     printf("  mul x0, x0, #%d\n");  
  //     break;
  //   case ND_DIV:
  //     printf("  sdiv x0, x0, #%d\n"); 
  //     break;
  // }
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

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  user_input = argv[1];
  token = tokenize(user_input);
  Node *node = expr();

  printf(".globl _main\n");
  printf("_main:\n");
  //printf("  mov x0, #%d\n", expect_number());

  gen(node);
  

  // while(!at_eof()) {
  //   if(consume('+')) {
  //     printf("  add x0, x0, #%d\n", expect_number());
  //     continue;
  //   }
  //   expect('-');
  //   printf("  sub x0, x0, #%d\n", expect_number());
  // }
 
  printf("  LDR x0, [sp], #16\n");
  printf("  ret\n");
  return 0;
}