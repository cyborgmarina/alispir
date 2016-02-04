all: alispir.c
	gcc -std=c99 -Wall alispir.c mpc.c -ledit -lm -o alispir
clean: 
	$(RM) alispir
	$(RM) -r alispir.dSYM
