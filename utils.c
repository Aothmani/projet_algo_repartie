#include "utils.h"


void receive_addr_array(int tag, struct node_addr* addr_arr, int len)
{
	MPI_Status status;
	int i;
	int *data;
	data = malloc(sizeof(int) * 2 * len);
	
	MPI_Probe(MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status);
	MPI_Recv(data, len, MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD,
		 &status);
	
	for (i = 0; i < len; i++) {
		addr_arr[i].mpi = data[2 * i];
		addr_arr[i].chord = data[2 * i + 1];
	}

	free(data);
}


void send_addr_array(int dest, int tag, struct node_addr* addr_array,
			    int len)
{
	int *data;
	int i;
	data = malloc(sizeof(int) * 2 * len);
	for (i = 0; i < len; i++) {
		data[2 * i] = addr_array[i].mpi;
		data[2 * i + 1] = addr_array[i].chord;
	}
	MPI_Send(data, 2 * len, MPI_INT, dest, tag, MPI_COMM_WORLD);
	free(data);
}


/* A VERIFIER */
/* Returns 1 if a <= b, 0 otherwise */
int ring_compare(int a, int b, int mod)
{
	int n;
	if (b >= a)
		n = b - a;
	else
		n = b - a + mod;
	if (n >= 0 && n <= mod/2)
		return 1;
	else
		return 0;
}

