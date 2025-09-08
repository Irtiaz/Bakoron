#include <stdio.h>
#include <stdlib.h>

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

typedef struct {
  int *symbols;
} Bakoron;

typedef struct Bakoron_Tree_Struct Bakoron_Tree;

struct Bakoron_Tree_Struct {
  Bakoron_Tree **children;
};

typedef enum { BK_VARIABLE, BK_TERMINAL } Bakoron_Symbol_Type;

void bakoron_register_symbol(Bakoron *bakoron, int symbol,
                             Bakoron_Symbol_Type type) {
  (void)bakoron;
  (void)symbol;
  (void)type;

  fprintf(stderr, "ERROR: Not implemented yet\n");
  exit(1);
}

void bakoron_register_rule(Bakoron *bakoron, int symbol, int *rule,
                           size_t rule_size) {
  (void)bakoron;
  (void)symbol;
  (void)rule;
  (void)rule_size;

  fprintf(stderr, "ERROR: Not implemented yet\n");
  exit(1);
}

Bakoron_Tree *bakoron_parse_string(Bakoron *bakoron, int start_symbol,
                                   size_t (*get_next_token)(const char *string,
                                                            int *token,
                                                            void *user_data),
                                   const char *string, void *user_data) {
  (void)bakoron;
  (void)start_symbol;
  (void)get_next_token;
  (void)string;
  (void)user_data;

  fprintf(stderr, "ERROR: Not implemented yet\n");
  exit(1);

  return NULL;
}

void bakoron_cleanup(Bakoron *bakoron) {
  (void)bakoron;

  fprintf(stderr, "ERROR: Not implemented yet\n");
  exit(1);
}

void bakoron_cleanup_tree(Bakoron_Tree *tree) {
  (void)tree;

  fprintf(stderr, "ERROR: Not implemented yet\n");
  exit(1);
}

int main(void) {
  Bakoron bakoron;
  Bakoron_Tree *tree;

  const char *input = "1+0";

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
