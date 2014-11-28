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

void getFile(char *mmap, int start, char *fileName, int numOfBlocks, int fileSize){
	FILE *fp;
	fp = fopen(fileName, "w");

	int blockSize = getBlockSize(mmap); // Block size
	int startFat = fatStart(mmap); // Block where FAT starts
	int index = (blockSize*startFat) + (start*4); // index of FAT for next block
	int i,j;

	unsigned char *temp1 = malloc(sizeof(unsigned char));
	unsigned char *temp2 = malloc(sizeof(unsigned char));
	unsigned char *temp3 = malloc(sizeof(unsigned char));
	unsigned char *temp4 = malloc(sizeof(unsigned char));

	// printf("%d\n", numOfBlocks);

	for(i = 0; i < numOfBlocks; i++){
		for(j = start*blockSize; j < start*blockSize+blockSize; j++){
			fprintf(fp, "%c", mmap[j]);
		}
		index = (blockSize*startFat) + (start*4);
		*temp1 = mmap[index];
		*temp2 = mmap[index+1];
		*temp3 = mmap[index+2];
		*temp4 = mmap[index+3];

		index = (blockSize*startFat) + (start*4);
		start = ((*temp1)<<24) + ((*temp2)<<16) + ((*temp3)<<8) + (*temp4);
	}

	fclose(fp);

}

void findFile(char *mmap, char *fileName){

	int i, j = 0; // Iterators
    int length = 64; // Length of an entry
    int block_size = getBlockSize(mmap); // Size of 1 block
    char *root_entry = (char *)malloc(sizeof(char) * length); // Holds the 64B of the entry
    char *file_name_bytes = (char *)malloc(sizeof(char) * 31); // Holds the file name
    unsigned char *starting_block = (unsigned char *)malloc(sizeof(unsigned char) * 4);
    unsigned char *file_size = (unsigned char *)malloc(sizeof(unsigned char) * 4);
    unsigned char *num_of_blocks = (unsigned char *)malloc(sizeof(unsigned char) * 4);
    unsigned char *file_create_bytes = (unsigned char *)malloc(sizeof(unsigned char) * 7);
    unsigned char *file_size_bytes = (unsigned char *)malloc(sizeof(unsigned char) * FILESIZE); // Holds the bytes of the file size
    int offset = getRootStart(mmap) * block_size; // Where Root Directory starts

    int start = 0; // Starting block of the file to be read
    int numOfBlocks = 0; // Number of blocks the file takes up
    int fileSize = 0; // Size of the file

    int found = 0; // boolean to see if file was found

    int numRootBlocks = getRootBlocks(mmap); // Number of blocks in root directory

    for(i = 0; i < numRootBlocks; i++){ // Loop through the number of blocks in the root directory
        for(j = 0; j < 8; j++){ // Each directory is 64B so there are 8 directory entries per block
            root_entry = memcpy(root_entry, mmap+offset+block_size*i+length*j, length);
            if(((root_entry[0] & 0x03) == 0x03) || (root_entry[0] & 0x05) == 0x05){

                // Make sure it is a file
                if((root_entry[0] >> 1) & 0x01){ // If bit 1 is set (to 1) then it is a file

	                // Gets the name of the file
	                file_name_bytes = memcpy(file_name_bytes, root_entry + 27, 31);
	                if(!strcmp(fileName, file_name_bytes)){ // Check if file is in filesystem
	                	found = 1;

	                	starting_block = memcpy(starting_block, root_entry + 1, 4); // Starting block of the file
	                	start = (starting_block[0] << 24) + (starting_block[1] << 16) + (starting_block[2] << 8) + starting_block[3];

	                	num_of_blocks = memcpy(num_of_blocks, root_entry + 5, 4); // Number of blocks in the file
	                	numOfBlocks = (num_of_blocks[0] << 24) + (num_of_blocks[1] << 16) + (num_of_blocks[2] << 8) + num_of_blocks[3];

	                	file_size = memcpy(file_size, root_entry + 9, 4);
	                	fileSize = (file_size[0] << 24) + (file_size[1] << 16) + (file_size[2] << 8) + file_size[3];

	                	getFile(mmap, start, fileName, numOfBlocks, fileSize); // Now get it
	                	return;
	                }
            	}
            }
        }
    }

    if(found == 0){
    	printf("File not found\n");
    }

    free(root_entry);
    free(file_name_bytes);
    free(file_create_bytes);
    free(file_size_bytes);

}

void putFile(FILE *fp, FILE *disk, struct stat sf, char *fileName, char *mmap){
	char modifyTime[200]; // Will hold modify time
	char createTime[200]; // Will hold create time

	// Get file size
	int fileSize = (int)sf.st_size;

	// Calculate number of blocks
	int block_size = getBlockSize(mmap);
	int numOfBlocks = fileSize / block_size;
	if((fileSize % block_size) != 0){
		numOfBlocks = numOfBlocks + 1;
	}

	// Get times
	time_t mt = sf.st_mtime; // modify time
	time_t ct = sf.st_ctime; // Create time
	struct tm *tm;
	struct tm *tm1;

	// Get modify time in local time
	tm = localtime(&mt);
	strftime(modifyTime, sizeof(modifyTime), "%Y%m%d%H%M%S", tm);

	// Get create time in local time
	tm1 = localtime(&ct);
	strftime(createTime, sizeof(createTime), "%Y%m%d%H%M%S", tm1);

	printf("filename: %s\n", fileName);
	printf("filesize: %d\n", fileSize);
	printf("number of blocks: %d\n", numOfBlocks);
	printf("modify time: %s\n", modifyTime);
	printf("create time: %s\n", createTime);



	// Writes all of fp to disk. Don't need to do endian translation except when copying to FAT, and root dir.
	// fwrite(fp, 1, 2560, disk);

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
	    int fd; // File descriptor
	    struct stat sf; // struct stat holds information about a file
	    char *p; // Pointer to file

	    if(argc != 3){
	        printf("usage: %s filename\n", argv[0]);
	        return 0;
	    }

	    if((fd=open(argv[1], O_RDONLY))){
	        // fstat returns information about the file into sf
	        fstat(fd, &sf);

	        p = mmap(NULL, sf.st_size, PROT_READ, MAP_SHARED, fd, 0);

	        findFile(p, argv[2]);

	    }

		return 0;

	#elif defined(PART4)
		int fd, fd2; // File descriptor
		struct stat sf, sf2; // struct stat holds information about a file
		FILE *fp;
		FILE *disk;
		char *p; // Pointer to file

	    if(argc != 3){
	        printf("usage: %s filename\n", argv[0]);
	        return 0;
	    }

	    // fd is now the file descriptor for the opened file
	    if((fd=open(argv[2], O_RDONLY))){
	        // fstat returns information about the file into sf
	        fstat(fd, &sf);

	        if((fd2=open(argv[1], O_RDONLY))){

	        	fstat(fd2, &sf2);

		        p = mmap(NULL, sf2.st_size, PROT_READ, MAP_SHARED, fd2, 0);

				fp = fopen(argv[2], "r");
				if(fp){
					disk = fopen(argv[1], "r+");
					putFile(fp, disk, sf, argv[2], p);
				}
				else{
					printf("File not found\n");
					exit(0);
				}

		        fclose(fp);

	    	}

    	}

		return 0;

	#else

		printf("Error\n");

		return 0;

	#endif

}
