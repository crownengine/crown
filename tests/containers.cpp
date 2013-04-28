#include "Crown.h"
#include <cstdio>
#include <cassert>

using namespace crown;

int main()
{
	MallocAllocator allocator;

	List<int> int_list(allocator);

	assert(int_list.size() == 0);

	int_list.push_back(10);
	int_list.push_back(20);
	int_list.push_back(30);
	int_list.push_back(40);
	int_list.push_back(50);
	int_list.push_back(60);

	assert(int_list.size() == 6);
	assert(int_list.front() == 10);
	assert(int_list.back() == 60);

	int_list.pop_back();

	assert(int_list.size() == 5);
	assert(int_list.front() == 10);
	assert(int_list.back() == 50);

	int_list.clear();

	assert(int_list.size() == 0);

	return 0;
}

