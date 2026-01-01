/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#if CROWN_BUILD_UNIT_TESTS
#include "core/command_line.h"
#include "core/containers/array.inl"
#include "core/containers/hash_map.inl"
#include "core/containers/hash_set.inl"
#include "core/containers/vector.inl"
#include "core/filesystem/file.h"
#include "core/filesystem/file_monitor.h"
#include "core/filesystem/filesystem_disk.h"
#include "core/filesystem/path.h"
#include "core/guid.inl"
#include "core/json/json.h"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/math/aabb.inl"
#include "core/math/color4.inl"
#include "core/math/constants.h"
#include "core/math/intersection.h"
#include "core/math/math.h"
#include "core/math/matrix3x3.inl"
#include "core/math/matrix4x4.inl"
#include "core/math/obb.inl"
#include "core/math/quaternion.inl"
#include "core/math/sphere.inl"
#include "core/math/vector2.inl"
#include "core/math/vector3.inl"
#include "core/math/vector4.inl"
#include "core/memory/memory.inl"
#include "core/memory/pool_allocator.h"
#include "core/memory/temp_allocator.inl"
#include "core/murmur.h"
#include "core/option.inl"
#include "core/os.h"
#include "core/process.h"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string.inl"
#include "core/strings/string_id.inl"
#include "core/strings/string_view.inl"
#include "core/thread/condition_variable.h"
#include "core/thread/mutex.h"
#include "core/thread/thread.h"
#include "core/time.h"
#include "resource/expression_language.h"
#include "resource/lua_resource.h"
#include "world/types.h"
#include <stdlib.h> // EXIT_SUCCESS, EXIT_FAILURE
#include <stdio.h>  // printf

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
	Allocator &a = default_allocator();
	{
		void *p = a.allocate(32);
		ENSURE(a.allocated_size(p) >= 32);
		a.deallocate(p);
	}
	{
		char A[100];
		char B[countof(A)];
		memset(A, 'A', countof(A));
		memset(B, 'B', countof(B));

		for (int align = 1; align <= 16; align *= 2) {
			for (size_t bs = sizeof(void *); bs < countof(A); ++bs) {
				PoolAllocator pool(a, 2, bs, align);
				char *a = (char *)pool.allocate(bs, align);
				char *b = (char *)pool.allocate(bs, align);
				memset(a, 'A', bs);
				memset(b, 'B', bs);

				ENSURE(memcmp(a, A, bs) == 0);
				ENSURE(memcmp(b, B, bs) == 0);
				memset(a, 0, bs);
				memset(b, 0, bs);

				pool.deallocate(a);
				pool.deallocate(b);
			}
		}
	}
	memory_globals::shutdown();
}

static void test_array()
{
	memory_globals::init();
	Allocator &a = default_allocator();
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
	Allocator &a = default_allocator();
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
	Allocator &a = default_allocator();
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

		for (s32 i = 0; i < 12; ++i) {
			s32 key = 12 + i;
			hash_map::set(m, key, 99);
			hash_map::set(m, i, i);
			hash_map::remove(m, key);
		}

		ENSURE(hash_map::size(m) == 12);

		for (s32 i = 0; i < 12; ++i)
			ENSURE(hash_map::get(m, i, 0) == i);
	}
	{
		HashMap<s32, s32> m(a);
		hash_map_internal::grow(m);
		ENSURE(hash_map::capacity(m) == 16);

		hash_map::set(m, 0, 7);

		hash_map::set(m, 1, 1);

		for (s32 i = 2; i < 150; ++i) {
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
	{
		Vector<int> v(default_allocator());
		vector::push_back(v, 0);
		HashMap<int, Vector<int>> a(default_allocator());
		HashMap<int, Vector<int>> b(default_allocator());
		hash_map::set(a, 0, v);
		hash_map::set(b, 0, v);
		a = b;
	}
	memory_globals::shutdown();
}

static void test_hash_set()
{
	memory_globals::init();
	Allocator &a = default_allocator();
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
		HashSet<s32> m(a);

		for (s32 i = 0; i < 12; ++i) {
			s32 key = 12 + i;
			hash_set::insert(m, key);
			hash_set::insert(m, i);
			hash_set::remove(m, key);
		}

		ENSURE(hash_set::size(m) == 12);

		for (s32 i = 0; i < 12; ++i)
			ENSURE(hash_set::has(m, i));
	}
	{
		HashSet<s32> ma(a);
		HashSet<s32> mb(a);
		hash_set::insert(ma, 0);
		ma = mb;
	}
	{
		DynamicString str(default_allocator());
		str = "test";
		HashSet<DynamicString> a(default_allocator());
		HashSet<DynamicString> b(default_allocator());
		hash_set::insert(a, str);
		hash_set::insert(b, str);
		a = b;
	}
	memory_globals::shutdown();
}

