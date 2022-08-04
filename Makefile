mysh: mysh.c
	gcc -g -o mysh mysh.c -Wall -Werror

clean: 
	rm -f mysh