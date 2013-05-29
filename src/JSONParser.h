#pragma once

#include "Types.h"
#include "OS.h"
#include "File.h"

namespace crown
{

/// JSON Token types
enum JSONType
{
	JSON_PRIMITIVE 	= 0,	// Number, boolean or null
	JSON_OBJECT 	= 1,	// Object
	JSON_ARRAY 		= 2,	// Array
	JSON_STRING 	= 3		// String
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
	char 		m_value[MAX_TOKEN_LEN];	// Token's value
	int32_t 	m_start;				// Starting byte
	int32_t 	m_end;					// Ending byte
	size_t 		m_size;					// Token's dimension
	int32_t 	m_parent;				// Token's parent


	inline void print()
	{
		os::printf("Value:\t%s\n", m_value);
		os::printf("Type:\t%d\n", m_type);
		os::printf("Start:\t%d\n", m_start);
		os::printf("End:\t%d\n", m_end);
		os::printf("Parent:\t%d\n", m_parent);
		os::printf("Size:\t%d\n", m_size);
		os::printf("\n");		
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
	/// Parse JSON data and fill tokens
	JSONError 		parse();
	/// Get all tokens
	JSONToken*		get_tokens();
	/// Get next token
	int32_t			get_tokens_number();

	bool 			get_bool(const char* name);

	int32_t 		get_int(const char* name);

	float			get_float(const char* name);

	// void			get_string(const char* name, List<char>& string);

	// void			get_array(const char* name, List<const char*>& array);

	// void			get_object(const char* name, Dictionary<String, const char*>& obj);

private:

	/// Parse string in JSON data
	JSONError		parse_string();
	/// Parse number or boolean in JSON data
	JSONError		parse_primitive();
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
	/// true if the current token is a key, false otherwise
	bool 			m_key_set;

	/// JSON tokens list, used as default
	JSONToken		m_tokens_list[1024];
	/// JSON tokens ptr (used only if we need more then 1024 tokens)
	JSONToken* 		m_tokens;
	/// m_tokens default size, default 1024
	size_t			m_size;


};

} // namespace crown