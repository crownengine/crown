/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "config.h"

#if CROWN_BUILD_UNIT_TESTS

#include "core/command_line.h"
#include "core/containers/array.inl"
#include "core/containers/hash_map.inl"
#include "core/containers/hash_set.inl"
#include "core/containers/vector.inl"
#include "core/filesystem/path.h"
#include "core/guid.inl"
#include "core/json/json.h"
#include "core/json/sjson.h"
#include "core/math/aabb.inl"
#include "core/math/color4.inl"
#include "core/math/constants.h"
#include "core/math/math.h"
#include "core/math/matrix3x3.inl"
#include "core/math/matrix4x4.inl"
#include "core/math/quaternion.inl"
#include "core/math/sphere.inl"
#include "core/math/vector2.inl"
#include "core/math/vector3.inl"
#include "core/math/vector4.inl"
#include "core/memory/memory.inl"
#include "core/memory/temp_allocator.inl"
#include "core/murmur.h"
#include "core/os.h"
#include "core/process.h"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string.inl"
#include "core/strings/string_id.inl"
#include "core/strings/string_view.inl"
#include "core/thread/thread.h"
#include "core/time.h"
#include <stdlib.h> // EXIT_SUCCESS, EXIT_FAILURE

#undef CE_ASSERT
#undef CE_ENSURE
#undef CE_FATAL
#define ENSURE(condition)                                \
	do                                                   \
	{                                                    \
		if (!(condition))                                \
		{                                                \
			printf("Assertion failed: '%s' in %s:%d\n\n" \
				, #condition                             \
				, __FILE__                               \
				, __LINE__                               \
				);                                       \
			exit(EXIT_FAILURE);                          \
		}                                                \
	}                                                    \
	while (0)

