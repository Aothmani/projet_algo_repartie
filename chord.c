#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

#define NB_SITE 6 /* site simulateur inclus */
#define M 6
#define K 64
#define TAGINIT 0;


int find_resp_finger(int * finger, int wanted)
{
	int i;
	for (i = 0; i < M-1; i++){
		if (finger[i] <= wanted)
			return finger[i];
	}
	return finger[0];
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
	int cid, status, msg;
	int fingers[M];
	
	
	
	MPI_Recv(&cid, 1, MPI_INT, 0, TAGINIT, MPI_COMMWORLD, &status);
	MPI_Recv(&fingers, M, MPI_INT, 0, TAGINIT, MPI_COMMWORLD, &status);

	MPI_Recv(&msg, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMMWORLD, &status);
	while(status.MPI_TAG != TAGSTOP){
		
		MPI_Recv(&msg, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMMWORLD, &status);
	}
	
}

void simulateur(void)
{
	srand(getpid());

	int id[NB_SITE], finger[NB_SITE][M], i, val, j, k;
	
	memset(id, -1, NB_SITE);
	memset(finger[0]; -1; M);
	
	/* Calcul des id CHORD  */
	for(i = 1; i < NB_SITE; i++){
		val = rand() % K;
		for(j = 0; j < NB_SITE; j++){
			if(val == id[j]){
				val = rand%64;
				j = 0;
			}
		}
		id[i] = val;
	}

	qsort(&id, sizeof(id), sizeof(int), tri);
	
	/* Calcul des finger tables */
	for(i = 1; i < NB_SITE; i++){
		
		/* Calcul de la finger table de i */
		for(j = 0; j < M; j++){
			val = id[i] + pow(2, j);
			k = 1;
			while(k < NB_SITE && id[k] < val){
				k++;
			}
			if(j == NB_SITE)
				finger[i][j] = id[1];
			else
				finger[i][j] = id[k];
		}
	}

	for(i = 1; i < NB_SITE; i++){
		MPI_Send(id[i], 1 MPI_INT, i, TAGINIT, MPI_COMM_WORLD);
		MPI_Send(finger[i], M, MPI_INT, i, TAGINIT, MPI_COMM_WORLD);
	}
	
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
