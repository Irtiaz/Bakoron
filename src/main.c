#include "bakoron.h"
#include <stdio.h>
#include <stdlib.h>

typedef enum { EXPRESSION, BIT, PLUS } Symbol;
typedef enum { EXPRESSION__BIT_PLUS_BIT } Rule;

size_t get_next_token(const char *string, int *token, void *user_data) {
  (void)user_data;

  if (string[0] == '+') {
    *token = PLUS;
    return 1;
  }

  else if (string[0] == '1') {
    *token = BIT;
    return 1;
  }

  else if (string[0] == '0') {
    *token = BIT;
    return 1;
  }

  else {
    /* TODO handle error */
    return 0;
  }
}

int evaluate_tree(Bakoron_Tree *tree) {
  if (tree->symbol == EXPRESSION) {
    if (tree->rule_descriptor == EXPRESSION__BIT_PLUS_BIT) {

      return evaluate_tree(tree->children[0]) +
             evaluate_tree(tree->children[2]);
    }

    else {
      fprintf(stderr, "ERROR: Undefined rule found for EXPRRESSION\n");
      exit(1);
    }
  }

  else if (tree->symbol == BIT) {
    return tree->lexeme[0] - '0';
  }

  fprintf(stderr, "ERROR: Neither EXPRESSION nor BIT encountered\n");
  exit(1);
}

int main(void) {
  Bakoron bakoron;
  Bakoron_Tree *tree;

  const char *input = "1+0";

  bakoron_init(&bakoron);

  bakoron_register_symbol(&bakoron, EXPRESSION, BK_VARIABLE);
  bakoron_register_symbol(&bakoron, PLUS, BK_TERMINAL);
  bakoron_register_symbol(&bakoron, BIT, BK_TERMINAL);

  {
    int rule[] = {BIT, PLUS, BIT};
    bakoron_register_rule(&bakoron, EXPRESSION, EXPRESSION__BIT_PLUS_BIT, rule,
                          sizeof(rule) / sizeof(rule[0]));
  }

  tree =
      bakoron_parse_string(&bakoron, EXPRESSION, get_next_token, input, NULL);

  printf("Result: %d\n", evaluate_tree(tree));

  bakoron_cleanup(&bakoron);
  bakoron_cleanup_tree(tree);

  return 0;
}