static void test_vector2()
{
	{
		const Vector2 a = { 1.2f,  4.2f };
		const Vector2 b = { 2.7f, -1.9f };
		const Vector2 c = a - b;
		ENSURE(fequal(c.x, -1.5f, 0.0001f));
		ENSURE(fequal(c.y,  6.1f, 0.0001f));
	}
	{
		const Vector2 a = { 1.2f,  4.2f };
		const Vector2 b = { 2.7f, -1.9f };
		const Vector2 c = a + b;
		ENSURE(fequal(c.x,  3.9f, 0.0001f));
		ENSURE(fequal(c.y,  2.3f, 0.0001f));
	}
	{
		const Vector2 a = { 1.2f,  4.2f };
		const Vector2 b = a * 2.0f;
		ENSURE(fequal(b.x,  2.4f, 0.0001f));
		ENSURE(fequal(b.y,  8.4f, 0.0001f));
	}
	{
		const Vector2 a = { 1.2f,  4.2f };
		const Vector2 b = { 2.7f, -1.9f };
		const f32 c = dot(a, b);
		ENSURE(fequal(c,  -4.74f, 0.0001f));
	}
	{
		const Vector2 a = { 1.2f,  4.2f };
		const f32 c = length_squared(a);
		ENSURE(fequal(c, 19.08f, 0.0001f));
	}
	{
		const Vector2 a = { 1.2f,  4.2f };
		const f32 c = length(a);
		ENSURE(fequal(c,  4.36806f, 0.0001f));
	}
	{
		Vector2 a = { 1.2f,  4.2f };
		normalize(a);
		ENSURE(fequal(length(a), 1.0f, 0.00001f));
	}
	{
		const Vector2 a = { 1.2f,  4.2f };
		const Vector2 b = { 2.7f, -1.9f };
		const float c = distance_squared(a, b);
		ENSURE(fequal(c, 39.46f, 0.00001f));
	}
	{
		const Vector2 a = { 1.2f,  4.2f };
		const Vector2 b = { 2.7f, -1.9f };
		const float c = distance(a, b);
		ENSURE(fequal(c, 6.28171f, 0.00001f));
	}
	{
		const Vector2 a = { 1.2f,  4.2f };
		const Vector2 b = { 2.7f, -1.9f };
		const Vector2 c = max(a, b);
		ENSURE(fequal(c.x,  2.7f, 0.00001f));
		ENSURE(fequal(c.y,  4.2f, 0.00001f));
	}
	{
		const Vector2 a = { 1.2f,  4.2f };
		const Vector2 b = { 2.7f, -1.9f };
		const Vector2 c = min(a, b);
		ENSURE(fequal(c.x,  1.2f, 0.00001f));
		ENSURE(fequal(c.y, -1.9f, 0.00001f));
	}
}

static void test_vector3()
{
	{
		const Vector3 a = { 1.2f,  4.2f, -2.3f };
		const Vector3 b = { 2.7f, -1.9f, -4.1f };
		const Vector3 c = a - b;
		ENSURE(fequal(c.x, -1.5f, 0.0001f));
		ENSURE(fequal(c.y,  6.1f, 0.0001f));
		ENSURE(fequal(c.z,  1.8f, 0.0001f));
	}
	{
		const Vector3 a = { 1.2f,  4.2f, -2.3f };
		const Vector3 b = { 2.7f, -1.9f, -4.1f };
		const Vector3 c = a + b;
		ENSURE(fequal(c.x,  3.9f, 0.0001f));
		ENSURE(fequal(c.y,  2.3f, 0.0001f));
		ENSURE(fequal(c.z, -6.4f, 0.0001f));
	}
	{
		const Vector3 a = { 1.2f,  4.2f, -2.3f };
		const Vector3 b = a * 2.0f;
		ENSURE(fequal(b.x,  2.4f, 0.0001f));
		ENSURE(fequal(b.y,  8.4f, 0.0001f));
		ENSURE(fequal(b.z, -4.6f, 0.0001f));
	}
	{
		const Vector3 a = { 1.2f,  4.2f, -2.3f };
		const Vector3 b = { 2.7f, -1.9f, -4.1f };
		const f32 c = dot(a, b);
		ENSURE(fequal(c,  4.69f, 0.0001f));
	}
	{
		const Vector3 z_axis = cross(VECTOR3_XAXIS, VECTOR3_YAXIS);
		ENSURE(fequal(z_axis.x, VECTOR3_ZAXIS.x, 0.0001f));
		ENSURE(fequal(z_axis.y, VECTOR3_ZAXIS.y, 0.0001f));
		ENSURE(fequal(z_axis.z, VECTOR3_ZAXIS.z, 0.0001f));

		const Vector3 x_axis = cross(VECTOR3_YAXIS, VECTOR3_ZAXIS);
		ENSURE(fequal(x_axis.x, VECTOR3_XAXIS.x, 0.0001f));
		ENSURE(fequal(x_axis.y, VECTOR3_XAXIS.y, 0.0001f));
		ENSURE(fequal(x_axis.z, VECTOR3_XAXIS.z, 0.0001f));

		const Vector3 y_axis = cross(VECTOR3_ZAXIS, VECTOR3_XAXIS);
		ENSURE(fequal(y_axis.x, VECTOR3_YAXIS.x, 0.0001f));
		ENSURE(fequal(y_axis.y, VECTOR3_YAXIS.y, 0.0001f));
		ENSURE(fequal(y_axis.z, VECTOR3_YAXIS.z, 0.0001f));
	}
	{
		const Vector3 a = { 1.2f,  4.2f, -2.3f };
		const f32 c = length_squared(a);
		ENSURE(fequal(c, 24.37f, 0.0001f));
	}
	{
		const Vector3 a = { 1.2f,  4.2f, -2.3f };
		const f32 c = length(a);
		ENSURE(fequal(c,  4.93659f, 0.0001f));
	}
	{
		Vector3 a = { 1.2f,  4.2f, -2.3f };
		normalize(a);
		ENSURE(fequal(length(a), 1.0f, 0.00001f));
	}
	{
		const Vector3 a = { 1.2f,  4.2f, -2.3f };
		const Vector3 b = { 2.7f, -1.9f, -4.1f };
		const float c = distance_squared(a, b);
		ENSURE(fequal(c, 42.70f, 0.00001f));
	}
	{
		const Vector3 a = { 1.2f,  4.2f, -2.3f };
		const Vector3 b = { 2.7f, -1.9f, -4.1f };
		const float c = distance(a, b);
		ENSURE(fequal(c, 6.53452f, 0.00001f));
	}
	{
		const Vector3 a = { 1.2f,  4.2f, -2.3f };
		const Vector3 b = { 2.7f, -1.9f, -4.1f };
		const Vector3 c = max(a, b);
		ENSURE(fequal(c.x,  2.7f, 0.00001f));
		ENSURE(fequal(c.y,  4.2f, 0.00001f));
		ENSURE(fequal(c.z, -2.3f, 0.00001f));
	}
	{
		const Vector3 a = { 1.2f,  4.2f, -2.3f };
		const Vector3 b = { 2.7f, -1.9f, -4.1f };
		const Vector3 c = min(a, b);
		ENSURE(fequal(c.x,  1.2f, 0.00001f));
		ENSURE(fequal(c.y, -1.9f, 0.00001f));
		ENSURE(fequal(c.z, -4.1f, 0.00001f));
	}
}

