#include "fileutils.c"
#include <time.h>

FILE *in;

long int file_size_in_bytes = 0;
int number_of_blocks_needed= 0;

void checkFile(char * file){
	int i = 0;
	for(i = 0; i < files.size; i ++){
		if(strcmp(files.files[i].name, file) == 0){
			printf("File Already exists in Disk!!\n");
			exit(1);
		}
	}

	printf("No file with same name, Proceeding to upload file.\n");
	
}

int findNextAvailableFatPosition(){
	int i = 0;
	int temp = 0;
	//printf("Number of fatblocks: %d\n", FATBlocks);
	for(i = 0; i < FATBlocks*512/4; i++){
		temp = readFourBlocksInt(diskFile, (FATStart*blockSize)+(i*4));
		if(temp == 0x00000000){
			return i;
		}
	}
	printf("No more room in FAT TABLE!!\n");
	exit(1);
	return -1;
}

void addToRootDir(int start, int blocks, int size, char * name ){
	int *hold = (int *) malloc(sizeof(int));

	time_t t = time(0);
	struct tm *tm;

	tm = gmtime(&t);
	//strftime(date, sizeof(date), "%Y%m%d", tm);

	int offset = (rootStart*blockSize) + (64*files.size);

	//status
	fseek(diskFile, offset, SEEK_SET);
	*hold = 3;
	fwrite(hold, 1,1, diskFile);
	offset += 1;

	//Starting Block 4 bytes
	writeFourBlocksInt(diskFile, offset, start);
	offset += 4;

	//Number of Blocks 4 bytes
	writeFourBlocksInt(diskFile, offset, blocks);
	offset += 4;

	//file size in bytes 4 bytes
	writeFourBlocksInt(diskFile, offset, size);
	offset += 4;

	//create Time 7 bytes set so current
	//year 
	writeTwoBlocksInt(diskFile, offset, tm->tm_year + 1900);
	offset += 2;

	//month
	//int *hold;
	*hold = tm->tm_mon + 1;
	fwrite(hold, 1,1,diskFile);
	offset += 1;

	//day
	*hold = tm->tm_mday;
	fwrite(hold, 1,1,diskFile);
	offset += 1;

	//hour
	*hold = tm->tm_hour;
	fwrite(hold, 1,1,diskFile);
	offset += 1;

	//min
	*hold = tm->tm_min;
	fwrite(hold, 1,1,diskFile);
	offset += 1;

	//second
	*hold = tm->tm_sec;
	fwrite(hold, 1,1,diskFile);
	offset += 1;

	//modify Time 7 bytes set to current
	//year 
	writeTwoBlocksInt(diskFile, offset, tm->tm_year+1900);
	offset += 2;

	//month
	*hold = tm->tm_mon + 1;
	fwrite(hold, 1,1,diskFile);
	offset += 1;

	//day
	*hold = tm->tm_mday;
	fwrite(hold, 1,1,diskFile);
	offset += 1;

	//hour
	*hold = tm->tm_hour;
	fwrite(hold, 1,1,diskFile);
	offset += 1;

	//min
	*hold = tm->tm_min;
	fwrite(hold, 1,1,diskFile);
	offset += 1;

	//second
	*hold = tm->tm_sec;
	fwrite(hold, 1,1,diskFile);
	offset += 1;

	//File name 31 bytes
	fwrite(name, 31, 1, diskFile);
	offset += 31;

	//unused 6 bytes set to 0xFF
	writeFourBlocksInt(diskFile,offset, 0xFFFFFFFF);
	offset += 4;
	writeTwoBlocksInt(diskFile, offset, 0xFFFF);
	offset += 2;

}


int main ( int argc, char *argv[] )
{
	char * filename = NULL;
	char infile[31] = {0};
	if ( argc < 3 )
	{
		printf ("Usage: ./diskput {diskimage} {filename}\n");
		printf ("\t\tdiskimage is the input .img to use \n");
		printf ("\t\tfilename is the input .txt to get \n");
		exit(0);
	}

	
	filename = argv[1];
	strncpy(infile, argv[2],31);

	diskFile = fopen(filename, "r+");
	if (diskFile == NULL)
	    exit(EXIT_FAILURE);

	initVariables();
	grabRootFiles();

	checkFile(infile);

	in = fopen(infile, "r");
	if(in == NULL){ 
		printf("File not found.\n");
		exit(EXIT_FAILURE);
	}

	fseek(in, 0, SEEK_END);
	file_size_in_bytes = ftell(in);
	fseek(in, 0, SEEK_SET);

	int remaing = file_size_in_bytes % 512;
	number_of_blocks_needed = ((file_size_in_bytes - remaing) / 512);
	if(remaing > 0) number_of_blocks_needed += 1;
	
	//printf("This file will need this many blocks: %d\n", number_of_blocks_needed );

	//printf("Length of input file is: %li\n",file_size_in_bytes );

	int blocks[number_of_blocks_needed];

	int firstblock = findNextAvailableFatPosition();

	blocks[0] = firstblock;
	//printf("firstblock: %d\n", firstblock);


	addToRootDir(firstblock, number_of_blocks_needed, file_size_in_bytes, infile);

	int i = 0;
	//printf("Number of blocks number_of_blocks_needed: %d\n",number_of_blocks_needed );
	writeFourBlocksInt(diskFile, FATStart*blockSize + blocks[0]*4, 0xFFFFFFFF);
	for(i = 1; i < number_of_blocks_needed; i++){
		blocks[i] = findNextAvailableFatPosition();
		writeFourBlocksInt(diskFile, FATStart*blockSize + blocks[i-1]*4, blocks[i]);
		writeFourBlocksInt(diskFile, FATStart*blockSize + blocks[i]*4, 0xFFFFFFFF);
		//printf("After things current blocks[i]is: %d\n",blocks[i]);
		//printf("Count of blocks: %d\n", i+1 );
	}

	char * read = (char *)malloc(sizeof(char)*512);
	int remaining_bytes = file_size_in_bytes % 512;
	for(i = 0; i < number_of_blocks_needed; i++){

		fseek(diskFile, blocks[i]*512, SEEK_SET);

		if(i == number_of_blocks_needed - 1 && remaining_bytes > 0){

			fread(read, remaining_bytes, 1, in);
			fwrite(read, remaining_bytes, 1, diskFile);

		}else{
			fread(read, 512, 1, in);
			fwrite(read, 512, 1, diskFile);
		}
	}


	fclose(in);
	fclose(diskFile);

	return 0;
}