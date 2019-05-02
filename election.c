#include "election.h"

#include <stdlib.h>
#include <stdio.h>

#include "election.h"
#include "utils.h"


void election(int rank, int next)
{
	SEND_INT(next, TAGELECT, rank);
}

void receive_elect(struct node *node, int *elect_state, int *leader)
{
	int j;
	MPI_Status status;
	int mpi_rank = node->mpi_rank;
	struct node_addr addr;
	addr.mpi = mpi_rank;
	addr.chord = node->mpi_rank;
	
	MPI_Recv(&j, 1, MPI_INT, MPI_ANY_SOURCE, TAGELECT, MPI_COMM_WORLD,
		 &status);
	
	if (mpi_rank > j) {
		if (*elect_state == ELECT_NOTCANDIDATE)
			*elect_state = ELECT_CANDIDATE;
		SEND_INT(node->next_addr.mpi, TAGELECT, mpi_rank);
	} else if (mpi_rank < j) {
		*elect_state = ELECT_LOST;
		*leader = j;
		SEND_INT(node->next_addr.mpi, TAGELECT, j);
	} else {
		*elect_state = ELECT_LEADER;
		send_addr_array(node->next_addr.mpi, TAGTAB, &addr, 1);
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

void receive_tab(struct node_addr* addr, struct node_addr* next, int leader)
{
	struct node_addr *tab;
	MPI_Status status;
	int msize;
	int i;
	
	MPI_Probe(MPI_ANY_SOURCE, TAGTAB, MPI_COMM_WORLD, &status);
	MPI_Get_count(&status, MPI_INT, &msize);
	tab = malloc((msize + 1) * sizeof(int));
	receive_addr_array(TAGTAB, tab, &msize);

	printf("P%d> msize = %d, tab = [", addr->mpi, msize);
	for (i = 0; i < msize; i++) {
		printf("(%d, %d), ", tab[i].mpi, tab[i].chord);
	}
	printf("]\n");
	
	if (leader) {
		qsort((void*)tab, msize, sizeof(struct node_addr),
		      __compare_addr);
		send_addr_array(next->mpi, TAGTABANN, tab, msize);
	} else {
		tab[msize] = *addr;
		msize++;
		send_addr_array(next->mpi, TAGTAB, tab, msize);
	}
	free(tab);
}

void receive_tabann(struct node* node, int leader, int *reception)
{
	struct node_addr *tab;
	int msize;
	int i;
	MPI_Status status;
	
	MPI_Probe(MPI_ANY_SOURCE, TAGTABANN, MPI_COMM_WORLD, &status);
	MPI_Get_count(&status, MPI_INT, &msize);
	
	tab = malloc(msize * sizeof(int));
	receive_addr_array(TAGTABANN, tab, &msize);

	printf("P%d> msize = %d, tab = [", node->mpi_rank, msize);
	for (i = 0; i < msize; i++) {
		printf("(%d, %d), ", tab[i].mpi, tab[i].chord);
	}
	printf("]\n");

	
	*reception = 1;
	if (leader != ELECT_LEADER) {
		send_addr_array(node->next_addr.mpi, TAGTABANN, tab, msize);
	}
	calc_fingers(node->mpi_rank, tab, msize, node->fingers->data,
		     node->fingers->size);

	free(tab);
}

void calc_fingers(int rank, struct node_addr* tab, int size,
		  struct node_addr *fingers, int fingerCnt)
{
	int posp = 0, pos = 0, prev;
	int i, fi = 0;
	int htableSize = (1 << fingerCnt); /* 2^fingerCnt */
	int fingerOffset;
	
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

	printf("P%d> calc_fingers - size = %d\n", rank, size);
	printf("P%d> posp = %d, tab[posp] = %d\n", rank, posp, tab[posp].chord);
	
	/* TODO: Check algorithm */
	pos = (posp + 1) % size;
	fingerOffset = 1;
	while (fi < fingerCnt) {
		while (in_interval(pos, rank, rank + fingerOffset, htableSize))
			pos = (pos + 1) % size;
		prev = (pos + size - 1) % size;
		fingers[fi] = tab[prev];
		fi++;
		fingerOffset *= 2;		
	}	
}
