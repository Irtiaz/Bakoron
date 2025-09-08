#include "bakoron.h"
#include <stdlib.h>
#include <stdio.h>

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
