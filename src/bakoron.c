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
  int nullable;
};

typedef struct {
  Bakoron_Symbol *key;
  int value;
} Bakoron_Symbol_Set;

void bakoron_init(Bakoron *bakoron) {
  bakoron->symbols = NULL;

  bakoron->symbol_to_index_map = NULL;
  hmdefault(bakoron->symbol_to_index_map, -1);

  bakoron_register_symbol(bakoron, BK_EPSILON, BK_TERMINAL);
}

static int _symbol_is_registered(Bakoron *bakoron, int symbol) {
  return hmget(bakoron->symbol_to_index_map, symbol) >= 0;
}

void bakoron_register_symbol(Bakoron *bakoron, int symbol,
                             Bakoron_Symbol_Type type) {
  Bakoron_Symbol bakoron_symbol;

  if (_symbol_is_registered(bakoron, symbol)) {
    fprintf(stderr, "ERROR: Symbol %d is already registered\n", symbol);
    exit(1);
  }

  bakoron_symbol.symbol = symbol;
  bakoron_symbol.type = type;
  bakoron_symbol.rules = NULL;
  bakoron_symbol.nullable = 0;

  arrput(bakoron->symbols, bakoron_symbol);
  hmput(bakoron->symbol_to_index_map, symbol, arrlen(bakoron->symbols) - 1);
}

static Bakoron_Symbol *_symbol_int_to_symbol_struct(Bakoron *bakoron,
                                                    int symbol_int) {
  int symbol_index = hmget(bakoron->symbol_to_index_map, symbol_int);

  if (symbol_index < 0) {
    fprintf(stderr, "Symbol %d is not registered\n", symbol_int);
    exit(1);
  }

  return &bakoron->symbols[symbol_index];
}

static int _symbol_is_nullable_from_set(Bakoron_Symbol *symbol,
                                        Bakoron_Symbol_Set *nullable_set) {
  return symbol->symbol == BK_EPSILON || hmget(nullable_set, symbol) == 1;
}

static int _rule_is_nullable(Bakoron *bakoron, Bakoron_Rule *rule,
                             Bakoron_Symbol_Set *nullable_set) {
  int i;

  for (i = 0; i < arrlen(rule->children); ++i) {
    int symbol = rule->children[i];
    Bakoron_Symbol *bk_symbol = _symbol_int_to_symbol_struct(bakoron, symbol);

    if (!_symbol_is_nullable_from_set(bk_symbol, nullable_set))
      return 0;
  }

  return 1;
}

static int _symbol_has_nullable_rule(Bakoron *bakoron, Bakoron_Symbol *symbol,
                                     Bakoron_Symbol_Set *nullable_set) {
  int i;

  for (i = 0; i < arrlen(symbol->rules); ++i) {
    Bakoron_Rule *rule = symbol->rules[i];
    if (_rule_is_nullable(bakoron, rule, nullable_set))
      return 1;
  }

  return 0;
}

