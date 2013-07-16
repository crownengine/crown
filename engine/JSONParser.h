#pragma once

#include <cstdarg>
#include "Types.h"
#include "OS.h"
#include "File.h"
#include "List.h"
#include "Dictionary.h"
#include "Allocator.h"

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

///
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
					JSONParser(Allocator& allocator, File* file, size_t size = 1024);
	/// Destructor
					~JSONParser();
	/// Get root element
	JSONParser&		get_root();
	/// Get object @a key
	JSONParser&		get_object(const char* key);
	/// Get array @a key and element @a index  
	JSONParser&		get_array(const char* key, uint32_t index);
	/// Get string @a key
	JSONParser& 	get_string(const char* key);
	/// Get number @a key
	JSONParser&		get_number(const char* key);		
	/// Get boolean @a key
	JSONParser&		get_bool(const char* key);
	/// Convert element taken from @a get_string to string
	void			to_string(char* value);
	/// Convert element taken from @a get_number to float
	void			to_float(float& value);
	/// Convert element taken from @a get_number to int
	void			to_int(int& value);
	/// Convert element taken from @a get_bool to boolean
	void			to_bool(bool& value);

private:
	/// Parse JSON data and fill tokens
	void	 		parse();
	/// Parse string in JSON data
	void			parse_string();
	/// Parse boolean in JSON data
	void			parse_bool();
	/// Parse float in JSON data
	void			parse_number();
	/// Allocate token node
	JSONToken* 		allocate_token();
	/// Fill @a token with @a type and boundaries (@a start and @a stop)
	void			fill_token(JSONToken* token, JSONType type, int32_t start, int32_t end);
	/// Reset all JSON nodes
	void			reset_nodes();

	/// JSONParser allocator
	Allocator& 		m_allocator;
	/// JSON data
	File*			m_file;

	/// Next token to allocate				
	int32_t			m_next_token;
	/// Previous token e.g parent or array		
	int32_t			m_prev_token;
	/// JSON tokens list, used as default
	JSONToken		m_tokens_list[1024];
	/// JSON tokens ptr (used only if we need more then 1024 tokens)
	JSONToken* 		m_tokens;
	/// m_tokens default size, default 1024
	size_t			m_tokens_number;

	/// DOM-like abstraction
	JSONNode*		m_nodes;
	/// Number of nodes in DOM-like abtraction
	uint32_t 		m_nodes_count;

};

} // namespace crown