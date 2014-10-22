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
pthread_mutex_t joiner = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t *condArray; // Global array of condition variables

//int trainsWaiting = 0;

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
	int loadTime;
	struct PQueue *next;
}PQ;

PQ *head = NULL; // Initialize head to null

// Insert the train into the PQ, in a special way:
// High priority in first half of PQ in order of being loaded
// Low priority in second half of PQ in order of being loaded
void insertTrain(int newID, char *newPriority, int newLoadTime){
	PQ *newTrain = (PQ *)malloc(sizeof(PQ));
	if(newTrain == NULL){
		exit(1);
	}

	newTrain->id = newID;
	newTrain->priority = newPriority;
	newTrain->loadTime = newLoadTime;

	PQ *p;
	p = head;

	PQ *sudoHead;
	sudoHead = head;

	int front = 0;

	if(head == NULL){ // List is empty
		newTrain->next = NULL;
		head = newTrain;
	}
	else if(!strcmp(newTrain->priority, "East") || !strcmp(newTrain->priority, "West")){ // Insert in high priority half
		while(p->next != NULL && strcmp(p->next->priority, "east") && strcmp(p->next->priority, "west")){ // This needs to stop in the right place
			if((newTrain->loadTime == p->loadTime) && strcmp(p->priority, "east") && strcmp(p->priority, "west")){
				if((newTrain->id - p->id) > 0){ // Check if we add this train to the right
					if((newTrain->id - p->id) == 1){
						break;
					}
					if((newTrain->loadTime == p->next->loadTime) && strcmp(p->next->priority, "east") && strcmp(p->next->priority, "west")){
						if((newTrain->id - p->next->id) < 0){
							break;
						}
						sudoHead = p;
						p = p->next;
						continue;
					}
					else if(!strcmp(p->next->priority, "east") || !strcmp(p->next->priority, "west")){
						break;
					}
				}
				else{ // Check if we add this train to the left
					// Insert infront
					newTrain->next = sudoHead;
					sudoHead = newTrain;
					front = 1;
					break;
				}
			}
			sudoHead = p;
			p = p->next;
		}
		if(front){
			front = 0;
		}
		else{
			newTrain->next = p->next;
			p->next = newTrain;
		}
	}
	else if(!strcmp(newTrain->priority, "east") || !strcmp(newTrain->priority, "west")){ // Insert in low priority half
		// newTrain->next = NULL; // May not be null after new logic for same loadTime's is implemented

		// Get it to the start of the low priority
		while(p->next != NULL  && strcmp(p->priority, "east") && strcmp(p->priority, "west")){
			sudoHead = p;
			p = p->next;
		}

		while(p->next != NULL){ // This needs to stop in the right place
			if((newTrain->loadTime == p->loadTime) && (!strcmp(p->priority, "east") || !strcmp(p->priority, "west"))){
				if((newTrain->id - p->id) > 0){ // Check if we add this train to the right
					if((newTrain->id - p->id) == 1){
						break;
					}
					if(newTrain->loadTime == p->next->loadTime){
						if((newTrain->id - p->next->id) < 0){
							break;
						}
						sudoHead = p;
						p = p->next;
						continue;
					}
				}
				else{ // Check if we need to add train to the left
					// Insert front
					front = 1;
					newTrain->next = sudoHead;
					sudoHead = newTrain;
					break;
				}
			}
			p = p->next;
		}
		if(front){
			front = 0;
		}
		else{
			newTrain->next = p->next;
			p->next = newTrain;
		}

	}
	else{
		printf("This should never happen, check if input is correct\n");
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
	insertTrain(t_cpy->id, t_cpy->priority, t_cpy->loadTime);

	printf("Train %2d is ready to go %4s\n", t_cpy->id, t_cpy->priority);
	pthread_mutex_unlock(&data_struct);

	pthread_mutex_lock(&track); // Lock track

	condArray[t_cpy->id] = t_cpy->con; // Put condition variabled into global array

	pthread_cond_wait(&condArray[t_cpy->id], &track); // Wait to cross

	printf("Train %2d is ON the main track going %4s\n", t_cpy->id, t_cpy->priority);
	usleep(t_cpy->crossTime*100000); // Crossing time
	printf("Train %2d is OFF the main track after going %4s\n", t_cpy->id, t_cpy->priority);

	/// Lock PQ and delete from queue because crossing is complete
	pthread_mutex_lock(&data_struct);
	deleteTrain(t_cpy->id);
	pthread_mutex_unlock(&data_struct);

	pthread_mutex_unlock(&track); // Unlock track

	// PQ *w;

	// for(w = head; w != NULL; w = w->next){
	// 	printf("%s: %d\n", w->priority, w->id);
	// }

	pthread_mutex_lock(&joiner);
	usleep(1000); // Small delta delay
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

	// SCHEDULE! WORK IN PROGRESS
	int count2 = 0;
	PQ *p;
	char *lastPriority;
	int temp;
	for(;;){

		pthread_mutex_lock(&data_struct);

		if(pthread_mutex_trylock(&track) == 0){
			if(head != NULL){
				pthread_mutex_lock(&joiner);
				// temp = head->id;
				// lastPriority = head->priority;

				if(count2 == 0){ // Then this is the first train
					temp = head->id;
					lastPriority = head->priority;
					for(p = head; p != NULL; p = p->next){ // Count amount of items in the list
						if(!strcmp(p->priority, "East")){
							temp = p->id;
							lastPriority = p->priority;
							break;
						}
					}
					if(strcmp(lastPriority, "East")){
						for(p = head; p != NULL; p = p->next){
							if(!strcmp(p->priority, "West")){
								temp = p->id;
								lastPriority = p->priority;
								break;
							}
						}
						if(strcmp(lastPriority, "West")){
							for(p = head; p != NULL; p = p->next){
								if(!strcmp(p->priority, "east")){
									temp = p->id;
									lastPriority = p->priority;
									break;
								}
							}
						}
					}
				}
				else{
					if(!strcmp(lastPriority, "East") || !strcmp(lastPriority, "West")){ // Then last one was High Priority
						if(!strcmp(lastPriority, "East")){ // Last one was High priority East
							for(p = head; p != NULL; p = p->next){
								if(!strcmp(p->priority, "west")){
									temp = p->id;
									lastPriority = p->priority;
									break;
								}
							}
							if(strcmp(lastPriority, "west")){
								for(p = head; p != NULL; p = p->next){
									if(!strcmp(p->priority, "east")){
										temp = p->id;
										lastPriority = p->priority;
										break;
									}
								}
							}
							for(p = head; p != NULL; p = p->next){
								if(!strcmp(p->priority, "West")){
									temp = p->id;
									lastPriority = p->priority;
									break;
								}
							}
							if(!strcmp(lastPriority, "west") || !strcmp(lastPriority, "east")){
								for(p = head; p != NULL; p = p->next){
									if(!strcmp(p->priority, "East") && (p->id != temp)){
										temp = p->id;
										lastPriority = p->priority;
										break;
									}
								}
							}
						}
						else if(!strcmp(lastPriority, "West")){ // Last one was High priority West
							for(p = head; p != NULL; p = p->next){
								if(!strcmp(p->priority, "east")){
									temp = p->id;
									lastPriority = p->priority;
									break;
								}
							}
							if(strcmp(lastPriority, "east")){
								for(p = head; p != NULL; p = p->next){
									if(!strcmp(p->priority, "west")){
										temp = p->id;
										lastPriority = p->priority;
										break;
									}
								}
							}
							for(p = head; p != NULL; p = p->next){
								if(!strcmp(p->priority, "East")){
									temp = p->id;
									lastPriority = p->priority;
									break;
								}
							}
							if(!strcmp(lastPriority, "east") || !strcmp(lastPriority, "west")){
								for(p = head; p != NULL; p = p->next){
									if(!strcmp(p->priority, "West") && (p->id != temp)){
										temp = p->id;
										lastPriority = p->priority;
										break;
									}
								}
							}
						}
					}
					else{ // Then last one was Low Priority
						if(!strcmp(lastPriority, "east")){ // Last one was low priority east
							for(p = head; p != NULL; p = p->next){
								if(!strcmp(p->priority, "West")){
									temp = p->id;
									lastPriority = p->priority;
									break;
								}
							}
							if(!strcmp(lastPriority, "east")){
								for(p = head; p != NULL; p = p->next){
									if(!strcmp(p->priority, "East")){
										temp = p->id;
										lastPriority = p->priority;
										break;
									}
								}
							}
							if(strcmp(lastPriority, "West") && strcmp(lastPriority, "East")){
								for(p = head; p != NULL; p = p->next){
									if(!strcmp(p->priority, "west")){
										temp = p->id;
										lastPriority = p->priority;
										break;
									}
								}
								if(strcmp(lastPriority, "west")){
									for(p = head; p != NULL; p = p->next){
										if(!strcmp(p->priority, "east")){
											temp = p->id;
											lastPriority = p->priority;
											break;
										}
									}
								}
							}
						}
						else if(!strcmp(lastPriority, "west")){ // Last one was low priority west
							for(p = head; p != NULL; p = p->next){
								if(!strcmp(p->priority, "East")){
									temp = p->id;
									lastPriority = p->priority;
									break;
								}
							}
							if(!strcmp(lastPriority, "west")){
								for(p = head; p != NULL; p = p->next){
									if(!strcmp(p->priority, "West")){
										temp = p->id;
										lastPriority = p->priority;
										break;
									}
								}
							}
							if(strcmp(lastPriority, "West") && strcmp(lastPriority, "East")){
								for(p = head; p != NULL; p = p->next){
									if(!strcmp(p->priority, "east")){
										temp = p->id;
										lastPriority = p->priority;
										break;
									}
								}
								if(strcmp(lastPriority, "east")){
									for(p = head; p != NULL; p = p->next){
										if(!strcmp(p->priority, "west")){
											temp = p->id;
											lastPriority = p->priority;
											break;
										}
									}
								}
							}
						}
					}
				}

				pthread_cond_signal(&condArray[temp]);

				pthread_mutex_unlock(&track);

				pthread_mutex_unlock(&data_struct);

				pthread_mutex_unlock(&joiner);

				pthread_join(thread[temp], NULL);
				pthread_mutex_unlock(&joiner);

				count2++;
			}
			else if(count2 == atoi(argv[2])){
				pthread_mutex_unlock(&track);
				pthread_mutex_unlock(&data_struct);
				break;
			}
			else{
				pthread_mutex_unlock(&track);
				pthread_mutex_unlock(&data_struct);
				continue;
			}
		}
	}

	free(cmd_in);
	fclose(fp);
	return 0;
}
