#include "election.h"

#include <stdlib.h>

#include "utils.h"


void election(int rank, int next, int round)
{
	int data[2];
	data[0] = rank;
	data[1] = round;
	SEND_INT(next, TAGELECT, rank);
}



int __compare_int(const void* a, const void* b)
{
	int *c = (int*)a;
	int *d = (int*)c;
	if (*c == *d)
		return 0;
        return (*c < *d ? -1 : 1);
}

void receive_tab(int next, int leader, int *tab, int msize)
{
	MPI_Status status;
	int *newtab = NULL;
	MPI_Recv(tab, msize, MPI_INT, MPI_ANY_SOURCE, TAGTAB, MPI_COMM_WORLD,
		 &status);

	if (leader) {
		qsort((void*)tab, msize, sizeof(int), __compare_int);
		SEND_NINT(next, TAGTABANN, &tab, msize);
	} else {
		newtab = (int*)malloc((msize + 1) * sizeof(int));
		SEND_NINT(next, TAGTAB, newtab, msize+1);
		free(newtab);
	}
}

void receive_tabann(int rank, int next, int leader, int msize, int* fingers,
		    int fingerCnt, int *reception)
{
	MPI_Status status;
	int *tab = (int*)malloc(msize * sizeof(int));

	MPI_Recv((void*)tab, msize, MPI_INT, MPI_ANY_TAG, TAGTABANN,
		 MPI_COMM_WORLD, &status);
	
	if (leader) {
		*reception = 1;
	} else {
		SEND_NINT(next, TAGTABANN, tab, msize);
	}
	calc_fingers(rank, tab, msize, fingers, fingerCnt);

	free(tab);
}

void calc_fingers(int rank, int* tab, int size, int *fingers, int fingerCnt)
{
	int posp = 0, pos = 0;
	int i, fi = 0;
	int htableSize = (1 << fingerCnt); /* 2^fingerCnt */
	
	/* 
	 * Find the position of the current node inside the array that contains 
	 * all the node in the ring 
	 */
	for (i = 0; i < size; i++) {
		if (tab[i] == rank) {
			posp = i;
			break;
		}
	}

	/* TODO: Check algorithm */
	pos = (posp + 1) % size; /* id of next node */
	while (ring_compare(pos, posp, htableSize)) {
		if (tab[pos] > (pos + htableSize) % size) {
			fingers[fi] = tab[(pos - 1) % size];
			fi++;
		}
		pos++;
	}
}
