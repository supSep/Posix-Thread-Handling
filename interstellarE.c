/*
 * Thread Handling 
 * Threads are incoming in terms of H (hydrogren) or C ( carbon) and we are trying to create C2H ( 2 carbons and 1 hydrogen)
 *  *
 * Interstellar-space problem: skeleton with some meat
 * 
 * Sepehr Taheri
 *
 *
 */

#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>


/* Random # below threshold indicates H; otherwise C. */
#define ATOM_THRESHOLD 0.55
#define DEFAULT_NUM_ATOMS 50
#define TRUE   1
#define FALSE  0


/* Global / shared variables */
int  cNum = 0, hNum = 0;     
long numAtoms;

sem_t hSem;     // thread of H
sem_t cSem;     // threads of C
sem_t mutex;	// control thread


//int Cid[10000], Hid[10000]; // array to queue up C and H IDs DID NOT USE
int Cidcounter=1, Hidcounter=1; // counter for used atoms
int cid[2];				// array for temp C IDs
int id3;					// temp ID for H

int nCa=0, nH=0;  // counter for Active C , H

int ccount=0, hcount=0, RadCount=0;// total counters for C, H and C2H


/* Whatever needs doing for your global data structures, do them here. */
void init()
{
	sem_init(&hSem, 0, 0);
	sem_init(&cSem, 0, 0);
	sem_init(&mutex, 0, 1);
	cid[0] = 0,cid[1] =0;
	id3=0;
}


/* Needed to pass legit copy of an integer argument to a pthread */

int *dupInt( int i )
{
	int *pi = (int *)malloc(sizeof(int));
	assert( pi != NULL);
	*pi = i;
	return pi;
}


void makeRadical(int C1, int C2, int H)
{

	sem_post(&hSem);
	sem_post(&cSem);

	fprintf(stdout, "A ethynyl radical was made: c%03d  c%03d  h%03d\n",
		C1, C2, H);
	fprintf(stdout, "Total Ca:%d  Total H: %d \n", ccount, hcount);
	
	RadCount++;				 // increment radical counter
	Cidcounter +=2;     // increment ID counter for c array
	Hidcounter ++;      // increment ID counter for d array
	cid[0]=0,cid[1]=0;
	
	
}

void *hReady( void *arg )
{
	
		sem_wait(&mutex);   // lift lock
		nH++;								// increment active and total counters
		hcount++;
	
	
		int id = *((int *)arg);
		printf("h%d is alive\n", id);
		id3 = id;
		
		//Hid[hcount] = id;  // store id DID NOT USE

		
		if( nCa>= 2 && nH>=1 ){ //radical condtion
			sem_post(&cSem); // increment C semaphore twice
			sem_post(&cSem); // because we require 2 C
			nCa -=2;
			sem_post(&hSem); // increment H semaphore 
			nH--;
			makeRadical(cid[0],cid[1],id3); // make radical with IDs
		}
		else{
			sem_post(&mutex);  // if radical conditions not met, look for C
		}
			sem_post(&mutex); // if radical made, continue the flow
			sem_wait(&hSem);  // pause H thread

}


void *cReady( void *arg )
{
	
	sem_wait(&mutex); // lift lock
	nCa++;            // increment active and total counters
	ccount++;

	int id = *((int *)arg);
	printf("c%d is alive\n", id);

	//Cid[ccount] = id; // store id DID NOT USE
	 if (cid[0]==0){				// IF cId[0] is used , use CID[1]
	 			cid[0]=id;
	 		}
	 		else{
	 			cid[1]=id;
	 		}
	
	if( nCa>= 2 && nH>=1 ){   //radical conditions
			sem_post(&cSem);      // increment C semaphore twice
			sem_post(&cSem);
			nCa -=2; 							// decrement active counters
			sem_post(&hSem);			// increment H semaphore
			nH--;
			makeRadical(cid[0],cid[1],id3); // make radical with IDs // whoop whoop
		}
	else{
		        sem_post(&mutex); // look for more atoms
		}
	sem_wait(&cSem); // open the flow
		                     
}


int main(int argc, char *argv[])
{
	long seed;
	numAtoms = DEFAULT_NUM_ATOMS;
	pthread_t **atom;
	int i;
	int status;

	if ( argc < 2 ) {
		fprintf(stderr, "usage: %s <seed> [<num atoms>]\n", argv[0]);
		exit(1);
	}

	if ( argc >= 2) {
		seed = atoi(argv[1]);
	}

	if (argc == 3) {
		numAtoms = atoi(argv[2]);
		if (numAtoms < 0) {
			fprintf(stderr, "%ld is not a valid number of atoms\n",
				numAtoms);
			exit(1);
		}
	}

	init();
	srand(seed);
	atom = (pthread_t **)malloc(numAtoms * sizeof(pthread_t *));
	assert (atom != NULL);
	
	for (i = 0; i < numAtoms; i++) {
		usleep(6000); // prevent overflow
		atom[i] = (pthread_t *)malloc(sizeof(pthread_t));
		if ( (double)rand()/(double)RAND_MAX < ATOM_THRESHOLD ) {
			hNum++;
			status = pthread_create (
					atom[i], NULL, hReady,
					(void *)dupInt(hNum)
				);
		} else {
			cNum++;
			status = pthread_create (
					atom[i], NULL, cReady,
					(void *)dupInt(cNum)
				);
		}
		if (status != 0) {
			fprintf(stderr, "Error creating atom thread\n");
			exit(1);
		}
	}
	usleep(5000);
	printf("\nEnd of algorthim with C:%d     H:%d    \n\n", ccount, hcount);
	printf("Total of -> %d <- Radicals made\n\n", RadCount);
	 if(ccount % 2 == 0 ){ // even # of C
	 	 if(ccount/2 > hcount){  // every H is used
	 	 printf("Expected # of Radicals :%d\n\n", (Cidcounter/2));
	 	 }
	 	 else{  // every C is used
	 	 printf("Expected # of Radicals :%d\n\n", Hidcounter-1);
	 	 }
	 }
	 	else { // odd # of C
	         	 if(ccount+1/2 > hcount){ //to prevent half ints
	         	 	 printf("Expected # of Radicals :%d\n\n", (Cidcounter+1)/2);
	         	 }
	         	 else{
	         	 	 printf("Expected # of Radicals :%d\n\n", Hidcounter-1);
	         	 }
	      }
}