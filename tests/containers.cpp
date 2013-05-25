#include "Crown.h"
#include <cstdio>
#include "Assert.h"

using namespace crown;

int main()
{
	MallocAllocator allocator;

	List<int> int_list(allocator);

	ce_assert(int_list.size() == 0, "Size differs from expected value");

	int_list.push_back(10);
	int_list.push_back(20);
	int_list.push_back(30);
	int_list.push_back(40);
	int_list.push_back(50);
	int_list.push_back(60);

	ce_assert(int_list.size() == 6, "Size differs from expected value");
	ce_assert(int_list.front() == 10, "Front differs from expected value");
	ce_assert(int_list.back() == 60, "Back differs from expected value");

	int_list.pop_back();

	ce_assert(int_list.size() == 5, "Size differs from expected value");
	ce_assert(int_list.front() == 10, "Front differs from expected value");
	ce_assert(int_list.back() == 50, "Back differs from expected value");

	int_list.clear();

	ce_assert(int_list.size() == 0, "Size differs from expected value");

	return 0;
}

