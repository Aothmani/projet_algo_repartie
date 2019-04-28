#include <stdlib.h>
#include <mpi.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "utils.h"
#include "election.h"

#define PROC_COUNT 6
#define M 6
#define K (1 << M)

#define MSWAP(a, b) {typeof(a) swap_#a#b;	\
		swap_#a#b = a;			\
		a = b;				\
		b = swap_#a#b;}


void swap(int *a, int *b)
{
	int c;
	c = *b;
	*b = *a;
	*a = c;
}

void sequence_array(int *arr, size_t len)
{
	size_t i;
	for (i = 0; i < len; i++) {
		arr[i] = i;
	}
}

void init_node(struct node *node)
{
	MPI_Status status;
	MPI_Recv(&node->rank, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
	MPI_Recv(&node->next, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
}

void print_fingers(struct node_addr* fingers, int len)
{
	int i;
	for (i = 0; i < len; i++) {
		printf("%d ", fingers[i].chord);
	}
	printf("\n");
}

void node(int rank)
{
	struct node node;
	int next;
	MPI_Status status;
	int elect_state, leader = -1;
	int msize;
	int reception = 0;
	struct node_addr addr;
	
	node.mpi_rank = rank;
	node.leader = 0;
	node.fingers = malloc(sizeof(struct node_addr) * M);
	node.fingers->size = M;
	
	memset(node.fingers->data, 0, sizeof(struct node_addr) * M);
	
	init_node(&node);

	addr.chord = node.rank;
	addr.mpi = node.mpi_rank;
	
	if (node.leader) {
		election(rank, NEXT(rank, M));
		elect_state = ELECT_CANDIDATE;
	}

	
	
	next = NEXT(rank, M);
	while (!reception) {
		MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		switch (status.MPI_TAG) {
		case TAGELECT:
			receive_elect(&node, next, &elect_state, &leader);
			break;
		case TAGTAB:
			receive_tab(&addr, next,
				    elect_state == ELECT_LEADER ? 1 : 0);
			break;
		case TAGTABANN:
			receive_tabann(rank, next, leader, node.fingers->data,
				       M, &reception);
			break;
		}
	}
	free(node.fingers);
}

void shuffle(int *arr, size_t len)
{
	size_t i;
	int target;
	for (i = 0; i < len; i++) {
		target = rand() % len;
		swap(arr + i, arr + target);
	}
}



void simulator(void)
{
	int mpi_ranks[M] = {1, 2, 3, 4, 5, 6};
	int chord_ids[M];
        int i;
	int next;

	/* Randomize chords ids */
	sequence_array(chord_ids, M);
	shuffle(chord_ids, M);
	
	/* Send chords ids and mpi ids for next nodes */
	for (i = 0; i < M; i++) {
		SEND_INT(mpi_ranks[i], TAGINIT, chord_ids + i);
		next = (i + 1) % M;
		SEND_INT(mpi_ranks[i], TAGINIT, next);
	}
	
}


int main(int argc, char *argv[])
{
	int proc_count, rank;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &proc_count);
	
	if (proc_count != PROC_COUNT + 1) {
		printf("Nombre de processus incorrect !\n");
		MPI_Finalize();
		exit(2);
	}
  
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	srand(time(NULL)^rank);
	
	if (rank == 0)
		simulator();
	else 
		node(rank);
	  
	MPI_Finalize();
	return 0;
}
