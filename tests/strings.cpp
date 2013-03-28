#include "String.h"
#include <cassert>
#include <stdio.h>

using namespace crown;

const char* hello_string = "/h.ello.everybody.stri/ng";
const char* path_string = "/home/project/texture.tga";

int main()
{
	// Test strlen
	assert(string::strlen("ciao") == 4);
	// FIXME add UTF-8 test case

	// Test begin/end
	assert(string::begin(hello_string) == &hello_string[0]);
	
	assert(string::end(hello_string) == &hello_string[24] + 2);
	
	// Test find_first/find_last
	assert(string::find_first(hello_string, '.') == &hello_string[2]);
	
	assert(string::find_last(hello_string, '.') == &hello_string[17]);
	
	assert(string::find_first(hello_string, '?') == string::end(hello_string));
	
	assert(string::find_last(hello_string, '?') == string::end(hello_string));
	
	assert(string::find_last(hello_string, '/') == &hello_string[22]);
	
	assert(string::find_last(path_string, '/') == &path_string[13]);
	
	// Test substring
	char string_buffer[64];
	
	memset(string_buffer, 'a', 64);
	string::substring(string::begin(hello_string), string::end(hello_string), string_buffer, 64);
	assert(string::strcmp(hello_string, string_buffer) == 0);
	
	memset(string_buffer, 'a', 64);
	string::substring(string::begin(hello_string), &hello_string[11], string_buffer, 64);
	assert(string::strcmp(string_buffer, "/h.ello.eve") == 0);
	
	memset(string_buffer, 'a', 64);
	string::substring(string::begin(hello_string), string::end(hello_string), string_buffer, 5);
	assert(string::strcmp(string_buffer, "/h.el") == 0);
}

