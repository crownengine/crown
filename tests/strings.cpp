#include "String.h"
#include "Assert.h"
#include <stdio.h>

using namespace crown;

const char* hello_string = "/h.ello.everybody.stri/ng";
const char* path_string = "/home/project/texture.tga";

int main()
{
	// Test strlen
	ce_assert(string::strlen("ciao") == 4);
	// FIXME add UTF-8 test case

	// Test begin/end
	ce_assert(string::begin(hello_string) == &hello_string[0]);
	
	ce_assert(string::end(hello_string) == &hello_string[24] + 2);
	
	// Test find_first/find_last
	ce_assert(string::find_first(hello_string, '.') == &hello_string[2]);
	
	ce_assert(string::find_last(hello_string, '.') == &hello_string[17]);
	
	ce_assert(string::find_first(hello_string, '?') == string::end(hello_string));
	
	ce_assert(string::find_last(hello_string, '?') == string::end(hello_string));
	
	ce_assert(string::find_last(hello_string, '/') == &hello_string[22]);
	
	ce_assert(string::find_last(path_string, '/') == &path_string[13]);
	
	// Test substring
	char string_buffer[64];
	
	memset(string_buffer, 'a', 64);
	string::substring(string::begin(hello_string), string::end(hello_string), string_buffer, 64);
	ce_assert(string::strcmp(hello_string, string_buffer) == 0);
	
	memset(string_buffer, 'a', 64);
	string::substring(string::begin(hello_string), &hello_string[11], string_buffer, 64);
	ce_assert(string::strcmp(string_buffer, "/h.ello.eve") == 0);
	
	memset(string_buffer, 'a', 64);
	string::substring(string::begin(hello_string), string::end(hello_string), string_buffer, 5);
	ce_assert(string::strcmp(string_buffer, "/h.el") == 0);
}

