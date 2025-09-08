#include <stdio.h>
#include <stdlib.h>
#include "bakoron.h"

typedef enum { EXPRESSION, BIT, PLUS } Symbol;

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
    bakoron_register_rule(&bakoron, EXPRESSION, rule,
                          sizeof(rule) / sizeof(rule[0]));
  }

  tree =
      bakoron_parse_string(&bakoron, EXPRESSION, get_next_token, input, NULL);

  bakoron_cleanup(&bakoron);
  bakoron_cleanup_tree(tree);

  return 0;
}
