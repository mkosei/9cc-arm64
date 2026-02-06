#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
bool consume(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op) {
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
    if (*p == '+' || *p == '-') {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }
    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }
    error(token->str, "トークナイズできません");
  }
  new_token(TK_EOF, cur, p);
  return head.next;
}


int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  token = tokenize(argv[1]);


  printf(".globl _main\n");
  printf("_main:\n");
  printf("  mov x0, #%d\n", expect_number());

  while(!at_eof()) {
    if(consume('+')) {
      printf("  add x0, x0, #%d\n", expect_number());
      continue;
    }
    expect('-');
    printf("  sub x0, x0, #%d\n", expect_number());
  }

  printf("  ret\n");
  return 0;
}