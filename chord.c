#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <mpi.h>
#include <math.h>
#include "utils.h"

#define NB_SITE 6 /* site simulateur inclus */
#define M 6
#define K 64


int find_resp_finger(const int * finger, int wanted)
{
	int i;
	for (i = 0; i < M-1; i++){
		if (ring_compare(finger[i], wanted, K))
			return finger[i];
	}
	return finger[0];
}

/* 
 * Tries to find the mpi_id corresponding with the chord_id parameter,
 * contained in the associative table parameter
 * 
 * Return : the corresponding id if known, else 0.
 * 
 */ 
int find_corresponding_mpi_id(int chord_id, int * associative_table)
{
	int i;
	for(i = 0, i < NB_SITE, i++){
		if(associative_table[i] == chord_id)
			return associative_table[i];
	}
	return 0;
}

/* A VERIFIER */
/* Returns 1 if a <= b, 0 otherwise */
int ring_compare(int a, int b, int mod)
{
	int n;
	if (b >= a)
		n = b - a;
	else
		n = b - a + mod;
	if (n >= 0 && n <= mod/2)
		return 1;
	else
		return 0;
}

/* Sort function for qsort(). 
 * @return: -1 if a < b
 *           0 if a = b 
 *           1 if a > b 
 */ 
int tri(const void * c, const void * d)
{
	const int * a = (const int *) c;
	const int * b = (const int *) d;
	if (*a == *b)
		return 0;
	return (ring_compare(*a, *b, K)) ? -1 : 1;
}

void node()
{
	/* cid : chord id, key : buffer, sent in a message, caller : initiatior */
	int cid, key, dest, wanted, chord_id, caller_chord;
	MPI_Status status;
	int fingers[M], associative_table[NB_SITE];
	
	
	
	MPI_Recv(&cid, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
	MPI_Recv(&fingers, M, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
	MPI_Recv(&associative_table, NB_SITE, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);

	/* Initialisation */

	
	MPI_Recv(&key, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	while(status.MPI_TAG != TAGTERM){
		switch (status.MPI_TAG){
		case TAGSEARCH:
			/* Receiving the caller's id for the answer */
			MPI_Recv(&caller_chord, 1, MPI_INT, status.MPI_SOURCE,
				 TAGSEARCH, MPI_COMM_WORLD, &status);
						
			/* If this node is responsible of the key */
			if (ring_compare(key, cid, K)){
				chord_id = find_resp_finger(fingers, caller_chord);
				if(chord_resp){
					dest = find_corresponding_mpi_id(chord_id,
									associative_table);
					SEND_INT(dest, TAGFOUND, cid);
					SEND_INT(dest, TAGFOUND, caller_chord);
				} else {
					printf("Error : chord_id not found.\n");
				}
				
			/* It's not the responsible node, transmit */	
			} else {
				chord_id = find_resp_finger(fingers, key);
				if(chord_id){
					dest = find_corresponding_mpi_id(chord_id,
									associative_table);
					SEND_INT(dest, TAGSEARCH, key);
					SEND_INT(dest, TAGSEARCH, caller_chord);
				} else {
					printf("Error : chord_id not found.\n");
				}
			}
			break;

		case TAGFOUND:
			MPI_Recv(&caller_chord, 1, MPI_INT, status.MPI_SOURCE,
				 TAGFOUND, MPI_COMM_WORLD, &status);
			/*
			 * The init node received the resp id,
			 * send terminate message to simulator
			 */
			if(caller == cid){
				printf("Caller found the node %d responsible of the key\n", key);
				SEND_INT(0, TAGTERM, key);

                        /* this node is not the recipient, transmit */
			} else {
				chord_id = find_resp_finger(fingers, caller_chord);
				if(chord_id){
					dest = find_corresponding_mpi_id(chord_id,
									associative_table);
					SEND_INT(dest, TAGFOUND, key);
					SEND_INT(dest, TAGFOUND, caller_chord);
				} else {
					printf("Error : chord_id not found.\n");
				}
			}
			break;

		case TAGINIT:
			if(key == cid){
				printf("Init : key wanted is the init node, no need for research.\n");
			} else {
				chord_id = find_resp_finger(fingers, key);
				if(chord_id){
					dest = find_corresponding_mpi_id(chord_id,
									associative_table);
					SEND_INT(dest, TAGSEARCH, key);
					SEND_INT(dest, TAGSEARCH, cid);
				} else {
						printf("Error : chord_id not found.\n");
				}
			}
		default:
			printf("Error : unknown tag.\n");
			break;
		}
		MPI_Recv(&key, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	}
	
}

void simulateur(void)
{
	srand(getpid());

	int chord_id[NB_SITE];
	int finger[NB_SITE][M], associative_table[NB_SITE][NB_SITE];
	int i, val, j, k;
	
	memset(chord_id, -1, NB_SITE);

	/* Initialisation des tables de finger et id chord */
	for(i = 0; i < NB_SITE, i++){
		memset(finger[i], -1, M);
		memset(associative_table[i], -1, NB_SITE);
	}
		
	/* Calcul des id CHORD  */
	for(i = 1; i < NB_SITE; i++){
		val = rand() % K;
		for(j = 0; j < NB_SITE; j++){
			if(val == chord_id[j]){
				val = rand() % 64;
				j = 0;
			}
		}
		chord_id[i] = val;
	}

	qsort(&id, sizeof(id), sizeof(int), tri);
	
	/*
	 * Calculation of the finger array and filling of the chord id array 
	 * and associative table array
	 *    
	 */
	for(i = 1; i < NB_SITE; i++){
		
		/* Calcul de la finger table de i */
		for(j = 0; j < M; j++){
			val = id[i] + pow(2, j);
			k = 1;
			while(k < NB_SITE && mpi_id[k] < val){
				k++;
			}
			if(j == NB_SITE) {
				finger[i][j] = chord_id[1];
				associative_table[i][1] = chord_id[1];
			} else {
				finger[i][j] = chord_id[k];
				associative_table[i][k] = chord_id[k];
			}
		}
	}

	for(i = 1; i < NB_SITE; i++){
		SEND_INT(i, TAGINIT, chord_id[i]);
		SEND_NINT(i, TAGINIT, finger[i], M);
		SEND_NINT(i, TAGINIT, associative_table[i], NB_SITE);
	}

	val = rand() % K;
	while ((i = rand() % M) == val){}
	SEND_INT(chord_id[i], TAGINIT, val);

	
}

int main(int argc, char *argv[])
{
	int nb_proc,rang;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);

	if (nb_proc != NB_SITE+1) {
		printf("Nombre de processus incorrect !\n");
		MPI_Finalize();
		exit(2);
	}
  
	MPI_Comm_rank(MPI_COMM_WORLD, &rang);
  
	if (rang == 0)
		simulateur();
	else 
		node();
	  
	MPI_Finalize();
	return 0;
}
