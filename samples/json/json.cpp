#include <stdio.h>
#include "JSONParser.h"

using namespace crown;

int main(int argc, char** argv)
{
	const char* src = "'test' : {string' : 'dio', 'number' : 33}";

	JSONParser* parser = new JSONParser();
	json_error error;
	JSONToken* tokens;

	parser->init();
	error = parser->parse(src);
	tokens = parser->get_tokens();

	for (int i = 0; i < parser->get_tokens_number(); i++)
	{
		tokens[i].print();
	}

	printf("return: %d\n", error);

	return 0;
}