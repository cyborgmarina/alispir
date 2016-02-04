#build with -g flag for debug info
all: alispir.c
	gcc -std=c99 -Wall alispir.c mpc.c -ledit -lm -g -o alispir
clean: 
	$(RM) alispir
	$(RM) -r alispir.dSYM
