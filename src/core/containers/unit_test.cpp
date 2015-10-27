#include "map.h"
#include "memory.h"
#include "color4.h"
#include "array.h"
#include <assert.h>
#include <stdio.h>

using namespace crown;

struct Foo
{
	Foo(Allocator& a)
		: _array0(a)
		, _array1(a)
		, _array2(a)
	{
		printf("Allocator called\n");
	}

	Array<int> _array0;
	Array<char> _array1;
	Array<float> _array2;

	ALLOCATOR_AWARE;
};

void test_vector()
{
	memory_globals::init();
	{
		Vector<int> v(default_allocator());
		vector::push_back(v, 1);

		Vector<Foo> vfoo(default_allocator());
		Foo foo(default_allocator());
		array::push_back(foo._array0, (int)42);
		array::push_back(foo._array1, 'x');
		array::push_back(foo._array2, 3.14f);
		vector::push_back(vfoo, foo);

		printf("%d\n", vfoo[0]._array0[0]);
		printf("%c\n", vfoo[0]._array1[0]);
		printf("%f\n", vfoo[0]._array2[0]);

		Vector<int> vint(default_allocator());
		vector::push_back(vint, 9);
		vector::push_back(vint, 5);
		vector::push_back(vint, 1);
		Vector<Vector<int> > badvec(default_allocator());
		vector::push_back(badvec, vint);

		assert(badvec[0][0] == 9);
		assert(badvec[0][1] == 5);
		assert(badvec[0][2] == 1);
	}
	memory_globals::shutdown();
}


void test_map()
{
	memory_globals::init();
	{
		Map<char, int> m(default_allocator());
		map::set(m, 'a', 1);
		assert(map::get(m, 'a', (int)0) == 1);
		map::set(m, 'a', 1);

		Map<int, Vector<int> > badmap(default_allocator());

		printf("size = %d\n", map::size(m));
		map::remove(m, 'a');
		printf("size = %d\n", map::size(m));
		map::remove(m, 'a');
		printf("a = %d\n", map::get(m, 'a', (int)0));
		assert(map::get(m, 'a', (int)0) == 0);
	}
	memory_globals::shutdown();
}

int main()
{
	printf("%f %f %f %f\n", COLOR4_RED.x, COLOR4_RED.y, COLOR4_RED.z, COLOR4_RED.w);
	test_vector();
	test_map();
	return 0;
}
