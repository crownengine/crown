#include "Crown.h"
#include <cstdio>
#include "Assert.h"

using namespace crown;

int main()
{
	HeapAllocator allocator;

	List<int> int_list(allocator);

	CE_ASSERT(int_list.size() == 0, "Size differs from expected value");

	int_list.push_back(10);
	int_list.push_back(20);
	int_list.push_back(30);
	int_list.push_back(40);
	int_list.push_back(50);
	int_list.push_back(60);

	CE_ASSERT(int_list.size() == 6, "Size differs from expected value");
	CE_ASSERT(int_list.front() == 10, "Front differs from expected value");
	CE_ASSERT(int_list.back() == 60, "Back differs from expected value");

	int_list.pop_back();

	CE_ASSERT(int_list.size() == 5, "Size differs from expected value");
	CE_ASSERT(int_list.front() == 10, "Front differs from expected value");
	CE_ASSERT(int_list.back() == 50, "Back differs from expected value");

	int_list.clear();

	CE_ASSERT(int_list.size() == 0, "Size differs from expected value");

	return 0;
}

