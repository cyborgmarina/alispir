enum { LVAL_INT, LVAL_FLOAT, LVAL_ERROR }; // Enum of possible lval types 
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM }; // Enum of possible error types

// Evaluation structure
typedef struct lval {
	int type; 
	long i_num;
	float f_num;

} lval;


