

all: chord

chord: chord.o utils.o
	mpicc -g $^ -lm -o $@

ex2: ex2.c utils.o election.o 
	mpicc -g $^ -lm -o $@

test: test.o chord.o utils.o
	mpicc -g $^ -o $@

run: chord
	mpirun --oversubscribe -np 7 $<

run-test:
	mpirun --oversubscribe -np 7 $<

run-ex2: ex2
	mpirun --oversubscribe -np 7 $<

%.o: %.c
	mpicc -g -c $^ -o $@
