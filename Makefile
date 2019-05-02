

all: chord

chord: chord.o utils.o
	mpicc -g $^ -lm -o $@

ex2: ex2.o utils.o election.o 
	mpicc -g $^ -lm -o $@

test: test.o chord.o utils.o
	mpicc -g $^ -o $@

test_ex2: test_ex2.o utils.o election.o
	mpicc -g $^ -o $@

run: chord
	mpirun --oversubscribe -np 9 $<

run-test: test_ex2
	mpirun --oversubscribe -np 2 $<

run-ex2: ex2
	mpirun --oversubscribe -np 9 $<

%.o: %.c
	mpicc -g -c $^ -o $@
