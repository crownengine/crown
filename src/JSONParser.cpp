#include "JSONParser.h"
#include "DiskFile.h"
#include "OS.h"
#include "String.h"
#include <cassert>

namespace crown
{

//--------------------------------------------------------------------------
JSONParser::JSONParser(File* file, size_t size) :
	m_file(file),
	m_next_token(0),
	m_prev_token(-1),
	m_nodes_count(0)
{
	if (size > 1024)
	{
		m_tokens = new JSONToken[size];
	}
	else
	{
		m_tokens = m_tokens_list;
	}

	m_size = size;

	m_pos = m_file->position();

	parse();

	// Test
	for (int i = 0; i < m_next_token; i++)
	{
		m_tokens[i].print();
	}
}

//--------------------------------------------------------------------------
JSONParser::~JSONParser()
{
	if (m_size > 1024 && m_tokens != NULL)
	{
		delete m_tokens;
	}
}

//--------------------------------------------------------------------------
JSONError JSONParser::parse()
{
	JSONError error;
	JSONToken* token;

	char c;

	while(!m_file->end_of_file())
	{
		JSONType type;

		m_file->read(&c, 1);
		m_pos = m_file->position();

		switch(c)
		{
			case '{':
			case '[':
			{
				token = allocate_token();

				assert(token != NULL);

				if (m_prev_token != -1)
				{
					m_tokens[m_prev_token].m_size++;
					token->m_parent = m_prev_token;
				}

				token->m_type = c == '{' ? JSON_OBJECT : JSON_ARRAY;
				token->m_start = m_pos;
				m_prev_token = m_next_token - 1;

				break;
			}
			case '}':
			case ']':
			{
				type = c == '}' ? JSON_OBJECT : JSON_ARRAY;

				assert(m_next_token > 0);

				token = &m_tokens[m_next_token - 1];

				while (true)
				{
					if (token->m_start != -1 && token->m_end == -1)
					{
						// FIXME
						os::printf("Token:%s\n", token->m_value);
						os::printf("previous:%d\ncurrent:%d\n", token->m_type, type);
						assert(token->m_type == type);
						
						token->m_end = m_pos + 1;
						m_prev_token = token->m_parent;

						break;
					}

					if (token->m_parent == -1)
					{
						break;
					}

					token = &m_tokens[token->m_parent];
				}

				// token->m_size = token->m_end - token->m_start;
				fill_token(token, type, token->m_start, token->m_end);
				break;
			}
			case '\"':
			case '\'':
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
			assert(false); // FIXME
		}
	}

	return JSON_SUCCESS;
}

//--------------------------------------------------------------------------
void JSONParser::parse_string()
{
	JSONToken* token;

	int start = m_pos;

	char c; 

	while(!m_file->end_of_file())
	{	
		m_file->read(&c, 1);
		m_pos = m_file->position();

		if (c == '\"' || c == '\'')
		{
			token = allocate_token();

			assert(token != NULL);

			fill_token(token, JSON_STRING, start + 1, m_pos);
			token->m_parent = m_prev_token;

			return;
		}

		if (c == '\\')
		{
			m_file->read(&c, 1);
			m_pos = m_file->position();

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
                	assert(false);
                }
			}
		}
	}
	m_pos = start;
}

//--------------------------------------------------------------------------
void JSONParser::parse_number()
{
	JSONToken* token;

	int start = m_file->position();

	char c;

	while (!m_file->end_of_file())
	{
		m_file->read(&c, 1);
		m_pos = m_file->position();

		switch (c)
		{
			case ' ':
			case ',': 
			case '}':
			case ']':
			{
				token = allocate_token();

				assert(token != NULL);
				
				fill_token(token, JSON_NUMBER, start, m_pos);

				token->m_parent = m_prev_token;

				//m_file->seek(start);

				return;
			}
		}

		assert(c >= 32 || c < 127);
	}
}

//--------------------------------------------------------------------------
void JSONParser::parse_bool()
{
	JSONToken* token;

	int start = m_file->position();

	char c;

	while (!m_file->end_of_file())
	{
		m_file->read(&c, 1);
		m_pos = m_file->position();

		switch (c)
		{
			case ' ':
			case ',': 
			case '}':
			case ']':
			{
				token = allocate_token();

				assert(token != NULL);
				
				fill_token(token, JSON_BOOL, start, m_pos);

				token->m_parent = m_prev_token;

				m_file->seek(start);

				return;
			}
		}

		assert(c >= 32 || c < 127);
	}	
}

//--------------------------------------------------------------------------
JSONToken* JSONParser::allocate_token()
{
	JSONToken* token;

	if (m_next_token >= m_size)
	{
		return NULL;
	}	

	int32_t id = m_next_token++;

	token = &m_tokens[id];
	token->m_id = id;
	token->m_start = token->m_end = -1;
	token->m_size = 0;
	token->m_parent = -1;

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

	char tmp[token->m_size+1];
	m_file->seek(token->m_start-1);
	m_file->read(tmp, token->m_size);
	tmp[token->m_size] = '\0';
	string::strcpy(token->m_value, tmp);

	m_file->seek(cur_pos);
}

