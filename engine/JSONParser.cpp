#include "JSONParser.h"
#include "DiskFile.h"
#include "OS.h"
#include "StringUtils.h"
#include "Assert.h"
#include "Log.h"

namespace crown
{

//--------------------------------------------------------------------------
JSONParser::JSONParser(Allocator& allocator, const char* s) :
	m_buffer(s),
	m_nodes(allocator)
{
}

//--------------------------------------------------------------------------
JSONParser&	JSONParser::root()
{
	m_nodes.clear();

	List<JSONPair> tmp(default_allocator());

	JSON::parse_object(m_buffer, tmp);

	JSONNode node;

	for (int i = 0; i < tmp.size(); i++)
	{
		node.type = JSON::type(tmp[i].val);
		node.key = tmp[i].key;
		node.val = tmp[i].val;

		m_nodes.push_back(node);
	}

	return *this;
}

//--------------------------------------------------------------------------
JSONParser&	JSONParser::object(const char* key)
{
	bool found = false;

	for (int i = 0; i < m_nodes.size(); i++)
	{
		if (m_nodes[i].type == JT_OBJECT)
		{
			List<char> str(default_allocator());
			JSON::parse_string(m_nodes[i].key, str);

			if (string::strcmp(key, str.begin()) == 0)
			{
				List<JSONPair> obj(default_allocator());
				JSON::parse_object(m_nodes[i].val, obj);

				m_nodes.clear();

				JSONNode node;

				for (int j = 0; j < obj.size(); j++)
				{
					node.type = JSON::type(obj[j].val);
					node.key = obj[j].key;
					node.val = obj[j].val;

					m_nodes.push_back(node);
				}

				found = true;

				break;
			}
		}
	}

	CE_ASSERT(found, "Object called %s not found", key);

	return *this;
}

//--------------------------------------------------------------------------
JSONParser&	JSONParser::array(const char* key, uint32_t index)
{
	bool found = false;

	for (int i = 0; i < m_nodes.size(); i++)
	{
		if (m_nodes[i].type == JT_ARRAY)
		{
			List<char> str(default_allocator());
			JSON::parse_string(m_nodes[i].key, str);

			if (string::strcmp(key, str.begin()) == 0)
			{
				List<const char*> arr(default_allocator());
				JSON::parse_array(m_nodes[i].val, arr);

				m_nodes.clear();

				JSONNode node;

				node.type = JSON::type(arr[index]);
				node.key = NULL;
				node.val = arr[index];

				m_nodes.push_back(node);

				found = true;

				break;
			}
		}
	}

	CE_ASSERT(found, "Array called %s not found", key);

	return *this;
}

//--------------------------------------------------------------------------
const char* JSONParser::string(const char* key, List<char>& str)
{
	bool found = false;

	for (int i = 0; i < m_nodes.size(); i++)
	{
		if (JSON::type(m_nodes[i].val) == JT_STRING)
		{
			List<char> tmp(default_allocator());
			JSON::parse_string(m_nodes[i].key, tmp);

			if (string::strcmp(key, tmp.begin()) == 0)
			{
				JSON::parse_string(m_nodes[i].val, str);	

				found = true;

				break;
			}
		}
	}

	CE_ASSERT(found, "String not found");
}

//--------------------------------------------------------------------------
double JSONParser::number(const char* key)
{
	for (int i = 0; i < m_nodes.size(); i++)
	{
		if (JSON::type(m_nodes[i].val) == JT_NUMBER)
		{
			if (m_nodes[i].key == NULL)
			{
				return JSON::parse_number(m_nodes[i].val);	
			}

			List<char> tmp(default_allocator());
			JSON::parse_string(m_nodes[i].key, tmp);


			if (string::strcmp(key, tmp.begin()) == 0)
			{			
				return JSON::parse_number(m_nodes[i].val);	
			}
		}
	}

	CE_ASSERT(found, "Number not found");
}		

//--------------------------------------------------------------------------
bool JSONParser::boolean(const char* key)
{
	for (int i = 0; i < m_nodes.size(); i++)
	{
		if (JSON::type(m_nodes[i].val) == JT_BOOL)
		{
			if (m_nodes[i].key == NULL)
			{
				return JSON::parse_bool(m_nodes[i].val);	
			}

			List<char> tmp(default_allocator());
			JSON::parse_string(m_nodes[i].key, tmp);

			if (string::strcmp(key, tmp.begin()) == 0)
			{			
				return JSON::parse_bool(m_nodes[i].val);	
			}
		}
	}

	CE_ASSERT(found, "Boolean not found");
}

//--------------------------------------------------------------------------
void JSONParser::print_nodes()
{
	for (int i = 0; i < m_nodes.size(); i++)
	{
		Log::i("Index: %d", i);	
		Log::i("Type : %d", m_nodes[i].type);
		Log::i("Key  : %s", m_nodes[i].key);
		Log::i("Val  : %s", m_nodes[i].val);
	}
}






} //namespace crown