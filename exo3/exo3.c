#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <mpi.h>
#include "../utils.h"

#define NB_SITE 7 /* site simulateur non inclus */
#define M 6
#define K (1 << M)


int inInterval(int n, int a, int b)
{
	if (a < b)
		return (n >= a && n < b);
	else
		return ((n >= a && n < K) || (n >= 0 && n < b));
			
}


int find_resp_finger(const int * finger, int wanted, int cid)
{
	int i;
	for (i = M-1; i >= 0; i--){
		if (finger[i] != cid && (finger[i] == wanted || inInterval(wanted, finger[i], cid)))
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
	for(i = 1; i < NB_SITE + 1; i++){
		if(associative_table[i] == chord_id)
			return i;
	}
	return 0;
}

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

void node(int rank)
{
	/* cid : chord id, key : buffer, sent in a message, caller : initiatior */
	int cid, key, dest, wanted, chord_id, caller_chord, i, j, found, node, new_node;
	MPI_Status status;
	int fingers[M], tmp_fingers[M], associative_table[NB_SITE+1];
	int buff[2], reverse[NB_SITE+1], tmp_reverse[NB_SITE+1];
	
	
	/* Initialisation */
	MPI_Recv(&cid, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
	if (rank != NB_SITE){
		MPI_Recv(&fingers, M, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
	}
	MPI_Recv(&associative_table, NB_SITE + 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
	MPI_Recv(&reverse, NB_SITE + 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);
			
	MPI_Recv(&buff, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	while(status.MPI_TAG != TAGTERM){
		switch (status.MPI_TAG){
		case TAGSEARCH:
			/* Receiving the key to search and the caller's id for the answer */
			key = buff[0];
			caller_chord = buff[1];
			
			printf("Node %d : received TAGSEARCH message.\n", cid);

			
			chord_id = find_resp_finger(fingers, key, cid);
			dest = find_corresponding_mpi_id(chord_id, associative_table);


			if (chord_id == key || inInterval(key, cid, fingers[0]) && chord_id == fingers[0]){
				printf("Node %d : transmitting the request TAGANSWER to node %d fo key %d\n", cid, chord_id, key);
				SEND_NINT(dest, TAGANSWER, buff, 2);
			} else {
				printf("Node %d : transmitting the request TAGSEARCH to node %d\n", cid, chord_id);
				SEND_NINT(dest, TAGSEARCH, buff, 2);
			}
			break;

		case TAGANSWER:
			key = buff[0];
			caller_chord = buff[1];
			
			printf("Node %d : received TAGANSWER message.\n", cid);
			printf("\nNode %d is responsible for the key %d\n\n", cid, key);
			
			chord_id = find_resp_finger(fingers, caller_chord, cid);
			dest = find_corresponding_mpi_id(chord_id, associative_table);
			printf("Node %d : sending answer to caller %d\n", cid, caller_chord);
			printf("Node %d : transmitting the answer to node %d\n", cid, chord_id);

			buff[0] = cid;
			SEND_NINT(dest, TAGFOUND, buff, 2);

			break;

		case TAGFOUND:
			key = buff[0];
			caller_chord = buff[1];
			
			printf("Node %d : received TAGFOUND message.\n", cid);
				
			/* this node is not the recipient, transmit */
			
			chord_id = find_resp_finger(fingers, caller_chord, cid);
			dest = find_corresponding_mpi_id(chord_id,
							 associative_table);
			printf("Node %d : transmitting the answer to node %d\n", cid, chord_id);

			SEND_NINT(dest, TAGFOUND, buff, 2);			
			
			break;

		case TAGINIT:
			key = buff[0];			
			if(key == cid){
				printf("Init : key wanted is the init node, no need for research.\n");
				SEND_INT(0, TAGTERM, key);
			} else {
				printf("Node %d is initiator, searching for key %d\n", cid, key);
				chord_id = find_resp_finger(fingers, key, cid);				
				dest = find_corresponding_mpi_id(chord_id, associative_table);
				printf("Calculated mpi rank from %d : %d\n", chord_id, dest);
				printf("Transmitting the request to node %d\n", chord_id);

				buff[0] = key;
				buff[1] = cid;
				SEND_NINT(dest, TAGSEARCH, buff, 2);
			}
			break;
		case TAGINSERT:
			node = buff[0];
			printf("Node %d : received TAGINSERT message.\n", cid);

			dest = find_corresponding_mpi_id(node, associative_table);

			printf("Node %d : asking help to node %d\n", cid, node);

			buff[0] = cid;
			SEND_NINT(dest, TAGASKFINGER, buff, 2);
			break;

		case TAGASKFINGER:
		{
			printf("Node %d : received TAGASKFINGER message.\n", cid);
			node = buff[0];
			int node_mpi =  status.MPI_SOURCE;
			for (i = 0; i < M; i++){
				key = (node + (1 << i)) % K;
				
				chord_id = find_resp_finger(fingers, key, cid);				
				dest = find_corresponding_mpi_id(chord_id, associative_table);
				printf("Node %d : transmitting the request to node %d\n", cid, chord_id);

				buff[0] = key;
				buff[1] = cid;
				if (chord_id == key || inInterval(key, cid, fingers[0]) && chord_id == fingers[0])
					SEND_NINT(dest, TAGANSWER, buff, 2);
				else
					SEND_NINT(dest, TAGSEARCH, buff, 2);
				
				MPI_Recv(&buff, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
				/*
				 * if this node is found as a finger of the node to insert,
				 * add it to the finger table and update his reverse table
				 */
				if (status.MPI_TAG == TAGANSWER)
					tmp_fingers[i] = cid;

				/* else add the node received to the finger table */ 
				else if (status.MPI_TAG == TAGFOUND)
					tmp_fingers[i] = buff[0];
				else
					printf("Node %d : error while constructing the finger table : wrong tag\n", cid);
			}

			SEND_INT(node_mpi, TAGGIVEFINGER,node);
			SEND_NINT(node_mpi, TAGGIVEFINGER, tmp_fingers, M);
			break;
		}

		case TAGGIVEFINGER:
			MPI_Recv(&fingers, M, MPI_INT, MPI_ANY_SOURCE, TAGGIVEFINGER, MPI_COMM_WORLD, &status);
			printf("Node %d : received TAGGIVEFINGER message\n", cid);
			printf("\nNode %d : received finger table : \n\n", cid);

			for (i = 0; i < M; i++){
				printf("Node %d : finger[%d] = %d\n", cid, i, fingers[i]);
				dest = find_corresponding_mpi_id(fingers[i], associative_table);
				SEND_INT(dest, TAGNEWREVERSE, cid);
			}

			SEND_INT(0, TAGTERM, fingers[0]);
			dest = find_corresponding_mpi_id(fingers[0], associative_table);
			SEND_INT(dest, TAGCHECKREVERSE, cid);

		case TAGNEWREVERSE:
			node = buff[0];
			i = 0;
			while(reverse[i] != node && reverse[i] != -1)
				i++;
			if(reverse[i] == -1){
				reverse[i] = node;
				printf("Node %d : new reverse added : node %d\n", cid, buff[0]);
			}
			
			break;
			
		case TAGCHECKREVERSE:
			printf("Node %d : received TAGCHECKREVERSE message from %d\n", cid, buff[0]);
			node = buff[0];
			i = 0;
			while (i < M && reverse[i] != -1){
				dest = find_corresponding_mpi_id(reverse[i], associative_table);
				
				buff[0] = cid;
				buff[1] = node;
				SEND_NINT(dest, TAGMODIFYFINGER, buff, 2);
				i++;
			}
			break;
				
		case TAGMODIFYFINGER:
			printf("Node %d : received TAGMODIFYFINGER message from %d\n", cid, buff[1]);
			node = buff[0];
			new_node = buff[1];
		
			i = 0;
			while (i < M && fingers[i] != node)
				i++;

			key = cid + (1 << i);
			printf("Node %d : node = %d new_node = %d key = %d\n", cid, node, new_node, key);
			if (key <= new_node){
				fingers[i] = new_node;
				printf("Node %d : modified finger table : node %d replaced with node %d for key %d\n",cid, node, new_node, key);
				dest = find_corresponding_mpi_id(node, associative_table);
				SEND_INT(dest, TAGDELETEREVERSE, cid);

				dest = find_corresponding_mpi_id(new_node, associative_table);
				SEND_INT(dest, TAGNEWREVERSE, cid);
			}

			for(i = 0; i < M; i++)
				printf("Node %d : finger[%d] = %d\n", cid, i, fingers[i]);
			printf("\n");
			SEND_INT(0, TAGTERM, cid);
			break;

		case TAGDELETEREVERSE:
			for (i = 0; i < NB_SITE + 1; i++){
				if(reverse[i] == buff[0]){
					printf("Node %d : deleted node %d from reverse table\n", cid, reverse[i]);
					reverse[i] = -1;
				}
			}
			break;
			
		default:
			printf("Error : unknown tag.\n");
			break;
		}
		MPI_Recv(&buff, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	}
	printf("Node %d : TAGTERM received, terminating\n", rank);
	
}

void simulateur(void)
{
	srand(getpid());

	int chord_id[NB_SITE + 1];
	int finger[NB_SITE + 1][M], associative_table[NB_SITE + 1][NB_SITE+1], reverse[NB_SITE+1][NB_SITE+1];
	int i, val, j, k, l, m, cpt;
	
	for(i = 0; i < NB_SITE + 1; i++)
		chord_id[i] = -1;

	/* Initialisation des tables de finger et id chord */
	for(i = 0; i < NB_SITE + 1; i++)
		for(j = 0; j < M; j++)
			finger[i][j] = -1;

	for(i = 0; i < NB_SITE + 1; i++){
		for(j = 0; j < NB_SITE+1; j++){
			associative_table[i][j] = -1;
			reverse[i][j] = -1;
		}
	}
		
	/* Calcul des id CHORD  */
	for(i = 1; i < NB_SITE + 1; i++){
		val = rand() % K;
		for(j = 1; j < NB_SITE + 1; j++){
			if(val == chord_id[j]){
				val = rand() % 64;
				j = 0;
			}
		}
		chord_id[i] = val;
	}

	printf("\nChord id array :\n\n");
	for (i = 0; i < NB_SITE + 1; i++)
		printf("chord_id[%d] = %d\n", i, chord_id[i]);
	printf("\n");
	
	qsort(chord_id + 1, NB_SITE, sizeof(int), tri);

	printf("Sorted chord id array :\n\n");
	for (i = 0; i < NB_SITE + 1; i++)
		printf("chord_id[%d] = %d\n", i, chord_id[i]);
	printf("\n");
	/*
	 * Calculation of the finger array and filling of the chord id array 
	 * and associative table array
	 *    
	 */
	for(i = 1; i < NB_SITE; i++){
		
		/* Calcul de la finger table de i */
		for(j = 0; j < M; j++){
			val = (chord_id[i] + (1 << j));
			val = val % K;
       
			int tmp = K, min = K, l, n;
			for (k = 1; k < NB_SITE; k++){
				if (chord_id[k] < tmp && chord_id[k] >= val){
					tmp = chord_id[k];
					l = k;
				}
				if (chord_id[k] < min){
					min = chord_id[k];
					n = k;
				}
			}
			if (tmp == K){
				tmp = min;
				l = n;
			}
			printf("Node %d : finger[%d] = %d for key %d\n", chord_id[i], j, tmp, val);
			finger[i][j] = tmp;
			associative_table[i][l] = tmp;
		}
	
		printf("\n");
	}

	k = 0;
	l = 1;
	
	/* Filling the reverse array */
	for(i = 1; i < NB_SITE; i++){
		for(j = 0; j < M; j++){
			/* we search if the finger already exists in the reverse */
			while (l < NB_SITE){
				/* get the index of the finger in the chord_id table */
				if (chord_id[l] == finger[i][j])
					break;
				l++;
			}
			while (k < NB_SITE && reverse[l][k] != -1 && reverse[l][k] != chord_id[i])
				k++;
			/* if not, add it */
			if (reverse[l][k] == -1){
				reverse[l][k] = chord_id[i];
				associative_table[l][i] = chord_id[i];
			}
			k = 0;
			l = 1;
		}
	}

	for(i = 1; i < NB_SITE + 1; i++){
		for(j = 0; j < NB_SITE + 1; j++){
			printf("Node %d : reverse[%d] = %d\n", chord_id[i], j, reverse[i][j]);
		}
		printf("\n");
	}
	
/*	for(i = 1; i < NB_SITE + 1; i++){
		for(j = 0; j < NB_SITE + 1; j++){
			printf("Node %d : associative_table[%d] = %d\n", chord_id[i], j, associative_table[i][j]);
		}
		printf("\n");
	}
*/	
	for(i = 1; i < NB_SITE; i++){
		SEND_INT(i, TAGINIT, chord_id[i]);
		SEND_NINT(i, TAGINIT, finger[i], M);
		SEND_NINT(i, TAGINIT, associative_table[i], NB_SITE+1);
		SEND_NINT(i, TAGINIT, reverse[i], NB_SITE+1);
	}

	/* Send his id and associative table to the node to insert */
	SEND_INT(NB_SITE, TAGINIT, chord_id[NB_SITE]);
	SEND_NINT(NB_SITE, TAGINIT, chord_id, NB_SITE + 1);
	SEND_NINT(NB_SITE, TAGINIT, reverse[NB_SITE], NB_SITE + 1);
	
	i = rand() % (NB_SITE - 1) + 1;
	SEND_INT(NB_SITE, TAGINSERT, chord_id[i]);

	MPI_Recv(&val, 1, MPI_INT, MPI_ANY_SOURCE, TAGTERM, MPI_COMM_WORLD, NULL);
	printf("Simulator : received node %d\n", val);

	i = 0;
	while (i < NB_SITE && chord_id[i] != val)
		i++;

	j = 0;
	while (j < NB_SITE + 1 && reverse[i][j] != -1){
		j++;
	}
	printf("Simulator : cpt = %d\n", j);

	for (i = 0; i < j; i++)
		MPI_Recv(&val, 1, MPI_INT, MPI_ANY_SOURCE, TAGTERM, MPI_COMM_WORLD, NULL);
	for (i = 1; i < NB_SITE + 1; i++)
		SEND_INT(i, TAGTERM, val);
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
		node(rang);
	  
	MPI_Finalize();
	return 0;
}
