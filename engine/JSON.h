#pragma once

#include "Types.h"

namespace crown
{

enum JSONValue
{
	JSON_NULL,
	JSON_BOOL,
	JSON_NUMBER
};

class JSON
{
public:
						JSON();

	void 				skip_whitespace();

	static void			next(const char* str, const char c = 0, bool force_reset = false);

	static bool			parse_bool(const char* token);
	static int32_t 		parse_int(const char* token);
	static float		parse_float(const char* token);
	// static void			parse_string(const char* token, List<char>& str);
	
	// static void			parse_array(const char* token, List<const char*>& array);
	// static void			parse_object(const char* token, Dictionary<const char* key, const char* val>& dict);

private:

	char			m_buffer[1024];

	uint32_t		m_index;

	char			m_current;
};

} // namespace crown