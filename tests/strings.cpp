#include "String.h"
#include "Path.h"
#include <cassert>
#include <stdio.h>

using namespace crown;

int main()
{	
	char path_output[128];
	
	// Test pathname
	path::pathname("/home/project/texture.tga", path_output, 128);
	assert(string::strcmp("/home/project", path_output) == 0);

	path::pathname("/home/project", path_output, 128);
	assert(string::strcmp("/home", path_output) == 0);
	
	path::pathname("/home", path_output, 128);
	assert(string::strcmp("", path_output) == 0);
	
	path::pathname("/", path_output, 128);
	assert(string::strcmp("", path_output) == 0);
	
	path::pathname("", path_output, 128);
	assert(string::strcmp("", path_output) == 0);
	
	// Test filename
	path::filename("/home/project/texture.tga", path_output, 128);
	assert(string::strcmp("texture.tga", path_output) == 0);
	
	path::filename("/home/project/texture", path_output, 128);
	assert(string::strcmp("texture", path_output) == 0);
	
	path::filename("/home", path_output, 128);
	assert(string::strcmp("home", path_output) == 0);
	
	path::filename("/", path_output, 128);
	assert(string::strcmp("", path_output) == 0);
	
	path::filename("", path_output, 128);
	assert(string::strcmp("", path_output) == 0);
	
	// Test extension
	path::extension("/home/project/texture.tga", path_output, 128);
	assert(string::strcmp("tga", path_output) == 0);
	
	path::extension("/home/project/texture", path_output, 128);
	assert(string::strcmp("", path_output) == 0);
	
	path::extension("/home/project.x/texture.tga", path_output, 128);
	assert(string::strcmp("tga", path_output) == 0);
}

