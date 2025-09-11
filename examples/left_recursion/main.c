#include <stdio.h>
#include <stdlib.h>
#include "bakoron.h"

typedef enum {
  NUMBER,

  VARIABLE_MARKER,

  DIGIT,

  RULED_TERMINAL,
  
  UNRULED_TERMINAL,

  WHITESPACE
} Symbol;

typedef int (*Evaluator)(Bakoron_Tree *tree);
typedef struct {
  Symbol symbol;
  Symbol children[100];
  size_t children_count;

  Evaluator evaluate_tree;
} Rule;
int evaluate_tree(Bakoron_Tree *tree);


int number__number_digit(Bakoron_Tree *tree) {
  return evaluate_tree(tree->children[0]) * 10 + evaluate_tree(tree->children[1]);
}

int number__epsilon(Bakoron_Tree *tree) {
  (void)tree;
  return 0;
}

int tree_to_digit(Bakoron_Tree *tree) {
  return tree->lexeme[0] - '0';
}

Rule rules[] = {
  {
    NUMBER,
    {NUMBER, DIGIT}, 2,
    number__number_digit
  },

  {
    NUMBER,
    {BK_EPSILON}, 1,
    number__epsilon
  }
};

Evaluator terminal_evaluators[] = {
  tree_to_digit
};

int is_whitespace(char c) {
  switch (c) {
  case ' ':
  case '\t':
  case '\r':
  case '\n':
    return 1;
  }

  return 0;
}

int get_next_token(const char *string, size_t *consumed_size, void *user_data) {
  (void)user_data;

  if (string[0] >= '0' && string[0] <= '9') {
    *consumed_size = 1;
    return DIGIT;
  }

  else if (is_whitespace(string[0])) {
    *consumed_size = 0;

    do {
      ++*consumed_size;
      ++string;
    } while (is_whitespace(string[0]));
    
    return WHITESPACE;
  }

  fprintf(stderr, "%s:%d: Cannot get next token from string %s\n", __FILE__, __LINE__, string);
  exit(1);
}


int evaluate_tree(Bakoron_Tree *tree) {
  int symbol = tree->symbol;

  if (symbol < VARIABLE_MARKER) {
    return rules[tree->rule_descriptor].evaluate_tree(tree);
  }

  else if (symbol < RULED_TERMINAL) {
    int terminal_evaluators_count = sizeof(terminal_evaluators) / sizeof(terminal_evaluators[0]);
    int evaluator_index = symbol - VARIABLE_MARKER - 1;

    if (evaluator_index < terminal_evaluators_count) {
      return terminal_evaluators[evaluator_index](tree);
    }
  }

  fprintf(stderr, "%s:%d: No specification for how to evaluate symbol %d with rule_descriptor %d\n", __FILE__, __LINE__, symbol, tree->rule_descriptor);
  exit(1);
}

int main(void) {
  Bakoron bakoron;
  Bakoron_Tree *tree;
  size_t i;

  const char *input = "123";

  bakoron_init(&bakoron);

  for (i = 0; i < VARIABLE_MARKER; ++i) {
    bakoron_register_symbol(&bakoron, i, BK_VARIABLE);
  }
  for (i = VARIABLE_MARKER + 1; i < UNRULED_TERMINAL; ++i) {
    if (i == RULED_TERMINAL) continue;
    bakoron_register_symbol(&bakoron, i, BK_TERMINAL);
  }

  for (i = 0; i < sizeof(rules) / sizeof(rules[0]); ++i) {
    Rule rule = rules[i];
    bakoron_register_rule(&bakoron, rule.symbol, i, (int *)&rule.children, rule.children_count);
  }

  tree = bakoron_parse_string(&bakoron, NUMBER, get_next_token, input, NULL);

  if (tree == NULL) {
    fprintf(stderr, "%s:%d: ERROR: Syntax error\n", __FILE__, __LINE__);
  }

  else {
    printf("Result: %d\n", evaluate_tree(tree));
    bakoron_cleanup_tree(tree);
  }

  bakoron_cleanup(&bakoron);

  return 0;
}
