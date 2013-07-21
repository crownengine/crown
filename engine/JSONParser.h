#pragma once

#include <cstdarg>
#include "Types.h"
#include "List.h"
#include "HeapAllocator.h"
#include "JSON.h"

namespace crown
{

class JSONParser
{
public:
	/// Constructor
						JSONParser(Allocator& allocator, const char* s);
	/// Get root element
	JSONParser&			root();
	/// Get object @a key
	JSONParser&			object(const char* key);
	/// Get array @a key and element @a index  
	JSONParser&			array(const char* key, uint32_t index);
	/// Get string @a key
	JSONParser& 		string(const char* key);
	/// Get number @a key
	JSONParser&			number(const char* key);		
	/// Get boolean @a key
	JSONParser&			boolean(const char* key);

private:
	/// JSONParser allocator
	HeapAllocator& 		m_allocator;

	// List<JSONNode>		m_nodes;
};

} // namespace crown