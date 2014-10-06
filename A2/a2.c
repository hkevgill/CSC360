#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#define MAXLINE 100

/* Global Variables */
FILE *fp;

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

int main(int argc, char *argv[]){

	char ReadLine[MAXLINE];
	char *tokens[1024];
	char *cmd_in = (char*) malloc(sizeof(char) * 100);
	int count;
	int loadTime;
	int crossTime;

	if(argc != 3){
		printf("usage: %s filename\n", argv[0]);
		return 0;
	}

	fp = fopen(argv[1], "r");
	if(fp == NULL){
		printf("Can't open file, Bye\n");
		return 0;
	}

	for(count = 0; count < atoi(argv[2]); count++){
		fgets(ReadLine, MAXLINE, fp);
		chomp(ReadLine);
		parse(ReadLine, tokens, cmd_in);

		printf("%s ", tokens[0]);
		printf("%s ", tokens[1]);
		printf("%s\n", tokens[2]);
	}



	fclose(fp);
	return 0;
}