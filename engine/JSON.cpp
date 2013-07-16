#include "Assert.h"
#include "JSON.h"
#include "Types.h"

namespace crown
{

/// This function is EPIC. Love it. Now.
//-----------------------------------------------------------------------------
CE_EXPORT void JSON::next(const char* str, const char c, bool force_reset)
{
	static char* current_str = (char*)str;
	static size_t index = 0;
	static char current = str[index];

	if (current_str != str || force_reset == true)
	{
		current_str = (char*)str;
		index = 0;
		current = str[index];
	}

	if (c && c != current)
	{
		CE_ASSERT(false, "Expected '%c' got '%c'", c, current);
	}

	index++;
	current = str[index];
}

//-----------------------------------------------------------------------------
CE_EXPORT bool JSON::parse_bool(const char* token)
{
	CE_ASSERT_NOT_NULL(token);

	switch(token[0])
	{
		case 't':
		{
			next(token, 't');
			next(token, 'r');
			next(token, 'u');
			next(token, 'e');
			return true;
		}
		case 'f':
		{
			next(token, 'f');
			next(token, 'a');
			next(token, 'l');
			next(token, 's');			
			next(token, 'e');
			return false;
		}
		default:
		{
			CE_ASSERT(false, "Current token is not a boolean");
		}
	}
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t	JSON::parse_int(const char* token)
{
	// uint32_t index = 0;

	// uint32_t start = index;

	// uint32_t end = -1;

	// while(token[index] != '\0' || end == -1)
	// {
	// 	switch(token[index])
	// 	{
	//         case '-':
	//         {
	        	
	//         }
 //            case '0':
 //            case '1':
 //            case '2':
 //            case '3':
 //            case '4':
 //            case '5':
 //            case '6':
 //            case '7':
 //            case '8':
 //            case '9':
 //            {
 //            	index++;
 //            }
	// 		case '\t': 
	// 		case '\r': 
	// 		case '\n': 
	// 		case ' ':
	// 		case ',': 
	// 		case '}':
	// 		case ']':
	// 		{
	// 			end = index - 1;
	// 		}
	// 	}
	// }
}

//-----------------------------------------------------------------------------
CE_EXPORT float JSON::parse_float(const char* token)
{

}

// //-----------------------------------------------------------------------------
// void JSON::parse_string(const char* token, List<char>& str)
// {

// }

// //-----------------------------------------------------------------------------
// void JSON::parse_array(const char* token, List<const char*>& array)
// {

// }

// //-----------------------------------------------------------------------------
// void JSON::parse_object(const char* token, Dictionary<const char* key, const char* val>& dict)
// {

// }


} // namespace crown