namespace crown
{
static void test_memory()
{
	memory_globals::init();
	Allocator& a = default_allocator();

	void* p = a.allocate(32);
	ENSURE(a.allocated_size(p) >= 32);
	a.deallocate(p);

	memory_globals::shutdown();
}

static void test_array()
{
	memory_globals::init();
	Allocator& a = default_allocator();
	{
		Array<int> v(a);

		ENSURE(array::size(v) == 0);
		array::push_back(v, 1);
		ENSURE(array::size(v) == 1);
		ENSURE(v[0] == 1);
	}
	memory_globals::shutdown();
}

static void test_vector()
{
	memory_globals::init();
	Allocator& a = default_allocator();
	{
		Vector<int> v(a);

		ENSURE(vector::size(v) == 0);
		vector::push_back(v, 1);
		ENSURE(vector::size(v) == 1);
		ENSURE(v[0] == 1);
	}
	memory_globals::shutdown();
}

static void test_hash_map()
{
	memory_globals::init();
	Allocator& a = default_allocator();
	{
		HashMap<s32, s32> m(a);

		ENSURE(hash_map::size(m) == 0);
		ENSURE(hash_map::get(m, 0, 42) == 42);
		ENSURE(!hash_map::has(m, 10));

		for (s32 i = 0; i < 100; ++i)
			hash_map::set(m, i, i*i);
		for (s32 i = 0; i < 100; ++i)
			ENSURE(hash_map::get(m, i, 0) == i*i);

		hash_map::remove(m, 20);
		ENSURE(!hash_map::has(m, 20));

		hash_map::remove(m, 2000);
		ENSURE(!hash_map::has(m, 2000));

		hash_map::remove(m, 50);
		ENSURE(!hash_map::has(m, 50));

		hash_map::clear(m);

		for (s32 i = 0; i < 100; ++i)
			ENSURE(!hash_map::has(m, i));
	}
	{
		HashMap<s32, s32> m(a);
		hash_map_internal::grow(m);
		ENSURE(hash_map::capacity(m) == 16);

		hash_map::set(m, 0, 7);

		hash_map::set(m, 1, 1);

		for (s32 i = 2; i < 150; ++i)
		{
			hash_map::set(m, i, 2);
			ENSURE(hash_map::has(m, 0));
			ENSURE(hash_map::has(m, 1));
			ENSURE(hash_map::has(m, i));
			hash_map::remove(m, i);
		}
	}
	{
		HashMap<s32, s32> ma(a);
		HashMap<s32, s32> mb(a);
		hash_map::set(ma, 0, 0);
		ma = mb;
	}
	memory_globals::shutdown();
}

static void test_hash_set()
{
	memory_globals::init();
	Allocator& a = default_allocator();
	{
		HashSet<s32> m(a);

		ENSURE(hash_set::size(m) == 0);
		ENSURE(!hash_set::has(m, 10));

		for (s32 i = 0; i < 100; ++i)
			hash_set::insert(m, i*i);
		for (s32 i = 0; i < 100; ++i)
			ENSURE(hash_set::has(m, i*i));

		hash_set::remove(m, 5*5);
		ENSURE(!hash_set::has(m, 5*5));

		hash_set::remove(m, 80*80);
		ENSURE(!hash_set::has(m, 80*80));

		hash_set::remove(m, 40*40);
		ENSURE(!hash_set::has(m, 40*40));

		hash_set::clear(m);

		for (s32 i = 0; i < 100; ++i)
			ENSURE(!hash_set::has(m, i*i));
	}
	{
		HashSet<s32> ma(a);
		HashSet<s32> mb(a);
		hash_set::insert(ma, 0);
		ma = mb;
	}
	memory_globals::shutdown();
}

static void test_vector2()
{
	{
		const Vector2 a = vector2(1.2f,  4.2f);
		const Vector2 b = vector2(2.7f, -1.9f);
		const Vector2 c = a - b;
		ENSURE(fequal(c.x, -1.5f, 0.0001f));
		ENSURE(fequal(c.y,  6.1f, 0.0001f));
	}
	{
		const Vector2 a = vector2(1.2f,  4.2f);
		const Vector2 b = vector2(2.7f, -1.9f);
		const Vector2 c = a + b;
		ENSURE(fequal(c.x,  3.9f, 0.0001f));
		ENSURE(fequal(c.y,  2.3f, 0.0001f));
	}
	{
		const Vector2 a = vector2(1.2f,  4.2f);
		const Vector2 b = a * 2.0f;
		ENSURE(fequal(b.x,  2.4f, 0.0001f));
		ENSURE(fequal(b.y,  8.4f, 0.0001f));
	}
	{
		const Vector2 a = vector2(1.2f,  4.2f);
		const Vector2 b = vector2(2.7f, -1.9f);
		const f32 c = dot(a, b);
		ENSURE(fequal(c,  -4.74f, 0.0001f));
	}
	{
		const Vector2 a = vector2(1.2f,  4.2f);
		const f32 c = length_squared(a);
		ENSURE(fequal(c, 19.08f, 0.0001f));
	}
	{
		const Vector2 a = vector2(1.2f,  4.2f);
		const f32 c = length(a);
		ENSURE(fequal(c,  4.36806f, 0.0001f));
	}
	{
		Vector2 a = vector2(1.2f,  4.2f);
		normalize(a);
		ENSURE(fequal(length(a), 1.0f, 0.00001f));
	}
	{
		const Vector2 a = vector2(1.2f,  4.2f);
		const Vector2 b = vector2(2.7f, -1.9f);
		const float c = distance_squared(a, b);
		ENSURE(fequal(c, 39.46f, 0.00001f));
	}
	{
		const Vector2 a = vector2(1.2f,  4.2f);
		const Vector2 b = vector2(2.7f, -1.9f);
		const float c = distance(a, b);
		ENSURE(fequal(c, 6.28171f, 0.00001f));
	}
	{
		const Vector2 a = vector2(1.2f,  4.2f);
		const Vector2 b = vector2(2.7f, -1.9f);
		const Vector2 c = max(a, b);
		ENSURE(fequal(c.x,  2.7f, 0.00001f));
		ENSURE(fequal(c.y,  4.2f, 0.00001f));
	}
	{
		const Vector2 a = vector2(1.2f,  4.2f);
		const Vector2 b = vector2(2.7f, -1.9f);
		const Vector2 c = min(a, b);
		ENSURE(fequal(c.x,  1.2f, 0.00001f));
		ENSURE(fequal(c.y, -1.9f, 0.00001f));
	}
}

static void test_vector3()
{
	{
		const Vector3 a = vector3(1.2f,  4.2f, -2.3f);
		const Vector3 b = vector3(2.7f, -1.9f, -4.1f);
		const Vector3 c = a - b;
		ENSURE(fequal(c.x, -1.5f, 0.0001f));
		ENSURE(fequal(c.y,  6.1f, 0.0001f));
		ENSURE(fequal(c.z,  1.8f, 0.0001f));
	}
	{
		const Vector3 a = vector3(1.2f,  4.2f, -2.3f);
		const Vector3 b = vector3(2.7f, -1.9f, -4.1f);
		const Vector3 c = a + b;
		ENSURE(fequal(c.x,  3.9f, 0.0001f));
		ENSURE(fequal(c.y,  2.3f, 0.0001f));
		ENSURE(fequal(c.z, -6.4f, 0.0001f));
	}
	{
		const Vector3 a = vector3(1.2f,  4.2f, -2.3f);
		const Vector3 b = a * 2.0f;
		ENSURE(fequal(b.x,  2.4f, 0.0001f));
		ENSURE(fequal(b.y,  8.4f, 0.0001f));
		ENSURE(fequal(b.z, -4.6f, 0.0001f));
	}
	{
		const Vector3 a = vector3(1.2f,  4.2f, -2.3f);
		const Vector3 b = vector3(2.7f, -1.9f, -4.1f);
		const f32 c = dot(a, b);
		ENSURE(fequal(c,  4.69f, 0.0001f));
	}
	{
		const Vector3 a = vector3(1.2f,  4.2f, -2.3f);
		const Vector3 b = vector3(2.7f, -1.9f, -4.1f);
		const Vector3 c = cross(a, b);
		ENSURE(fequal(c.x, -21.59f, 0.0001f));
		ENSURE(fequal(c.y,  -1.29f, 0.0001f));
		ENSURE(fequal(c.z, -13.62f, 0.0001f));
	}
	{
		const Vector3 a = vector3(1.2f,  4.2f, -2.3f);
		const f32 c = length_squared(a);
		ENSURE(fequal(c, 24.37f, 0.0001f));
	}
	{
		const Vector3 a = vector3(1.2f,  4.2f, -2.3f);
		const f32 c = length(a);
		ENSURE(fequal(c,  4.93659f, 0.0001f));
	}
	{
		Vector3 a = vector3(1.2f,  4.2f, -2.3f);
		normalize(a);
		ENSURE(fequal(length(a), 1.0f, 0.00001f));
	}
	{
		const Vector3 a = vector3(1.2f,  4.2f, -2.3f);
		const Vector3 b = vector3(2.7f, -1.9f, -4.1f);
		const float c = distance_squared(a, b);
		ENSURE(fequal(c, 42.70f, 0.00001f));
	}
	{
		const Vector3 a = vector3(1.2f,  4.2f, -2.3f);
		const Vector3 b = vector3(2.7f, -1.9f, -4.1f);
		const float c = distance(a, b);
		ENSURE(fequal(c, 6.53452f, 0.00001f));
	}
	{
		const Vector3 a = vector3(1.2f,  4.2f, -2.3f);
		const Vector3 b = vector3(2.7f, -1.9f, -4.1f);
		const Vector3 c = max(a, b);
		ENSURE(fequal(c.x,  2.7f, 0.00001f));
		ENSURE(fequal(c.y,  4.2f, 0.00001f));
		ENSURE(fequal(c.z, -2.3f, 0.00001f));
	}
	{
		const Vector3 a = vector3(1.2f,  4.2f, -2.3f);
		const Vector3 b = vector3(2.7f, -1.9f, -4.1f);
		const Vector3 c = min(a, b);
		ENSURE(fequal(c.x,  1.2f, 0.00001f));
		ENSURE(fequal(c.y, -1.9f, 0.00001f));
		ENSURE(fequal(c.z, -4.1f, 0.00001f));
	}
}

static void test_vector4()
{
	{
		const Vector4 a = vector4(1.2f,  4.2f, -2.3f, 5.5f);
		const Vector4 b = vector4(2.7f, -1.9f, -4.1f, 1.0f);
		const Vector4 c = a - b;
		ENSURE(fequal(c.x, -1.5f, 0.0001f));
		ENSURE(fequal(c.y,  6.1f, 0.0001f));
		ENSURE(fequal(c.z,  1.8f, 0.0001f));
		ENSURE(fequal(c.w,  4.5f, 0.0001f));
	}
	{
		const Vector4 a = vector4(1.2f,  4.2f, -2.3f, 5.5f);
		const Vector4 b = vector4(2.7f, -1.9f, -4.1f, 1.0f);
		const Vector4 c = a + b;
		ENSURE(fequal(c.x,  3.9f, 0.0001f));
		ENSURE(fequal(c.y,  2.3f, 0.0001f));
		ENSURE(fequal(c.z, -6.4f, 0.0001f));
		ENSURE(fequal(c.w,  6.5f, 0.0001f));
	}
	{
		const Vector4 a = vector4(1.2f,  4.2f, -2.3f, 1.5f);
		const Vector4 b = a * 2.0f;
		ENSURE(fequal(b.x,  2.4f, 0.0001f));
		ENSURE(fequal(b.y,  8.4f, 0.0001f));
		ENSURE(fequal(b.z, -4.6f, 0.0001f));
		ENSURE(fequal(b.w,  3.0f, 0.0001f));
	}
	{
		const Vector4 a = vector4(1.2f,  4.2f, -2.3f, 5.5f);
		const Vector4 b = vector4(2.7f, -1.9f, -4.1f, 1.0f);
		const f32 c = dot(a, b);
		ENSURE(fequal(c,  10.19f, 0.0001f));
	}
	{
		const Vector4 a = vector4(1.2f,  4.2f, -2.3f, 5.5f);
		const f32 c = length_squared(a);
		ENSURE(fequal(c, 54.62f, 0.0001f));
	}
	{
		const Vector4 a = vector4(1.2f,  4.2f, -2.3f, 5.5f);
		const f32 c = length(a);
		ENSURE(fequal(c,  7.39053f, 0.0001f));
	}
	{
		Vector4 a = vector4(1.2f,  4.2f, -2.3f, 5.5f);
		normalize(a);
		ENSURE(fequal(length(a), 1.0f, 0.00001f));
	}
	{
		const Vector4 a = vector4(1.2f,  4.2f, -2.3f, 5.5f);
		const Vector4 b = vector4(2.7f, -1.9f, -4.1f, 1.0f);
		const float c = distance_squared(a, b);
		ENSURE(fequal(c, 62.95f, 0.00001f));
	}
	{
		const Vector4 a = vector4(1.2f,  4.2f, -2.3f, 5.5f);
		const Vector4 b = vector4(2.7f, -1.9f, -4.1f, 1.0f);
		const float c = distance(a, b);
		ENSURE(fequal(c, 7.93410f, 0.00001f));
	}
	{
		const Vector4 a = vector4(1.2f,  4.2f, -2.3f, 5.5f);
		const Vector4 b = vector4(2.7f, -1.9f, -4.1f, 1.0f);
		const Vector4 c = max(a, b);
		ENSURE(fequal(c.x,  2.7f, 0.00001f));
		ENSURE(fequal(c.y,  4.2f, 0.00001f));
		ENSURE(fequal(c.z, -2.3f, 0.00001f));
		ENSURE(fequal(c.w,  5.5f, 0.00001f));
	}
	{
		const Vector4 a = vector4(1.2f,  4.2f, -2.3f, 5.5f);
		const Vector4 b = vector4(2.7f, -1.9f, -4.1f, 1.0f);
		const Vector4 c = min(a, b);
		ENSURE(fequal(c.x,  1.2f, 0.00001f));
		ENSURE(fequal(c.y, -1.9f, 0.00001f));
		ENSURE(fequal(c.z, -4.1f, 0.00001f));
		ENSURE(fequal(c.w,  1.0f, 0.00001f));
	}
}

static void test_quaternion()
{
	{
		const Quaternion a = from_elements(0.0f, 0.0f, 0.0f, 1.0f);
		ENSURE(fequal(a.x, 0.0f, 0.00001f));
		ENSURE(fequal(a.y, 0.0f, 0.00001f));
		ENSURE(fequal(a.z, 0.0f, 0.00001f));
		ENSURE(fequal(a.w, 1.0f, 0.00001f));
	}
}

static void test_color4()
{
	{
		const Color4 a = color4(1.3f, 2.6f, 0.2f, 0.6f);
		ENSURE(fequal(a.x, 1.3f, 0.00001f));
		ENSURE(fequal(a.y, 2.6f, 0.00001f));
		ENSURE(fequal(a.z, 0.2f, 0.00001f));
		ENSURE(fequal(a.w, 0.6f, 0.00001f));
	}
	{
		const Color4 a = from_rgba(63, 231, 12, 98);
		ENSURE(fequal(a.x, 0.24705f, 0.00001f));
		ENSURE(fequal(a.y, 0.90588f, 0.00001f));
		ENSURE(fequal(a.z, 0.04705f, 0.00001f));
		ENSURE(fequal(a.w, 0.38431f, 0.00001f));
	}
	{
		const Color4 a = from_rgb(63, 231, 12);
		ENSURE(fequal(a.x, 0.24705f, 0.00001f));
		ENSURE(fequal(a.y, 0.90588f, 0.00001f));
		ENSURE(fequal(a.z, 0.04705f, 0.00001f));
		ENSURE(fequal(a.w, 1.0f    , 0.00001f));
	}
	{
		const Color4 a = from_rgba(0x3fe70c62);
		ENSURE(fequal(a.x, 0.24705f, 0.00001f));
		ENSURE(fequal(a.y, 0.90588f, 0.00001f));
		ENSURE(fequal(a.z, 0.04705f, 0.00001f));
		ENSURE(fequal(a.w, 0.38431f, 0.00001f));
	}
	{
		const Color4 a = from_rgba(63, 231, 12, 98);

		const u32 rgba = to_rgba(a);
		ENSURE(rgba == 0x3fe70c62);

		const u32 rgb = to_rgb(a);
		ENSURE(rgb == 0x3fe70cff);

		const u32 bgr = to_bgr(a);
		ENSURE(bgr == 0xff0ce73f);

		const u32 abgr = to_abgr(a);
		ENSURE(abgr == 0x620ce73f);
	}
}

static void test_matrix3x3()
{
	{
		const Matrix3x3 a = from_elements(1.2f, -2.3f, 5.1f
			,  2.2f, -5.1f,  1.1f
			,  3.2f,  3.3f, -3.8f
			);
		const Matrix3x3 b = from_elements(3.2f, 4.8f, 6.0f
			, -1.6f, -7.1f, -2.4f
			, -3.1f, -2.2f,  8.9f
			);
		const Matrix3x3 c = a + b;
		ENSURE(fequal(c.x.x,   4.4f, 0.00001f));
		ENSURE(fequal(c.x.y,   2.5f, 0.00001f));
		ENSURE(fequal(c.x.z,  11.1f, 0.00001f));
		ENSURE(fequal(c.y.x,   0.6f, 0.00001f));
		ENSURE(fequal(c.y.y, -12.2f, 0.00001f));
		ENSURE(fequal(c.y.z,  -1.3f, 0.00001f));
		ENSURE(fequal(c.z.x,   0.1f, 0.00001f));
		ENSURE(fequal(c.z.y,   1.1f, 0.00001f));
		ENSURE(fequal(c.z.z,   5.1f, 0.00001f));
	}
	{
		const Matrix3x3 a = from_elements(1.2f, -2.3f, 5.1f
			,  2.2f, -5.1f,  1.1f
			,  3.2f,  3.3f, -3.8f
			);
		const Matrix3x3 b = from_elements(3.2f, 4.8f, 6.0f
			, -1.6f, -7.1f, -2.4f
			, -3.1f, -2.2f,  8.9f
			);
		const Matrix3x3 c = a - b;
		ENSURE(fequal(c.x.x,  -2.0f, 0.00001f));
		ENSURE(fequal(c.x.y,  -7.1f, 0.00001f));
		ENSURE(fequal(c.x.z,  -0.9f, 0.00001f));
		ENSURE(fequal(c.y.x,   3.8f, 0.00001f));
		ENSURE(fequal(c.y.y,   2.0f, 0.00001f));
		ENSURE(fequal(c.y.z,   3.5f, 0.00001f));
		ENSURE(fequal(c.z.x,   6.3f, 0.00001f));
		ENSURE(fequal(c.z.y,   5.5f, 0.00001f));
		ENSURE(fequal(c.z.z, -12.7f, 0.00001f));
	}
	{
		const Matrix3x3 a = from_elements(1.2f, -2.3f, 5.1f
			,  2.2f, -5.1f,  1.1f
			,  3.2f,  3.3f, -3.8f
			);
		const Matrix3x3 b = from_elements(3.2f, 4.8f, 6.0f
			, -1.6f, -7.1f, -2.4f
			, -3.1f, -2.2f,  8.9f
			);
		const Matrix3x3 c = a * b;
		ENSURE(fequal(c.x.x,  -8.29f, 0.00001f));
		ENSURE(fequal(c.x.y,  10.87f, 0.00001f));
		ENSURE(fequal(c.x.z,  58.11f, 0.00001f));
		ENSURE(fequal(c.y.x,  11.79f, 0.00001f));
		ENSURE(fequal(c.y.y,  44.35f, 0.00001f));
		ENSURE(fequal(c.y.z,  35.23f, 0.00001f));
		ENSURE(fequal(c.z.x,  16.74f, 0.00001f));
		ENSURE(fequal(c.z.y,   0.29f, 0.00001f));
		ENSURE(fequal(c.z.z, -22.54f, 0.00001f));
	}
	{
		const Matrix3x3 a = from_elements(1.2f, -2.3f, 5.1f
			,  2.2f, -5.1f,  1.1f
			,  3.2f,  3.3f, -3.8f
			);
		const Matrix3x3 b = get_inverted(a);
		ENSURE(fequal(b.x.x,  0.140833f, 0.00001f));
		ENSURE(fequal(b.x.y,  0.072339f, 0.00001f));
		ENSURE(fequal(b.x.z,  0.209954f, 0.00001f));
		ENSURE(fequal(b.y.x,  0.106228f, 0.00001f));
		ENSURE(fequal(b.y.y, -0.186705f, 0.00001f));
		ENSURE(fequal(b.y.z,  0.088524f, 0.00001f));
		ENSURE(fequal(b.z.x,  0.210848f, 0.00001f));
		ENSURE(fequal(b.z.y, -0.101221f, 0.00001f));
		ENSURE(fequal(b.z.z, -0.009478f, 0.00001f));
	}
	{
		const Matrix3x3 a = from_elements(1.2f, -2.3f, 5.1f
			,  2.2f, -5.1f,  1.1f
			,  3.2f,  3.3f, -3.8f
			);
		const Matrix3x3 b = get_transposed(a);
		ENSURE(fequal(b.x.x,  1.2f, 0.00001f));
		ENSURE(fequal(b.x.y,  2.2f, 0.00001f));
		ENSURE(fequal(b.x.z,  3.2f, 0.00001f));
		ENSURE(fequal(b.y.x, -2.3f, 0.00001f));
		ENSURE(fequal(b.y.y, -5.1f, 0.00001f));
		ENSURE(fequal(b.y.z,  3.3f, 0.00001f));
		ENSURE(fequal(b.z.x,  5.1f, 0.00001f));
		ENSURE(fequal(b.z.y,  1.1f, 0.00001f));
		ENSURE(fequal(b.z.z, -3.8f, 0.00001f));
	}
}

static void test_matrix4x4()
{
	{
		const Matrix4x4 a = from_elements(1.2f, -2.3f, 5.1f, -1.2f
			,  2.2f, -5.1f,  1.1f, -7.4f
			,  3.2f,  3.3f, -3.8f, -9.2f
			, -6.8f, -2.9f,  1.0f,  4.9f
			);
		const Matrix4x4 b = from_elements(3.2f, 4.8f, 6.0f, 5.3f
			, -1.6f, -7.1f, -2.4f, -6.2f
			, -3.1f, -2.2f,  8.9f,  8.3f
			,  3.8f,  9.1f, -3.1f, -7.1f
			);
		const Matrix4x4 c = a + b;
		ENSURE(fequal(c.x.x,   4.4f, 0.00001f));
		ENSURE(fequal(c.x.y,   2.5f, 0.00001f));
		ENSURE(fequal(c.x.z,  11.1f, 0.00001f));
		ENSURE(fequal(c.x.w,   4.1f, 0.00001f));
		ENSURE(fequal(c.y.x,   0.6f, 0.00001f));
		ENSURE(fequal(c.y.y, -12.2f, 0.00001f));
		ENSURE(fequal(c.y.z,  -1.3f, 0.00001f));
		ENSURE(fequal(c.y.w, -13.6f, 0.00001f));
		ENSURE(fequal(c.z.x,   0.1f, 0.00001f));
		ENSURE(fequal(c.z.y,   1.1f, 0.00001f));
		ENSURE(fequal(c.z.z,   5.1f, 0.00001f));
		ENSURE(fequal(c.z.w,  -0.9f, 0.00001f));
		ENSURE(fequal(c.t.x,  -3.0f, 0.00001f));
		ENSURE(fequal(c.t.y,   6.2f, 0.00001f));
		ENSURE(fequal(c.t.z,  -2.1f, 0.00001f));
		ENSURE(fequal(c.t.w,  -2.2f, 0.00001f));
	}
	{
		const Matrix4x4 a = from_elements(1.2f, -2.3f, 5.1f, -1.2f
			,  2.2f, -5.1f,  1.1f, -7.4f
			,  3.2f,  3.3f, -3.8f, -9.2f
			, -6.8f, -2.9f,  1.0f,  4.9f
			);
		const Matrix4x4 b = from_elements(3.2f, 4.8f, 6.0f, 5.3f
			, -1.6f, -7.1f, -2.4f, -6.2f
			, -3.1f, -2.2f,  8.9f,  8.3f
			,  3.8f,  9.1f, -3.1f, -7.1f
			);
		const Matrix4x4 c = a - b;
		ENSURE(fequal(c.x.x,  -2.0f, 0.00001f));
		ENSURE(fequal(c.x.y,  -7.1f, 0.00001f));
		ENSURE(fequal(c.x.z,  -0.9f, 0.00001f));
		ENSURE(fequal(c.x.w,  -6.5f, 0.00001f));
		ENSURE(fequal(c.y.x,   3.8f, 0.00001f));
		ENSURE(fequal(c.y.y,   2.0f, 0.00001f));
		ENSURE(fequal(c.y.z,   3.5f, 0.00001f));
		ENSURE(fequal(c.y.w,  -1.2f, 0.00001f));
		ENSURE(fequal(c.z.x,   6.3f, 0.00001f));
		ENSURE(fequal(c.z.y,   5.5f, 0.00001f));
		ENSURE(fequal(c.z.z, -12.7f, 0.00001f));
		ENSURE(fequal(c.z.w, -17.5f, 0.00001f));
		ENSURE(fequal(c.t.x, -10.6f, 0.00001f));
		ENSURE(fequal(c.t.y, -12.0f, 0.00001f));
		ENSURE(fequal(c.t.z,   4.1f, 0.00001f));
		ENSURE(fequal(c.t.w,  12.0f, 0.00001f));
	}
	{
		const Matrix4x4 a = from_elements(1.2f, -2.3f, 5.1f, -1.2f
			,  2.2f, -5.1f,  1.1f, -7.4f
			,  3.2f,  3.3f, -3.8f, -9.2f
			, -6.8f, -2.9f,  1.0f,  4.9f
			);
		const Matrix4x4 b = from_elements(3.2f, 4.8f, 6.0f, 5.3f
			, -1.6f, -7.1f, -2.4f, -6.2f
			, -3.1f, -2.2f,  8.9f,  8.3f
			,  3.8f,  9.1f, -3.1f, -7.1f
			);
		const Matrix4x4 c = a * b;
		ENSURE(fequal(c.x.x, -12.85f, 0.00001f));
		ENSURE(fequal(c.x.y,  -0.05f, 0.00001f));
		ENSURE(fequal(c.x.z,  61.83f, 0.00001f));
		ENSURE(fequal(c.x.w,  71.47f, 0.00001f));
		ENSURE(fequal(c.y.x, -16.33f, 0.00001f));
		ENSURE(fequal(c.y.y, -22.99f, 0.00001f));
		ENSURE(fequal(c.y.z,  58.17f, 0.00001f));
		ENSURE(fequal(c.y.w, 104.95f, 0.00001f));
		ENSURE(fequal(c.z.x, -18.22f, 0.00001f));
		ENSURE(fequal(c.z.y, -83.43f, 0.00001f));
		ENSURE(fequal(c.z.z,   5.98f, 0.00001f));
		ENSURE(fequal(c.z.w,  30.28f, 0.00001f));
		ENSURE(fequal(c.t.x,  -1.60f, 0.00001f));
		ENSURE(fequal(c.t.y,  30.34f, 0.00001f));
		ENSURE(fequal(c.t.z, -40.13f, 0.00001f));
		ENSURE(fequal(c.t.w, -44.55f, 0.00001f));
	}
	{
		const Matrix4x4 a = from_elements(1.2f, -2.3f, 5.1f, -1.2f
			,  2.2f, -5.1f,  1.1f, -7.4f
			,  3.2f,  3.3f, -3.8f, -9.2f
			, -6.8f, -2.9f,  1.0f,  4.9f
			);
		const Matrix4x4 b = get_inverted(a);
		ENSURE(fequal(b.x.x, -0.08464f, 0.00001f));
		ENSURE(fequal(b.x.y,  0.06129f, 0.00001f));
		ENSURE(fequal(b.x.z, -0.15210f, 0.00001f));
		ENSURE(fequal(b.x.w, -0.21374f, 0.00001f));
		ENSURE(fequal(b.y.x,  0.14384f, 0.00001f));
		ENSURE(fequal(b.y.y, -0.18486f, 0.00001f));
		ENSURE(fequal(b.y.z,  0.14892f, 0.00001f));
		ENSURE(fequal(b.y.w,  0.03565f, 0.00001f));
		ENSURE(fequal(b.z.x,  0.26073f, 0.00001f));
		ENSURE(fequal(b.z.y, -0.09877f, 0.00001f));
		ENSURE(fequal(b.z.z,  0.07063f, 0.00001f));
		ENSURE(fequal(b.z.w,  0.04729f, 0.00001f));
		ENSURE(fequal(b.t.x, -0.08553f, 0.00001f));
		ENSURE(fequal(b.t.y, -0.00419f, 0.00001f));
		ENSURE(fequal(b.t.z, -0.13735f, 0.00001f));
		ENSURE(fequal(b.t.w, -0.08108f, 0.00001f));
	}
	{
		const Matrix4x4 a = from_elements(1.2f, -2.3f, 5.1f, -1.2f
			,  2.2f, -5.1f,  1.1f, -7.4f
			,  3.2f,  3.3f, -3.8f, -9.2f
			, -6.8f, -2.9f,  1.0f,  4.9f
			);
		const Matrix4x4 b = get_transposed(a);
		ENSURE(fequal(b.x.x,  1.2f, 0.00001f));
		ENSURE(fequal(b.x.y,  2.2f, 0.00001f));
		ENSURE(fequal(b.x.z,  3.2f, 0.00001f));
		ENSURE(fequal(b.x.w, -6.8f, 0.00001f));
		ENSURE(fequal(b.y.x, -2.3f, 0.00001f));
		ENSURE(fequal(b.y.y, -5.1f, 0.00001f));
		ENSURE(fequal(b.y.z,  3.3f, 0.00001f));
		ENSURE(fequal(b.y.w, -2.9f, 0.00001f));
		ENSURE(fequal(b.z.x,  5.1f, 0.00001f));
		ENSURE(fequal(b.z.y,  1.1f, 0.00001f));
		ENSURE(fequal(b.z.z, -3.8f, 0.00001f));
		ENSURE(fequal(b.z.w,  1.0f, 0.00001f));
		ENSURE(fequal(b.t.x, -1.2f, 0.00001f));
		ENSURE(fequal(b.t.y, -7.4f, 0.00001f));
		ENSURE(fequal(b.t.z, -9.2f, 0.00001f));
		ENSURE(fequal(b.t.w,  4.9f, 0.00001f));
	}
	{
		Matrix4x4 m;
		Quaternion q;

		m = MATRIX4X4_IDENTITY;
		m.x.x = 0.0f;
		q = rotation(m);
		ENSURE(memcmp(&q, &QUATERNION_IDENTITY, sizeof(q)) == 0);
		m = MATRIX4X4_IDENTITY;
		m.y.y = 0.0f;
		q = rotation(m);
		ENSURE(memcmp(&q, &QUATERNION_IDENTITY, sizeof(q)) == 0);
		m = MATRIX4X4_IDENTITY;
		m.z.z = 0.0f;
		q = rotation(m);
		ENSURE(memcmp(&q, &QUATERNION_IDENTITY, sizeof(q)) == 0);
	}
}

static void test_aabb()
{
	{
		AABB a;
		aabb::reset(a);
		ENSURE(a.min == VECTOR3_ZERO);
		ENSURE(a.max == VECTOR3_ZERO);
	}
	{
		AABB a;
		a.min = vector3(-2.3f, 1.2f, -4.5f);
		a.max = vector3( 3.7f, 5.3f, -2.9f);
		const Vector3 c = aabb::center(a);
		ENSURE(fequal(c.x,  0.70f, 0.00001f));
		ENSURE(fequal(c.y,  3.25f, 0.00001f));
		ENSURE(fequal(c.z, -3.70f, 0.00001f));
	}
	{
		AABB a;
		a.min = vector3(-2.3f, 1.2f, -4.5f);
		a.max = vector3( 3.7f, 5.3f, -2.9f);
		const float c = aabb::volume(a);
		ENSURE(fequal(c, 39.36f, 0.00001f));
	}
	{
		const Vector3 points[] =
		{
			{ -1.2f,  3.4f,  5.5f },
			{  8.2f, -2.4f, -1.5f },
			{ -5.9f,  9.2f,  6.0f }
		};
		AABB a;
		aabb::from_points(a, countof(points), points);
		ENSURE(fequal(a.min.x, -5.9f, 0.00001f));
		ENSURE(fequal(a.min.y, -2.4f, 0.00001f));
		ENSURE(fequal(a.min.z, -1.5f, 0.00001f));
		ENSURE(fequal(a.max.x,  8.2f, 0.00001f));
		ENSURE(fequal(a.max.y,  9.2f, 0.00001f));
		ENSURE(fequal(a.max.z,  6.0f, 0.00001f));
	}
	{
		const Vector3 points[] =
		{
			{ -1.2f,  3.4f,  5.5f },
			{  8.2f, -2.4f, -1.5f },
			{ -5.9f,  9.2f,  6.0f },

			{ -2.8f, -3.5f,  1.9f },
			{ -8.3f, -3.1f,  1.9f },
			{  4.0f, -3.9f, -1.4f },

			{ -0.4f, -1.8f, -2.2f },
			{ -8.6f, -4.8f,  2.8f },
			{  4.1f,  4.7f, -0.4f }
		};
		AABB boxes[3];
		aabb::from_points(boxes[0], countof(points)/3, &points[0]);
		aabb::from_points(boxes[1], countof(points)/3, &points[3]);
		aabb::from_points(boxes[2], countof(points)/3, &points[6]);

		AABB d;
		aabb::from_boxes(d, countof(boxes), boxes);
		ENSURE(fequal(d.min.x, -8.6f, 0.00001f));
		ENSURE(fequal(d.min.y, -4.8f, 0.00001f));
		ENSURE(fequal(d.min.z, -2.2f, 0.00001f));
		ENSURE(fequal(d.max.x,  8.2f, 0.00001f));
		ENSURE(fequal(d.max.y,  9.2f, 0.00001f));
		ENSURE(fequal(d.max.z,  6.0f, 0.00001f));
	}
	{
		AABB a;
		a.min = vector3(-2.3f, 1.2f, -4.5f);
		a.max = vector3( 3.7f, 5.3f, -2.9f);
		ENSURE( aabb::contains_point(a, vector3(1.2f,  3.0f, -4.4f)));
		ENSURE(!aabb::contains_point(a, vector3(3.8f,  3.0f, -4.4f)));
		ENSURE(!aabb::contains_point(a, vector3(1.2f, -1.0f, -4.4f)));
		ENSURE(!aabb::contains_point(a, vector3(1.2f,  3.0f, -4.6f)));
	}
}

static void test_sphere()
{
	{
		Sphere a;
		sphere::reset(a);
		ENSURE(a.c == VECTOR3_ZERO);
		ENSURE(fequal(a.r, 0.0f, 0.00001f));
	}
	{
		Sphere a;
		a.c = VECTOR3_ZERO;
		a.r = 1.61f;
		const float b = sphere::volume(a);
		ENSURE(fequal(b, 17.48099f, 0.00001f));
	}
	{
		Sphere a;
		sphere::reset(a);

		const Vector3 points[] =
		{
			{ -1.2f,  3.4f,  5.5f },
			{  8.2f, -2.4f, -1.5f },
			{ -5.9f,  9.2f,  6.0f }
		};
		sphere::add_points(a, countof(points), points);
		ENSURE(fequal(a.c.x, 0.0f, 0.00001f));
		ENSURE(fequal(a.c.y, 0.0f, 0.00001f));
		ENSURE(fequal(a.c.z, 0.0f, 0.00001f));
		ENSURE(fequal(a.r, 12.46795f, 0.00001f));
	}
	{
		Sphere spheres[3];
		sphere::reset(spheres[0]);
		sphere::reset(spheres[1]);
		sphere::reset(spheres[2]);

		const Vector3 points[] =
		{
			{  6.6f,  3.5f, -5.7f },
			{ -5.3f, -9.1f, -7.9f },
			{ -1.5f,  4.4f, -5.8f },

			{  7.2f, -2.4f, -9.5f },
			{  4.0f, -8.1f,  6.6f },
			{ -8.2f,  2.2f,  4.6f },

			{  2.9f, -4.8f, -6.8f },
			{ -7.6f, -7.0f,  0.8f },
			{  8.2f,  2.8f, -4.8f }
		};
		sphere::add_points(spheres[0], countof(points)/3, &points[0]);
		sphere::add_points(spheres[1], countof(points)/3, &points[3]);
		sphere::add_points(spheres[2], countof(points)/3, &points[6]);

		Sphere d;
		sphere::reset(d);
		sphere::add_spheres(d, countof(spheres), spheres);
		ENSURE(fequal(d.r, 13.16472f, 0.00001f));
	}
	{
		Sphere a;
		a.c = vector3(-2.3f, 1.2f, -4.5f);
		a.r = 1.0f;
		ENSURE( sphere::contains_point(a, vector3(-2.9f, 1.6f, -4.0f)));
		ENSURE(!sphere::contains_point(a, vector3(-3.9f, 1.6f, -4.0f)));
		ENSURE(!sphere::contains_point(a, vector3(-2.9f, 2.6f, -4.0f)));
		ENSURE(!sphere::contains_point(a, vector3(-2.9f, 1.6f, -6.0f)));
	}
}

static void test_murmur()
{
	const u32 m = murmur32("murmur32", 8, 0);
	ENSURE(m == 0x7c2365dbu);
	const u64 n = murmur64("murmur64", 8, 0);
	ENSURE(n == 0x90631502d1a3432bu);
}

static void test_string_id()
{
	memory_globals::init();
	{
		StringId32 a("murmur32");
		ENSURE(a._id == 0x7c2365dbu);

		StringId32 b("murmur32", 8);
		ENSURE(a._id == 0x7c2365dbu);

		char str[9];
		a.to_string(str, sizeof(str));
		ENSURE(strcmp(str, "7c2365db") == 0);
	}
	{
		StringId64 a("murmur64");
		ENSURE(a._id == 0x90631502d1a3432bu);

		StringId64 b("murmur64", 8);
		ENSURE(a._id == 0x90631502d1a3432bu);

		char str[17];
		a.to_string(str, sizeof(str));
		ENSURE(strcmp(str, "90631502d1a3432b") == 0);
	}
	{
		StringId32 id(0x2dd65fa6u);
		char str[9];
		id.to_string(str, sizeof(str));
		StringId32 parsed;
		parsed.parse(str);
		ENSURE(id == parsed);
	}
	{
		StringId64 id(0xa73491922dd65fa6u);
		char str[17];
		id.to_string(str, sizeof(str));
		StringId64 parsed;
		parsed.parse(str);
		ENSURE(id == parsed);
	}
	memory_globals::shutdown();
}

static void test_dynamic_string()
{
	memory_globals::init();
	{
		TempAllocator1024 ta;
		DynamicString str(ta);
		ENSURE(str.empty());

		str.set("murmur32", 8);
		ENSURE(str.length() == 8);

		const StringId32 id = str.to_string_id();
		ENSURE(id._id == 0x7c2365dbu);
	}
	{
		TempAllocator1024 ta;
		DynamicString str(ta);
		str += "Test ";
		str += "string.";
		ENSURE(strcmp(str.c_str(), "Test string.") == 0);
	}
	{
		TempAllocator1024 ta;
		DynamicString str(ta);
		str.set("   \tSushi\t   ", 13);
		str.ltrim();
		ENSURE(strcmp(str.c_str(), "Sushi\t   ") == 0);
	}
	{
		TempAllocator1024 ta;
		DynamicString str(ta);
		str.set("   \tSushi\t   ", 13);
		str.rtrim();
		ENSURE(strcmp(str.c_str(), "   \tSushi") == 0);
	}
	{
		TempAllocator1024 ta;
		DynamicString str(ta);
		str.set("   \tSushi\t   ", 13);
		str.trim();
		ENSURE(strcmp(str.c_str(), "Sushi") == 0);
	}
	{
		TempAllocator1024 ta;
		DynamicString str(ta);
		str.set("Hello everyone!", 15);

		ENSURE( str.has_prefix("Hello"));
		ENSURE(!str.has_prefix("Helloo"));

		ENSURE( str.has_suffix("one!"));
		ENSURE(!str.has_suffix("one"));

		ENSURE(!str.has_prefix("Hello everyone!!!"));
		ENSURE(!str.has_suffix("Hello everyone!!!"));
	}
	{
		Guid guid = guid::parse("dd733419-bbd0-4248-bc84-e0e8363d7165");

		TempAllocator128 ta;
		DynamicString str(ta);
		str.from_guid(guid);
		ENSURE(str.length() == 36);
		ENSURE(strcmp(str.c_str(), "dd733419-bbd0-4248-bc84-e0e8363d7165") == 0);
	}
	{
		StringId32 id = StringId32("test");

		TempAllocator128 ta;
		DynamicString str(ta);
		str.from_string_id(id);
		ENSURE(str.length() == 8);
		ENSURE(strcmp(str.c_str(), "1812752e") == 0);
	}
	{
		StringId64 id = StringId64("test");

		TempAllocator128 ta;
		DynamicString str(ta);
		str.from_string_id(id);
		ENSURE(str.length() == 16);
		ENSURE(strcmp(str.c_str(), "2f4a8724618f4c63") == 0);
	}
	{
		TempAllocator128 ta;
		DynamicString ds1(ta);
		DynamicString ds2(ta);
		ds1.set("foo", 3);
		ds2.set("foo", 3);
		ENSURE(ds1 == ds2);
	}
	{
		TempAllocator128 ta;
		DynamicString ds1(ta);
		DynamicString ds2(ta);
		ds1.set("foo", 3);
		ds2.set("bar", 3);
		ENSURE(ds1 != ds2);
	}
	{
		TempAllocator128 ta;
		DynamicString ds1(ta);
		DynamicString ds2(ta);
		ds1.set("bar", 3);
		ds2.set("foo", 3);
		ENSURE(ds1 < ds2);
	}
	memory_globals::shutdown();
}

static void test_string_view()
{
	memory_globals::init();
	{
		const char* str = "foo";
		StringView sv(str);
		ENSURE(sv._length == 3);
		ENSURE(sv._data == &str[0]);
	}
	{
		StringView sv1("foo");
		StringView sv2("foo");
		ENSURE(sv1 == sv2);
	}
	{
		StringView sv1("foo");
		StringView sv2("bar");
		ENSURE(sv1 != sv2);
	}
	{
		StringView sv1("bar");
		StringView sv2("foo");
		ENSURE(sv1 < sv2);
	}
	{
		StringView sv1("foo");
		StringView sv2("fooo");
		ENSURE(sv1 < sv2);
	}
	memory_globals::shutdown();
}

static void test_guid()
{
	memory_globals::init();
	guid_globals::init();
	{
		Guid guid = guid::new_guid();
		char str[37];
		guid::to_string(str, sizeof(str), guid);
		Guid parsed = guid::parse(str);
		ENSURE(guid == parsed);
	}
	{
		Guid guid;
		ENSURE(guid::try_parse(guid, "961f8005-6a7e-4371-9272-8454dd786884"));
		ENSURE(!guid::try_parse(guid, "961f80056a7e-4371-9272-8454dd786884"));
	}
	{
		Guid guid1 = guid::parse("8ec79062-c8fd-41b5-b044-cb545afc9976");
		Guid guid2 = guid::parse("8f879a4e-e9dd-4981-8b9e-344bb917d7dc");
		ENSURE(guid1 < guid2);
	}
	guid_globals::shutdown();
	memory_globals::shutdown();
}

static void test_json()
{
	memory_globals::init();
	{
		JsonValueType::Enum t = json::type("null");
		ENSURE(t == JsonValueType::NIL);
	}
	{
		JsonValueType::Enum t = json::type("true");
		ENSURE(t == JsonValueType::BOOL);
	}
	{
		JsonValueType::Enum t = json::type("false");
		ENSURE(t == JsonValueType::BOOL);
	}
	{
		JsonValueType::Enum t = json::type("3.14");
		ENSURE(t == JsonValueType::NUMBER);
	}
	{
		JsonValueType::Enum t = json::type("\"foo\"");
		ENSURE(t == JsonValueType::STRING);
	}
	{
		JsonValueType::Enum t = json::type("[]");
		ENSURE(t == JsonValueType::ARRAY);
	}
	{
		JsonValueType::Enum t = json::type("{}");
		ENSURE(t == JsonValueType::OBJECT);
	}
	{
		const s32 a = json::parse_int("3.14");
		ENSURE(a == 3);
	}
	{
		const f32 a = json::parse_float("3.14");
		ENSURE(fequal(a, 3.14f));
	}
	{
		const bool a = json::parse_bool("true");
		ENSURE(a == true);
	}
	{
		const bool a = json::parse_bool("false");
		ENSURE(a == false);
	}
	{
		TempAllocator1024 ta;
		DynamicString str(ta);
		json::parse_string(str, "\"This is JSON\"");
		ENSURE(strcmp(str.c_str(), "This is JSON") == 0);
	}
	memory_globals::shutdown();
}

static void test_sjson()
{
	memory_globals::init();
	{
		JsonValueType::Enum t = sjson::type("null");
		ENSURE(t == JsonValueType::NIL);
	}
	{
		JsonValueType::Enum t = sjson::type("true");
		ENSURE(t == JsonValueType::BOOL);
	}
	{
		JsonValueType::Enum t = sjson::type("false");
		ENSURE(t == JsonValueType::BOOL);
	}
	{
		JsonValueType::Enum t = sjson::type("3.14");
		ENSURE(t == JsonValueType::NUMBER);
	}
	{
		JsonValueType::Enum t = sjson::type("\"foo\"");
		ENSURE(t == JsonValueType::STRING);
	}
	{
		JsonValueType::Enum t = sjson::type("[]");
		ENSURE(t == JsonValueType::ARRAY);
	}
	{
		JsonValueType::Enum t = sjson::type("{}");
		ENSURE(t == JsonValueType::OBJECT);
	}
	{
		const s32 a = sjson::parse_int("3.14");
		ENSURE(a == 3);
	}
	{
		const f32 a = sjson::parse_float("3.14");
		ENSURE(fequal(a, 3.14f));
	}
	{
		const bool a = sjson::parse_bool("true");
		ENSURE(a == true);
	}
	{
		const bool a = sjson::parse_bool("false");
		ENSURE(a == false);
	}
	{
		TempAllocator1024 ta;
		DynamicString str(ta);
		sjson::parse_string(str, "\"This is JSON\"");
		ENSURE(strcmp(str.c_str(), "This is JSON") == 0);
	}
	{
		const Vector2 a = sjson::parse_vector2("[ 1.2 -2.5 ]");
		ENSURE(fequal(a.x,  1.2f));
		ENSURE(fequal(a.y, -2.5f));
	}
	{
		const Vector3 a = sjson::parse_vector3("[ 3.1 0.5 -5.7]");
		ENSURE(fequal(a.x,  3.1f));
		ENSURE(fequal(a.y,  0.5f));
		ENSURE(fequal(a.z, -5.7f));
	}
	{
		const Vector4 a = sjson::parse_vector4("[ 6.7 -1.3 2.9 -0.4 ]");
		ENSURE(fequal(a.x,  6.7f));
		ENSURE(fequal(a.y, -1.3f));
		ENSURE(fequal(a.z,  2.9f));
		ENSURE(fequal(a.w, -0.4f));
	}
	{
		const Quaternion a = sjson::parse_quaternion("[ -1.5 -3.4 9.1 -3.5 ]");
		ENSURE(fequal(a.x, -1.5f));
		ENSURE(fequal(a.y, -3.4f));
		ENSURE(fequal(a.z,  9.1f));
		ENSURE(fequal(a.w, -3.5f));
	}
	{
		const Matrix4x4 a = sjson::parse_matrix4x4(
			"["
			"-3.2  5.3 -0.7  4.1 "
			" 5.6  7.0 -3.2 -1.2 "
			"-6.3  9.0  3.9  1.1 "
			" 0.4 -7.3  8.9 -0.1 "
			"]"
			);
		ENSURE(fequal(a.x.x, -3.2f));
		ENSURE(fequal(a.x.y,  5.3f));
		ENSURE(fequal(a.x.z, -0.7f));
		ENSURE(fequal(a.x.w,  4.1f));
		ENSURE(fequal(a.y.x,  5.6f));
		ENSURE(fequal(a.y.y,  7.0f));
		ENSURE(fequal(a.y.z, -3.2f));
		ENSURE(fequal(a.y.w, -1.2f));
		ENSURE(fequal(a.z.x, -6.3f));
		ENSURE(fequal(a.z.y,  9.0f));
		ENSURE(fequal(a.z.z,  3.9f));
		ENSURE(fequal(a.z.w,  1.1f));
		ENSURE(fequal(a.t.x,  0.4f));
		ENSURE(fequal(a.t.y, -7.3f));
		ENSURE(fequal(a.t.z,  8.9f));
		ENSURE(fequal(a.t.w, -0.1f));
	}
	{
		const StringId32 a = sjson::parse_string_id("\"murmur32\"");
		ENSURE(a._id == 0x7c2365dbu);
	}
	{
		const StringId64 a = sjson::parse_resource_name("\"murmur64\"");
		ENSURE(a._id == 0x90631502d1a3432bu);
	}
	{
		const Guid guid = guid::parse("0f6c3b1c-9cba-4282-9096-2a77ca047b1b");
		const Guid parsed = sjson::parse_guid("\"0f6c3b1c-9cba-4282-9096-2a77ca047b1b\"");
		ENSURE(guid == parsed);
	}
	{
		TempAllocator128 ta;
		DynamicString str(ta);
		sjson::parse_verbatim(str, "\"\"\"verbatim\"\"\"");
		ENSURE(strcmp(str.c_str(), "verbatim") == 0);
	}
	memory_globals::shutdown();
}

static void test_path()
{
#if CROWN_PLATFORM_POSIX
	{
		const bool a = path::is_absolute("/home/foo");
		ENSURE(a == true);
		const bool b = path::is_absolute("home/foo");
		ENSURE(b == false);
	}
	{
		const bool a = path::is_relative("/home/foo");
		ENSURE(a == false);
		const bool b = path::is_relative("home/foo");
		ENSURE(b == true);
	}
	{
		const bool a = path::is_root("/");
		ENSURE(a == true);
		const bool b = path::is_root("/home");
		ENSURE(b == false);
	}
	{
		ENSURE(path::has_trailing_separator("/home/foo/"));
		ENSURE(!path::has_trailing_separator("/home/foo"));
	}
	{
		TempAllocator128 ta;
		DynamicString clean(ta);
		path::reduce(clean, "/home//foo/");
		ENSURE(clean == "/home/foo");
	}
	{
		TempAllocator128 ta;
		DynamicString clean(ta);
		path::reduce(clean, "\\home\\\\foo\\");
		ENSURE(clean == "/home/foo");
	}
#else
	{
		const bool a = path::is_absolute("C:\\Users\\foo");
		ENSURE(a == true);
		const bool b = path::is_absolute("Users\\foo");
		ENSURE(b == false);
	}
	{
		const bool a = path::is_relative("D:\\Users\\foo");
		ENSURE(a == false);
		const bool b = path::is_relative("Users\\foo");
		ENSURE(b == true);
	}
	{
		const bool a = path::is_root("E:\\");
		ENSURE(a == true);
		const bool b = path::is_root("E:\\Users");
		ENSURE(b == false);
	}
	{
		ENSURE(path::has_trailing_separator("C:\\Users\\foo\\"));
		ENSURE(!path::has_trailing_separator("C:\\Users\\foo"));
	}
	{
		TempAllocator128 ta;
		DynamicString clean(ta);
		path::reduce(clean, "C:\\Users\\\\foo\\");
		ENSURE(clean == "C:\\Users\\foo");
	}
	{
		TempAllocator128 ta;
		DynamicString clean(ta);
		path::reduce(clean, "C:/Users//foo/");
		ENSURE(clean == "C:\\Users\\foo");
	}
#endif // CROWN_PLATFORM_POSIX
	{
		const char* p = path::basename("");
		ENSURE(strcmp(p, "") == 0);
	}
	{
		const char* q = path::basename("/");
		ENSURE(strcmp(q, "") == 0);
	}
	{
		const char* p = path::basename("boot.config");
		ENSURE(strcmp(p, "boot.config") == 0);
	}
	{
		const char* p = path::basename("foo/boot.config");
		ENSURE(strcmp(p, "boot.config") == 0);
	}
	{
		const char* p = path::basename("/foo/boot.config");
		ENSURE(strcmp(p, "boot.config") == 0);
	}
	{
		const char* p = path::extension("");
		ENSURE(p == NULL);
	}
	{
		const char* p = path::extension("boot");
		ENSURE(p == NULL);
	}
	{
		const char* p = path::extension("boot.bar.config");
		ENSURE(strcmp(p, "config") == 0);
	}
	{
		const char* p = path::extension(".bar");
		ENSURE(p == NULL);
	}
	{
		const char* p = path::extension("foo/.bar");
		ENSURE(p == NULL);
	}
	{
		TempAllocator128 ta;
		DynamicString path(ta);
		path::join(path, "", "");
		ENSURE(path == "");
	}
	{
		TempAllocator128 ta;
		DynamicString path(ta);
		path::join(path, "foo", "");
		ENSURE(path == "foo");
	}
	{
		TempAllocator128 ta;
		DynamicString path(ta);
		path::join(path, "", "bar");
		ENSURE(path == "bar");
	}
#if CROWN_PLATFORM_POSIX
	{
		TempAllocator128 ta;
		DynamicString path(ta);
		path::join(path, "foo", "bar");
		ENSURE(path == "foo/bar");
	}
#else
	{
		TempAllocator128 ta;
		DynamicString path(ta);
		path::join(path, "C:\\foo", "bar");
		ENSURE(path == "C:\\foo\\bar");
	}
#endif // CROWN_PLATFORM_POSIX
}

static void test_command_line()
{
	const char* argv[] =
	{
		"args",
		"-s",
		"--switch",
		"--argument",
		"orange"
	};

	CommandLine cl(countof(argv), argv);
	ENSURE(cl.has_option("switch", 's'));
	const char* orange = cl.get_parameter(0, "argument");
	ENSURE(orange != NULL && strcmp(orange, "orange") == 0);
}

static void test_thread()
{
	Thread thread;
	ENSURE(!thread.is_running());

	thread.start([](void*) { return 0xbadc0d3; }, NULL);
	thread.stop();
	ENSURE(thread.exit_code() == 0xbadc0d3);
}

static void test_process()
{
#if CROWN_PLATFORM_LINUX
	{
#if CROWN_DEVELOPMENT
		#define CROWN_SUFFIX "development"
#elif CROWN_DEBUG
		#define CROWN_SUFFIX "debug"
#else
		#define CROWN_SUFFIX "release"
#endif
		const char* argv[] =
		{
			EXE_PATH("crown-" CROWN_SUFFIX)
			, "--version"
			, NULL
		};

		Process pr;
		s32 err = pr.spawn(argv, ProcessFlags::STDOUT_PIPE);
		ENSURE(err == 0);
		u32 nbr;
		char buf[128] = {0};
		pr.read(&nbr, buf, sizeof(buf));
		const char* ver = "Crown " CROWN_VERSION;
		ENSURE(strncmp(buf, ver, strlen32(ver)) == 0);
		s32 ec = pr.wait();
		ENSURE(ec == 0);
	}
#endif // CROWN_PLATFORM_LINUX
}

static void test_filesystem()
{
#if CROWN_PLATFORM_POSIX
	guid_globals::init();
	{
		Guid id = guid::new_guid();
		char dir[5 + GUID_BUF_LEN] = "/tmp/";
		guid::to_string(dir + 5, sizeof(dir) - 5, id);

		DeleteResult dr = os::delete_directory(dir);
		ENSURE(dr.error == DeleteResult::NO_ENTRY);
	}
	{
		Guid id = guid::new_guid();
		char dir[5 + GUID_BUF_LEN] = "/tmp/";
		guid::to_string(dir + 5, sizeof(dir) - 5, id);

		os::delete_directory(dir);
		CreateResult cr = os::create_directory(dir);
		ENSURE(cr.error == CreateResult::SUCCESS);
		             cr = os::create_directory(dir);
		ENSURE(cr.error == CreateResult::ALREADY_EXISTS);
		DeleteResult dr = os::delete_directory(dir);
		ENSURE(dr.error == DeleteResult::SUCCESS);
	}
	guid_globals::shutdown();
#endif // CROWN_PLATFORM_POSIX
}

#define RUN_TEST(name)      \
	do {                    \
		printf(#name "\n"); \
		name();             \
	} while (0)

int main_unit_tests()
{
	RUN_TEST(test_memory);
	RUN_TEST(test_array);
	RUN_TEST(test_vector);
	RUN_TEST(test_hash_map);
	RUN_TEST(test_hash_set);
	RUN_TEST(test_vector2);
	RUN_TEST(test_vector3);
	RUN_TEST(test_vector4);
	RUN_TEST(test_quaternion);
	RUN_TEST(test_color4);
	RUN_TEST(test_matrix3x3);
	RUN_TEST(test_matrix4x4);
	RUN_TEST(test_aabb);
	RUN_TEST(test_sphere);
	RUN_TEST(test_murmur);
	RUN_TEST(test_string_id);
	RUN_TEST(test_dynamic_string);
	RUN_TEST(test_string_view);
	RUN_TEST(test_guid);
	RUN_TEST(test_json);
	RUN_TEST(test_sjson);
	RUN_TEST(test_path);
	RUN_TEST(test_command_line);
	RUN_TEST(test_thread);
	RUN_TEST(test_process);
	RUN_TEST(test_filesystem);

	return EXIT_SUCCESS;
}

} // namespace crown

#endif // CROWN_BUILD_UNIT_TESTS
