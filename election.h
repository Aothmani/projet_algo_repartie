#ifndef CHORD_ELECTION_H
#define CHORD_ELECTION_H

#include "utils.h"

enum ElectState {
		 ELECT_CANDIDATE,
		 ELECT_LEADER,
		 ELECT_LOST,
		 ELECT_NOTCANDIDATE
};

/* 
 * election - Initiate a leader election using the Chang & Roberts algorithm
 * @rank initator rank
 * @next rank of next node in the ring
 */
void election(int rank, int next);

/* receive_elect - Handle the reception of a TAGELECT message during an 
 * election using the Chang & Roberts algorithm 
 */
void receive_elect(struct node *node, int next, int *elect_state, int *leader);

/* receive_tab - handle the reception of a TAGTAB message
 * If the current node initiated the sequence, it will intiate the TAGTABANN 
 * sequence and transmit to his neighbor a sorted array that contains all the 
 * ids of the nodes in the network.
 * Otherwise, the current node will simply pass add its CHORD id to the array 
 * and transmit it to the next node.
 * @next MPI rank of next node in the ring
 * @leader 1 if current node is leader, 0 otherwise
 */
void receive_tab(int next, int leader);

/* receive_tabann - handle the reception of a TAGTABANN message and calculate 
 * the finger table of the current node 
 * @rank MPI rank of current node
 * @next MPI rank of next node
 * @leader 1 if current node is leader, 0 otherwise
 * @fingers finger table that will be calculated
 * @fingerCnt number of fingers in the finger table
 * @reception will be set to 1 if the current process is leader and when the 
 * message has been processed
 */
void receive_tabann(int rank, int next, int leader, struct node_addr* fingers,
		    int fingerCnt, int *reception);

/* calc_fingers - Calculate the finger table using list 
 * of all nodes in the network
 * @rank CHORD id of current node
 * @tab array that contains the CHORD ids of every node in the network
 * @size array size
 * @fingers output finger table
 * @fingerCnt size of finger table
 */
void calc_fingers(int rank, struct node_addr* tab, int size,
		  struct node_addr *fingers, int fingerCnt);

#endif /* CHORD_ELECTION_H */
