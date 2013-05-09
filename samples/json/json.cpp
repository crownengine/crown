#include <stdio.h>
#include <string.h>
#include "JSONParser.h"
#include "Filesystem.h"
#include "FileStream.h"

using namespace crown;

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		printf("Configuration root path must be provided. Aborting!");
		return -1;
	}

	Filesystem conf_root(argv[1]);

	if (!conf_root.exists("json.json"))
	{
		printf("Configuration file does not exists. Aborting!\n");
		return -1;
	}
	
	char dst[10][256];

	FileStream* stream = (FileStream*)conf_root.open("json.json", SOM_READ);

 	JSONParser* parser = new JSONParser(stream);

	JSONError error;
	JSONToken* tokens;

	error = parser->parse();
	tokens = parser->get_tokens();

	for (int i = 0; i < parser->get_tokens_number(); i++)
	{
		printf("token[%d]\n", i);
		printf("type: %d\n", tokens[i].m_type);
		printf("size: %d\n", tokens[i].m_size);
		printf("start: %d\n",tokens[i].m_start);
		printf("end: %d\n",tokens[i].m_end);
		printf("parent token: %d\n",tokens[i].m_parent);
		printf("\n");
	}

	printf("return: %d\n", error);

	conf_root.close(stream);

	return 0;
}