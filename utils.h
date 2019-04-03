#ifndef CHORD_CONSTANTS
#define CHORD_CONSTANTS

#include <mpi.h>

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

#define SEND_INT(dest, tag, data) MPI_Send((void*)&data, 1, MPI_INT, dest, tag, MPI_COMM_WORLD)
#define SEND_NINT(dest, tag, data, n) MPI_Send((void*)data, n, MPI_INT, dest, tag, MPI_COMM_WORLD)
/*
 * find_finger - Determines the finger to which the request should be transmitted to
 * @fingers finger table
 * @m size of finger table
 * @wanted wanted value
 */
void find_finger(const int* fingers, int m, int wanted);

int ring_compare(void* a, void* b);


#endif // CHORD_CONSTANTS
