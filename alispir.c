// Compiler flags: -std=c99 -Wall alispir.c mpc.c -ledit -lm -o alispir

#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>

#include "mpc.h"

#define DEBUG 0
#define IS_INT 0
#define IS_FLOAT 1

typedef struct eval_s {
	int typeof_num; 
	long i_num;
	float f_num;

} eval_s;

// Evaluate arithmetic operations
long eval_op(long x, char* op, long y) {
	if (strcmp(op, "add") == 0) { return x + y; }
	if (strcmp(op, "sub") == 0) { return x - y; }
	if (strcmp(op, "mul") == 0) { return x * y; }
	if (strcmp(op, "div") == 0) { return x / y; }
	if (strcmp(op, "mod") == 0) { return x % y; }
	return 0;
}

float eval_op_float(float x, char* op, float y) {
	if (strcmp(op, "add") == 0) { return x + y; }
	if (strcmp(op, "sub") == 0) { return x - y; }
	if (strcmp(op, "mul") == 0) { return x * y; }
	if (strcmp(op, "div") == 0) { return x / y; }
//	if (strcmp(op, "mod") == 0) { return (float) ((int) x % (int) y); }
	return 0;
}

eval_s eval_init_op(eval_s x, char* op, eval_s y) {
	eval_s result; 
	if(x.typeof_num == IS_INT && y.typeof_num == IS_INT) {
		result.typeof_num = IS_INT;
		result.i_num = eval_op(x.i_num, op, y.i_num);
	} else {
		result.typeof_num = IS_FLOAT;

		if(x.typeof_num == IS_INT) {
			x.typeof_num = IS_FLOAT;
			x.f_num = (float) x.i_num;
		}

		if(y.typeof_num == IS_INT) {
			y.typeof_num = IS_FLOAT;
			y.f_num = (float) y.i_num;
		}

		result.f_num = eval_op_float(x.f_num, op, y.f_num);
	}

	return result;

}

// Evaluate Syntax Tree
eval_s eval(mpc_ast_t* t) {

	if(strstr(t->tag, "integer")) {
		eval_s i_calc;
		i_calc.i_num = atoi(t->contents);
		i_calc.typeof_num = IS_INT;
		i_calc.f_num = 0;
		return i_calc;
	} 

	if(strstr(t->tag, "float")) {
		eval_s f_calc;
		f_calc.f_num = atof(t->contents);
		f_calc.typeof_num = IS_FLOAT;
		f_calc.i_num = 0;
		return f_calc;
	}

	// Assume the operator is the second child
	int op_count = 1;

	// If it is the third child, set counter to 2
	if(strstr(t->children[2]->tag, "operator")) {
		op_count = 2;
	}

	// Assign operator to op variable
	char* op = t->children[op_count]->contents;

	// Increase counter, to get the child abstract syntax tree (ast)
	op_count++;
	eval_s x = eval(t->children[op_count]);

	// Evaluate every child expression
	int i = op_count + 1;
	while(strstr(t->children[i]->tag, "expr")) {
		x = eval_init_op(x, op, eval(t->children[i]));
		i++;
	}

	return x;
}

int main(int argc, char** argv) {

  mpc_parser_t* Integer = mpc_new("integer");
  mpc_parser_t* Float = mpc_new("float");
  mpc_parser_t* Operator = mpc_new("operator");
  mpc_parser_t* Expr = mpc_new("expr");
  mpc_parser_t* Lispy = mpc_new("lispy");

  // Grammar
  // e.g. Alispir expression: (add mul(4 4) 2)
  mpca_lang(MPCA_LANG_DEFAULT,
    "                                                               \
      integer  : /-?[0-9]+/ ;                             	    \
      float    : /-?[0-9]+(\\.[0-9])+/ ;                            \
      operator : /add/ | /sub/ | /mul/ | /div/ | /mod/;    	    \
      expr     : <float> | <integer> | '(' <operator> <expr>+ ')';  \
      lispy    : /^/ '(' <operator> <expr>+ ')' /$/ ;               \
    ",
    Integer, Float, Operator, Expr, Lispy);

  puts("Alispir Version 0.3");
  puts("Press Ctrl+C to Exit");

  // REPL
  while(1) {

    char* input = readline("alispir> ");

    add_history(input);

    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
	/* on DEBUG mode, it prints the whole tree */
	if(!DEBUG) {
		eval_s result = eval(r.output);
		if(result.typeof_num == IS_INT) {
			printf("%li\n", result.i_num);
		} else {
			printf("%f\n", result.f_num);
		}
	} else {
		mpc_ast_print(r.output);
	}
    	mpc_ast_delete(r.output);
    } else {
    	mpc_err_print(r.error);
    	mpc_err_delete(r.error);
    }

    free(input);

    }

    mpc_cleanup(5, Integer, Float, Operator, Expr, Lispy);
    return 0;

}
