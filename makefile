# compile: make xsh
# run: ./xsh

xsh: main.c commandParser.c pipe.c signo.c build_in.c shell.h 
	gcc -std=gnu11 -Wall -g -o xsh main.c commandParser.c pipe.c signo.c build_in.c

clean:
	rm -f *.o
