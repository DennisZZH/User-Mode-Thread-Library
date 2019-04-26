thread_lib:
	gcc -c -o threads.o threads.c

sample_grader:
	gcc -o application autograder_main.c threads.o

clean:
	rm threads.o
	rm application
