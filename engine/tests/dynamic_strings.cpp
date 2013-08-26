#include "DynamicString.h"
#include "Log.h"
#include "Allocator.h"

using namespace crown;

//-----------------------------------------------------------------------------
void creation_test()
{
	DynamicString string(default_allocator(), "Creation Test");
	Log::i("%s", string.c_str());

	string = "Creation Test OK!";
	Log::i("%s", string.c_str());
}

//-----------------------------------------------------------------------------
void equal_test()
{
	DynamicString string(default_allocator(), "Equal Test");
	Log::i("%s", string.c_str());

	DynamicString string1(default_allocator(), "DynamicString assigned!");	

	string = string1;
	Log::i("%s", string.c_str());

	string = "C-string assigned";
	Log::i("%s", string.c_str());

	string = 'C';
	Log::i("%s", string.c_str());

	string = "Equal Test OK!";
	Log::i("%s", string.c_str());
}

//-----------------------------------------------------------------------------
void plus_equal_test()
{
	DynamicString string(default_allocator(), "PlusEqual Test");
	DynamicString string1(default_allocator(), " DynamicString appended!");

	string += string1;
	Log::i("%s", string.c_str());

	string += " C-string appended! ";
	Log::i("%s", string.c_str());

	string += 'C';
	Log::i("%s", string.c_str());

	string = "PlusEqual Test OK!";
	Log::i("%s", string.c_str());
}

int main()
{
	creation_test();
	equal_test();
	plus_equal_test();

	return 0;
}