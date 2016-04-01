#include <unistd.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include "Constants.h"
#include <arpa/inet.h>

FILE *diskFile;

uint16_t blockSize = 0;
uint32_t blockCount = 0;

uint32_t FATStart = 0;
uint32_t FATBlocks = 0;

uint32_t rootStart = 0;
uint32_t rootBlocks = 0;

int free_count = 0; 
int reserved_count = 0;
int allocated_count = 0;

typedef struct File File;

struct File{	
	char status;
	int size;
	char name[31];
	char modify_date[20];
	int start_block;
	int number_of_blocks;
};

typedef struct FileHolder FileHolder;

struct FileHolder{
	int size;
	int max_size;
	File *files;
};

FileHolder files;


enum { FILEINIT = 1, FILEGROW = 2 };

uint32_t readFourBlocksInt(FILE *fp, int offset){

	int * hold = malloc(sizeof(int));
	uint32_t val;
	
	fseek(fp, offset, SEEK_SET);

	fread(hold, 4, 1, fp);
	val = htonl(*hold);
	
	free(hold);

	return val;
}

uint16_t readTwoBlocksInt(FILE *fp, int offset){

	int * hold = malloc(sizeof(int));
	uint16_t val;
	
	fseek(fp, offset, SEEK_SET);

	fread(hold, 2, 1, fp);
	val = htons(*hold);
	
	free(hold);

	return val;
}

void writeFourBlocksInt(FILE *fp, int offset, int val){

	int * hold = malloc(sizeof(int));
	*hold = htonl(val);
	
	fseek(fp, offset, SEEK_SET);

	fwrite(hold, 4, 1, fp);
	
	free(hold);
	return;
}

void writeTwoBlocksInt(FILE *fp, int offset, int val){

	int * hold = malloc(sizeof(int));
	*hold = htons(val);
	
	fseek(fp, offset, SEEK_SET);

	fwrite(hold, 2, 1, fp);
	
	free(hold);
	return;
	//return val;
}



void addFile(File file){
	
	File *newfile;
	if(files.files == NULL){
		files.files = (File *) malloc(FILEINIT * sizeof(File));
		if(files.files == NULL) {printf("Error mallocing File:1\n"); exit(1);}
		files.max_size = FILEINIT;
 		files.size = 0;
	}else if(files.size >= files.max_size){
		newfile = (File *) realloc(files.files, (FILEGROW * files.max_size) * sizeof(File));
		if (newfile == NULL) {printf("Error mallocing File:2\n"); exit(1);}
		files.max_size *= FILEGROW;
		files.files = newfile;
	}
	files.files[files.size] = file;
	files.size++;

	//printf("Added file \n");

	return;
}

void grabRootFiles(){
	int status = 0;
	uint32_t starting_block = 0;
	uint32_t number_blocks = 0;
	uint32_t file_size = 0;

	char * file_name = malloc(sizeof(char) * 31);
	
	int * hold = malloc(sizeof(int));

	int * MM = malloc(sizeof(int));
	int * dd = malloc(sizeof(int));
	int * hh = malloc(sizeof(int));
	int * mm = malloc(sizeof(int));
	int * ss = malloc(sizeof(int));

	int offset = 0;

	int i = 0;
	for(i = 0; i < rootBlocks*8; i++){
		fseek(diskFile, (rootStart*blockSize) + (64*i), SEEK_SET);
		
		//status
		fread(hold, 1, 1, diskFile);
		status = *hold;
		*hold = 0;
		offset += 1;

		//Starting block
		starting_block =  readFourBlocksInt(diskFile, (rootStart*blockSize)+offset);
		offset += 4;

		//Number of block
		number_blocks = readFourBlocksInt(diskFile, (rootStart*blockSize)+offset);
		offset += 4;

		//File Size
		file_size = readFourBlocksInt(diskFile, (rootStart*blockSize)+offset);
		offset += 4;

		//Create Time
		fread(hold, 7, 1, diskFile);
		*hold = 0;
		offset += 7;

		//Modify Time
		uint16_t year = readTwoBlocksInt(diskFile, (rootStart*blockSize)+offset);
		fread(MM, 1, 1, diskFile);
		fread(dd, 1, 1, diskFile);
		fread(hh, 1, 1, diskFile);
		fread(mm, 1, 1, diskFile);
		fread(ss, 1, 1, diskFile);
		offset += 7;

		
		//File Name
		fread(file_name, 31, 1, diskFile);
		offset += 31;

		//Un-used
		fread(hold, 6, 1, diskFile);
		//unused = *hold;
		*hold = 0;
		offset += 6;
		if(!(status % 2 == 0)){
			File *newfile = (File *)malloc(sizeof(FILE));

			if(status == DIRECTORY_ENTRY_FILE + DIRECTORY_ENTRY_USED){
				newfile->status = 'F';
			}else{
				newfile->status = 'D';
			}

			newfile->size = file_size;
			newfile->number_of_blocks = number_blocks;
			newfile->start_block = starting_block;
			strncpy(newfile->name, file_name, 31);
			sprintf(newfile->modify_date, "%4d/%02d/%02d %02d:%02d:%02d", year,*MM,*dd, *hh, *mm, *ss);
			newfile->modify_date[20] = '\0';

			addFile(*newfile);
		}

		*hold = 0;
	}

	free(hold);
	free(file_name);
	free(MM);
	free(dd);
	free(hh);
	free(mm);
	free(ss);
}


void initVariables(){
	blockSize = readTwoBlocksInt(diskFile, BLOCKSIZE_OFFSET);
	blockCount = readFourBlocksInt(diskFile, BLOCKCOUNT_OFFSET);
	FATStart = readFourBlocksInt(diskFile, FATSTART_OFFSET);
	FATBlocks = readFourBlocksInt(diskFile, FATBLOCKS_OFFSET);
	rootStart = readFourBlocksInt(diskFile, ROOTDIRSTART_OFFSET);
	rootBlocks = readFourBlocksInt(diskFile, ROOTDIRBLOCKS_OFFSET);

	int i = 0;
	uint32_t val;

	for(i = 0; i < FATBlocks * blockSize; i = i + FAT_ENTRY_SIZE){

		val = readFourBlocksInt(diskFile, (FATStart*blockSize)+i);
		
		if( val == FAT_FREE ) free_count++;
		else if ( val == FAT_RESERVED ) reserved_count++;
		else allocated_count++;
	}

	return;
}