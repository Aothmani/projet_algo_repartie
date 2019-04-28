#ifndef CHORD_CONSTANTS
#define CHORD_CONSTANTS

#include <mpi.h>
#include <stdlib.h>

enum Tags {
	TAGINIT, /* Sent by the simulator to define the variables */
	TAGSEARCH, /* Searching a value in the hash table */
	TAGFOUND, /* Sending back the result of the search */
	TAGELECT, /* Choosing a leader for the election */
	TAGANN, /* Annoucing the leader at the end of the election */
	TAGTAB, /* Collecting all the node ids */
	TAGTABANN, /* Broadcasting the completed table with all the node ids */
	TAGTELLFINGER, /* Searching for a finger */
	TAGTELLFINGERRESP, /* Response to TAGTELLFINGER from the new finger */
	TAGUPDATE, /* Tell a node to update its fingers in regards to the newly inserted node */
	TAGUPDATERESP, /* Response to TAGTELLFINGER, contains the new finger */
	TAGINSERT, /* Insert a node in the ring */
	TAGINSERTRESP, /* Response of TAGINSERT, indicate that the node has been inserted in the ring */
	TAGTERM, /* Terminate a node */
};

/* SEND_INT - send an integer to a given node 
 * @dest MPI rank of destination node
 * @tag message tag
 * @data integer
 */
#define SEND_INT(dest, tag, data) MPI_Send((void*)&data, 1, MPI_INT, dest, tag, MPI_COMM_WORLD)

/* SEND_NINT - send integer array to a given node
 * @dest MPI rank of destination node
 * @tag message tag
 * @data integer array
 * @n array length
 */
#define SEND_NINT(dest, tag, data, n) MPI_Send((void*)data, n, MPI_INT, dest, tag, MPI_COMM_WORLD)


#define NEXT(rank, size) ((rank + 1) % size)
#define PREV(rank, size) ((rank - 1 + size) % size)

struct node_addr {
	int mpi;
	int chord;
};

/* struct array - resizeable array */
struct array {
	size_t size;
	struct node_addr data[0];
};

inline void receive_addr(int tag, struct node_addr* addr)
{
	int data[2];
	MPI_Status status;
	MPI_Recv(data, 2, MPI_INT, tag, MPI_ANY_SOURCE, MPI_COMM_WORLD,
		 &status);
	addr->chord = data[0];
	addr->mpi = data[1];
}

inline void send_addr(int dest, int tag, struct node_addr* addr)
{
	int data[2];
	data[0] = addr->mpi;
	data[1] = addr->chord;
	MPI_Send(data, 2, MPI_INT, dest, tag, MPI_COMM_WORLD);
}

void receive_addr_array(int tag, struct node_addr* addr_arr, int len);
void send_addr_array(int dest, int tag, struct node_addr* addr_array,
		     int len);

/* struct node - description of a chord node */
struct node {
	int mpi_rank; 
	int rank; /* id in the ring */
	int next; /* chord id of the next node in the ring */
	int leader; /* 1 if leader, 0 otherwise */
	struct array *fingers;
};

/*
 * find_finger - Determines the finger to which the request should be 
 * transmitted to
 * @fingers finger table
 * @m size of finger table
 * @wanted wanted value
 */
int find_resp_finger(const int* fingers, /*int m,*/ int wanted);

int ring_compare(int a, int b, int m);

#endif // CHORD_CONSTANTS
