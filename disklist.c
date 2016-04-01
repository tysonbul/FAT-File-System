#include "fileutils.c"

void rootBlockInfo(){
	int i = 0;
	printf("Root block info:\n");
	for(i=0; i < files.size; i++){

		//printf("Starting block: %d, Number of blocks: %d\n",files.files[i].start_block, files.files[i].number_of_blocks );
		printf("%c %10d %30s %19s\n",files.files[i].status, files.files[i].size, files.files[i].name, files.files[i].modify_date);
	}
}



int main ( int argc, char *argv[] )
{
	char * filename = NULL;
	if ( argc < 2 )
	{
		printf ("Usage: ./disklist {filename}\n");
		printf ("\t\tfilename is the input .img to use \n");
		exit(0);
	}
 
	filename = argv[1];

	diskFile = fopen(filename, "r");
	if (diskFile == NULL)
	    exit(EXIT_FAILURE);

	initVariables();
	grabRootFiles();
	//printf("Number of files: %d\n", files.size);
	//printf("Number of rootblocks: %d\n", rootBlocks);

	rootBlockInfo();

	fclose(diskFile);

	return 0;
}