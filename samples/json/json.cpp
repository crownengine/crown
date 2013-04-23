#include <stdio.h>
#include <string.h>
#include "JSONParser.h"
#include "Filesystem.h"
#include "FileStream.h"

using namespace crown;

int main(int argc, char** argv)
{
	// const char* src = "\"test\":{\"params1\": [1,2,3,4], \"params2\": [5,6,7,8]}";
	Filesystem conf_root("/home/mikymod/test");

	if (!conf_root.exists("json.json"))
	{
		printf("Configuration file does not exists. Abort!\n");
		return -1;
	}
	
	char dst[10][256];

	FileStream* stream = (FileStream*)conf_root.open("json.json", SOM_READ);

 	JSONParser* parser = new JSONParser(stream);

	json_error error;
	JSONToken* tokens;

	parser->init();
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

	// for (int i = 0; i < parser->get_tokens_number(); i++)
	// {
	// 	// strncpy((char*)dst[i], &src[tokens[i].m_start], tokens[i].m_size);
	// 	dst[i][tokens[i].m_size] = '\0';

	// 	printf("token[%d]\n", i);
	// 	printf("type: %d\n", tokens[i].m_type);
	// 	printf("size: %d\n", tokens[i].m_size);
	// 	printf("start: %d\n",tokens[i].m_start);
	// 	printf("end: %d\n",tokens[i].m_end);
	// 	printf("parent token: %d\n",tokens[i].m_parent);
	// 	printf("string: %s\n", dst[i]);
	// 	printf("\n");
	// }
	// parser->shutdown();

	printf("return: %d\n", error);

	conf_root.close(stream);

	return 0;
}