#ifndef CHORD_ELECTION_H
#define CHORD_ELECTION_H

#include "utils.h"

/* 
 * election - Initiate a leader election
 * @rank initator rank
 * @procCount process count
 * @next rank of next node in the ring
 */
void election(int rank, int next, int round);

/* receive_tab - handle the reception of a TAGTAB message
 * If the current node initiated the sequence, it will intiate the TAGTABANN 
 * sequence and transmit to his neighbor a sorted array that contains all the 
 * CHORD ids of the nodes in the network.
 * Otherwise, the current node will simply pass add its CHORD id to the array 
 * and transmit it to the next node.
 */
void receive_tab(int next, int leader, int *tab, int msize);

/* receive_tabann - handle the reception of a TAGTABANN message and calculate 
 * the finger table of the current node 
 */
void receive_tabann(int rank, int next, int leader, int msize, int* fingers,
		    int fingerCnt, int *reception);

/* calc_fingers - Calculate the finger table using list 
 * of all nodes in the network
 * @rank CHORD id of current node
 * @tab array that contains the CHORD ids of every node in the network
 * @size array size
 * @fingers output finger table
 * @fingerCnt size of finger table
 */
void calc_fingers(int rank, int* tab, int size, int *fingers, int fingerCnt);

#endif /* CHORD_ELECTION_H */