static void test_vector4()
{
	{
		const Vector4 a = { 1.2f,  4.2f, -2.3f, 5.5f };
		const Vector4 b = { 2.7f, -1.9f, -4.1f, 1.0f };
		const Vector4 c = a - b;
		ENSURE(fequal(c.x, -1.5f, 0.0001f));
		ENSURE(fequal(c.y,  6.1f, 0.0001f));
		ENSURE(fequal(c.z,  1.8f, 0.0001f));
		ENSURE(fequal(c.w,  4.5f, 0.0001f));
	}
	{
		const Vector4 a = { 1.2f,  4.2f, -2.3f, 5.5f };
		const Vector4 b = { 2.7f, -1.9f, -4.1f, 1.0f };
		const Vector4 c = a + b;
		ENSURE(fequal(c.x,  3.9f, 0.0001f));
		ENSURE(fequal(c.y,  2.3f, 0.0001f));
		ENSURE(fequal(c.z, -6.4f, 0.0001f));
		ENSURE(fequal(c.w,  6.5f, 0.0001f));
	}
	{
		const Vector4 a = { 1.2f,  4.2f, -2.3f, 1.5f };
		const Vector4 b = a * 2.0f;
		ENSURE(fequal(b.x,  2.4f, 0.0001f));
		ENSURE(fequal(b.y,  8.4f, 0.0001f));
		ENSURE(fequal(b.z, -4.6f, 0.0001f));
		ENSURE(fequal(b.w,  3.0f, 0.0001f));
	}
	{
		const Vector4 a = { 1.2f,  4.2f, -2.3f, 5.5f };
		const Vector4 b = { 2.7f, -1.9f, -4.1f, 1.0f };
		const f32 c = dot(a, b);
		ENSURE(fequal(c,  10.19f, 0.0001f));
	}
	{
		const Vector4 a = { 1.2f,  4.2f, -2.3f, 5.5f };
		const f32 c = length_squared(a);
		ENSURE(fequal(c, 54.62f, 0.0001f));
	}
	{
		const Vector4 a = { 1.2f,  4.2f, -2.3f, 5.5f };
		const f32 c = length(a);
		ENSURE(fequal(c,  7.39053f, 0.0001f));
	}
	{
		Vector4 a = { 1.2f,  4.2f, -2.3f, 5.5f };
		normalize(a);
		ENSURE(fequal(length(a), 1.0f, 0.00001f));
	}
	{
		const Vector4 a = { 1.2f,  4.2f, -2.3f, 5.5f };
		const Vector4 b = { 2.7f, -1.9f, -4.1f, 1.0f };
		const float c = distance_squared(a, b);
		ENSURE(fequal(c, 62.95f, 0.00001f));
	}
	{
		const Vector4 a = { 1.2f,  4.2f, -2.3f, 5.5f };
		const Vector4 b = { 2.7f, -1.9f, -4.1f, 1.0f };
		const float c = distance(a, b);
		ENSURE(fequal(c, 7.93410f, 0.00001f));
	}
	{
		const Vector4 a = { 1.2f,  4.2f, -2.3f, 5.5f };
		const Vector4 b = { 2.7f, -1.9f, -4.1f, 1.0f };
		const Vector4 c = max(a, b);
		ENSURE(fequal(c.x,  2.7f, 0.00001f));
		ENSURE(fequal(c.y,  4.2f, 0.00001f));
		ENSURE(fequal(c.z, -2.3f, 0.00001f));
		ENSURE(fequal(c.w,  5.5f, 0.00001f));
	}
	{
		const Vector4 a = { 1.2f,  4.2f, -2.3f, 5.5f };
		const Vector4 b = { 2.7f, -1.9f, -4.1f, 1.0f };
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
		const Quaternion a = { 0.0f, 0.0f, 0.0f, 1.0f };
		ENSURE(fequal(a.x, 0.0f, 0.00001f));
		ENSURE(fequal(a.y, 0.0f, 0.00001f));
		ENSURE(fequal(a.z, 0.0f, 0.00001f));
		ENSURE(fequal(a.w, 1.0f, 0.00001f));
	}
}

