#include "fileutils.c"

FILE *out;

File found;

void getFile(char * file){
	int i = 0;
	for(i = 0; i < files.size; i ++){
		if(strcmp(files.files[i].name, file) == 0){
			printf("File Found!!\n");
			found = files.files[i];
			printf("Found start block: %d, Found number of blocks: %d \n",found.start_block, found.number_of_blocks );
			return;
		}
	}
	printf("File Not Found!!!\n");
	exit(1);
}

void getBlocks(int blocks[]){

	//printf("Number of blocks in file: %d\n", found.number_of_blocks);
	//printf("Fat blocks in file: %d\n", FATBlocks);
	//printf("Fat entries in file: %d\n", FATBlocks*512/4);
	int i = 0;
	blocks[0] = found.start_block;
	//if(readFourBlocksInt(diskFile,FATStart*blockSize + blocks[0]*4 ) == 0xFFFFFFFF) printf("End of one block file\n");
	for(i = 1; i < found.number_of_blocks; i++){
		blocks[i] = readFourBlocksInt(diskFile, FATStart*blockSize + blocks[i-1]*4);
		//printf("Location of next block: %d\n", blocks[i]);
	}
	if(readFourBlocksInt(diskFile,FATStart*blockSize + blocks[found.number_of_blocks-1]*4 ) != 0xFFFFFFFF){
		printf("Error in Fat Table values!!!\n");
		exit(1);
	}
	return;

}

void copyFile(int blocks[]){
	int i = 0;
	//int j = 0;
	int bytes_written = 0;
	char * read = (char *)malloc(sizeof(char)*512);
	int remaining_bytes = found.size % 512;
	if(remaining_bytes == 0) remaining_bytes = 512;
	for (i = 0; i < found.number_of_blocks; i++)
	{
		fseek(diskFile, blocks[i]*blockSize, SEEK_SET);
		
		fread(read, 512, 1, diskFile);

		//if last block clean up trailing bytes
		if(i == found.number_of_blocks - 1){

			char hold[remaining_bytes + 1];
			strncpy(hold, read, remaining_bytes);
			hold[remaining_bytes] = '\0';
			fwrite(hold, remaining_bytes, 1, out);

		}else{
			fwrite(read, 512, 1, out);
			bytes_written += 512;
		}

	}

	free(read);
}

int main ( int argc, char *argv[] )
{
	char * filename = NULL;
	char * outfile = NULL;
	if ( argc < 3 )
	{
		printf ("Usage: ./diskget {diskimage} {filename}\n");
		printf ("\t\tdiskimage is the input .img to use \n");
		printf ("\t\tfilename is the input .txt to get \n");
		exit(0);
	}

	filename = argv[1];
	outfile = argv[2];

	diskFile = fopen(filename, "r");
	if (diskFile == NULL)
	    exit(EXIT_FAILURE);

	initVariables();
	grabRootFiles();

	getFile(outfile);

	out = fopen(outfile, "w");
		if(out == NULL) 
			exit(EXIT_FAILURE);

	int blocks[found.number_of_blocks];

	getBlocks(blocks);

	copyFile(blocks);

	fclose(out);
	fclose(diskFile);

	return 0;
}