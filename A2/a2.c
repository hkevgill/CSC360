#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#define MAXLINE 100

/* Global Variables */

FILE *fp; // File pointer

pthread_mutex_t track = PTHREAD_MUTEX_INITIALIZER; // Track mutex
pthread_mutex_t data_struct = PTHREAD_MUTEX_INITIALIZER; // PQ mutex
pthread_mutex_t condMutex = PTHREAD_MUTEX_INITIALIZER; // conArray mutex

pthread_cond_t *condArray; // Global array of condition variables
pthread_cond_t dispatchCond = PTHREAD_COND_INITIALIZER;

int trainsWaiting = 0;

// Struct for each thread
typedef struct train{
	int loadTime;
	int crossTime;
	char *priority;
	pthread_cond_t con;
	int id;
}train;

// Linked list (PQ) that keeps track of currently loaded trains
typedef struct PQueue{
	int id;
	char *priority;
	struct PQueue *next;
}PQ;

PQ *head = NULL; // Initialize head to null

// Insert the train into the PQ, in a special way:
// High priority in first half of PQ in order of being loaded
// Low priority in second half of PQ in order of being loaded
void insertTrain(int newID, char *newPriority){
	PQ *newTrain = (PQ *)malloc(sizeof(PQ));
	if(newTrain == NULL){
		exit(1);
	}

	newTrain->id = newID;
	newTrain->priority = newPriority;

	PQ *p;
	p = head;

	if(head == NULL){
		newTrain->next = NULL;
		head = newTrain;
	}
	else if(!strcmp(newTrain->priority, "East") || !strcmp(newTrain->priority, "West")){
		while(p->next != NULL && !strcmp(p->next->priority, "east") && !strcmp(p->next->priority, "west")){
			p = p->next;
		}
		newTrain->next = p->next;
		p->next = newTrain;
	}
	else if(!strcmp(newTrain->priority, "east") || !strcmp(newTrain->priority, "west")){
		newTrain->next = NULL;

		while(p->next != NULL){
			p = p->next;
		}
		p->next = newTrain;

	}
	else{
		printf("Train was scheduled wrong, break program");
		exit(1);
	}

}

// Delete train from PQ by its id
void deleteTrain(int newID){
	PQ *curr = head;
	PQ *prev;
	while(curr != NULL){
		if(( (curr->id) == newID)){
			if(curr == head){
				head = curr->next;
				free(curr);
				return;
			}
			else{
				prev->next = curr->next;
				free(curr);
				return;
			}
		}
		else{
			prev = curr;
			curr = curr->next;
		}
	}
}

// Removes newline '\n' characters, assumes they are at the end of the line
void chomp(char line[]){
	assert(line != NULL);

	if(strlen(line) == 0){
		return;
	}

	while(line[strlen(line)-1] == '\n'){
		line[strlen(line)-1] = '\0';
	}

	return;
}

// Tokenize the text file to create new threads
void parse(char ReadLine[MAXLINE], char *tokens[1024], char *cmd_in){
	int count = 0;

	cmd_in = strtok(ReadLine, ":");

	while(cmd_in != NULL){
		tokens[count] = cmd_in;
		count++;
		cmd_in = strtok(NULL, ",");
	}

}

// Train handler function
void *trains(void *args){

	train *t_cpy = args;
	usleep(t_cpy->loadTime*100000); // Loading time

	// Lock PQ and insert into the queue because the train has loaded
	pthread_mutex_lock(&data_struct);
	insertTrain(t_cpy->id, t_cpy->priority);
	trainsWaiting++;
	printf("Train %2d is ready to go %4s\n", t_cpy->id, t_cpy->priority);
	pthread_mutex_unlock(&data_struct);

	pthread_mutex_lock(&track); // Lock track

	condArray[t_cpy->id] = t_cpy->con; // Put condition variabled into global array

	pthread_cond_signal(&dispatchCond);
	//pthread_cond_wait(&condArray[t_cpy->id], &track); // Wait to cross

	printf("Train %2d is ON the main track going %4s\n", t_cpy->id, t_cpy->priority);
	usleep(t_cpy->crossTime*100000); // Crossing time
	printf("Train %2d is OFF the main track after going %4s\n", t_cpy->id, t_cpy->priority);

	/// Lock PQ and delete from queue because crossing is complete
	pthread_mutex_lock(&data_struct);
	trainsWaiting--;
	deleteTrain(t_cpy->id);
	pthread_mutex_unlock(&data_struct);

	pthread_mutex_unlock(&track); // Unlock track


	return ((void *)0);
}

