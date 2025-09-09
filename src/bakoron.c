#include "bakoron.h"
#include "stb_ds.h"
#include <stdio.h>
#include <stdlib.h>

#define BK_EPSILON -1

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

static void _tree_init(Bakoron_Tree *tree, int symbol, char *lexeme,
                       int rule_descriptor) {
  tree->symbol = symbol;
  tree->lexeme = lexeme;
  tree->rule_descriptor = rule_descriptor;

  tree->children = NULL;
  tree->number_of_children = 0;
}

static void _tree_insert(Bakoron_Tree *root, Bakoron_Tree *child) {
  arrput(root->children, child);
  ++root->number_of_children;
}

static char *_strndup(const char *string, size_t n) {
  char *copy = (char *)malloc((n + 1) * sizeof(char));
  size_t i;

  for (i = 0; i < n; ++i) {
    copy[i] = string[i];
  }
  copy[n] = '\0';

  return copy;
}

static Bakoron_Tree *_parse_string_recursive(
    Bakoron *bakoron, Bakoron_Symbol *start_symbol,
    int (*get_next_token)(const char *string, size_t *consumed_size,
                          void *user_data),
    const char *string, void *user_data, size_t *parsed_length) {

  int i;
  int string_length = strlen(string);

  if (parsed_length != NULL)
    *parsed_length = 0;

  if (start_symbol->type == BK_TERMINAL) {

    while (1) {
      size_t consumed_size;
      int next_token = get_next_token(string, &consumed_size, user_data);
      if (parsed_length != NULL)
        *parsed_length += consumed_size;

      if (hmget(bakoron->symbol_to_index_map, next_token) >= 0) {
        char *lexeme;
        Bakoron_Tree *tree;

        if (start_symbol->symbol != next_token) {
          fprintf(stderr, "%s:%d: Syntax error: Expected %d, found %d\n",
                  __FILE__, __LINE__, start_symbol->symbol, next_token);
          return NULL;
        }

        lexeme = _strndup(string, consumed_size);
        tree = (Bakoron_Tree *)malloc(sizeof(Bakoron_Tree));
        _tree_init(tree, next_token, lexeme, -1);

        return tree;
      }

      string += consumed_size;
      string_length -= consumed_size;

      if (string_length <= 0)
        break;
    }

    fprintf(stderr, "ERROR: %s:%d: Should be unreachable code here\n", __FILE__,
            __LINE__);
    exit(1);
  }

  for (i = 0; i < arrlen(start_symbol->rules); ++i) {
    Bakoron_Rule *rule = start_symbol->rules[i];
    int j;
    const char *remaining_string = string;
    int remaining_string_length = string_length;
    int rule_succeded = 1;

    Bakoron_Tree *tree = (Bakoron_Tree *)malloc(sizeof(Bakoron_Tree));
    _tree_init(tree, start_symbol->symbol, NULL, rule->rule_descriptor);

    for (j = 0; j < arrlen(rule->children); ++j) {
      int child_symbol = rule->children[j];
      int child_symbol_index =
          hmget(bakoron->symbol_to_index_map, child_symbol);
      Bakoron_Symbol *child = &bakoron->symbols[child_symbol_index];

      size_t child_parsed_length;
      Bakoron_Tree *child_tree = _parse_string_recursive(
          bakoron, child, get_next_token, remaining_string, user_data,
          &child_parsed_length);

      remaining_string += child_parsed_length;
      remaining_string_length -= child_parsed_length;

      if (child_tree == NULL || remaining_string_length < 0) {
        /* TODO or remaining_string_length == 0 but there are other non nullable
         * siblings after me */

	rule_succeded = 0;
        break;
      }

      _tree_insert(tree, child_tree);
      if (parsed_length != NULL)
        *parsed_length += child_parsed_length;
    }

    if (!rule_succeded) {
      bakoron_cleanup_tree(tree);
      return NULL;
    }

    return tree;
  }

  return NULL;
}

Bakoron_Tree *bakoron_parse_string(Bakoron *bakoron, int start_symbol,
                                   int (*get_next_token)(const char *string,
                                                         size_t *consumed_size,
                                                         void *user_data),
                                   const char *string, void *user_data) {

  /* TODO compute nullables and check if grammar is sane */

  int start_symbol_index = hmget(bakoron->symbol_to_index_map, start_symbol);
  Bakoron_Symbol *bk_start_symbol;
  Bakoron_Tree *tree;
  int remaining_token;
  size_t parsed_length;

  if (start_symbol_index < 0) {
    fprintf(stderr, "ERROR: Start symbol %d is not registered\n", start_symbol);
    exit(1);
  }

  bk_start_symbol = &bakoron->symbols[start_symbol_index];

  tree = _parse_string_recursive(bakoron, bk_start_symbol, get_next_token,
                                 string, user_data, &parsed_length);

  if (string[parsed_length] != '\0') {
    remaining_token =
        get_next_token(string + parsed_length, &parsed_length, user_data);

    if (hmget(bakoron->symbol_to_index_map, remaining_token) >= 0) {
      bakoron_cleanup_tree(tree);
      fprintf(stderr,
              "%s:%d: Syntax error: unexpected token %d at end of input\n",
              __FILE__, __LINE__, remaining_token);
      return NULL;
    }
  }

  return tree;
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

  if (tree->lexeme) free(tree->lexeme);

  for (i = 0; i < arrlen(tree->children); ++i) {
    bakoron_cleanup_tree(tree->children[i]);
  }

  arrfree(tree->children);
  free(tree);
}