//--------------------------------------------------------------------------
JSONParser&	JSONParser::get_object(const char* key)
{
	int32_t begin = m_nodes_count != 0 ? m_nodes[m_nodes_count-1].m_id : 0;
	// For each token
	for (int i = begin; i < m_next_token; i++)
	{
		// Check key and type
		if ((string::strcmp(m_tokens[i].m_value, key) == 0)	&& m_tokens[i].m_type == JSON_STRING)
		{
			// Check if the successive token is an array
			assert(m_tokens[i+1].m_type == JSON_OBJECT);

			// Store token's id in a json node
			m_nodes[m_nodes_count].m_id = m_tokens[i+1].m_id;	
			m_nodes[m_nodes_count].m_type = JSON_OBJECT;
			m_nodes[m_nodes_count].print();

			// If token stored has parent
			if (m_tokens[i+1].has_parent())
			{
				// Check if precedent token stored is the parent of current token
				assert(m_nodes_count && m_nodes[m_nodes_count-1].m_id == m_tokens[i+1].m_parent);
			}

			break;
		}
	}
	// Incremente nodes count for the next token
	m_nodes_count++;

	return *this;
}

//--------------------------------------------------------------------------
JSONParser& JSONParser::get_array(const char* key)
{
	int32_t begin = m_nodes_count != 0 ? m_nodes[m_nodes_count-1].m_id : 0;

	// For each token
	for (int i = begin; i < m_next_token; i++)
	{
		// Check key and type
		if ((string::strcmp(m_tokens[i].m_value, key) == 0)	&& m_tokens[i].m_type == JSON_STRING)
		{
			// Check if the successive token is an array
			assert(m_tokens[i+1].m_type == JSON_ARRAY);

			// Store token's id in a json node
			m_nodes[m_nodes_count].m_id = m_tokens[i+1].m_id;	
			m_nodes[m_nodes_count].m_type = JSON_ARRAY;
			m_nodes[m_nodes_count].print();

			// If token stored has parent
			if (m_tokens[i+1].has_parent())
			{
				// Check if precedent token stored is the parent of current token
				assert(m_nodes_count && m_nodes[m_nodes_count-1].m_id == m_tokens[i+1].m_parent);
			}

			break;
		}
	}
	// Increment nodes count for the next token
	m_nodes_count++;

	return *this;
}

//--------------------------------------------------------------------------
JSONParser&	JSONParser::get_string(const char* key)
{
	int32_t begin = m_nodes_count != 0 ? m_nodes[m_nodes_count-1].m_id : 0;

	// For each token
	for (int i = begin; i < m_next_token; i++)
	{
		// Check key and type
		if ((string::strcmp(m_tokens[i].m_value, key) == 0)	&& m_tokens[i].m_type == JSON_STRING)
		{
			// Check if the successive token is an array
			assert(m_tokens[i+1].m_type == JSON_STRING);

			// Store token's id in a json node
			m_nodes[m_nodes_count].m_id = m_tokens[i+1].m_id;	
			m_nodes[m_nodes_count].m_type = JSON_STRING;
			m_nodes[m_nodes_count].print();

			// If token stored has parent
			if (m_tokens[i+1].has_parent())
			{
				// Check if precedent token stored is the parent of current token
				assert(m_nodes_count && m_nodes[m_nodes_count-1].m_id == m_tokens[i+1].m_parent);
			}

			break;
		}
	}
	// Increment nodes count for the next token
	m_nodes_count++;

	return *this;
}

//--------------------------------------------------------------------------
JSONParser& JSONParser::get_number(const char* key)
{
	int32_t begin = m_nodes_count != 0 ? m_nodes[m_nodes_count-1].m_id : 0;

	for (int i = begin; i < m_next_token; i++)
	{
		if ((string::strcmp(m_tokens[i].m_value, key) == 0)	&& m_tokens[i].m_type == JSON_STRING)
		{
			assert(m_tokens[i+1].m_type == JSON_NUMBER);

			m_nodes[m_nodes_count].m_id = m_tokens[i+1].m_id;	
			m_nodes[m_nodes_count].m_type = JSON_NUMBER;
			m_nodes[m_nodes_count].print();

			if (m_tokens[i+1].has_parent())
			{
				assert(m_nodes_count && m_nodes[m_nodes_count-1].m_id == m_tokens[i+1].m_parent);
			}

			break;
		}
	}
	m_nodes_count++;

	return *this;
}

//--------------------------------------------------------------------------
JSONParser& JSONParser::get_bool(const char* key)
{
	int32_t begin = m_nodes_count != 0 ? m_nodes[m_nodes_count-1].m_id : 0;

	for (int i = begin; i < m_next_token; i++)
	{
		if ((string::strcmp(m_tokens[i].m_value, key) == 0)	&& m_tokens[i].m_type == JSON_STRING)
		{
			assert(m_tokens[i+1].m_type == JSON_BOOL);

			m_nodes[m_nodes_count].m_id = m_tokens[i+1].m_id;	
			m_nodes[m_nodes_count].m_type = JSON_BOOL;
			m_nodes[m_nodes_count].print();

			if (m_tokens[i+1].has_parent())
			{
				assert(m_nodes_count && m_nodes[m_nodes_count-1].m_id == m_tokens[i+1].m_parent);
			}

			break;
		}
	}
	m_nodes_count++;

	return *this;
}

// const char* JSONParser::to_string()
// {
// 	return m_tokens[m_nodes[m_nodes_count-1].m_id].m_value;
// }


} //namespace crown