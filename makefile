all: sample_grader

sample_grader:autograder_main.c thread_lib 
	gcc autograder_main.c threads.o -o sample_grader

thread_lib:threads.c
	gcc -c threads.c -o threads.o

clean:
	rm threads.o
	rm sample_grader
