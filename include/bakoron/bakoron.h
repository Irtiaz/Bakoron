#include <stddef.h>

typedef struct Bakoron_Symbol Bakoron_Symbol;

typedef struct {
  Bakoron_Symbol *symbols;

  struct { int key; int value; } *symbol_to_index_map;
} Bakoron;

typedef struct Bakoron_Tree_Struct Bakoron_Tree;

struct Bakoron_Tree_Struct {
  int symbol;
  char *lexeme;
  int rule_descriptor;
  
  Bakoron_Tree **children;
  int number_of_children;
};

typedef enum { BK_VARIABLE, BK_TERMINAL } Bakoron_Symbol_Type;


void bakoron_init(Bakoron *bakoron);

void bakoron_register_symbol(Bakoron *bakoron, int symbol,
                             Bakoron_Symbol_Type type);


void bakoron_register_rule(Bakoron *bakoron, int symbol, int rule_descriptor, int *rule,
                           size_t rule_length);

Bakoron_Tree *bakoron_parse_string(Bakoron *bakoron, int start_symbol,
                                   int (*get_next_token)(const char *string,
                                                            size_t *token,
                                                            void *user_data),
                                   const char *string, void *user_data);

void bakoron_cleanup(Bakoron *bakoron);

void bakoron_cleanup_tree(Bakoron_Tree *tree);
