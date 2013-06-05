#include "String.h"
#include "Path.h"
#include "Assert.h"
#include <stdio.h>

using namespace crown;

int main()
{	
	char path_output[128];
	
	// Test is_valid_segment
	CE_ASSERT(path::is_valid_segment(".") == false, "");
	
	CE_ASSERT(path::is_valid_segment("/") == false, "");
	
	CE_ASSERT(path::is_valid_segment("\\") == false, "");

	CE_ASSERT(path::is_valid_segment(":") == false, "");
	
	CE_ASSERT(path::is_valid_segment("tga/") == false, "");
	
	CE_ASSERT(path::is_valid_segment("tga\\foo") == false, "");
	
	CE_ASSERT(path::is_valid_segment("tga") == true, "");
	
	CE_ASSERT(path::is_valid_segment("back_texture") == true, "");
	
	// Test pathname
	path::pathname("/home/project/texture.tga", path_output, 128);
	CE_ASSERT(string::strcmp("/home/project", path_output) == 0, "");

	path::pathname("/home/project", path_output, 128);
	CE_ASSERT(string::strcmp("/home", path_output) == 0, "");
	
	path::pathname("/home", path_output, 128);
	CE_ASSERT(string::strcmp("", path_output) == 0, "");
	
	path::pathname("/", path_output, 128);
	CE_ASSERT(string::strcmp("", path_output) == 0, "");
	
	path::pathname("", path_output, 128);
	CE_ASSERT(string::strcmp("", path_output) == 0, "");
	
	// Test filename
	path::filename("/home/project/texture.tga", path_output, 128);
	CE_ASSERT(string::strcmp("texture.tga", path_output) == 0, "");
	
	path::filename("/home/project/texture", path_output, 128);
	CE_ASSERT(string::strcmp("texture", path_output) == 0, "");
	
	path::filename("/home", path_output, 128);
	CE_ASSERT(string::strcmp("home", path_output) == 0, "");
	
	path::filename("/", path_output, 128);
	CE_ASSERT(string::strcmp("", path_output) == 0, "");
	
	path::filename("", path_output, 128);
	CE_ASSERT(string::strcmp("", path_output) == 0, "");

	// Test basename
	path::basename("/home/project/texture.tga", path_output, 128);
	CE_ASSERT(string::strcmp("texture", path_output) == 0, "");
	
	path::basename("/home/project/textureabc", path_output, 128);
	printf(path_output);
	CE_ASSERT(string::strcmp("textureabc", path_output) == 0, "");
	
	path::basename("/hom.e/proj./e.ct/textu.reabc", path_output, 128);
	CE_ASSERT(string::strcmp("textu", path_output) == 0, "");
	
	path::basename("texture.tga", path_output, 128);
	CE_ASSERT(string::strcmp("texture", path_output) == 0, "");
	
	path::basename("/home", path_output, 128);
	CE_ASSERT(string::strcmp("home", path_output) == 0, "");
	
	path::basename("/", path_output, 128);
	CE_ASSERT(string::strcmp("", path_output) == 0, "");
	
	path::basename("", path_output, 128);
	CE_ASSERT(string::strcmp("", path_output) == 0, "");
	
	// Test extension
	path::extension("/home/project/texture.tga", path_output, 128);
	CE_ASSERT(string::strcmp("tga", path_output) == 0, "");
	
	path::extension("/home/project/texture", path_output, 128);
	CE_ASSERT(string::strcmp("", path_output) == 0, "");
	
	path::extension("/home/project.x/texture.tga", path_output, 128);
	CE_ASSERT(string::strcmp("tga", path_output) == 0, "");
	
	// Test filename_without_extension
	path::filename_without_extension("/home/project/texture.tga", path_output, 128);
	CE_ASSERT(string::strcmp("/home/project/texture", path_output) == 0, "");
	
	// Test strip_trailing_separator
	path::strip_trailing_separator("/home/project/texture.tga", path_output, 128);
	CE_ASSERT(string::strcmp("/home/project/texture.tga", path_output) == 0, "");
	
	path::strip_trailing_separator("/home/project/texture2.tga/", path_output, 128);
	CE_ASSERT(string::strcmp("/home/project/texture2.tga", path_output) == 0, "");
	
	path::strip_trailing_separator("texture.tga", path_output, 128);
	CE_ASSERT(string::strcmp("texture.tga", path_output) == 0, "");
}

