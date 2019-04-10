#ifndef CHORD_ELECTION_H
#define CHORD_ELECTION_H

#include "utils.h"

/* 
 * election - Envoie un message d'election de leader 
 * @rank initator rank
 * @procCount process count
 * @next rank of next node in the ring
 */
void election(int rank, int next, int round);

void receive_tab(int next, int leader, int *tab, int msize);
void receive_tabann(int rank, int next, int leader, int msize, int* fingers,
		    int fingerCnt, int *reception);

void calc_fingers(int rank, int* tab, int size, int *fingers, int fingerCnt);

#endif /* CHORD_ELECTION_H */
