
all: chord

chord: chord.o
	mpicc -g $^ -o $@

run: chord
	mpirun --oversubscribe -np 5 $<

%.o: %.c
	mpicc -g -c $^ -o $@
