#ifndef CHORD_ELECTION_H
#define CHORD_ELECTION_H

#include "utils.h"

enum ElectState {
		 ELECT_CANDIDATE, /* node is candidate for leadership */
		 ELECT_LEADER, /* node has been elected as leader */
		 ELECT_LOST, /* node lost the election */
		 ELECT_NOTCANDIDATE /* node isn't candidate */
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
void receive_elect(struct node *node, int *elect_state, int *leader);

/* receive_tab - handle the reception of a TAGTAB message
 * If the current node initiated the sequence, it will intiate the TAGTABANN 
 * sequence and transmit to his neighbor a sorted array that contains all the 
 * ids of the nodes in the network.
 * Otherwise, the current node will simply pass add its CHORD id to the array 
 * and transmit it to the next node.
 * @addr address of current node
 * @next MPI next node in the ring
 * @leader 1 if current node is leader, 0 otherwise
 */
void receive_tab(struct node_addr* addr, struct node_addr* next, int leader);


/* receive_tabann - handle the reception of a TAGTABANN message and calculate 
 * the finger table of the current node 
 * @node current node
 * @reception will be set to 1 if the current process is leader and when the 
 * message has been processed
 */
void receive_tabann(struct node* node, int leader, int *reception);

/* calc_fingers - Calculate the finger table using list 
 * of all nodes in the network
 * @rank CHORD id of current node
 * @tab array that contains the CHORD ids of every node in the network
 * @size array size
 * @fingers output finger table
 * @fingerCnt size of finger table
 */
void calc_fingers(struct node *node, struct node_addr* tab, int size);

#endif /* CHORD_ELECTION_H */
