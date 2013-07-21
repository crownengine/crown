#include "JSONParser.h"
#include "DiskFile.h"
#include "OS.h"
#include "StringUtils.h"
#include "Assert.h"
#include <stdlib.h>

namespace crown
{

//--------------------------------------------------------------------------
JSONParser::JSONParser(Allocator& allocator, const char* s) :
	m_allocator(allocator)
{
}

//--------------------------------------------------------------------------
JSONParser&	JSONParser::root()
{
}

//--------------------------------------------------------------------------
JSONParser&	JSONParser::object(const char* key)
{
	//JSON::parse_object()
}

//--------------------------------------------------------------------------
JSONParser&	JSONParser::array(const char* key, uint32_t index)
{

}

//--------------------------------------------------------------------------
JSONParser& JSONParser::string(const char* key)
{

}

//--------------------------------------------------------------------------
JSONParser&	JSONParser::number(const char* key)
{

}		

//--------------------------------------------------------------------------
JSONParser&	JSONParser::boolean(const char* key)
{

}



} //namespace crown