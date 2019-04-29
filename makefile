all: sample_grader mytest

sample_grader:autograder_main.c thread_lib 
	g++ autograder_main.c threads.o -g -o sample_grader

mytest:mytest.cpp thread_lib
	g++ mytest.cpp threads.o -g -o mytest

thread_lib:threads.cpp
	g++ -g -c threads.cpp -o threads.o

clean:
	rm threads.o
	rm mytest
	rm sample_grader
