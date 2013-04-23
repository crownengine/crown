#include "JSONParser.h"

namespace crown
{

//--------------------------------------------------------------------------
JSONParser::JSONParser(size_t size)
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
}

//--------------------------------------------------------------------------
void 
JSONParser::init()
{
	m_pos = 0;
	m_next_token = 0;
	m_prev_token = -1;

	is_init = true;
}

//--------------------------------------------------------------------------
json_error 
JSONParser::parse(const char* src)
{
	json_error error;
	JSONToken* token;

	if (!is_init)
	{
		return JSON_NO_INIT; 
	}

	while(src[m_pos] != '\0')
	{
		json_type type;

		char c;

		c = src[m_pos];

		switch(c)
		{
			case '{':
			case '[':
			{
				token = allocate_token();

				if (token == NULL)
				{
					return JSON_NO_MEMORY;
				}
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

				if (m_next_token < 1)
				{
					return JSON_INV_CHAR;
				}

				token = &m_tokens[m_next_token - 1];

				while (true)
				{
					if (token->m_start != -1 && token->m_end == -1)
					{
						if (token->m_type != type)
						{
							return JSON_INV_CHAR;
						}
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

				token->m_size = token->m_end - token->m_start;
				break;
			}
			case '\"':
			{
				error = parse_string(src);
            	if (m_prev_token != -1)
            	{
            		m_tokens[m_prev_token].m_size++;
            	}
				break;
			}
            case '\t': 
            case '\r': 
            case '\n': 
            case ':': 
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
            case 't':
            case 'f':
            case 'n':
            {
            	error = parse_primitive(src);
            	if (m_prev_token != -1)
            	{
            		m_tokens[m_prev_token].m_size++;
            	}
            	break;
            }
		}
		m_pos++;
	}

	for (int i = m_next_token - 1; i >= 0; i--)
	{
		if (m_tokens[i].m_start != -1 && m_tokens[i].m_end == -1)
		{
			return JSON_INV_PART;
		}
	}

	return JSON_SUCCESS;
}

//--------------------------------------------------------------------------
json_error
JSONParser::parse_string(const char* src)
{
	JSONToken* token;

	int start = m_pos;

	m_pos++;

	while(src[m_pos] != '\0')
	{	
		char c = src[m_pos];

		if (c == '\"' || c == '\'')
		{
			token = allocate_token();

			if (token == NULL)
			{
				m_pos = start;
				return JSON_NO_MEMORY;
			}

			fill_token(token, JSON_STRING, start + 1, m_pos);
			token->m_parent = m_prev_token;

			return JSON_SUCCESS;
		}

		if (c == '\\')
		{
			m_pos++;

			switch(src[m_pos])
			{
				case '\"': 
				case '/' : 
				case '\\': 
				case 'b':
                case 'f': 
                case 'r': 
                case 'n': 
                case 't':
                {
                	break;
                }
                case 'u' :
                {
                	// TODO
                	break;
                }
                default:
               	{
                	m_pos = start;
                	return JSON_INV_CHAR;
                	break;
                }
			}
		}
		m_pos++;
	}
	m_pos = start;
	return JSON_INV_PART;
}

//--------------------------------------------------------------------------
json_error
JSONParser::parse_primitive(const char* src)
{
	JSONToken* token;

	int start = m_pos;

	while (src[m_pos] != '\0')
	{
		char c = src[m_pos];

		switch (c)
		{

			case ' ':
			case ',': 
			case '}':
			case ']':
			{
				token = allocate_token();
				if (token == NULL)
				{
					m_pos = start;
					return JSON_NO_MEMORY;
				}

				fill_token(token, JSON_PRIMITIVE, start, m_pos);

				token->m_parent = m_prev_token;
				m_pos--;

				return JSON_SUCCESS;
			}
		}

		if (c < 32 || c >= 127)
		{
			m_pos = start;
			return JSON_INV_CHAR;
		}

		m_pos++;
	}
}

//--------------------------------------------------------------------------
JSONToken* 
JSONParser::allocate_token()
{
	JSONToken* token;

	if (m_next_token >= m_size)
	{
		return NULL;
	}	

	token = &m_tokens[m_next_token++];
	token->m_start = token->m_end = -1;
	token->m_size = 0;
	token->m_parent = -1;

	return token;
}

//--------------------------------------------------------------------------
void JSONParser::fill_token(JSONToken* token, json_type type, int32_t start, int32_t end)
{
	token->m_type = type;
	token->m_start = start;
	token->m_end = end;
	token->m_size = token->m_end - token->m_start;
}

//--------------------------------------------------------------------------
JSONToken* JSONParser::get_tokens()
{
	return m_tokens;
}

//--------------------------------------------------------------------------
int32_t JSONParser::get_tokens_number()
{
	return m_next_token;
}

} //namespace crown