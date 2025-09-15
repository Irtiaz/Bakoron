#ifndef BAKORON_H_43543
#define BAKORON_H_43543

#include <stdio.h>

typedef struct BK_Parser BK_Parser;

typedef struct BK_Tree BK_Tree;

struct BK_Tree {
  int symbol;

  union {
    BK_Tree **children;
    size_t index;
  } content;

  size_t number_of_children;
};

BK_Parser *bk_parser(void);
void bk_parser_destroy(BK_Parser *parser);

void bk_rule(BK_Parser *parser, int rule_length, ...);
void bk_rule_v(BK_Parser *parser, int rule_length, va_list rule);
void bk_rule_a(BK_Parser *parser, int rule_length, int *rule);

BK_Tree *bk_tree(BK_Parser *parser, int start_symbol, int *tokens, size_t tokens_length);
void bk_tree_destroy(BK_Tree *tree);

#endif

