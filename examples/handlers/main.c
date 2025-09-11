#include <stdio.h>
#include <stdlib.h>
#include "bakoron.h"

typedef enum {
  EXPRESSION,

  VARIABLE_MARKER,

  NUMBER,

  RULED_TERMINAL,
  
  PLUS,
  TIMES,

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


int expression__number_plus_number(Bakoron_Tree *tree) {
  return evaluate_tree(tree->children[0]) + evaluate_tree(tree->children[2]);
}

int expression__number_times_number(Bakoron_Tree *tree) {
  return evaluate_tree(tree->children[0]) * evaluate_tree(tree->children[2]);
}

int tree_to_number(Bakoron_Tree *tree) {
  return atoi(tree->lexeme);
}

Rule rules[] = {
  {
    EXPRESSION,

    {NUMBER, PLUS, NUMBER}, 3,

    expression__number_plus_number
  },
  {
    EXPRESSION,

    {NUMBER, TIMES, NUMBER}, 3,

    expression__number_times_number
  }
};

Evaluator terminal_evaluators[] = {
  tree_to_number
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

  if (string[0] == '+') {
    *consumed_size = 1;
    return PLUS;
  }

  else if (string[0] == '*') {
    *consumed_size = 1;
    return TIMES;
  }

  else if (string[0] >= '0' && string[0] <= '9') {
    *consumed_size = 0;
    do {
      ++*consumed_size;
      ++string;
    } while (string[0] >= '0' && string[0] <= '9');

    return NUMBER;
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

  const char *input = "20 * 19";

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

  tree = bakoron_parse_string(&bakoron, EXPRESSION, get_next_token, input, NULL);

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
