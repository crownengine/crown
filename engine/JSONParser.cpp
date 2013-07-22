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

				JSONNode node;

				for (int j = 0; j < obj.size(); j++)
				{
					node.type = JSON::type(obj[j].val);
					node.key = obj[j].key;
					node.val = obj[j].val;

					m_nodes.push_back(node);
				}
			}
		}
	}

	print_nodes();

	return *this;
}

//--------------------------------------------------------------------------
JSONParser&	JSONParser::array(const char* key, uint32_t index)
{
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

				JSONNode node;

				node.type = JSON::type(arr[index]);
				node.key = NULL;
				node.val = arr[index];

				m_nodes.push_back(node);
			}
		}
	}

	print_nodes();

	return *this;
}

//--------------------------------------------------------------------------
const char* JSONParser::string(List<char>& str)
{
	uint32_t last = m_nodes.size() - 1;

	JSON::parse_string(m_nodes[last].val, str);
}

//--------------------------------------------------------------------------
double JSONParser::number()
{
	uint32_t last = m_nodes.size() - 1;

	Log::i("Index in number: %d", last);
	Log::i("Val in number: %s", m_nodes[last].val);


	return JSON::parse_number(m_nodes[last].val);
}		

//--------------------------------------------------------------------------
bool JSONParser::boolean()
{
	uint32_t last = m_nodes.size() - 1;

	return JSON::parse_bool(m_nodes[last].val);
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