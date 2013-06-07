#include "JSONParser.h"
#include "DiskFile.h"
#include "OS.h"
#include "StringUtils.h"
#include "Assert.h"
#include <stdlib.h>

namespace crown
{

//--------------------------------------------------------------------------
JSONParser::JSONParser(Allocator& allocator, File* file, size_t size) :
	m_allocator(allocator),
	m_file(file),
	m_next_token(0),
	m_prev_token(-1),
	m_nodes_count(0)
{
	if (size > 1024)
	{
		m_tokens = CE_NEW(m_allocator, JSONToken[1024]);
	}
	else
	{
		m_tokens = m_tokens_list;
	}

	m_tokens_number = size;

	parse();

	m_nodes = CE_NEW(m_allocator, JSONNode[16]);
}

//--------------------------------------------------------------------------
JSONParser::~JSONParser()
{
	if (m_tokens_number > 1024 && m_tokens != NULL)
	{
		CE_DELETE(m_allocator, m_tokens);
	}
	if (m_nodes != NULL)
	{
		CE_DELETE(m_allocator, m_nodes);
	}
}

//--------------------------------------------------------------------------
void JSONParser::parse()
{
	JSONToken* token;

	char c;

	while(!m_file->end_of_file())
	{
		JSONType type;

		m_file->read(&c, 1);

		switch(c)
		{
			case '{':
			case '[':
			{
				token = allocate_token();

				CE_ASSERT(token != NULL, "Cannot allocate a new token for parsing.\n");

				if (m_prev_token != -1)
				{
					m_tokens[m_prev_token].m_size++;
					token->m_parent = m_prev_token;
				}

				token->m_type = c == '{' ? JSON_OBJECT : JSON_ARRAY;
				token->m_start = m_file->position() - 1;
				m_prev_token = m_next_token - 1;

				break;
			}
			case '}':
			case ']':
			{
				type = c == '}' ? JSON_OBJECT : JSON_ARRAY;

				CE_ASSERT(m_next_token > 0, "");

				token = &m_tokens[m_next_token - 1];

				while (true)
				{
					// If token does not have a parent
					if (token->m_parent == -1)
					{
						token->m_end = m_file->position();
						break;
					}
					// If token is started but not finished
					if (token->m_start != -1 && token->m_end == -1)
					{
						CE_ASSERT(token->m_type == type, "Token %d does not have type %d.\n", token->m_id, type);
						
						token->m_end = m_file->position();
						m_prev_token = token->m_parent;

						break;
					}

					token = &m_tokens[token->m_parent];
				}

				fill_token(token, type, token->m_start, token->m_end);
				break;
			}
			case '\"':
			{
				parse_string();
            	if (m_prev_token != -1)
            	{
            		m_tokens[m_prev_token].m_size++;
            	}
				break;
			}
			case ':':
			{
				break;
			}
            case '\t': 
            case '\r': 
            case '\n': 
            case ',': 
            case ' ': 
            {
            	break;
            }
            case '-':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            {
            	parse_number();
            	if (m_prev_token != -1)
            	{
            		m_tokens[m_prev_token].m_size++;
            	}
            	break;
            }
            case 't':	// true
            case 'f':	// false
            {
            	parse_bool();
            	if (m_prev_token != -1)
            	{
            		m_tokens[m_prev_token].m_size++;
            	}
            	break;
            }
            case 'n':	// null
            {
            	break;
            }
		}
	}

	for (int i = m_next_token - 1; i >= 0; i--)
	{
		if (m_tokens[i].m_start != -1 && m_tokens[i].m_end == -1)
		{
			CE_ASSERT(false, "There is an error in JSON syntax.");
		}
	}
}

//--------------------------------------------------------------------------
void JSONParser::parse_string()
{
	JSONToken* token;

	int start = m_file->position();

	char c; 

	while(!m_file->end_of_file())
	{	
		m_file->read(&c, 1);

		if (c == '\"' || c == '\'')
		{
			token = allocate_token();

			CE_ASSERT(token != NULL, "Cannot allocate a new token for parsing.\n");

			fill_token(token, JSON_STRING, start, m_file->position() - 1);
			token->m_parent = m_prev_token;

			return;
		}

		if (c == '\\')
		{
			m_file->read(&c, 1);

			switch(c)
			{
				case '\"': 
				case '/' : 
				case '\\': 
				case 'b':
                case 'f': 
                case 'r': 
                case 'n': 
                case 't':
                case 'u':
                {
                	break;
                }
                default:
               	{
                	CE_ASSERT(false, "Wrong character.\n");
                }
			}
		}
	}
}

//--------------------------------------------------------------------------
void JSONParser::parse_number()
{
	JSONToken* token;

	int start = m_file->position() - 1;

	char c;

	while (!m_file->end_of_file())
	{
		m_file->read(&c, 1);

		switch (c)
		{
			case '\t': 
			case '\r': 
			case '\n': 
			case ' ':
			case ',': 
			case '}':
			case ']':
			{
				token = allocate_token();

				CE_ASSERT(token != NULL, "Cannot allocate a new token for parsing.\n");
				
				fill_token(token, JSON_NUMBER, start, m_file->position() - 1);

				token->m_parent = m_prev_token;

				m_file->seek(m_file->position() - 1);

				return;
			}
		}

		CE_ASSERT(c >= 32 || c < 127, "Wrong character.\n");
	}
}

//--------------------------------------------------------------------------
void JSONParser::parse_bool()
{
	JSONToken* token;

	int start = m_file->position() - 1;

	char c;

	while (!m_file->end_of_file())
	{
		m_file->read(&c, 1);

		switch (c)
		{
			case '\t': 
			case '\r': 
			case '\n': 
			case ' ':
			case ',': 
			case '}':
			case ']':
			{
				token = allocate_token();

				CE_ASSERT(token != NULL, "Cannot allocate a new token.\n");
				
				fill_token(token, JSON_BOOL, start, m_file->position() - 1);

				token->m_parent = m_prev_token;

				m_file->seek(m_file->position() - 1);

				return;
			}
		}

		CE_ASSERT(c >= 32 || c < 127, "Wrong character.\n");
	}	
}

//--------------------------------------------------------------------------
JSONToken* JSONParser::allocate_token()
{
	JSONToken* token;

	if (m_next_token >= m_tokens_number)
	{
		return NULL;
	}	

	int32_t id = m_next_token;

	token = &m_tokens[id];
	token->m_id = id;
	token->m_start = -1;
	token->m_end = -1;
	token->m_size = 0;
	token->m_parent = -1;

	m_next_token++;

	return token;
}

//--------------------------------------------------------------------------
void JSONParser::fill_token(JSONToken* token, JSONType type, int32_t start, int32_t end)
{
	uint32_t cur_pos = m_file->position();

	token->m_type = type;
	token->m_start = start;
	token->m_end = end;
	token->m_size = token->m_end - token->m_start;

	char tmp[1024];
	m_file->seek(token->m_start);
	m_file->read(tmp, token->m_size);
	tmp[token->m_size] = '\0';
	string::strcpy(token->m_value, tmp);

	m_file->seek(cur_pos);
}

//--------------------------------------------------------------------------
void JSONParser::reset_nodes()
{
	CE_DELETE(m_allocator, m_nodes);

	m_nodes = CE_NEW(m_allocator, JSONNode[16]);

	// reset nodes counter 	
	m_nodes_count = 0;
}

//--------------------------------------------------------------------------
JSONParser& JSONParser::get_root()
{
	// Check if root node is an object and if it's the first
	CE_ASSERT(m_tokens[0].m_type == JSON_OBJECT && m_nodes_count == 0, "JSON root element '{'' must be first.\n");

	m_nodes[m_nodes_count].m_id = m_tokens[0].m_id;	
	m_nodes[m_nodes_count].m_type = JSON_OBJECT;

	m_nodes_count++;

	return *this;
}

//--------------------------------------------------------------------------
JSONParser&	JSONParser::get_object(const char* key)
{
	bool found = false;

	int32_t begin = m_nodes_count != 0 ? m_nodes[m_nodes_count-1].m_id : 0;
	// For each token
	for (int i = begin; i < m_next_token; i++)
	{
		// Check key and type
		if ((string::strcmp(m_tokens[i].m_value, key) == 0)	&& m_tokens[i].m_type == JSON_STRING)
		{
			// Check if the successive token is an array
			CE_ASSERT(m_tokens[i+1].m_type == JSON_OBJECT, "Token %d is not an Object.\n", m_tokens[i+1].m_id);

			found = true;

			// Store token's id in a json node
			m_nodes[m_nodes_count].m_id = m_tokens[i+1].m_id;	
			m_nodes[m_nodes_count].m_type = JSON_OBJECT;

			// If token stored has parent
			if (m_tokens[i+1].has_parent())
			{
				// Check if precedent token stored is the parent of current token
				CE_ASSERT(m_nodes_count && m_nodes[m_nodes_count-1].m_id == m_tokens[i+1].m_parent, "The precedent node is not parent of current.\n");
			}

			break;
		}
	}

	CE_ASSERT(found, "Node '%s' not found!\n", key);

	// Incremente nodes count for the next token
	m_nodes_count++;

	return *this;
}

//--------------------------------------------------------------------------
JSONParser& JSONParser::get_array(const char* key, uint32_t element)
{
	bool found = false;

	int32_t begin = m_nodes_count != 0 ? m_nodes[m_nodes_count-1].m_id : 0;

	element++;

	// For each token
	for (int i = begin; i < m_next_token; i++)
	{
		// Check key and type
		if ((string::strcmp(m_tokens[i].m_value, key) == 0)	&& m_tokens[i].m_type == JSON_STRING)
		{
			// Check if the successive token is an array
			CE_ASSERT(m_tokens[i + 1].m_type == JSON_ARRAY, "Token %d is not an Array.\n", m_tokens[i+1].m_id);

			found = true;

			// Store array-token's id in a json node
			m_nodes[m_nodes_count].m_id = m_tokens[i + 1].m_id;	
			m_nodes[m_nodes_count].m_type = JSON_ARRAY;

			// If token stored has parent
			if (m_tokens[i + 1].has_parent())
			{
				// Check if precedent token stored is the parent of current token
				CE_ASSERT(m_nodes_count && m_nodes[m_nodes_count-1].m_id == m_tokens[i + 1].m_parent,
					"The precedent node is not parent of current.\n");
			}

			m_nodes_count++;

			// Store element-token's id in a json node
			m_nodes[m_nodes_count].m_id = m_tokens[i + 1 + element].m_id;	
			m_nodes[m_nodes_count].m_type = JSON_ARRAY;

			if (m_tokens[i + 1 + element].has_parent())
			{
				// Check if precedent token stored is the parent of current token
				CE_ASSERT(m_nodes[m_nodes_count-1].m_id == m_tokens[i + 1 + element].m_parent, 
					"The precedent node is not parent of current.\n");				
			}

			break;
		}
	}

	CE_ASSERT(found, "Node '%s' not found!\n", key);

	m_nodes_count++;

	return *this;
}

//--------------------------------------------------------------------------
JSONParser&	JSONParser::get_string(const char* key)
{
	bool found = false;

	int32_t begin = m_nodes_count != 0 ? m_nodes[m_nodes_count-1].m_id : 0;

	for (int i = begin; i < m_next_token; i++)
	{
		if ((string::strcmp(m_tokens[i].m_value, key) == 0)	&& m_tokens[i].m_type == JSON_STRING)
		{
			CE_ASSERT(m_tokens[i+1].m_type == JSON_STRING, "Token %d is not a String.\n", m_tokens[i+1].m_id);

			found = true;

			m_nodes[m_nodes_count].m_id = m_tokens[i+1].m_id;	
			m_nodes[m_nodes_count].m_type = JSON_STRING;

			if (m_tokens[i+1].has_parent())
			{
				CE_ASSERT(m_nodes_count && m_nodes[m_nodes_count-1].m_id == m_tokens[i+1].m_parent,
					"The precedent node is not parent of current.\n");
			}

			break;
		}
	}

	CE_ASSERT(found, "Node '%s' not found!\n", key);

	m_nodes_count++;

	return *this;
}

//--------------------------------------------------------------------------
JSONParser& JSONParser::get_number(const char* key)
{
	bool found = false;

	int32_t begin = m_nodes_count != 0 ? m_nodes[m_nodes_count-1].m_id : 0;

	for (int i = begin; i < m_next_token; i++)
	{
		if ((string::strcmp(m_tokens[i].m_value, key) == 0)	&& m_tokens[i].m_type == JSON_STRING)
		{
			CE_ASSERT(m_tokens[i+1].m_type == JSON_NUMBER, "Token %d is not a Number.\n", m_tokens[i+1].m_id);

			found = true;

			m_nodes[m_nodes_count].m_id = m_tokens[i+1].m_id;	
			m_nodes[m_nodes_count].m_type = JSON_NUMBER;

			if (m_tokens[i+1].has_parent())
			{
				CE_ASSERT(m_nodes_count && m_nodes[m_nodes_count-1].m_id == m_tokens[i+1].m_parent,
					"The precedent node is not parent of current.\n");
			}

			break;
		}
	}

	CE_ASSERT(found, "Node '%s' not found!\n", key);

	m_nodes_count++;

	return *this;
}

//--------------------------------------------------------------------------
JSONParser& JSONParser::get_bool(const char* key)
{
	bool found = false;

	int32_t begin = m_nodes_count != 0 ? m_nodes[m_nodes_count-1].m_id : 0;

	for (int i = begin; i < m_next_token; i++)
	{
		if ((string::strcmp(m_tokens[i].m_value, key) == 0)	&& m_tokens[i].m_type == JSON_STRING)
		{
			CE_ASSERT(m_tokens[i+1].m_type == JSON_BOOL, "Token %d is not a Boolean.\n", m_tokens[i+1].m_id);

			found = true;

			m_nodes[m_nodes_count].m_id = m_tokens[i+1].m_id;	
			m_nodes[m_nodes_count].m_type = JSON_BOOL;

			if (m_tokens[i+1].has_parent())
			{
				CE_ASSERT(m_nodes_count && m_nodes[m_nodes_count-1].m_id == m_tokens[i+1].m_parent,
					"The precedent node is not parent of current.\n");
			}

			break;
		}
	}

	CE_ASSERT(found, "Node '%s' not found!\n", key);

	m_nodes_count++;

	return *this;
}

//--------------------------------------------------------------------------
void JSONParser::to_string(char* value)
{
	string::strcpy(value, m_tokens[m_nodes[m_nodes_count-1].m_id].m_value);

	reset_nodes();
}

//--------------------------------------------------------------------------
void JSONParser::to_float(float& value)
{
	value = atof(m_tokens[m_nodes[m_nodes_count-1].m_id].m_value);

	reset_nodes();
}

//--------------------------------------------------------------------------
void JSONParser::to_int(int& value)
{
	value = atoi(m_tokens[m_nodes[m_nodes_count-1].m_id].m_value);

	reset_nodes();
}

//--------------------------------------------------------------------------
void JSONParser::to_bool(bool& value)
{
	if (string::strcmp(m_tokens[m_nodes[m_nodes_count-1].m_id].m_value, "true") == 0)
	{
		value = true;
	}
	else
	{
		value = false;
	}

	reset_nodes();
}

} //namespace crown