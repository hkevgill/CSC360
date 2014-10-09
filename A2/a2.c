#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#define MAXLINE 100

/* Global Variables */

pthread_cond_t *condArray;

FILE *fp;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct train{
	int loadTime;
	int crossTime;
	char priority;
	pthread_cond_t con;
	int id;
}train;

/*
 * Removes newline '\n' characters
 * Assumes they are at the end of the line
 */
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

void parse(char ReadLine[MAXLINE], char *tokens[1024], char *cmd_in){
	int count = 0;

	cmd_in = strtok(ReadLine, ":");

	while(cmd_in != NULL){
		tokens[count] = cmd_in;
		count++;
		cmd_in = strtok(NULL, ",");
	}

}

void *trains(void *args){

	train *t_cpy = args;
	usleep(t_cpy->loadTime);

	pthread_mutex_lock(&mutex);

	condArray[t_cpy->id] = t_cpy->con;

	pthread_cond_wait(&condArray[t_cpy->id], &mutex);

	printf("THREAD FINISHED %d\n", t_cpy->loadTime);

	pthread_mutex_unlock(&mutex);

	return ((void *)0);
}

int main(int argc, char *argv[]){

	condArray = (pthread_cond_t *)malloc(sizeof(pthread_cond_t)*atoi(argv[2]));

	char ReadLine[MAXLINE];
	char *tokens[1024];
	char *cmd_in = (char*) malloc(sizeof(char) * 100);
	int count;
	int ret;

	if(argc != 3){
		printf("usage: %s filename\n", argv[0]);
		return 0;
	}

	fp = fopen(argv[1], "r");
	if(fp == NULL){
		printf("Can't open file, Bye\n");
		return 0;
	}

	pthread_t *thread = (pthread_t *)malloc(sizeof(pthread_t)*atoi(argv[2]));
	pthread_cond_t *c = (pthread_cond_t *)malloc(sizeof(pthread_cond_t)*atoi(argv[2]));

	for(count = 0; count < atoi(argv[2]); count++){
		fgets(ReadLine, MAXLINE, fp);
		chomp(ReadLine);
		parse(ReadLine, tokens, cmd_in);

		train *t = (train*)malloc(sizeof(train));
		t->priority = atoi(tokens[0]);
		t->loadTime = atoi(tokens[1]);
		t->crossTime = atoi(tokens[2]);
		t->id = count;
		t->con = c[count];

		ret = pthread_create(&thread[count], NULL, &trains, t);
		if(ret != 0){
			perror("pthread create error\n");
		}
	}

	usleep(300);
	pthread_mutex_lock(&mutex);
	pthread_cond_signal(&condArray[2]);
	pthread_mutex_unlock(&mutex);

	pthread_join(thread[2], NULL);
	pthread_mutex_lock(&mutex);
	pthread_cond_signal(&condArray[1]);
	pthread_mutex_unlock(&mutex);

	pthread_join(thread[1], NULL);
	pthread_mutex_lock(&mutex);
	pthread_cond_signal(&condArray[0]);
	pthread_mutex_unlock(&mutex);

	pthread_join(thread[0], NULL);



	// int i;
	// for(i = 0; i < atoi(argv[2]); i++){
	// 	pthread_join(thread[i], NULL);
	// }

	printf("MAIN FINISHED!!!\n");

	free(cmd_in);
	fclose(fp);
	return 0;
}