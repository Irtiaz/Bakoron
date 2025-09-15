#include "bakoron.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define ARR_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))

#define LOG_ERROR(format, ...)                                                 \
  fprintf(stderr, __FILE__ ":%d:" format, __LINE__, __VA_ARGS__)

enum { EXPRESSION, NUMBER, PLUS };
enum { EXPRESSION__NUMBER_PLUS_NUMBER };

int evaluate_tree(BK_Tree *tree, char **lexemes) {
  if (tree->symbol == EXPRESSION) {
    assert(tree->number_of_children == 3);
    assert(tree->content.rule_descriptor == 0);

    return evaluate_tree(tree->content.children[0], lexemes) +
           evaluate_tree(tree->content.children[2], lexemes);
  }

  else if (tree->symbol == NUMBER) {
    assert(tree->number_of_children == 0);

    char *lexeme = lexemes[tree->content.index];
    int value = atoi(lexeme);

    return value;
  }

  LOG_ERROR(
      "can't evaluate symbol that is neither EXPRESSION nor NUMBER - %d\n",
      tree->symbol);
  exit(1);
}

int main(void) {
  BK_Parser *parser = bk_parser();

  bk_rule(parser, EXPRESSION__NUMBER_PLUS_NUMBER, 4, EXPRESSION, NUMBER, PLUS, NUMBER);

  int token[] = {NUMBER, PLUS, NUMBER};
  char *lexemes[] = {"12", "+", "3"};

  BK_Tree *tree = bk_tree(parser, EXPRESSION, token, ARR_COUNT(token));

  int result = evaluate_tree(tree, lexemes);
  printf("Result: %d\n", result);

  bk_tree_destroy(tree);
  bk_parser_destroy(parser);
  return 0;
}