// Main scheduling thread
// Figures out who goes next
// Sends a signal to that thread
int main(int argc, char *argv[]){

	condArray = (pthread_cond_t *)malloc(sizeof(pthread_cond_t)*atoi(argv[2])); // Allocate space for global convar array

	char ReadLine[MAXLINE];
	char *tokens[1024];
	char *cmd_in = (char*) malloc(sizeof(char) * 100);
	int count;
	int ret;

	// Make sure the right amount of arguments are passed
	if(argc != 3){
		printf("usage: %s filename\n", argv[0]);
		return 0;
	}

	// Open the file in read mode
	fp = fopen(argv[1], "r");
	if(fp == NULL){
		printf("Can't open file, Bye\n");
		return 0;
	}

	// FIGURE OUT WHAT ORDER EVERYTHING IS SCHEDULED HERE

	pthread_t *thread = (pthread_t *)malloc(sizeof(pthread_t)*atoi(argv[2])); // Array of threads
	pthread_cond_t *c = (pthread_cond_t *)malloc(sizeof(pthread_cond_t)*atoi(argv[2])); // Array of convars

	// Loop through and create a thread for each train
	for(count = 0; count < atoi(argv[2]); count++){
		fgets(ReadLine, MAXLINE, fp);
		chomp(ReadLine);
		parse(ReadLine, tokens, cmd_in);

		train *t = (train*)malloc(sizeof(train));
		if(!strcmp(tokens[0], "E"))
			t->priority = "East";
		if(!strcmp(tokens[0], "W"))
			t->priority = "West";
		if(!strcmp(tokens[0], "e"))
			t->priority = "east";
		if(!strcmp(tokens[0], "w"))
			t->priority = "west";
		t->loadTime = atoi(tokens[1]);
		t->crossTime = atoi(tokens[2]);
		t->id = count;
		t->con = c[count];

		ret = pthread_create(&thread[count], NULL, &trains, t);
		if(ret != 0){
			perror("pthread create error\n");
		}
	}

	// Test stuff
	// usleep(3*100000);
	// pthread_mutex_lock(&track);
	// pthread_cond_signal(&condArray[2]);tokens[1024],
	// pthread_mutex_unlock(&track);

	// pthread_join(thread[2], NULL);
	// pthread_mutex_lock(&track);
	// pthread_cond_signal(&condArray[1]);
	// pthread_mutex_unlock(&track);

	// pthread_join(thread[1], NULL);
	// pthread_mutex_lock(&track);
	// pthread_cond_signal(&condArray[0]);
	// pthread_mutex_unlock(&track);

	// pthread_join(thread[0], NULL);


	// Print what's in PQ
	// PQ *p;
	// for(p = head; p != NULL; p = p->next){
	// 	printf("%d ", p->id);
	// }


	// SCHEDULE! WORK IN PROGRESS
	int locked = 1;
	int temp;
	int i;
	for(i = 0; i < atoi(argv[2]); i++){
		locked = 1;
		pthread_mutex_lock(&data_struct);
		if(trainsWaiting == 0){
			pthread_cond_wait(&dispatchCond, &data_struct);
		}
		locked = pthread_mutex_trylock(&track);
		temp = head->id;
		pthread_cond_signal(&condArray[temp]);
		pthread_mutex_unlock(&track);

		pthread_mutex_unlock(&data_struct);
		if(locked == 0){
			pthread_join(thread[temp], NULL);
		}
	}

	//printf("MAIN FINISHED!!!\n");

	free(cmd_in);
	fclose(fp);
	return 0;
}