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
#include <time.h>

#define FILESIZE 4

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

void listFiles(char *mmap){
    int i, j, k = 0; // Iterators
    int length = 64; // Length of an entry
    int block_size = getBlockSize(mmap); // Size of 1 block
    char *root_entry = (char *)malloc(sizeof(char) * length); // Holds the 64B of the entry
    char *file_name_bytes = (char *)malloc(sizeof(char) * 31); // Holds the file name
    unsigned char *file_create_bytes = (unsigned char *)malloc(sizeof(unsigned char) * 7);
    unsigned char *file_size_bytes = (unsigned char *)malloc(sizeof(unsigned char) * FILESIZE); // Holds the bytes of the file size
    int offset = getRootStart(mmap) * block_size; // Where Root Directory starts
    int file_size = 0; // Size of the file

    char *f = "F";
    char *d = "D";

    int year, month, day, hour, minute, second = 0;

    int numRootBlocks = getRootBlocks(mmap); // Number of blocks in root directory

    for(i = 0; i < numRootBlocks; i++){ // Loop through the number of blocks in the root directory
        for(j = 0; j < 8; j++){ // Each directory is 64B so there are 8 directory entries per block
            root_entry = memcpy(root_entry, mmap+offset+block_size*i+length*j, length);
            if(((root_entry[0] & 0x03) == 0x03) || (root_entry[0] & 0x05) == 0x05){

                // Gets whether it is an F or a D
                if((root_entry[0] >> 1) & 0x01){ // If bit 1 is set (to 1) then it is a file
                    printf("%s ", f);

                }
                else if((root_entry[0] >> 2) & 0x01){ // If bit 2 is set then it is a directory
                    printf("%s ", d);
                }

                // Gets the name of the file
                file_name_bytes = memcpy(file_name_bytes, root_entry + 27, 31);

                // Gets the size of the file
                file_size_bytes = memcpy(file_size_bytes, root_entry + 9, 4);
                file_size = 0;
                for(k = 0; k < FILESIZE; k++){
                    file_size += ((int)file_size_bytes[k]<<(8 * (-k + FILESIZE - 1)));
                }
                printf("%10d", file_size);
                printf("%30s", file_name_bytes);

                // Gets the create time
                file_create_bytes = memcpy(file_create_bytes, root_entry + 13, 7);
                year = (file_create_bytes[0] << 8) + file_create_bytes[1];
                month = (file_create_bytes[2]);
                day = (file_create_bytes[3]);
                hour = (file_create_bytes[4]);
                minute = (file_create_bytes[5]);
                second = (file_create_bytes[6]);
                printf("%5d/%02d/%02d %02d:%02d:%02d\n", year, month, day, hour, minute, second);

            }
        }
    }

    free(root_entry);
    free(file_name_bytes);
    free(file_create_bytes);
    free(file_size_bytes);

}

int main(int argc, char *argv[]){
	#if defined(PART1)
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

	#elif defined(PART2)
		int fd; // File descriptor
	    struct stat sf; // struct stat holds information about a file
	    char *p; // Pointer to file

	    if(argc != 2){
	        printf("usage: %s filename\n", argv[0]);
	        return 0;
	    }

	    // fd is now the file descriptor for the opened file
	    if((fd=open(argv[1], O_RDONLY))){
	        // fstat returns information about the file into sf
	        fstat(fd, &sf);

	        p = mmap(NULL, sf.st_size, PROT_READ, MAP_SHARED, fd, 0);

	        listFiles(p);

	    }

	    return 0;
	#elif defined(PART3)

		return 0;
	#elif defined(PART4)

		return 0;
	#else
		printf("Error\n");
		return 0;
	#endif

}
