#include "bakoron.h"

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>

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

struct BK_Parser {
  Symbol **symbols;
};

typedef struct {
  BK_Tree *tree;
  size_t end_marker;
} Parse_Result;

static bool symbol_is_terminal(Symbol *symbol) { return symbol->rules == NULL; }

static Parse_Result **parse_recursive(BK_Parser *parser, Symbol *symbol,
                                      size_t parse_from, int *tokens,
                                      size_t tokens_length);

static BK_Tree *tree_term_init(Symbol *term) {
  assert(symbol_is_terminal(term));
  BK_Tree *tree = malloc(sizeof(BK_Tree));
  tree->number_of_children = 0;
  tree->symbol = term->number;
  tree->content.index = term->index;
  return tree;
}

static BK_Tree *tree_rule_init(Rule *rule) {
  BK_Tree *tree = malloc(sizeof(BK_Tree));
  tree->number_of_children = arrlen(rule->rule_body);
  tree->content.children = NULL;
  tree->content.rule_descriptor = rule->rule_descriptor;
  return tree;
}

static Parse_Result *tree_with_end_marker(BK_Tree *tree, size_t end_marker) {
  Parse_Result *result = (Parse_Result *)malloc(sizeof(Parse_Result));

  result->tree = tree;
  result->end_marker = end_marker;

  return result;
}

static Symbol *get_symbol_from_number(BK_Parser *parser, int number) {
  for (int i = 0; i < arrlen(parser->symbols); ++i) {
    if (parser->symbols[i]->number == number)
      return parser->symbols[i];
  }
  return NULL;
}

// Returns true if added or false if it was already there
static bool add_symbol(BK_Parser *parser, int symbol_number) {
  Symbol *symbol = get_symbol_from_number(parser, symbol_number);
  if (symbol != NULL)
    return false;

  symbol = (Symbol *)malloc(sizeof(Symbol));
  symbol->rules = NULL;
  symbol->number = symbol_number;

  arrput(parser->symbols, symbol);
  return true;
}

BK_Parser *bk_parser(void) {
  BK_Parser *parser = (BK_Parser *)malloc(sizeof(BK_Parser));

  parser->symbols = NULL;
  add_symbol(parser, BK_EPSILON);
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

static Rule *create_rule(BK_Parser *parser, Symbol *variable,
                         int rule_descriptor, int *rule_body,
                         int rule_body_length) {
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

static void add_rule(BK_Parser *parser, int rule_descriptor, int *rule_raw,
                     int rule_length) {
  Symbol *variable = get_symbol_from_number(parser, rule_raw[0]);
  Rule *rule = create_rule(parser, variable, rule_descriptor, rule_raw + 1,
                           rule_length - 1);

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

static Parse_Result **parse_terminal(Symbol *terminal, size_t parse_from,
                                     int *tokens) {
  int token = tokens[parse_from];

  if (terminal->number == BK_EPSILON || terminal->number == token) {
    Parse_Result **results = NULL;

    BK_Tree *terminal_tree = tree_term_init(terminal);
    Parse_Result *terminal_result = tree_with_end_marker(
        terminal_tree,
        terminal->number == BK_EPSILON ? parse_from : parse_from + 1);

    arrput(results, terminal_result);
    return results;
  }

  else
    return NULL;
}

static Parse_Result *merge_result(Parse_Result *left, Parse_Result *right) {
  for (int i = 0; i < arrlen(right->tree->content.children); ++i) {
    arrput(left->tree->content.children, right->tree->content.children[i]);
  }
  arrfree(right->tree->content.children);
  free(right->tree);
  free(right);
  return left;
}

static Parse_Result **parse_subrule(BK_Parser *parser, Rule *rule, size_t begin,
                                    size_t parse_from, int *tokens,
                                    size_t tokens_length) {
  Symbol *first_symbol = rule->rule_body[begin];
  Parse_Result **first =
      parse_recursive(parser, first_symbol, parse_from, tokens, tokens_length);

  Parse_Result **left = NULL;
  for (int i = 0; i < arrlen(first); ++i) {
    BK_Tree *root = tree_rule_init(rule);
    arrput(root->content.children, first[i]->tree);
    arrput(left, tree_with_end_marker(root, first[i]->end_marker));
  }

  if (begin == (size_t)arrlen(rule->rule_body) - 1) {
    return left;
  }

  Parse_Result **result = NULL;

  for (int i = 0; i < arrlen(left); ++i) {
    Parse_Result **right = parse_subrule(
        parser, rule, begin + 1, left[i]->end_marker, tokens, tokens_length);
    for (int j = 0; j < arrlen(right); ++j) {
      Parse_Result *merged = merge_result(left[i], right[j]);
      arrput(result, merged);
    }
  }
  return result;
}

static Parse_Result **parse_variable(BK_Parser *parser, Symbol *variable,
                                     size_t parse_from, int *tokens,
                                     size_t tokens_length) {}

static Parse_Result **parse_recursive(BK_Parser *parser, Symbol *symbol,
                                      size_t parse_from, int *tokens,
                                      size_t tokens_length) {
  if (symbol_is_terminal(symbol)) {
    return parse_terminal(symbol, parse_from, tokens);
  } else {
    return parse_variable(parser, symbol, parse_from, tokens, tokens_length);
  }
}

BK_Tree *bk_tree(BK_Parser *parser, int start_symbol, int *tokens,
                 size_t tokens_length);
void bk_tree_destroy(BK_Tree *tree);
