/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#if CROWN_BUILD_UNIT_TESTS

#include "array.h"
#include "config.h"
#include "dynamic_string.h"
#include "json.h"
#include "math_utils.h"
#include "memory.h"
#include "murmur.h"
#include "sjson.h"
#include "string_utils.h"
#include "temp_allocator.h"
#include "vector.h"

namespace crown
{

static void test_array()
{
	memory_globals::init();
	Allocator& a = default_allocator();
	{
		Array<int> v(a);

		CE_ENSURE(array::size(v) == 0);
		array::push_back(v, 1);
		CE_ENSURE(array::size(v) == 1);
		CE_ENSURE(v[0] == 1);
	}
	memory_globals::shutdown();
}

static void test_vector()
{
	memory_globals::init();
	Allocator& a = default_allocator();
	{
		Vector<int> v(a);

		CE_ENSURE(vector::size(v) == 0);
		vector::push_back(v, 1);
		CE_ENSURE(vector::size(v) == 1);
		CE_ENSURE(v[0] == 1);
	}
	memory_globals::shutdown();
}

static void test_murmur()
{
	const u32 m = murmur32("murmur32", 8, 0);
	CE_ENSURE(m == 0x7c2365dbu);
	const u64 n = murmur64("murmur64", 8, 0);
	CE_ENSURE(n == 0x90631502d1a3432bu);
}

static void test_json()
{
	memory_globals::init();
	{
		JsonValueType::Enum type = json::type("null");
		CE_ENSURE(type == JsonValueType::NIL);
		type = json::type("true");
		CE_ENSURE(type == JsonValueType::BOOL);
		type = json::type("false");
		CE_ENSURE(type == JsonValueType::BOOL);
		type = json::type("3.14");
		CE_ENSURE(type == JsonValueType::NUMBER);
		type = json::type("\"foo\"");
		CE_ENSURE(type == JsonValueType::STRING);
		type = json::type("[]");
		CE_ENSURE(type == JsonValueType::ARRAY);
		type = json::type("{}");
		CE_ENSURE(type == JsonValueType::OBJECT);

		const s32 i = json::parse_int("3.14");
		CE_ENSURE(i == 3);
		const f32 f = json::parse_float("3.14");
		CE_ENSURE(fequal(f, 3.14f));

		const bool b = json::parse_bool("true");
		CE_ENSURE(b == true);
		const bool c = json::parse_bool("false");
		CE_ENSURE(c == false);

		TempAllocator1024 ta;
		DynamicString str(ta);
		json::parse_string("\"This is JSON\"", str);
		CE_ENSURE(strcmp(str.c_str(), "This is JSON") == 0);
	}
	memory_globals::shutdown();
}

static void test_sjson()
{
	memory_globals::init();
	{
		JsonValueType::Enum type = json::type("null");
		CE_ENSURE(type == JsonValueType::NIL);
		type = sjson::type("true");
		CE_ENSURE(type == JsonValueType::BOOL);
		type = sjson::type("false");
		CE_ENSURE(type == JsonValueType::BOOL);
		type = sjson::type("3.14");
		CE_ENSURE(type == JsonValueType::NUMBER);
		type = sjson::type("\"foo\"");
		CE_ENSURE(type == JsonValueType::STRING);
		type = sjson::type("[]");
		CE_ENSURE(type == JsonValueType::ARRAY);
		type = sjson::type("{}");
		CE_ENSURE(type == JsonValueType::OBJECT);

		const s32 i = sjson::parse_int("3.14");
		CE_ENSURE(i == 3);
		const f32 f = sjson::parse_float("3.14");
		CE_ENSURE(fequal(f, 3.14f));

		const bool b = sjson::parse_bool("true");
		CE_ENSURE(b == true);
		const bool c = sjson::parse_bool("false");
		CE_ENSURE(c == false);

		TempAllocator1024 ta;
		DynamicString str(ta);
		sjson::parse_string("\"This is SJSON\"", str);
		CE_ENSURE(strcmp(str.c_str(), "This is SJSON") == 0);
	}
	memory_globals::shutdown();
}

static void run_unit_tests()
{
	test_array();
	test_vector();
	test_murmur();
	test_json();
	test_sjson();
}

} // namespace crown

#endif // CROWN_BUILD_UNIT_TESTS
