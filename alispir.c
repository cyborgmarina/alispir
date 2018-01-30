#include <stdio.h>
#include <stdlib.h>
#include <editline/readline.h>
#include "mpc.h"
#include "alispir.h"

#define DEBUG 0

// Print result of evaluation
void lval_print (lval v) 
{
	switch (v.type) {
	case LVAL_INT: 
		printf("%li", v.i_num); 
		break;
	case LVAL_FLOAT: 
		printf("%f", v.f_num); 
		break;
	case LVAL_ERROR:
		if (v.i_num == LERR_DIV_ZERO)
			printf("Error %li: Division by Zero", v.i_num);
		if (v.i_num == LERR_BAD_OP)
			printf("Error %li: Invalid Operator", v.i_num);
		if (v.i_num == LERR_BAD_NUM)
			printf("Error %li: Invalid Number", v.i_num);
		break;
	}	
}

// Print result of evaluation followed by a new line
void lval_println (lval v) {
	lval_print(v); putchar('\n');
};

// Parse lval number
lval lval_num(int type, mpc_ast_t* t) 
{
	lval new_lval;
	switch (type) {
	case LVAL_INT: 
		new_lval.type = LVAL_INT;
		new_lval.i_num = atoi(t->contents);
		return new_lval;
		break;
	case LVAL_FLOAT:
		new_lval.type = LVAL_FLOAT;
		new_lval.f_num = atof(t->contents);
		return new_lval;
		break;
	default:
		new_lval.type = LVAL_ERROR;
		new_lval.i_num = LERR_BAD_NUM;
		return new_lval;
		break;
	}
}	

// Evaluate arithmetic operations
long eval_op(long x, char* op, long y) 
{
	if (strcmp(op, "sum") == 0) 
		return x + y;
	if (strcmp(op, "sub") == 0)
		return x - y;
	if (strcmp(op, "mul") == 0)
		return x * y;
	if (strcmp(op, "div") == 0)
		return x / y;
	if (strcmp(op, "mod") == 0) 
		return x % y;
	return 0;
}

float eval_op_float(float x, char* op, float y) 
{
	if (strcmp(op, "sum") == 0) 
		return x + y; 
	if (strcmp(op, "sub") == 0) 
		return x - y; 
	if (strcmp(op, "mul") == 0)  
		return x * y; 
	if (strcmp(op, "div") == 0)  
		return x / y; 
	if (strcmp(op, "mod") == 0)  
		return (float) ((int) x % (int) y); 
	return 0;
}

// Check for errors 
lval eval_error(lval x, char* op, lval y) 
{
	lval result;

	if (x.type == LVAL_ERROR)
		return x;
	

	if (y.type == LVAL_ERROR) 
		return y;
	
	
	if (strcmp(op, "div") == 0) {
		if ((y.type == LVAL_FLOAT && y.f_num == 0) 
		|| (y.type == LVAL_INT && y.i_num == 0)) {
			result.type = LVAL_ERROR;
			result.i_num = LERR_DIV_ZERO;
		} 
	}
	
	return result;
}

lval eval_init_op(lval x, char* op, lval y) 
{
	lval result = eval_error(x, op, y); 
	if (result.type == LVAL_ERROR) 
		return result;

	if (x.type == LVAL_INT && y.type == LVAL_INT) {
		result.type = LVAL_INT;
		result.i_num = eval_op(x.i_num, op, y.i_num);
	} else {
		result.type = LVAL_FLOAT;
		if (x.type == LVAL_INT) {
			x.type = LVAL_FLOAT;
			x.f_num = (float) x.i_num;
		}
		if (y.type == LVAL_INT) {
			y.type = LVAL_FLOAT;
			y.f_num = (float) y.i_num;
		}
		result.f_num = eval_op_float(x.f_num, op, y.f_num);
	}

	return result;
}

// Evaluate Syntax Tree
lval eval(mpc_ast_t* t) 
{
	if (strstr(t->tag, "integer"))
		return lval_num(LVAL_INT, t);

	if (strstr(t->tag, "float"))
		return lval_num(LVAL_FLOAT, t);

	// Assume the operator is the second child
	int op_count = 1;

	if (strstr(t->children[2]->tag, "operator"))
		op_count = 2;
	

	// Assign operator to op variable
	char* op = t->children[op_count]->contents;

	// Increase counter, to get the child abstract syntax tree (ast)
	op_count++;
	lval x = eval(t->children[op_count]);

	// Evaluate every child expression
	int i = op_count + 1;
	while (strstr(t->children[i]->tag, "expr")) {
		x = eval_init_op(x, op, eval(t->children[i]));
		i++;
	}

	if (x.type == LVAL_ERROR)
		return x;

	return x;
}



int main(int argc, char** argv) 
{
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
      operator : /sum/ | /sub/ | /mul/ | /div/ | /mod/;    	    \
      expr     : <float> | <integer> | '(' <operator> <expr>+ ')';  \
      lispy    : /^/ '(' <operator> <expr>+ ')' /$/ ;               \
    ",
    Integer, Float, Operator, Expr, Lispy);

  puts("Alispir Version 0.3");
  puts("Press Ctrl+C to Exit");

  // REPL
  while (1) {

    char* input = readline("alispir> ");

    add_history(input);

    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
	    lval result = eval(r.output);
	    lval_println(result);
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

