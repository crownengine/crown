#include "Crown.h"

using namespace crown;

int main()
{
	const char* json_string = 	"{"
    							"\"glossary\": { "
        						"	\"title\": \"example glossary\", "
								"	\"GlossDiv\": { "
            					"		\"title\": \"S\", "
								"		\"GlossList\": { "
                				"			\"GlossEntry\": { "
                    			"				\"ID\": \"SGML\", "
								"				\"SortAs\": \"SGML\", "
								"				\"GlossTerm\": \"Standard Generalized Markup Language\", "
								"				\"Acronym\": \"SGML\", "
								"				\"Abbrev\": \"ISO 8879:1986\", "
								"				\"GlossDef\": { "
                        		"					\"para\": \"A meta-markup language, used to create markup languages such as DocBook.\", "
								"					\"GlossSeeAlso\": [\"GML\", \"XML\"] "
                    			"				  }, "
								"				\"GlossSee\": \"markup\" "
                				"			 } "
            					"		  } "
        						"	   } "
    							"  } "
    							"}";

    JSONParser parser(json_string);

    JSONElement root = parser.root();

    CE_ASSERT(root.has_key("glossary"), "'glossary' not found!");

    Log::i("%s", root.key("glossary").key("GlossDiv").key("title").string_value());
    Log::i("%s", root.key("glossary").key("title").string_value());
    Log::i("%s", root.key("glossary").key("GlossDiv").key("GlossList").key("GlossEntry").key("GlossTerm").string_value());


	return 0;
}