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

int getFreeBlocks(char *mmap){
	int i;
	int freeBlocks = 0;
	int startFat = fatStart(mmap);
	int blockSize = getBlockSize(mmap);
	int numBlocks = fatBlocks(mmap);
	int startIndex = (startFat*blockSize);
	int endIndex = (startIndex + (blockSize*numBlocks));

	int value = 0;

	// printf("%d\n", startIndex);
	// printf("%d\n", endIndex);

	unsigned char *temp1 = malloc(sizeof(unsigned char));
	unsigned char *temp2 = malloc(sizeof(unsigned char));
	unsigned char *temp3 = malloc(sizeof(unsigned char));
	unsigned char *temp4 = malloc(sizeof(unsigned char));

	for(i = startIndex; i < endIndex; i = i + 4){
		*temp1 = mmap[i];
		*temp2 = mmap[i+1];
		*temp3 = mmap[i+2];
		*temp4 = mmap[i+3];

		value = ((*temp1)<<24) + ((*temp2)<<16) + ((*temp3)<<8) + (*temp4);
		if(value == 0){
			freeBlocks++;
		}
	}

	free(temp1);
	free(temp2);
	free(temp3);
	free(temp4);

	return freeBlocks;
}

int getReservedBlocks(char *mmap){
	int i;
	int reservedBlocks = 0;
	int startFat = fatStart(mmap);
	int blockSize = getBlockSize(mmap);
	int numBlocks = fatBlocks(mmap);
	int startIndex = (startFat*blockSize);
	int endIndex = (startIndex + (blockSize*numBlocks));

	int value = 0;

	// printf("%d\n", startIndex);
	// printf("%d\n", endIndex);

	unsigned char *temp1 = malloc(sizeof(unsigned char));
	unsigned char *temp2 = malloc(sizeof(unsigned char));
	unsigned char *temp3 = malloc(sizeof(unsigned char));
	unsigned char *temp4 = malloc(sizeof(unsigned char));

	for(i = startIndex; i < endIndex; i = i + 4){
		*temp1 = mmap[i];
		*temp2 = mmap[i+1];
		*temp3 = mmap[i+2];
		*temp4 = mmap[i+3];

		value = ((*temp1)<<24) + ((*temp2)<<16) + ((*temp3)<<8) + (*temp4);
		if(value == 1){
			reservedBlocks++;
		}
	}

	free(temp1);
	free(temp2);
	free(temp3);
	free(temp4);

	return reservedBlocks;
}

int getAllocatedBlocks(char *mmap){
	int allocatedBlocks = 0;

	int i;
	int startFat = fatStart(mmap);
	int blockSize = getBlockSize(mmap);
	int numBlocks = fatBlocks(mmap);
	int startIndex = (startFat*blockSize);
	int endIndex = (startIndex + (blockSize*numBlocks));

	unsigned int value = 0;

	unsigned char *temp1 = malloc(sizeof(unsigned char));
	unsigned char *temp2 = malloc(sizeof(unsigned char));
	unsigned char *temp3 = malloc(sizeof(unsigned char));
	unsigned char *temp4 = malloc(sizeof(unsigned char));

	for(i = startIndex; i < endIndex; i = i + 4){
		*temp1 = mmap[i];
		*temp2 = mmap[i+1];
		*temp3 = mmap[i+2];
		*temp4 = mmap[i+3];

		value = ((*temp1)<<24) + ((*temp2)<<16) + ((*temp3)<<8) + (*temp4);
		if(((value <= 0xFFFFFF00) && (value >= 2)) || (value == 0xFFFFFFFF)){
			allocatedBlocks++;
		}
	}

	free(temp1);
	free(temp2);
	free(temp3);
	free(temp4);

	return allocatedBlocks;
}

int main(int argc, char *argv[]){
	// FILE *fp; // File pointer
	int fd; // File descriptor
	struct stat sf; // struct stat holds information about a file
	char *p; // Pointer to file
	int size;

	if(argc != 2){
		printf("usage: %s filename\n", argv[0]);
		return 0;
	}

	// fd is now the file descriptor for the opened file
	if((fd=open(argv[1], O_RDONLY))){
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
		printf("Free Blocks: %d\n", getFreeBlocks(p));
		printf("Reserved Blocks:%d\n", getReservedBlocks(p));
		printf("Allocated Blocks:%d\n", getAllocatedBlocks(p));
	}

	return 0;
}
