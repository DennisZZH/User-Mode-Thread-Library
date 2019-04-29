all: sample_grader

sample_grader:autograder_main.c thread_lib 
	g++ autograder_main.c threads.o -o sample_grader

thread_lib:threads.cpp
	g++ -c threads.cpp -o threads.o

clean:
	rm threads.o
	rm sample_grader
