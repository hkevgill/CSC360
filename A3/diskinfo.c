#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

int getBlockSize(char *mmap) {
	unsigned char *temp = malloc(sizeof(char));
	int i;
	int j = 1;
	int value = 0;

	for(i = 8; i < 10; i++){
		*temp = mmap[i];
		value += ((*temp)<<(8*j));
		j--;
	}

	free(temp);

	return value;
}

int getBlockCount(char *mmap){
	unsigned char *temp = malloc(sizeof(unsigned char));

	int i;
	int j = 3;
	int value = 0;

	for(i = 10; i < 14; i++){
		*temp = mmap[i];
		value += ((*temp)<<(8*j));
		j--;
	}

	free(temp);

	return value;
}

int fatStart(char *mmap){
	unsigned char *temp = malloc(sizeof(unsigned char));

	int i;
	int j = 3;
	int value = 0;

	for(i = 14; i < 18; i++){
		*temp = mmap[i];
		value += ((*temp)<<(8*j));
		j--;
	}

	free(temp);

	return value;
}

int fatBlocks(char *mmap){
	unsigned char *temp = malloc(sizeof(unsigned char));

	int i;
	int j = 3;
	int value = 0;

	for(i = 18; i < 22; i++){
		*temp = mmap[i];
		value += ((*temp)<<(8*j));
		j--;
	}

	free(temp);

	return value;
}

int getRootStart(char *mmap){
	unsigned char *temp = malloc(sizeof(unsigned char));

	int i;
	int j = 3;
	int value = 0;

	for(i = 22; i < 26; i++){
		*temp = mmap[i];
		value += ((*temp)<<(8*j));
		j--;
	}

	free(temp);

	return value;
}

int getRootBlocks(char *mmap){
	unsigned char *temp = malloc(sizeof(unsigned char));

	int i;
	int j = 3;
	int value = 0;

	for(i = 26; i < 30; i++){
		*temp = mmap[i];
		value += ((*temp)<<(8*j));
		j--;
	}

	free(temp);

	return value;
}

int main(int argc, char *argv[]){
	// FILE *fp; // File pointer
	int fd; // File descriptor
	struct stat sf; // struct stat holds information about a file
	char *p; // Pointer to file
	int size;

	// fd is now the file descriptor for the opened file
	if((fd=open("test.img", O_RDONLY))){
		// fstat returns information about the file into sf
		fstat(fd, &sf);

		p = mmap(NULL, sf.st_size, PROT_READ, MAP_SHARED, fd, 0);

		printf("Super block information:\n");
		size = getBlockSize(p);
		printf("Block size: %d\n", size);
		printf("Block count: %d\n", getBlockCount(p));
		printf("FAT starts: %d\n", fatStart(p));
		printf("FAT blocks: %d\n", fatBlocks(p));
		printf("Root directory start: %d\n", getRootStart(p));
		printf("Root directory blocks: %d\n", getRootBlocks(p));

		printf("\n");

		printf("Fat information:\n");
	}

	return 0;
}
