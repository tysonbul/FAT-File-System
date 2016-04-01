#include "fileutils.c"

//FILE *diskFile;

void superBlockInfo(){
	printf("Super Block Information:\n");
	//Get Disk Name
	fseek(diskFile, IDENT_OFFSET, SEEK_SET);
	printf("Block size: %d\n", blockSize);
	printf("Block count: %d\n", blockCount);
	printf("FAT starts: %d\n", FATStart);
	printf("FAT blocks: %d\n", FATBlocks);
	printf("Root directory starts: %d\n", rootStart);
	printf("Root directory blocks: %d\n", rootBlocks);
	printf("\n");
	return;
}


void fatBlockInfo(){

	printf("Free blocks: %d\n", free_count);
	printf("Reserved blocks: %d\n", reserved_count);
	printf("Allocated blocks: %d\n", allocated_count);

	return;
}

int main ( int argc, char *argv[] )
{
	char * filename = NULL;
	if ( argc < 2 )
	{
		printf ("Usage: ./diskinfo {filename}\n");
		printf ("\t\tfilename is the input .img to use \n");
		exit(0);
	}

	filename = argv[1];

	diskFile = fopen(filename, "r");
	if (diskFile == NULL)
	    exit(EXIT_FAILURE);

	initVariables();

	superBlockInfo();
	fatBlockInfo();

	fclose(diskFile);

	return 0;
}