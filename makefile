# Haobo Xing 445434
# Yue Hu 444472
# Xueting Yu 447920
# Taoran Yin 445154
# CSE 422 Operating Systems Project 1
# compile: make xsh
# run: ./xsh

xsh: main.c commandParser.c pipe.c signo.c build_in.c shell.h 
	gcc -std=gnu11 -Wall -g -o xsh main.c commandParser.c pipe.c signo.c build_in.c

clean:
	rm -f *.o
