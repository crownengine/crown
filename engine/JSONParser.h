#pragma once

#include <cstdarg>
#include "Types.h"
#include "List.h"
#include "HeapAllocator.h"
#include "JSON.h"

namespace crown
{

struct JSONNode
{
	JSONType	type;
	const char* key;
	const char* val;
};

class JSONParser
{
public:
	/// Constructor
							JSONParser(Allocator& allocator, const char* s);
	/// Get root element
	JSONParser&				root();
	/// Get object @a key
	JSONParser&				object(const char* key);
	/// Get array @a key and element @a index  
	JSONParser&				array(const char* key, uint32_t index);
	/// Get string
	const char* 			string(List<char>& str);
	/// Get number
	double					number();		
	/// Get boolean
	bool 					boolean();

	// Could be usefull
	//void					fill_node();

	void					print_nodes();

private:

	const char*				m_buffer;

	List<JSONNode>			m_nodes;
};

} // namespace crown