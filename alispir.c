// Compiler flags: -std=c99 -Wall alispir.c mpc.c -ledit -lm -o alispir

#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>

#include "mpc.h"

int main(int argc, char** argv) {

  mpc_parser_t* Number = mpc_new("number");
  mpc_parser_t* Operator = mpc_new("operator");
  mpc_parser_t* Expr = mpc_new("expr");
  mpc_parser_t* Lispy = mpc_new("lispy");

  // Grammar
  // e.g. Alispir expression: (add 1 2)
  mpca_lang(MPCA_LANG_DEFAULT,
    "                                                                 \
      number   : /-?[0-9]+/ ;                             	    \
      operator : /add/ | /sub/ | /mul/ | /div/ | /mod/ | /pow/;      \
      expr     : <number> | '(' <operator> <expr>+ ')' ;              \
      lispy    : /^/ <expr>+ /$/ ;                         \
    ",
    Number, Operator, Expr, Lispy);

  puts("Alispir Version 0.1");
  puts("Press Ctrl+C to Exit");

  // REPL
  while(1) {

    char* input = readline("alispir> ");

    add_history(input);

    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
    	mpc_ast_print(r.output);
    	mpc_ast_delete(r.output);
    } else {
    	mpc_err_print(r.error);
    	mpc_err_delete(r.error);
    }

    free(input);

    }

    mpc_cleanup(4, Number, Operator, Expr, Lispy);
    return 0;

}
