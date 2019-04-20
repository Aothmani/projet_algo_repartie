#include "election.h"

#include <stdlib.h>

#include "election.h"
#include "utils.h"


void election(int rank, int next)
{
	SEND_INT(next, TAGELECT, rank);
}

void receive_elect(struct node *node, int next, int *elect_state, int *leader)
{
	int j;
	MPI_Status status;
	int rank = node->mpi_rank;
	struct node_addr addr;
	addr.mpi = rank;
	addr.chord = node->rank;
	
	MPI_Recv(&j, 1, MPI_INT, MPI_ANY_SOURCE, TAGELECT, MPI_COMM_WORLD,
		 &status);
	
	if (rank > j) {
		if (*elect_state == ELECT_NOTCANDIDATE)
			*elect_state = ELECT_CANDIDATE;
	} else if (rank < j) {
		*elect_state = ELECT_LOST;
		*leader = j;
		SEND_INT(next, TAGELECT, j);
	} else {
		*elect_state = ELECT_LEADER;
		send_addr_array(next, TAGTAB, &addr, 1);
	}
}

int __compare_int(const void* a, const void* b)
{
	int *c = (int*)a;
	int *d = (int*)c;
	if (*c == *d)
		return 0;
        return (*c < *d ? -1 : 1);
}

int __compare_addr(const void *a, const void *b)
{
	struct node_addr *c = (struct node_addr*)a;
	struct node_addr *d = (struct node_addr*)b;
	if (c->chord == d->chord)
		return 0;
	return c->chord < d->chord ? -1 : 1;
}

void receive_tab(struct node_addr* addr, int next, int leader)
{
	struct node_addr *tab;
	MPI_Status status;
	int msize;

	MPI_Probe(MPI_ANY_SOURCE, TAGTABANN, MPI_COMM_WORLD, &status);
	MPI_Get_count(&status, MPI_INT, &msize);
	tab = malloc((msize + 1) * sizeof(struct node_addr));
	receive_addr_array(TAGTAB, tab, msize);
	
	if (leader) {
		qsort((void*)tab, msize, sizeof(struct node_addr),
		      __compare_addr);
		send_addr_array(next, TAGTABANN, tab, msize);
	} else {
		tab[msize] = *addr;
		send_addr_array(next, TAGTABANN, tab, msize);
	}
	free(tab);
}

void receive_tabann(int rank, int next, int leader,
		    struct node_addr* fingers, int fingerCnt, int *reception)
{
	struct node_addr *tab;
	int msize;
	MPI_Status status;
	
	MPI_Probe(MPI_ANY_SOURCE, TAGTABANN, MPI_COMM_WORLD, &status);
	MPI_Get_count(&status, MPI_INT, &msize);
	
	tab = malloc(msize * sizeof(struct node_addr));
	receive_addr_array(TAGTABANN, tab, msize);
	
	if (leader) {
		*reception = 1;
	} else {
		send_addr_array(next, TAGTABANN, tab, msize);
	}
	calc_fingers(rank, tab, msize, fingers, fingerCnt);

	free(tab);
}

void calc_fingers(int rank, struct node_addr* tab, int size,
		  struct node_addr *fingers, int fingerCnt)
{
	int posp = 0, pos = 0, prev;
	int i, fi = 0;
	int htableSize = (1 << fingerCnt); /* 2^fingerCnt */
	
	/* 
	 * Find the position of the current node inside the array that contains 
	 * all the node in the ring 
	 */
	for (i = 0; i < size; i++) {
		if (tab[i].chord == rank) {
			posp = i;
			break;
		}
	}

	/* TODO: Check algorithm */
	pos = (posp + 1) % size; /* id of next node */
	
	while (ring_compare(pos, posp, htableSize)) {
		if (tab[pos].chord > (pos + htableSize) % size) {
			prev = ((pos - 1) + size) % size;
			fingers[fi] = tab[prev];
			fi++;
		}
		pos++;
	}
}
