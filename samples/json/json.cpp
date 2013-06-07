#include <stdio.h>
#include <string.h>
#include "JSONParser.h"
#include "Filesystem.h"
#include "DiskFile.h"

using namespace crown;

// REMOVE JSON SAMPLE, ADD UNIT TEST
int main(int argc, char** argv)
{
	// if (argc != 2)
	// {
	// 	printf("Configuration root path must be provided. Aborting!");
	// 	return -1;
	// }

	// Filesystem conf_root(argv[1]);

	// if (!conf_root.exists("json.json"))
	// {
	// 	printf("Configuration file does not exists. Aborting!\n");
	// 	return -1;
	// }
	
	// char dst[10][256];

	// DiskFile* stream = (DiskFile*)conf_root.open("json.json", FOM_READ);

 // 	JSONParser* parser = new JSONParser(stream);


	// conf_root.close(stream);

	return 0;
}