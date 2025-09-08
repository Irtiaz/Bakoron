#include "bakoron.h"
#include "stb_ds.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  int *children;
  int rule_descriptor;
} Bakoron_Rule;

struct Bakoron_Symbol {
  int symbol;
  Bakoron_Symbol_Type type;
  Bakoron_Rule **rules;
};

void bakoron_init(Bakoron *bakoron) {
  bakoron->symbols = NULL;

  bakoron->symbol_to_index_map = NULL;
  hmdefault(bakoron->symbol_to_index_map, -1);
}

void bakoron_register_symbol(Bakoron *bakoron, int symbol,
                             Bakoron_Symbol_Type type) {
  Bakoron_Symbol bakoron_symbol;
  bakoron_symbol.symbol = symbol;
  bakoron_symbol.type = type;
  bakoron_symbol.rules = NULL;

  arrput(bakoron->symbols, bakoron_symbol);
  hmput(bakoron->symbol_to_index_map, symbol, arrlen(bakoron->symbols) - 1);
}

void bakoron_register_rule(Bakoron *bakoron, int symbol, int rule_descriptor,
                           int *rule, size_t rule_length) {
  Bakoron_Rule *bk_rule;
  size_t i;
  int symbol_index;

  bk_rule = (Bakoron_Rule *)malloc(sizeof(Bakoron_Rule));

  bk_rule->rule_descriptor = rule_descriptor;
  bk_rule->children = NULL;

  for (i = 0; i < rule_length; ++i) {
    arrput(bk_rule->children, rule[i]);
  }

  symbol_index = hmget(bakoron->symbol_to_index_map, symbol);

  if (symbol_index >= 0) {

    if (bakoron->symbols[symbol_index].type == BK_TERMINAL) {
      fprintf(stderr, "Can not register rule to terminal %d\n", symbol);
      exit(1);
    }

    arrput(bakoron->symbols[symbol_index].rules, bk_rule);
  }

  else {
    bakoron_register_symbol(bakoron, symbol, BK_VARIABLE);
    arrput(arrlast(bakoron->symbols).rules, bk_rule);
  }
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

  fprintf(stderr, "ERROR: Parse string not implemented yet\n");
  exit(1);

  return NULL;
}

void bakoron_cleanup(Bakoron *bakoron) {
  int i;

  for (i = 0; i < arrlen(bakoron->symbols); ++i) {
    int j;
    for (j = 0; j < arrlen(bakoron->symbols[i].rules); ++j) {
      arrfree(bakoron->symbols[i].rules[j]->children);
      free(bakoron->symbols[i].rules[j]);
    }
    arrfree(bakoron->symbols[i].rules);
  }

  arrfree(bakoron->symbols);
  hmfree(bakoron->symbol_to_index_map);
}

void bakoron_cleanup_tree(Bakoron_Tree *tree) {
  int i;

  free(tree->lexeme);

  for (i = 0; i < arrlen(tree->children); ++i) {
    bakoron_cleanup_tree(tree->children[i]);
  }
}
