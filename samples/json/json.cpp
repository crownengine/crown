#include <stdio.h>
#include <string.h>
#include "JSONParser.h"

using namespace crown;

int main(int argc, char** argv)
{
	const char* src = "\"params1\": [1,2,3,4], \"params2\": [5,6,7,8]";

	char dst[10][256];

	JSONParser* parser = new JSONParser();
	json_error error;
	JSONToken* tokens;

	parser->init();
	error = parser->parse(src);
	tokens = parser->get_tokens();

	for (int i = 0; i < parser->get_tokens_number(); i++)
	{
		strncpy((char*)dst[i], &src[tokens[i].m_start], tokens[i].m_size);
		dst[i][tokens[i].m_size] = '\0';

		printf("token[%d]\n", i);
		printf("type: %d\n", tokens[i].m_type);
		printf("size: %d\n", tokens[i].m_size);
		printf("start: %d\n",tokens[i].m_start);
		printf("end: %d\n",tokens[i].m_end);
		printf("parent: %d\n",tokens[i].m_parent);
		printf("string: %s\n", dst[i]);
		printf("\n");
	}

	printf("return: %d\n", error);

	return 0;
}