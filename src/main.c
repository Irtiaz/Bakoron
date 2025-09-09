#include "bakoron.h"
#include <stdio.h>
#include <stdlib.h>

typedef enum { EXPRESSION, DIGIT, PLUS, WHITESPACE } Symbol;
typedef enum { EXPRESSION__DIGIT_PLUS_DIGIT } Rule;

int is_whitespace(char c) {
  switch (c) {
  case ' ':
  case '\t':
  case '\r':
  case '\n':
    return 1;
  }

  return 0;
}

int get_next_token(const char *string, size_t *consumed_size, void *user_data) {
  (void)user_data;

  if (string[0] == '+') {
    *consumed_size = 1;
    return PLUS;
  }

  else if (string[0] >= '0' && string[0] <= '9') {
    *consumed_size = 1;
    return DIGIT;
  }

  else if (is_whitespace(string[0])) {
    size_t whitespace_size = 0;
    while (is_whitespace(string[0])) {
      ++string;
      ++whitespace_size;
    }

    *consumed_size = whitespace_size;
    return WHITESPACE;
  }

  else {
    /* TODO handle error */
    fprintf(stderr, "%s:%d: TODO handle error in lexer\n", __FILE__, __LINE__);
    exit(1);
    return 0;
  }
}

int evaluate_tree(Bakoron_Tree *tree) {
  if (tree->symbol == EXPRESSION) {
    if (tree->rule_descriptor == EXPRESSION__DIGIT_PLUS_DIGIT) {
      int left_operand = evaluate_tree(tree->children[0]);
      int right_operand = evaluate_tree(tree->children[2]);

      return left_operand + right_operand;
    }

    else {
      fprintf(stderr, "%s:%d: ERROR: Undefined rule found for EXPRRESSION\n",
              __FILE__, __LINE__);
      exit(1);
    }
  }

  else if (tree->symbol == DIGIT) {
    return tree->lexeme[0] - '0';
  }

  fprintf(stderr,
          "%s:%d: ERROR: Neither EXPRESSION nor DIGIT encountered -> %d %s\n",
          __FILE__, __LINE__, tree->symbol, tree->lexeme);
  exit(1);
}

int main(void) {
  Bakoron bakoron;
  Bakoron_Tree *tree;

  const char *input = "1 + 8";

  bakoron_init(&bakoron);

  bakoron_register_symbol(&bakoron, EXPRESSION, BK_VARIABLE);
  bakoron_register_symbol(&bakoron, PLUS, BK_TERMINAL);
  bakoron_register_symbol(&bakoron, DIGIT, BK_TERMINAL);

  {
    int rule[] = {DIGIT, PLUS, DIGIT};
    bakoron_register_rule(&bakoron, EXPRESSION, EXPRESSION__DIGIT_PLUS_DIGIT,
                          rule, sizeof(rule) / sizeof(rule[0]));
  }

  tree =
      bakoron_parse_string(&bakoron, EXPRESSION, get_next_token, input, NULL);

  if (tree == NULL) {
    fprintf(stderr, "%s:%d: Syntax error\n", __FILE__, __LINE__);
    exit(1);
  }

  printf("Result: %d\n", evaluate_tree(tree));

  bakoron_cleanup(&bakoron);
  bakoron_cleanup_tree(tree);

  return 0;
}
