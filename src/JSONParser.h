#pragma once

#include <cstdarg>
#include "Types.h"
#include "OS.h"
#include "File.h"
#include "List.h"
#include "Dictionary.h"

namespace crown
{

/// JSON Token types
enum JSONType
{
	JSON_OBJECT 	= 0,	// Object
	JSON_ARRAY 		= 1,	// Array
	JSON_STRING 	= 2,	// String
	JSON_NUMBER		= 3,	// Number
	JSON_BOOL 		= 4		// Boolean

};

/// JSON error typology
enum JSONError
{
	JSON_NO_MEMORY	= 0,	// Not enough token provided
	JSON_INV_CHAR	= 1,	// Invalid character inside JSON string
	JSON_INV_PART	= 2,	// JSON string is incompleted
	JSON_SUCCESS	= 3		// Everything OK!
};

/// JSONToken is a container which have pointer to a single json entity 
/// (primitive, object, array or string) of a json file.
struct JSONToken
{
	static const uint32_t MAX_TOKEN_LEN = 1024;

	JSONType	m_type;					// Token's type
	int32_t		m_id;					// Token's id
	char 		m_value[MAX_TOKEN_LEN];	// Token's value
	int32_t 	m_start;				// Starting byte
	int32_t 	m_end;					// Ending byte
	size_t 		m_size;					// Token's dimension
	int32_t 	m_parent;				// Token's parent

	inline void print()
	{
		os::printf("Id:\t%d\n", m_id);
		os::printf("Value:\t%s\n", m_value);
		os::printf("Type:\t%d\n", m_type);
		os::printf("Start:\t%d\n", m_start);
		os::printf("End:\t%d\n", m_end);
		os::printf("Parent:\t%d\n", m_parent);
		os::printf("Size:\t%d\n", m_size);
		os::printf("\n");		
	}

	inline bool has_parent()
	{
		return m_parent != -1;
	}
};

struct JSONNode
{
	int32_t 	m_id;
	JSONType 	m_type;

	inline void print()
	{
		os::printf("----------------\n");
		os::printf("Id:\t%d\n", m_id);
		os::printf("----------------\n");
	}
};

/// JSONParser parses JSON file and stores all relative tokens.
/// It is designed to be robust (it should work with erroneus data)
/// and fast (data parsing on fly).
class JSONParser
{
public:
	/// Constructor
					JSONParser(File* file, size_t size = 1024);
	/// Destructor
					~JSONParser();

	JSONParser&		get_object(const char* key);

	JSONParser&		get_array(const char* key);

	JSONParser& 	get_string(const char* key);

	JSONParser&		get_number(const char* key);			// MUST BE IMPLEMENTED

	JSONParser&		get_bool(const char* key);				// MUST BE IMPLEMENTED

	void			to_string(List<const char*>& values);	// MUST BE IMPLEMENTED

	void			to_float(float* values);				// MUST BE IMPLEMENTED

	void			to_int(int* values);					// MUST BE IMPLEMENTED

	void			to_bool(bool* values);					// MUST BE IMPLEMENTED

private:
	/// Parse JSON data and fill tokens
	JSONError 		parse();
	/// Parse string in JSON data
	void			parse_string();
	/// Parse boolean in JSON data
	void			parse_bool();			// MUST BE IMPLEMENTED
	/// Parse float in JSON data
	void			parse_number();			// MUST BE IMPLEMENTED
	/// Allocate token node
	JSONToken* 		allocate_token();
	/// Fill token and set boundaries
	void			fill_token(JSONToken* token, JSONType type, int32_t start, int32_t end);

	/// JSON file of data
	File*			m_file;
	/// JSON string offset
	uint32_t 		m_pos;
	/// Next token to allocate				
	int32_t			m_next_token;
	/// Previous token e.g parent or array		
	int32_t			m_prev_token;
	/// JSON tokens list, used as default
	JSONToken		m_tokens_list[1024];
	/// JSON tokens ptr (used only if we need more then 1024 tokens)
	JSONToken* 		m_tokens;
	/// m_tokens default size, default 1024
	size_t			m_size;

	/// 
	JSONNode		m_nodes[128];
	///
	uint32_t 		m_nodes_count;

};

} // namespace crown