#include "bakoron.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>

#include "stb_ds.h"

typedef struct Symbol Symbol;

typedef struct {
  Symbol *variable;
  int rule_descriptor;
  Symbol **rule_body;
} Rule;

struct Symbol {
  size_t index;
  int number;

  Rule **rules;
};

typedef struct Tree Tree;

struct Tree {
  Symbol *symbol;
  union {
    struct {
      Tree **children;
      int rule_descriptor;
    };
    size_t index;
  } content;
  size_t number_of_children;
};

struct BK_Parser {
  Symbol **symbols;
};

BK_Parser *bk_parser(void) {
  BK_Parser *parser = (BK_Parser *)malloc(sizeof(BK_Parser));

  parser->symbols = NULL;
  return parser;
}

void bk_parser_destroy(BK_Parser *parser) {
  for (int i = 0; i < arrlen(parser->symbols); ++i) {

    Symbol *symbol = parser->symbols[i];
    for (int j = 0; j < arrlen(symbol->rules); ++j) {
      arrfree(symbol->rules[j]->rule_body);
      free(symbol->rules[j]);
    }
    arrfree(symbol->rules);

    free(symbol);
  }
  arrfree(parser->symbols);

  free(parser);
}

static Symbol *get_symbol_from_number(BK_Parser *parser, int number) {
  for (int i = 0; i < arrlen(parser->symbols); ++i) {
    if (parser->symbols[i]->number == number) return parser->symbols[i];
  }
  return NULL;
}

// Tries to add a symbol and returns true if added or false if it was already there
static bool add_symbol(BK_Parser *parser, int symbol_number) {
  Symbol *symbol = get_symbol_from_number(parser, symbol_number);
  if (symbol != NULL) return false;

  symbol = (Symbol *)malloc(sizeof(Symbol));
  symbol->rules = NULL;
  symbol->number = symbol_number;
  symbol->index = arrlen(parser->symbols);

  arrput(parser->symbols, symbol);
  return true;
}

static Rule *create_rule(BK_Parser *parser, Symbol *variable, int rule_descriptor, int *rule_body, int rule_body_length) {
  Rule *rule = (Rule *)malloc(sizeof(Rule));
  rule->variable = variable;
  rule->rule_descriptor = rule_descriptor;
  rule->rule_body = NULL;

  for (int i = 0; i < rule_body_length; ++i) {
    Symbol *symbol = get_symbol_from_number(parser, rule_body[i]);
    arrput(rule->rule_body, symbol);
  }

  return rule;
}

static void add_rule(BK_Parser *parser, int rule_descriptor, int *rule_raw, int rule_length) {
  Symbol *variable = get_symbol_from_number(parser, rule_raw[0]);
  Rule *rule = create_rule(parser, variable, rule_descriptor, rule_raw + 1, rule_length - 1);

  arrput(variable->rules, rule);
}

void bk_rule(BK_Parser *parser, int rule_descriptor, int rule_length, ...) {
  va_list args;
  va_start(args, rule_length);
  bk_rule_v(parser, rule_descriptor, rule_length, args);
  va_end(args);
}

void bk_rule_v(BK_Parser *parser, int rule_descriptor, int rule_length,
               va_list args) {
  int *rule = malloc(rule_length * sizeof(int));
  for (int i = 0; i < rule_length; ++i) {
    rule[i] = va_arg(args, int);
  }
  bk_rule_a(parser, rule_descriptor, rule_length, rule);
  free(rule);
}

void bk_rule_a(BK_Parser *parser, int rule_descriptor, int rule_length,
               int *rule) {

  assert(rule_length > 0);

  for (int i = 0; i < rule_length; ++i) {
    add_symbol(parser, rule[i]);
  }

  add_rule(parser, rule_descriptor, rule, rule_length);
}

BK_Tree *bk_tree(BK_Parser *parser, int start_symbol, int *tokens,
                 size_t tokens_length);
void bk_tree_destroy(BK_Tree *tree);
