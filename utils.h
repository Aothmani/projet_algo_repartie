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
	TAGANSWER,
	TAGTAB, /* Collecting all the node ids */
	TAGTABANN, /* Broadcasting the completed table with all the node ids */
	TAGTTELLFINGER, /* Searching for a finger */
	TAGTELLFINGERRESP, /* Response to TAGTELLFINGER from the new finger */
	TAGUPDATE, /* Tell a node to update its fingers in regards to the newly inserted node */
	TAGUPDATERESP, /* Response to TAGTELLFINGER, contains the new finger */
	TAGINSERT, /* Insert a node in the ring */
	TAGINSERTRESP, /* Response of TAGINSERT, indicate that the node has been inserted in the ring */
	TAGTERM, /* Terminate a node */
	TAGASKFINGER,
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

/* mpi rank of the first non-simulator node */
#define FIRST_NODE 1 

struct node_addr {
	int mpi;
	int chord;
};

/* struct array - resizeable array */
struct array {
	size_t size;
	struct node_addr data[0];
};

int in_interval(int n, int a, int b, int k);
void receive_addr(int tag, struct node_addr* addr);
void send_addr(int dest, int tag, struct node_addr* addr);

void receive_addr_array(int tag, struct node_addr* addr_arr, int *len);
void send_addr_array(int dest, int tag, struct node_addr* addr_array,
		     int len);

/* struct node - description of a chord node */
struct node {
	int mpi_rank; 
	int rank; /* id in the ring */
        struct node_addr next_addr; /* next node in the ring */
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

int ring_compare(int a, int b, int m);


#endif // CHORD_CONSTANTS
