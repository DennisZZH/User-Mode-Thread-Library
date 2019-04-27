all: sample_grader

sample_grader:autograder_main.c thread_lib 
	gcc autograder_main.c threads.o -o sample_grader

thread_lib:sample_pthread.c
	gcc -c sample_pthread.c -o threads.o

test:
	gcc -o test test.c threads.o

clean:
	rm threads.o
	rm application
