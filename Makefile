
all: chord

chord: chord.o utils.o
	mpicc -g $^ -o $@

test: test.o chord.o utils.o
	mpicc -g $^ -o $@

run: chord
	mpirun --oversubscribe -np 5 $<

run-test:
	mpi-run --oversubscribe -np 5 $<

%.o: %.c
	mpicc -g -c $^ -o $@