static void _check_nullables(Bakoron *bakoron) {
  Bakoron_Symbol_Set *nullable_set = NULL;
  int i;
  int previous_length;

  hmdefault(nullable_set, 0);

  do {
    int j;

    previous_length = hmlen(nullable_set);

    for (j = 0; j < arrlen(bakoron->symbols); ++j) {
      Bakoron_Symbol *symbol = &bakoron->symbols[j];

      if (_symbol_has_nullable_rule(bakoron, symbol, nullable_set)) {
        hmput(nullable_set, symbol, 1);
      }
    }

  } while (previous_length != hmlen(nullable_set));

  for (i = 0; i < arrlen(bakoron->symbols); ++i) {
    Bakoron_Symbol *symbol = &bakoron->symbols[i];

    symbol->nullable = hmget(nullable_set, symbol) != 0;
  }

  hmfree(nullable_set);
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

static int _get_next_registered_token(
    Bakoron *bakoron,
    int (*get_next_token)(const char *string, size_t *cosumed_size,
                          void *user_data),
    size_t *consumed_size, size_t *skip_size, void *user_data,
    const char *string) {

  int string_length = strlen(string);

  if (consumed_size != NULL)
    *consumed_size = 0;

  if (skip_size != NULL)
    *skip_size = 0;

  while (string_length > 0) {
    size_t parsed_length;
    int next_token = get_next_token(string, &parsed_length, user_data);

    if (consumed_size != NULL)
      *consumed_size += parsed_length;

    if (_symbol_is_registered(bakoron, next_token)) {
      return next_token;
    }

    string += parsed_length;
    string_length -= parsed_length;

    if (skip_size != NULL)
      *skip_size += parsed_length;
  }

  return -1;
}

static Bakoron_Tree *_parse_string_recursive(
    Bakoron *bakoron, Bakoron_Symbol *start_symbol,
    int (*get_next_token)(const char *string, size_t *consumed_size,
                          void *user_data),
    const char *string, void *user_data, size_t *parsed_length);

static Bakoron_Tree *_parse_string_with_terminal(
    Bakoron *bakoron, Bakoron_Symbol *start_symbol,
    int (*get_next_token)(const char *string, size_t *consumed_size,
                          void *user_data),
    const char *string, void *user_data, size_t *parsed_length) {

  char *lexeme;
  Bakoron_Tree *tree;

  if (start_symbol->type != BK_TERMINAL) {
    fprintf(stderr,
            "%s:%d: ERROR: cannot parse a non variable symbol with variable "
            "parsing\n",
            __FILE__, __LINE__);
    exit(1);
  }

  if (start_symbol->symbol != BK_EPSILON) {
    size_t skip_size;
    int next_token = _get_next_registered_token(
        bakoron, get_next_token, parsed_length, &skip_size, user_data, string);

    if (start_symbol->symbol != next_token)
      return NULL;

    lexeme = _strndup(string + skip_size, *parsed_length);
  } else {
    lexeme = NULL;
  }

  tree = (Bakoron_Tree *)malloc(sizeof(Bakoron_Tree));
  _tree_init(tree, start_symbol->symbol, lexeme, -1);

  return tree;
}

static Bakoron_Tree *_parse_string_with_rule(
    Bakoron *bakoron, Bakoron_Rule *rule, Bakoron_Symbol *start_symbol,
    int (*get_next_token)(const char *string, size_t *consumed_size,
                          void *user_data),
    const char *string, void *user_data, size_t *parsed_length) {
  int i;
  int string_length = strlen(string);
  int rule_succeded = 1;

  Bakoron_Tree *tree = (Bakoron_Tree *)malloc(sizeof(Bakoron_Tree));
  _tree_init(tree, start_symbol->symbol, NULL, rule->rule_descriptor);

  if (parsed_length != NULL)
    *parsed_length = 0;

  for (i = 0; i < arrlen(rule->children); ++i) {
    int child_symbol = rule->children[i];
    int child_symbol_index = hmget(bakoron->symbol_to_index_map, child_symbol);
    Bakoron_Symbol *child = &bakoron->symbols[child_symbol_index];

    size_t child_parsed_length;
    Bakoron_Tree *child_tree =
        _parse_string_recursive(bakoron, child, get_next_token, string,
                                user_data, &child_parsed_length);

    string += child_parsed_length;
    string_length -= child_parsed_length;

    if (child_tree == NULL || string_length < 0) {
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

static Bakoron_Tree *_parse_string_with_variable(
    Bakoron *bakoron, Bakoron_Symbol *start_symbol,
    int (*get_next_token)(const char *string, size_t *consumed_size,
                          void *user_data),
    const char *string, void *user_data, size_t *parsed_length) {

  int i;

  if (start_symbol->type != BK_VARIABLE) {
    fprintf(stderr,
            "%s:%d: ERROR: cannot parse a non variable symbol with variable "
            "parsing\n",
            __FILE__, __LINE__);
    exit(1);
  }

  for (i = 0; i < arrlen(start_symbol->rules); ++i) {
    Bakoron_Rule *rule = start_symbol->rules[i];
    Bakoron_Tree *tree =
        _parse_string_with_rule(bakoron, rule, start_symbol, get_next_token,
                                string, user_data, parsed_length);

    if (tree)
      return tree;
  }

  return NULL;
}

static Bakoron_Tree *_parse_string_recursive(
    Bakoron *bakoron, Bakoron_Symbol *start_symbol,
    int (*get_next_token)(const char *string, size_t *consumed_size,
                          void *user_data),
    const char *string, void *user_data, size_t *parsed_length) {

  if (parsed_length != NULL)
    *parsed_length = 0;

  if (start_symbol->type == BK_TERMINAL) {
    return _parse_string_with_terminal(bakoron, start_symbol, get_next_token,
                                       string, user_data, parsed_length);
  }

  else {
    return _parse_string_with_variable(bakoron, start_symbol, get_next_token,
                                       string, user_data, parsed_length);
  }
}

Bakoron_Tree *bakoron_parse_string(Bakoron *bakoron, int start_symbol,
                                   int (*get_next_token)(const char *string,
                                                         size_t *consumed_size,
                                                         void *user_data),
                                   const char *string, void *user_data) {

  int start_symbol_index = hmget(bakoron->symbol_to_index_map, start_symbol);
  Bakoron_Symbol *bk_start_symbol;
  Bakoron_Tree *tree;
  int remaining_token;
  size_t parsed_length;

  if (!_symbol_is_registered(bakoron, start_symbol)) {
    fprintf(stderr, "ERROR: Start symbol %d is not registered\n", start_symbol);
    exit(1);
  }

  /* TODO compute nullables and check if grammar is sane */
  _check_nullables(bakoron);
  {
    int i;
    for (i = 0; i < arrlen(bakoron->symbols); ++i) {
      if (bakoron->symbols[i].nullable) {
        printf("Nullable found - %d\n", bakoron->symbols[i].symbol);
      }
    }
  }

  bk_start_symbol = &bakoron->symbols[start_symbol_index];

  tree = _parse_string_recursive(bakoron, bk_start_symbol, get_next_token,
                                 string, user_data, &parsed_length);

  if (tree) {
    remaining_token = _get_next_registered_token(
        bakoron, get_next_token, NULL, NULL, user_data, string + parsed_length);

    if (remaining_token >= 0) {
      fprintf(stderr,
              "%s:%d: Syntax error: unexpected token %d at end of input\n",
              __FILE__, __LINE__, remaining_token);
      bakoron_cleanup_tree(tree);
      return NULL;
    }

    if (string[parsed_length] != '\0') {
      remaining_token =
          get_next_token(string + parsed_length, &parsed_length, user_data);

      if (_symbol_is_registered(bakoron, remaining_token)) {
      }
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

  if (tree->lexeme)
    free(tree->lexeme);

  for (i = 0; i < arrlen(tree->children); ++i) {
    bakoron_cleanup_tree(tree->children[i]);
  }

  arrfree(tree->children);
  free(tree);
}