static void test_color4()
{
	{
		const Color4 a = { 1.3f, 2.6f, 0.2f, 0.6f };
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
		ENSURE(fequal(a.w, 1.0f, 0.00001f));
	}
	{
		const Color4 a = from_rgba(0x3fe70c62);
		ENSURE(fequal(a.x, 0.24705f, 0.00001f));
		ENSURE(fequal(a.y, 0.90588f, 0.00001f));
		ENSURE(fequal(a.z, 0.04705f, 0.00001f));
		ENSURE(fequal(a.w, 0.38431f, 0.00001f));
	}
	{
		const Color4 a = from_rgb(0x3fe70c);
		ENSURE(fequal(a.x, 0.24705f, 0.00001f));
		ENSURE(fequal(a.y, 0.90588f, 0.00001f));
		ENSURE(fequal(a.z, 0.04705f, 0.00001f));
		ENSURE(fequal(a.w, 1.0f, 0.00001f));
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
		const Matrix3x3 a = {
			1.2f, -2.3f, 5.1f,
			2.2f, -5.1f,  1.1f,
			3.2f,  3.3f, -3.8f
		};
		const Matrix3x3 b = {
			3.2f, 4.8f, 6.0f,
			-1.6f, -7.1f, -2.4f,
			-3.1f, -2.2f,  8.9f
		};
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
		const Matrix3x3 a = {
			1.2f, -2.3f, 5.1f,
			2.2f, -5.1f,  1.1f,
			3.2f,  3.3f, -3.8f
		};
		const Matrix3x3 b = {
			3.2f, 4.8f, 6.0f,
			-1.6f, -7.1f, -2.4f,
			-3.1f, -2.2f,  8.9f
		};
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
		const Matrix3x3 a = {
			1.2f, -2.3f, 5.1f,
			2.2f, -5.1f,  1.1f,
			3.2f,  3.3f, -3.8f
		};
		const Matrix3x3 b = {
			3.2f, 4.8f, 6.0f,
			-1.6f, -7.1f, -2.4f,
			-3.1f, -2.2f,  8.9f
		};
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
		const Matrix3x3 a = {
			1.2f, -2.3f, 5.1f,
			2.2f, -5.1f,  1.1f,
			3.2f,  3.3f, -3.8f
		};
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
		const Matrix3x3 a = {
			1.2f, -2.3f, 5.1f,
			2.2f, -5.1f,  1.1f,
			3.2f,  3.3f, -3.8f
		};
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
	{
		const f32 angle = frad(10.0f);
		Matrix3x3 a;
		Matrix3x3 b;

		a = from_quaternion(from_axis_angle({ 1.0f, 0.0f, 0.0f }, angle));
		b = from_x_axis_angle(angle);
		ENSURE(fequal(a.x.x, b.x.x, 0.00001f));
		ENSURE(fequal(a.x.y, b.x.y, 0.00001f));
		ENSURE(fequal(a.x.z, b.x.z, 0.00001f));
		ENSURE(fequal(a.y.x, b.y.x, 0.00001f));
		ENSURE(fequal(a.y.y, b.y.y, 0.00001f));
		ENSURE(fequal(a.y.z, b.y.z, 0.00001f));
		ENSURE(fequal(a.z.x, b.z.x, 0.00001f));
		ENSURE(fequal(a.z.y, b.z.y, 0.00001f));
		ENSURE(fequal(a.z.z, b.z.z, 0.00001f));

		a = from_quaternion(from_axis_angle({ 0.0f, 1.0f, 0.0f }, angle));
		b = from_y_axis_angle(angle);
		ENSURE(fequal(a.x.x, b.x.x, 0.00001f));
		ENSURE(fequal(a.x.y, b.x.y, 0.00001f));
		ENSURE(fequal(a.x.z, b.x.z, 0.00001f));
		ENSURE(fequal(a.y.x, b.y.x, 0.00001f));
		ENSURE(fequal(a.y.y, b.y.y, 0.00001f));
		ENSURE(fequal(a.y.z, b.y.z, 0.00001f));
		ENSURE(fequal(a.z.x, b.z.x, 0.00001f));
		ENSURE(fequal(a.z.y, b.z.y, 0.00001f));
		ENSURE(fequal(a.z.z, b.z.z, 0.00001f));

		a = from_quaternion(from_axis_angle({ 0.0f, 0.0f, 1.0f }, angle));
		b = from_z_axis_angle(angle);
		ENSURE(fequal(a.x.x, b.x.x, 0.00001f));
		ENSURE(fequal(a.x.y, b.x.y, 0.00001f));
		ENSURE(fequal(a.x.z, b.x.z, 0.00001f));
		ENSURE(fequal(a.y.x, b.y.x, 0.00001f));
		ENSURE(fequal(a.y.y, b.y.y, 0.00001f));
		ENSURE(fequal(a.y.z, b.y.z, 0.00001f));
		ENSURE(fequal(a.z.x, b.z.x, 0.00001f));
		ENSURE(fequal(a.z.y, b.z.y, 0.00001f));
		ENSURE(fequal(a.z.z, b.z.z, 0.00001f));
	}
}

static void test_matrix4x4()
{
	{
		const Matrix4x4 a = {
			1.2f, -2.3f, 5.1f, -1.2f,
			2.2f, -5.1f,  1.1f, -7.4f,
			3.2f,  3.3f, -3.8f, -9.2f,
			-6.8f, -2.9f,  1.0f,  4.9f
		};
		const Matrix4x4 b = {
			3.2f, 4.8f, 6.0f, 5.3f,
			-1.6f, -7.1f, -2.4f, -6.2f,
			-3.1f, -2.2f,  8.9f,  8.3f,
			3.8f,  9.1f, -3.1f, -7.1f
		};
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
		const Matrix4x4 a = {
			1.2f, -2.3f, 5.1f, -1.2f,
			2.2f, -5.1f,  1.1f, -7.4f,
			3.2f,  3.3f, -3.8f, -9.2f,
			-6.8f, -2.9f,  1.0f,  4.9f
		};
		const Matrix4x4 b = {
			3.2f, 4.8f, 6.0f, 5.3f,
			-1.6f, -7.1f, -2.4f, -6.2f,
			-3.1f, -2.2f,  8.9f,  8.3f,
			3.8f,  9.1f, -3.1f, -7.1f
		};
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
		const Matrix4x4 a = {
			1.2f, -2.3f, 5.1f, -1.2f,
			2.2f, -5.1f,  1.1f, -7.4f,
			3.2f,  3.3f, -3.8f, -9.2f,
			-6.8f, -2.9f,  1.0f,  4.9f
		};
		const Matrix4x4 b = {
			3.2f, 4.8f, 6.0f, 5.3f,
			-1.6f, -7.1f, -2.4f, -6.2f,
			-3.1f, -2.2f,  8.9f,  8.3f,
			3.8f,  9.1f, -3.1f, -7.1f
		};
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
		const Matrix4x4 a = {
			1.2f, -2.3f, 5.1f, -1.2f,
			2.2f, -5.1f,  1.1f, -7.4f,
			3.2f,  3.3f, -3.8f, -9.2f,
			-6.8f, -2.9f,  1.0f,  4.9f
		};
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
		const Matrix4x4 a = {
			1.2f, -2.3f, 5.1f, -1.2f,
			2.2f, -5.1f,  1.1f, -7.4f,
			3.2f,  3.3f, -3.8f, -9.2f,
			-6.8f, -2.9f,  1.0f,  4.9f
		};
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
		a.min = { -2.3f, 1.2f, -4.5f };
		a.max = { 3.7f, 5.3f, -2.9f };
		const Vector3 c = aabb::center(a);
		ENSURE(fequal(c.x,  0.70f, 0.00001f));
		ENSURE(fequal(c.y,  3.25f, 0.00001f));
		ENSURE(fequal(c.z, -3.70f, 0.00001f));
	}
	{
		AABB a;
		a.min = { -2.3f, 1.2f, -4.5f };
		a.max = { 3.7f, 5.3f, -2.9f };
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
		a.min = { -2.3f, 1.2f, -4.5f };
		a.max = { 3.7f, 5.3f, -2.9f };
		ENSURE(aabb::contains_point(a, { 1.2f,  3.0f, -4.4f }));
		ENSURE(!aabb::contains_point(a, { 3.8f,  3.0f, -4.4f }));
		ENSURE(!aabb::contains_point(a, { 1.2f, -1.0f, -4.4f }));
		ENSURE(!aabb::contains_point(a, { 1.2f,  3.0f, -4.6f }));
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
		a.c = { -2.3f, 1.2f, -4.5f };
		a.r = 1.0f;
		ENSURE(sphere::contains_point(a, { -2.9f, 1.6f, -4.0f }));
		ENSURE(!sphere::contains_point(a, { -3.9f, 1.6f, -4.0f }));
		ENSURE(!sphere::contains_point(a, { -2.9f, 2.6f, -4.0f }));
		ENSURE(!sphere::contains_point(a, { -2.9f, 1.6f, -6.0f }));
	}
}

static void test_obb()
{
	{
		OBB a;
		obb::reset(a);
		ENSURE(a.tm == MATRIX4X4_IDENTITY);
		ENSURE(fequal(a.half_extents.x, 0.0f, 0.00001f));
		ENSURE(fequal(a.half_extents.y, 0.0f, 0.00001f));
		ENSURE(fequal(a.half_extents.z, 0.0f, 0.00001f));
	}
	{
		OBB a;
		obb::reset(a);
		a.half_extents = { 0.5f, 0.5f, 0.5f };

		Vector3 ray_origin = { 0.0f, -2.0f, 0.25 };
		Vector3 ray_target = { 0.0f,  0.0f, 0.25 };
		Vector3 ray_direction = ray_target - ray_origin;
		normalize(ray_direction);
		ENSURE(ray_obb_intersection(ray_origin, ray_direction, a.tm, a.half_extents) != -1.0f);
	}
	{
		OBB a;
		obb::reset(a);
		a.half_extents = { 0.5f, 0.5f, 0.5f };
		set_scale(a.tm, { 0.01, 0.01, 0.01 });

		Vector3 ray_origin = { 0.0f, -2.0f, 0.25 };
		Vector3 ray_target = { 0.0f,  0.0f, 0.25 };
		Vector3 ray_direction = ray_target - ray_origin;
		normalize(ray_direction);
		ENSURE(ray_obb_intersection(ray_origin, ray_direction, a.tm, a.half_extents) == -1.0f);
	}
}

static void test_murmur()
{
	const u64 n = murmur64("murmur64", 8, 0);
	ENSURE(n == 0x90631502d1a3432bu);
}

static void test_string_id()
{
	memory_globals::init();
	{
		StringId32 a("");
		ENSURE(a._id == 0);
	}
	{
		StringId64 a("");
		ENSURE(a._id == 0);
	}
	{
		StringId32 a("murmur32");
		ENSURE(a._id == 0x68bd9babu);

		StringId32 b("murmur32", 8);
		ENSURE(b._id == 0x68bd9babu);

		char str[9];
		a.to_string(str, sizeof(str));
		ENSURE(strcmp(str, "68bd9bab") == 0);
	}
	{
		StringId64 a("murmur64");
		ENSURE(a._id == 0x90631502d1a3432bu);

		StringId64 b("murmur64", 8);
		ENSURE(b._id == 0x90631502d1a3432bu);

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
		ENSURE(id._id == 0x68bd9babu);
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

		ENSURE(str.has_prefix("Hello"));
		ENSURE(!str.has_prefix("Helloo"));

		ENSURE(str.has_suffix("one!"));
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
		ENSURE(strcmp(str.c_str(), "618f4c63") == 0);
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
		const char *str = "foo";
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
	{
		TempAllocator1024 ta;
		JsonObject obj(ta);
		json::parse_object(obj, "{\"foo\":{\"}\":false}}");
	}
	{
		TempAllocator1024 ta;
		JsonObject obj(ta);
		json::parse_object(obj, "{\"foo\":[\"]\"]}");
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
		ENSURE(a._id == 0x68bd9bab);
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
	{
		TempAllocator128 ta;
		DynamicString str(ta);
		sjson::parse_verbatim(str, "\"\"\"\"foo\"\"\"\"");
		ENSURE(strcmp(str.c_str(), "\"foo\"") == 0);
	}
	{
		TempAllocator128 ta;
		DynamicString str(ta);
		sjson::parse_verbatim(str, "\"\"\"\"foo\"\"\"\"bananas\"\"\"");
		ENSURE(strcmp(str.c_str(), "\"foo\"") == 0);
	}
	{
		TempAllocator1024 ta;
		JsonObject obj(ta);
		sjson::parse_object(obj, "{foo={\"}\"=false}}");
	}
	{
		TempAllocator1024 ta;
		JsonObject obj(ta);
		sjson::parse_object(obj, "{foo={bar=\"\"\"}\"\"\"}}");
	}
	{
		TempAllocator1024 ta;
		JsonObject obj(ta);
		sjson::parse_object(obj, "{\"foo\":\"\"\"a\"\"\"\"}");
	}
	{
		TempAllocator1024 ta;
		JsonObject obj(ta);
		sjson::parse_object(obj, "{foo=[\"]\"]}");
	}
	{
		TempAllocator1024 ta;
		JsonObject obj(ta);
		sjson::parse_object(obj, "{foo=[/*]*/]}");
	}
	{
		TempAllocator1024 ta;
		JsonObject obj(ta);
		sjson::parse_object(obj, "{foo=[//]\n]}");
	}
	{
		TempAllocator1024 ta;
		JsonObject obj(ta);
		sjson::parse_object(obj, "{foo=\"\"\"verbatim1\"\"\" bar=\"\"\"verbatim2\"\"\"}");
		ENSURE(json_object::has(obj, "foo"));
		ENSURE(json_object::has(obj, "bar"));
	}
	memory_globals::shutdown();
}

static void test_path()
{
#if CROWN_PLATFORM_WINDOWS
	{
		ENSURE(path::is_absolute("C:\\Users\\foo"));
		ENSURE(path::is_absolute("/c/Users/foo"));
		ENSURE(!path::is_absolute("Users\\foo"));
		ENSURE(!path::is_absolute("Users/foo"));
	}
	{
		ENSURE(!path::is_relative("D:\\Users\\foo"));
		ENSURE(!path::is_relative("/d/Users/foo"));
		ENSURE(path::is_relative("Users\\foo"));
		ENSURE(path::is_relative("Users/foo"));
	}
	{
		ENSURE(path::is_root("E:\\"));
		ENSURE(path::is_root("/c"));
		ENSURE(!path::is_root("/home"));
		ENSURE(!path::is_root("E:\\Users"));
		ENSURE(!path::is_root("/c/Users"));
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
#else
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
#endif // if CROWN_PLATFORM_WINDOWS
	{
		const char *p = path::basename("");
		ENSURE(strcmp(p, "") == 0);
	}
	{
		const char *q = path::basename("/");
		ENSURE(strcmp(q, "") == 0);
	}
	{
		const char *p = path::basename("boot.config");
		ENSURE(strcmp(p, "boot.config") == 0);
	}
	{
		const char *p = path::basename("foo/boot.config");
		ENSURE(strcmp(p, "boot.config") == 0);
	}
	{
		const char *p = path::basename("/foo/boot.config");
		ENSURE(strcmp(p, "boot.config") == 0);
	}
	{
		const char *p = path::extension("");
		ENSURE(p == NULL);
	}
	{
		const char *p = path::extension("boot");
		ENSURE(p == NULL);
	}
	{
		const char *p = path::extension("boot.bar.config");
		ENSURE(strcmp(p, "config") == 0);
	}
	{
		const char *p = path::extension(".bar");
		ENSURE(p == NULL);
	}
	{
		const char *p = path::extension("foo/.bar");
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
#if CROWN_PLATFORM_WINDOWS
	{
		TempAllocator128 ta;
		DynamicString path(ta);
		path::join(path, "C:\\foo", "bar");
		ENSURE(path == "C:\\foo\\bar");
	}
#else
	{
		TempAllocator128 ta;
		DynamicString path(ta);
		path::join(path, "foo", "bar");
		ENSURE(path == "foo/bar");
	}
#endif
}

static void test_command_line()
{
	{
		const char *argv[] = { "prog", "-s", "--switch" };
		CommandLine cl(countof(argv), argv);

		ENSURE(cl.has_option("switch", 's'));
	}
	{
		const char *argv[] = { "prog", "--args", "orange", "apple" };
		CommandLine cl(countof(argv), argv);

		const char *orange = cl.get_parameter(0, "args");
		ENSURE(orange != NULL && strcmp(orange, "orange") == 0);
		const char *apple = cl.get_parameter(1, "args");
		ENSURE(apple != NULL && strcmp(apple, "apple") == 0);
		const char *banana = cl.get_parameter(2, "banana");
		ENSURE(banana == NULL);
	}
	{
		const char *argv[] = { "prog", "--bar", "--", "--baz" };
		CommandLine cl(countof(argv), argv);

		ENSURE(cl.has_option("bar"));
		ENSURE(!cl.has_option("baz"));
		ENSURE(!cl.has_option("--"));
		ENSURE(cl.get_parameter(0, "bar") == NULL);
	}
}

static void test_thread()
{
	Thread thread;
	ENSURE(!thread.is_running());

	thread.start([](void *) { return 0xbadc0d3; }, NULL);
	thread.stop();
	ENSURE(thread.exit_code() == 0xbadc0d3);
}

static void test_process()
{
#if !CROWN_PLATFORM_EMSCRIPTEN && !CROWN_PLATFORM_WINDOWS
	{
#define ECHO_TEXT "foobar"
		const char *argv[] =
		{
			"echo"
			, ECHO_TEXT
			, NULL
		};

		u32 nbr;
		char buf[128] = {0};
		Process pr;
		ENSURE(pr.spawn(argv, CROWN_PROCESS_STDOUT_PIPE) == 0);
		pr.read(&nbr, buf, sizeof(buf));
		ENSURE(strcmp(buf, ECHO_TEXT "\n") == 0);
		ENSURE(pr.wait() == 0);
	}
#endif // if !CROWN_PLATFORM_EMSCRIPTEN
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
#endif // if CROWN_PLATFORM_POSIX
}

static void test_file_monitor()
{
#if !CROWN_PLATFORM_EMSCRIPTEN
	struct UserData
	{
		bool done;
		Mutex m;
		ConditionVariable cv;
		FileMonitorFunction check;

		UserData()
			: done(false)
		{
		}

		void wait(u32 ms)
		{
			WaitResult wr;
			wr.error = WaitResult::SUCCESS;

			m.lock();
			while (!done && wr.error != WaitResult::TIMEOUT)
				wr = cv.wait(m, ms);
			m.unlock();
			ENSURE(done);
		}

		static void callback(void *user_data, FileMonitorEvent::Enum fme, bool is_dir, const char *path, const char *path_modified)
		{
			UserData *ud = (UserData *)user_data;
			ud->done = false;
			ud->check(user_data, fme, is_dir, path, path_modified);
			ud->done = true;
			ud->cv.signal();
		}
	};

	struct ScopedUniqueWorkingDir
	{
		char _workdir[1024];

		ScopedUniqueWorkingDir()
		{
			os::getcwd(_workdir, sizeof(_workdir));

			char unique_name[GUID_BUF_LEN];
			guid::to_string(unique_name, sizeof(unique_name), guid::new_guid());
			os::create_directory(unique_name);
			os::setcwd(unique_name);
		}

		~ScopedUniqueWorkingDir()
		{
			os::setcwd(_workdir);
		}
	};

	memory_globals::init();
	guid_globals::init();
	{
		ScopedUniqueWorkingDir uwd;

		FilesystemDisk fs(default_allocator());
		fs.set_prefix("dir");
		fs.create_directory("");

		const char *paths[] = { "dir" };
		UserData state;
		FileMonitor fm(default_allocator());
		fm.start(countof(paths), paths, true, state.callback, &state);

		state.check = [](auto user_data, auto fme, auto is_dir, auto path, auto path_modified) {
			CE_UNUSED_2(user_data, path_modified);
			ENSURE(fme == FileMonitorEvent::CREATED);
			ENSURE(is_dir == true);
			DynamicString os_path(default_allocator());
			path::join(os_path, "dir", "foo");
			ENSURE(strcmp(path, os_path.c_str()) == 0);
		};
		fs.create_directory("foo");
		state.wait(500);

		fm.stop();
	}
	{
		ScopedUniqueWorkingDir uwd;

		FilesystemDisk fs1(default_allocator());
		fs1.set_prefix("dir1");
		fs1.create_directory("");
		FilesystemDisk fs2(default_allocator());
		fs2.set_prefix("dir2");
		fs2.create_directory("");
		fs2.create_directory("foo");

		const char *paths[] = { "dir1" };
		UserData state;
		FileMonitor fm(default_allocator());
		fm.start(countof(paths), paths, true, state.callback, &state);

		state.check = [](auto user_data, auto fme, auto is_dir, auto path, auto path_modified) {
			CE_UNUSED_2(user_data, path_modified);
			ENSURE(fme == FileMonitorEvent::CREATED);
			ENSURE(is_dir == true);
			DynamicString os_path(default_allocator());
			path::join(os_path, "dir1", "foo");
			ENSURE(strcmp(path, os_path.c_str()) == 0);
		};
		DynamicString old_name(default_allocator());
		DynamicString new_name(default_allocator());
		path::join(old_name, "dir2", "foo");
		path::join(new_name, "dir1", "foo");
		os::rename(old_name.c_str(), new_name.c_str());
		state.wait(500);

		state.check = [](auto user_data, auto fme, auto is_dir, auto path, auto path_modified) {
			CE_UNUSED_2(user_data, path_modified);
			ENSURE(fme == FileMonitorEvent::DELETED);
#if !CROWN_PLATFORM_WINDOWS
			ENSURE(is_dir == true);
#endif
			DynamicString os_path(default_allocator());
			path::join(os_path, "dir1", "foo");
			ENSURE(strcmp(path, os_path.c_str()) == 0);
		};
		os::rename(new_name.c_str(), old_name.c_str());
		state.wait(500);

		fm.stop();
	}
	{
		ScopedUniqueWorkingDir uwd;

		FilesystemDisk fs(default_allocator());
		fs.set_prefix("dir");
		fs.create_directory("");
		fs.create_directory("foo");

		const char *paths[] = { "dir" };
		UserData state;
		FileMonitor fm(default_allocator());
		fm.start(countof(paths), paths, true, state.callback, &state);

		state.check = [](auto user_data, auto fme, auto is_dir, auto path, auto path_modified) {
			CE_UNUSED(user_data);
			ENSURE(fme == FileMonitorEvent::RENAMED);
			ENSURE(is_dir == true);
			DynamicString os_path(default_allocator());
			DynamicString os_path_modified(default_allocator());
			path::join(os_path, "dir", "foo");
			path::join(os_path_modified, "dir", "bar");
			ENSURE(strcmp(path, os_path.c_str()) == 0);
			ENSURE(strcmp(path_modified, os_path_modified.c_str()) == 0);
		};
		DynamicString old_name(default_allocator());
		DynamicString new_name(default_allocator());
		path::join(old_name, "dir", "foo");
		path::join(new_name, "dir", "bar");
		os::rename(old_name.c_str(), new_name.c_str());
		state.wait(500);

		fm.stop();
	}
	{
		ScopedUniqueWorkingDir uwd;

		FilesystemDisk fs(default_allocator());
		fs.set_prefix("dir");
		fs.create_directory("");
		File *file = fs.open("pi", FileOpenMode::WRITE);

		const char *paths[] = { "dir" };
		UserData state;
		FileMonitor fm(default_allocator());
		fm.start(countof(paths), paths, true, state.callback, &state);

		state.check = [](auto user_data, auto fme, auto is_dir, auto path, auto path_modified) {
			CE_UNUSED_2(user_data, path_modified);
			ENSURE(fme == FileMonitorEvent::CHANGED);
			ENSURE(is_dir == false);
			DynamicString os_path(default_allocator());
			path::join(os_path, "dir", "pi");
			ENSURE(strcmp(path, os_path.c_str()) == 0);
		};
		file->write("3.14", 4);
		file->sync();
		state.wait(500);

		fs.close(*file);

		fm.stop();
	}
	guid_globals::shutdown();
	memory_globals::shutdown();
#endif // if !CROWN_PLATFORM_EMSCRIPTEN
}

static void test_option()
{
	memory_globals::init();
	{
		Option<int> opt(3);
		ENSURE(opt.value() == 3);
		ENSURE(opt.has_changed() == false);

		opt.set_value(3);
		ENSURE(opt.has_changed() == true);

		opt.set_value(4);
		ENSURE(opt.value() == 4);
	}
	memory_globals::shutdown();
}

static void test_lua_resource()
{
#if CROWN_CAN_COMPILE
	memory_globals::init();
	{
		const char *lua = "";

		HashSet<StringView> req(default_allocator());
		lua_resource_internal::find_requirements(req, lua);

		ENSURE(hash_set::size(req) == 0);
	}
	{
		const char *lua = "require 'foo'";

		HashSet<StringView> req(default_allocator());
		lua_resource_internal::find_requirements(req, lua);

		ENSURE(hash_set::size(req) == 1);
		ENSURE(hash_set::has(req, StringView("foo")));
	}
	{
		const char *lua = "require \"foo\"";

		HashSet<StringView> req(default_allocator());
		lua_resource_internal::find_requirements(req, lua);

		ENSURE(hash_set::size(req) == 1);
		ENSURE(hash_set::has(req, StringView("foo")));
	}
	{
		const char *lua = "require (\"foo\")";

		HashSet<StringView> req(default_allocator());
		lua_resource_internal::find_requirements(req, lua);

		ENSURE(hash_set::size(req) == 1);
		ENSURE(hash_set::has(req, StringView("foo")));
	}
	{
		const char *lua = "require";

		HashSet<StringView> req(default_allocator());
		lua_resource_internal::find_requirements(req, lua);

		ENSURE(hash_set::size(req) == 0);
	}
	{
		const char *lua = "require '";

		HashSet<StringView> req(default_allocator());
		lua_resource_internal::find_requirements(req, lua);

		ENSURE(hash_set::size(req) == 0);
	}
	{
		const char *lua = "require ('";

		HashSet<StringView> req(default_allocator());
		lua_resource_internal::find_requirements(req, lua);

		ENSURE(hash_set::size(req) == 0);
	}
	{
		const char *lua = "-- require (\"foo\")";

		HashSet<StringView> req(default_allocator());
		lua_resource_internal::find_requirements(req, lua);

		ENSURE(hash_set::size(req) == 0);
	}
	{
		const char *lua = "require (\"foo\") -- comment";

		HashSet<StringView> req(default_allocator());
		lua_resource_internal::find_requirements(req, lua);

		ENSURE(hash_set::size(req) == 1);
		ENSURE(hash_set::has(req, StringView("foo")));
	}
	{
		const char *lua = "require --[[ comment --]] (\"foo\")";

		HashSet<StringView> req(default_allocator());
		lua_resource_internal::find_requirements(req, lua);

		ENSURE(hash_set::size(req) == 1);
		ENSURE(hash_set::has(req, StringView("foo")));
	}
	{
		const char *lua = "require ( --[[--]] \"foo\" --[[--]])";

		HashSet<StringView> req(default_allocator());
		lua_resource_internal::find_requirements(req, lua);

		ENSURE(hash_set::size(req) == 1);
		ENSURE(hash_set::has(req, StringView("foo")));
	}
	{
		const char *lua = "--[[ unmatched multi-line comment";

		HashSet<StringView> req(default_allocator());
		lua_resource_internal::find_requirements(req, lua);

		ENSURE(hash_set::size(req) == 0);
	}
	{
		const char *lua = "require --[[ unmatched multi-line comment after require";

		HashSet<StringView> req(default_allocator());
		lua_resource_internal::find_requirements(req, lua);

		ENSURE(hash_set::size(req) == 0);
	}
	{
		const char *lua =
			"require (\"foo\")"
			"require 'bar'"
			"require 'baz'"
			;

		HashSet<StringView> req(default_allocator());
		lua_resource_internal::find_requirements(req, lua);

		ENSURE(hash_set::size(req) == 3);
		ENSURE(hash_set::has(req, StringView("foo")));
		ENSURE(hash_set::has(req, StringView("bar")));
		ENSURE(hash_set::has(req, StringView("baz")));
	}
	{
		const char *lua =
			"function init()"
			"    require (\"foo\")"
			"    require 'bar'"
			"    require 'baz'"
			"end"
			;

		HashSet<StringView> req(default_allocator());
		lua_resource_internal::find_requirements(req, lua);

		ENSURE(hash_set::size(req) == 3);
		ENSURE(hash_set::has(req, StringView("foo")));
		ENSURE(hash_set::has(req, StringView("bar")));
		ENSURE(hash_set::has(req, StringView("baz")));
	}
	memory_globals::shutdown();
#endif // if CROWN_CAN_COMPILE
}

static void test_time()
{
	{
		s64 t = time::now();
		os::sleep(32);
		ENSURE(time::seconds(time::now() - t) >= 0.032);
	}
}

static void test_expression_language()
{
	namespace el = expression_language;
	memory_globals::init();
	{
		unsigned byte_code[1024];
		f32 stack_data[32];
		el::Stack stack(stack_data, countof(stack_data));

		u32 num = el::compile(""
			, 0
			, NULL
			, 0
			, NULL
			, NULL
			, byte_code
			, countof(byte_code)
			);
		ENSURE(num <= countof(byte_code));

		bool ok = el::run(byte_code, NULL, stack);
		ENSURE(ok && stack.size == 0);
	}
	{
		auto eval = [](const char *expr) {
			unsigned byte_code[1024];
			f32 stack_data[32];
			const char *variable_names[] = { "a", "b" };
			f32 variable_values[] = { 2.0f, 1.5f };
			const char *constant_names[] = { "PI" };
			f32 constant_values[] = { 3.14159265359f };
			el::Stack stack(stack_data, countof(stack_data));

			u32 num = el::compile(expr
				, countof(variable_names)
				, variable_names
				, countof(constant_names)
				, constant_names
				, constant_values
				, byte_code
				, countof(byte_code)
				);
			ENSURE(num <= countof(byte_code));

			bool ok = el::run(byte_code, variable_values, stack);
			ENSURE(ok && stack.size > 0);
			return stack_data[stack.size - 1];
		};

		ENSURE(fequal(eval("1 + 1"), 2.0f, 0.0001f));
		ENSURE(fequal(eval("a + b"), 3.5f, 0.0001f));
		ENSURE(fequal(eval("3*(sin(PI/2) + 2) - 1"), 8.0f, 0.0001f));
		ENSURE(fequal(eval("2*b*(sin(a)/2) + 2) - 1"), 2.36394f, 0.0001f));
	}
	memory_globals::shutdown();
}

static void test_unit_id()
{
	{
		UnitId u = make_unit(100, 50);
		ENSURE(u.index() == 100);
		ENSURE(u.id() == 50);
	}
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
	RUN_TEST(test_obb);
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
	RUN_TEST(test_file_monitor);
	RUN_TEST(test_option);
	RUN_TEST(test_lua_resource);
	RUN_TEST(test_time);
	RUN_TEST(test_expression_language);
	RUN_TEST(test_unit_id);

	return EXIT_SUCCESS;
}

} // namespace crown

#endif // if CROWN_BUILD_UNIT_TESTS
