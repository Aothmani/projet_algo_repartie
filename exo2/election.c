#include "election.h"

#include <stdlib.h>
#include <stdio.h>

#include "election.h"
#include "../utils.h"

#define LEFT 1
#define RIGHT 2

#define OPP(dir) (dir == LEFT ? RIGHT : LEFT)
#define DIRN(dir, node) (dir == LEFT ? node->left_addr.mpi : node->next_addr.mpi)


void election(struct node* node, int *k)
{
	int data[2];
	data[0] = node->mpi_rank;
	data[1] = (1 << *k);
	SEND_NINT(DIRN(LEFT, node), TAGELECT, data, 2);
	SEND_NINT(DIRN(RIGHT, node), TAGELECT, data, 2);
}

void receive_elect(struct node *node, int *elect_state, int *leader, int *cpt,
		   int *k)
{
	int j, ttl;
	int data[2];
	MPI_Status status;
	int mpi_rank = node->mpi_rank;	
	int dir;
	struct node_addr addr;
	addr.mpi = mpi_rank;
	addr.chord = node->rank;
	
	MPI_Recv(data, 2, MPI_INT, MPI_ANY_SOURCE, TAGELECT, MPI_COMM_WORLD,
		 &status);
	
	if (status.MPI_SOURCE == node->next_addr.mpi)
		dir = LEFT;
	else
		dir = RIGHT;
	
	j = data[0];
	ttl = data[1];

	printf("P%d> (ttl: %d, j: %d), cpt = %d, k = %d\n",
	       node->mpi_rank, ttl, j, *cpt, *k);

	if (*elect_state == ELECT_NOTCANDIDATE) {
		printf("P%d> not candidate\n", node->mpi_rank);
		SEND_NINT(DIRN(dir, node), TAGELECT, data, 2);
	} else if (mpi_rank == j && ttl > 0) {
		*elect_state = ELECT_LEADER;
		printf("P%d> leader\n", node->mpi_rank);
		send_addr_array(node->next_addr.mpi, TAGTAB, &addr, 1);
	} else if (j > mpi_rank && ttl >= 1) {
		*elect_state = ELECT_LOST;  
		data[0] = *leader = j;
		printf("P%d> lost\n", node->mpi_rank);
		if (ttl > 1) {
			data[1] = ttl - 1;
			SEND_NINT(DIRN(dir, node), TAGELECT, data, 2);
		} else {
			data[1] = 0;
			printf("P%d> opp = %d\n",
			       node->mpi_rank, DIRN(OPP(dir), node));
			SEND_NINT(DIRN(OPP(dir), node), TAGELECT, data, 2);
		}		
	} else if (ttl == 0){ 
		if (mpi_rank != j) {
			SEND_NINT(DIRN(dir, node), TAGELECT, data, 2);
		} else {
			(*cpt)++;
			if (*cpt == 2 && *elect_state != ELECT_LOST) {
				(*k)++;
				*cpt = 0;
				printf("P%d> starting new step (k = %d, cpt = %d)\n",
				       node->mpi_rank, *k, *cpt);
				election(node, k);
			}				
		}		
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
	calc_fingers(node, tab, msize);
	
	free(tab);
}

void calc_fingers(struct node *node, struct node_addr* tab, int size)
{
	int posp = 0, pos = 0;
	int i = 0, fi = 0;
	int htableSize = (1 << node->fingers->size); /* 2^fingerCnt */
	int fingerOffset = 1;
	int rank = node->rank;
	int fingerCnt = node->fingers->size;
	
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

	printf("P%d> calc_fingers - size = %d\n", node->mpi_rank, size);
	printf("P%d> posp = %d, tab[posp] = %d\n", node->mpi_rank, posp,
	       tab[posp].chord);
	
	pos = (posp + 1) % size;
	fingerOffset = 1;
	while (fi < fingerCnt) {
		while (in_interval(tab[pos].chord, rank,
				   (rank + fingerOffset) % htableSize,
				   htableSize)) {
		        if (pos == posp)
				break;
			pos = (pos + 1) % size;
		}
		node->fingers->data[fi] = tab[pos];
		fi++;
		fingerOffset *= 2;
	}	
}
