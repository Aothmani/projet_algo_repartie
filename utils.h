#ifndef CHORD_CONSTANTS
#define CHORD_CONSTANTS

#include <mpi.h>
#include <stdlib.h>

enum Tags {
	/* Exercice 1 */
	TAGINIT, /* Sent by the simulator to define the variables */
	TAGSEARCH, /* Searching a value in the hash table */
	TAGFOUND, /* Sending back the result of the search */
	TAGELECT, /* Choosing a leader for the election */

	/* Exercice 2 */
	TAGANN, /* Annoucing the leader at the end of the election */
	TAGANSWER,
	TAGTAB, /* Collecting all the node ids */
	TAGTABANN, /* Broadcasting the completed table with all the node ids */

	/* Exercice 3 */
	TAGINSERT, /* Telling a node to tinitiate the insertion by asking a node to help it */
	TAGASKFINGER, /* Calculate the finger table and gives it to the new node */
	TAGGIVEFINGER, /* Get the new finger table and ask for update */
	TAGCHECKREVERSE, /* Ask to the reverses to check if they need to update their finger table  */
	TAGMODIFYFINGER, /* Check if the finger 'node' need to be replaced with the new one */
	TAGNEWREVERSE, /* Add a new reverse to the reverse table*/
	TAGDELETEREVERSE, /* Delete a reverse from the reverse table  */
	TAGDONE, /* Update of a reverse's finger table finished */
	TAGEND, /* All system updated, signal it to the new node  */
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

void receive_addr(int tag, struct node_addr* addr);
void send_addr(int dest, int tag, struct node_addr* addr);

void receive_addr_array(int tag, struct node_addr* addr_arr, int *len);
void send_addr_array(int dest, int tag, struct node_addr* addr_array,
		     int len);

/* struct node - description of a chord node */
struct node {
	int mpi_rank; 
	int rank; /* id in the ring */
        struct node_addr next_addr; /* right node in the ring */
	struct node_addr left_addr; /* left node in the ring */
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
