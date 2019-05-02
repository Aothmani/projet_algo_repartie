#include <assert.h>
#include <stdio.h>
#include <mpi.h>

#include "utils.h"

void test_send(void) {
	struct node_addr addr;
	addr.mpi = 1;
	addr.chord = 2;

	send_addr(1, TAGINIT, &addr);
}

void test_receive(void) {
	struct node_addr addr;
	receive_addr(TAGINIT, &addr);

	printf("(%d, %d)\n", addr.mpi, addr.chord);
	assert(addr.mpi == 1 && addr.chord == 2);
}

void test_send_array(void) {
	int size = 2;
	struct node_addr addr_array[2];
	addr_array[0].mpi = 1;
	addr_array[0].chord = 11;
	addr_array[1].mpi = 2;
	addr_array[1].chord = 12;
	send_addr_array(1, TAGINIT, addr_array, size);
}

void test_receive_array(void) {
	struct node_addr addr_array[2];
	MPI_Status status;
	int size;
	int i;
	
	MPI_Probe(MPI_ANY_SOURCE, TAGINIT, MPI_COMM_WORLD, &status);
	MPI_Get_count(&status, MPI_INT, &size);
	
	receive_addr_array(TAGINIT, addr_array, &size);

	printf("size = %d\n", size);
	assert(size == 2);	

	printf("tab = [");
	for (i = 0; i < size; i++) {
		printf("(%d, %d), ", addr_array[i].mpi, addr_array[i].chord);
	}
	printf("]\n");
	
	assert(addr_array[0].mpi == 1 && addr_array[0].chord == 11
	       && addr_array[1].mpi == 2 && addr_array[1].chord == 12);
}


void test_in_interval(void)
{
	int k = 64;
	int a = 1, b = 2, c = 34, d = 63;

	assert(in_interval(2, a, b, k));
	assert(in_interval(12, a, c, k));
	assert(in_interval(43, b, a, k));
	assert(!in_interval(12, c, a, k));
	assert(in_interval(12, a, d, k));
	assert(in_interval(63, a, d, k));
}

int main(int argc, char *argv[])
{
	int proc_count, rank;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &proc_count);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	if (rank == 0) {
		//test_send();
		test_send_array();
	}
	else {
		//test_receive();
		test_receive_array();
		test_in_interval();
	}
	
	  
	MPI_Finalize();
	return 0;
}
