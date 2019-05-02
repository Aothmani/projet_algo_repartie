#include "utils.h"


int in_interval(int n, int a, int b, int k)
{
	if (a < b)
		return (n >= a && n < b);
	else
		return ((n >= a && n < k) || (n >= 0 && n < b));
}

void receive_addr(int tag, struct node_addr* addr)
{
	int data[2];
	MPI_Status status;
	MPI_Recv(data, 2, MPI_INT, tag, MPI_ANY_SOURCE, MPI_COMM_WORLD,
		 &status);
	addr->mpi = data[0];
	addr->chord = data[1];
}

void send_addr(int dest, int tag, struct node_addr* addr)
{
	int data[2];
	data[0] = addr->mpi;
	data[1] = addr->chord;
	MPI_Send(data, 2, MPI_INT, dest, tag, MPI_COMM_WORLD);
}

void receive_addr_array(int tag, struct node_addr* addr_arr, int *len)
{
	MPI_Status status;
	int i;
	int *data;
	data = malloc(sizeof(int) * *len);
	
	MPI_Probe(MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status);
	MPI_Recv(data, *len, MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD,
		 &status);

	*len /= 2;
	for (i = 0; i < *len; i++) {
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
int ring_compare(int a, int b, int m)
{
	int n;
	if (b >= a)
		n = b - a;
	else
		n = b - a + m;
	if (n >= 0 && n <= m/2)
		return 1;
	else
		return 0;
}

