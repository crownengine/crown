/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "ContainerTypes.h"
#include "Vector.h"

// #define RBTREE_VERIFY

namespace crown
{

/// Functions to manipulate Map
///
/// @ingroup Containers
namespace map
{
	/// Returns the number of items in the map @a m.
	template <typename TKey, typename TValue> uint32_t size(const Map<TKey, TValue>& m);

	/// Returns whether the given @a key exists in the map @a m.
	template <typename TKey, typename TValue> bool has(const Map<TKey, TValue>& m, const TKey key);

	/// Returns the value for the given @a key or @a deffault if
	/// the key does not exist in the map.
	template <typename TKey, typename TValue> const TValue& get(const Map<TKey, TValue>& m, const TKey key, const TValue& deffault);

	/// Sets the @a value for the @a key in the map.
	template <typename TKey, typename TValue> void set(Map<TKey, TValue>& m, const TKey& key, const TValue& value);

	/// Removes the @a key from the map if it exists.
	template <typename TKey, typename TValue> void remove(Map<TKey, TValue>& m, const TKey& key);

	/// Removes all the items in the map.
	/// @note Calls destructor on the items.
	template <typename TKey, typename TValue> void clear(Map<TKey, TValue>& m);

	/// Returns a pointer to the first item in the map, can be used to
	/// efficiently iterate over the elements (in random order).
	template <typename TKey, typename TValue> const typename Map<TKey, TValue>::Node* begin(const Map<TKey, TValue>& m);
	template <typename TKey, typename TValue> const typename Map<TKey, TValue>::Node* end(const Map<TKey, TValue>& m);
} // namespace map

namespace map_internal
{
	const uint32_t BLACK = 0xB1B1B1B1u;
	const uint32_t RED = 0xEDEDEDEDu;
	const uint32_t NIL = 0xFFFFFFFFu;

	template <typename TKey, typename TValue>
	inline uint32_t root(const Map<TKey, TValue>& m)
	{
		return m.m_root;
	}

	template <typename TKey, typename TValue>
	inline uint32_t parent(const Map<TKey, TValue>& m, uint32_t n)
	{
		CE_ASSERT(n < vector::size(m.m_data), "Index out of bounds (size = %d, n = %d)", vector::size(m.m_data), n);
		return m.m_data[n].parent;
	}
	
	template <typename TKey, typename TValue>
	inline uint32_t left(const Map<TKey, TValue>& m, uint32_t n)
	{
		CE_ASSERT(n < vector::size(m.m_data), "Index out of bounds (size = %d, n = %d)", vector::size(m.m_data), n);
		return m.m_data[n].left;
	}
	
	template <typename TKey, typename TValue>
	inline uint32_t right(const Map<TKey, TValue>& m, uint32_t n)
	{
		CE_ASSERT(n < vector::size(m.m_data), "Index out of bounds (size = %d, n = %d)", vector::size(m.m_data), n);
		return m.m_data[n].right;
	}

	template <typename TKey, typename TValue>
	inline uint32_t color(const Map<TKey, TValue>& m, uint32_t n)
	{
		CE_ASSERT(n < vector::size(m.m_data), "Index out of bounds (size = %d, n = %d)", vector::size(m.m_data), n);
		return m.m_data[n].color;
	}

	#ifdef RBTREE_VERIFY
	template<typename TKey, typename TValue>
	inline int32_t dbg_verify(Map<TKey, TValue>& m, uint32_t n)
	{
		if (n == m.m_sentinel)
		{
			return 0;
		}

		if (left(m, n) != m.m_sentinel)
		{
			CE_ASSERT(parent(m, left(m, n)) == n, "Bad RBTree");
			CE_ASSERT(m.m_data[left(m, n)].key < m.m_data[n].key, "Bad RBTree");
		}

		if (right(m, n) != m.m_sentinel)
		{
			CE_ASSERT(parent(m, right(m, n)) == n, "Bad RBTree");
			CE_ASSERT(m.m_data[n].key < m.m_data[right(m, n)].key, "Bad RBTree");
		}

		int32_t bhL = dbg_verify(m, left(m, n));
		int32_t bhR = dbg_verify(m, right(m, n));
		CE_ASSERT(bhL == bhR, "Bad RBTree");

		if (color(m, n) == BLACK)
		{
			bhL += 1;
		}
		else
		{
			if (parent(m, n) != NIL && color(m, parent(m, n)) == RED)
			{
				CE_ASSERT(false, "Bad RBTree");
			}
		}

		return bhL;
	}

	template<typename TKey, typename TValue>
	inline int32_t dump(Map<TKey, TValue>& m)
	{
		for (uint32_t i = 0; i < vector::size(m.m_data); i++)
		{
			printf("%d = [%d, %d, %d] ", i, parent(m, i), left(m, i), right(m, i));
		}
		printf("\n");
		return 0;
	}
	#endif

	template <typename TKey, typename TValue>
	inline uint32_t min(const Map<TKey, TValue>& m, uint32_t x)
	{
		if (x == m.m_sentinel)
		{
			return x;
		}

		while (left(m, x) != m.m_sentinel)
		{
			x = left(m, x);
		}

		return x;
	}

	template <typename TKey, typename TValue>
	inline uint32_t max(const Map<TKey, TValue>& m, uint32_t x)
	{
		if (x == m.m_sentinel)
		{
			return x;
		}

		while (right(m, x) != m.m_sentinel)
		{
			x = right(m, x);
		}

		return x;
	}

	template <typename TKey, typename TValue>
	inline uint32_t successor(const Map<TKey, TValue>& m, uint32_t x)
	{
		if (right(m, x) != m.m_sentinel)
		{
			return min(m, right(m, x));
		}

		uint32_t y = parent(m, x);

		while (y != NIL && x == right(m, y))
		{
			x = y;
			y = parent(m, y);
		}

		return y;
	}

	template <typename TKey, typename TValue>
	inline uint32_t predecessor(const Map<TKey, TValue>& m, uint32_t x)
	{
		if (left(m, x) != m.m_sentinel)
		{
			return max(m, left(m, x));
		}

		uint32_t y = parent(m, x);

		while (y != NIL && x == left(m, y))
		{
			x = y;
			y = parent(m, y);
		}

		return y;
	}

	template <typename TKey, typename TValue>
	inline void rotate_left(Map<TKey, TValue>& m, uint32_t x)
	{
		CE_ASSERT(x < vector::size(m.m_data), "Index out of bounds (size = %d, n = %d)", vector::size(m.m_data), x);

		uint32_t y = right(m, x);
		m.m_data[x].right = left(m, y);

		if (left(m, y) != m.m_sentinel)
		{
			m.m_data[left(m, y)].parent = x;
		}

		m.m_data[y].parent = parent(m, x);

		if (parent(m, x) == NIL)
		{
			m.m_root = y;
		}
		else
		{
			if (x == left(m, parent(m, x)))
			{
				m.m_data[parent(m, x)].left = y;
			}
			else
			{
				m.m_data[parent(m, x)].right = y;
			}
		}

		m.m_data[y].left = x;
		m.m_data[x].parent = y;
	}

	template <typename TKey, typename TValue>
	inline void rotate_right(Map<TKey, TValue>& m, uint32_t x)
	{
		CE_ASSERT(x < vector::size(m.m_data), "Index out of bounds (size = %d, n = %d)", vector::size(m.m_data), x);

		uint32_t y = left(m, x);
		m.m_data[x].left = right(m, y);

		if (right(m, y) != m.m_sentinel)
		{
			m.m_data[right(m, y)].parent = x;
		}

		m.m_data[y].parent = parent(m, x);

		if (parent(m, x) == NIL)
		{
			m.m_root = y;
		}
		else
		{
			if (x == left(m, parent(m, x)))
			{
				m.m_data[parent(m, x)].left = y;
			}
			else
			{
				m.m_data[parent(m, x)].right = y;
			}
		}

		m.m_data[y].right = x;
		m.m_data[x].parent = y;
	}

	template <typename TKey, typename TValue>
	inline void destroy(Map<TKey, TValue>& m, uint32_t n)
	{
		CE_ASSERT(n < vector::size(m.m_data), "Index out of bounds (size = %d, n = %d)", vector::size(m.m_data), n);

		uint32_t x = vector::size(m.m_data) - 1;

		if (x == m.m_root)
		{
			m.m_root = n;

			if (left(m, x) != NIL)
				m.m_data[left(m, x)].parent = n;
			if (right(m, x) != NIL)
				m.m_data[right(m, x)].parent = n;

			m.m_data[n] = m.m_data[x];
		}
		else
		{
			if (x != n)
			{
				if (x == left(m, parent(m, x)))
				{
					m.m_data[parent(m, x)].left = n;
				}
				else if (x == right(m, parent(m, x)))
				{
					m.m_data[parent(m, x)].right = n;
				}

				if (left(m, x) != NIL)
					m.m_data[left(m, x)].parent = n;
				if (right(m, x) != NIL)
					m.m_data[right(m, x)].parent = n;

				m.m_data[n] = m.m_data[x];
			}
		}

		#ifdef RBTREE_VERIFY
			dbg_verify(m, m.m_root);
		#endif

		vector::pop_back(m.m_data);
	}

	template <typename TKey, typename TValue>
	inline void insert_fixup(Map<TKey, TValue>& m, uint32_t n)
	{
		CE_ASSERT(n < vector::size(m.m_data), "Index out of bounds (size = %d, n = %d)", vector::size(m.m_data), n);

		uint32_t x;
		uint32_t y;

		while (n != root(m) && color(m, parent(m, n)) == RED)
		{
			x = parent(m, n);

			if (x == left(m, parent(m, x)))
			{
				y = right(m, parent(m, x));

				if (color(m, y) == RED)
				{
					m.m_data[x].color = BLACK;
					m.m_data[y].color = BLACK;
					m.m_data[parent(m, x)].color = RED;
					n = parent(m, x);
					continue;
				}
				else
				{
					if (n == right(m, x))
					{
						n = x;
						rotate_left(m, n);
						x = parent(m, n);
					}

					m.m_data[x].color = BLACK;
					m.m_data[parent(m, x)].color = RED;
					rotate_right(m, parent(m, x));
				}
			}
			else
			{
				y = left(m, parent(m, x));

				if (color(m, y) == RED)
				{
					m.m_data[x].color = BLACK;
					m.m_data[y].color = BLACK;
					m.m_data[parent(m, x)].color = RED;
					n = parent(m, x);
					continue;
				}
				else
				{
					if (n == left(m, x))
					{
						n = x;
						rotate_right(m, n);
						x = parent(m, n);
					}

					m.m_data[x].color = BLACK;
					m.m_data[parent(m, x)].color = RED;
					rotate_left(m, parent(m, x));
				}
			}
		}
	}

	template <typename TKey, typename TValue>
	inline uint32_t inner_find(const Map<TKey, TValue>& m, const TKey key)
	{
		uint32_t x = m.m_root;

		while (x != m.m_sentinel)
		{
			if (m.m_data[x].key < key)
			{
				if (right(m, x) == m.m_sentinel)
				{
					return x;
				}

				x = right(m, x);
			}
			else if (key < m.m_data[x].key)
			{
				if (left(m, x) == m.m_sentinel)
				{
					return x;
				}

				x = left(m, x);
			}
			else
			{
				break;
			}
		}

		return x;
	}

	template <typename TKey, typename TValue>
	inline uint32_t find_or_fail(const Map<TKey, TValue>& m, const TKey key)
	{
		uint32_t p = inner_find(m, key);

		if (p != m.m_sentinel && m.m_data[p].key == key)
			return p;

		return NIL;
	}

	template <typename TKey, typename TValue>
	inline uint32_t find_or_add(Map<TKey, TValue>& m, const TKey key)
	{
		uint32_t p = inner_find(m, key);

		if (p != m.m_sentinel && m.m_data[p].key == key)
		{
			return p;
		}

		typename Map<TKey, TValue>::Node n;
		n.key = key;
		n.value = TValue();
		n.color = RED;
		n.left = m.m_sentinel;
		n.right = m.m_sentinel;
		n.parent = NIL;

		if (p == m.m_sentinel)
		{
			m.m_root = n;
		}
		else
		{
			if (key < m.m_data[p].key)
			{
				m.m_data[p].left = n;
			}
			else
			{
				m.m_data[p].right = n;
			}

			m.m_data[n].parent = p;
		}

		add_fixup(m, n);
		m.m_data[m.m_root].color = BLACK;
		#ifdef RBTREE_VERIFY
			dbg_verify(m, m.m_root);
		#endif
		return n;
	}
} // namespace map_internal

namespace map
{
	template <typename TKey, typename TValue>
	uint32_t size(const Map<TKey, TValue>& m)
	{
		CE_ASSERT(vector::size(m.m_data) > 0, "Bad Map"); // There should be at least sentinel
		return vector::size(m.m_data) - 1;
	}

	template <typename TKey, typename TValue>
	inline bool has(const Map<TKey, TValue>& m, const TKey key)
	{
		return map_internal::find_or_fail(m, key) != map_internal::NIL;
	}

	template <typename TKey, typename TValue>
	inline const TValue& get(const Map<TKey, TValue>& m, const TKey key, const TValue& deffault)
	{
		uint32_t p = map_internal::inner_find(m, key);

		if (p != m.m_sentinel && m.m_data[p].key == key)
		{
			return m.m_data[p].value;
		}

		return deffault;
	}

	template <typename TKey, typename TValue>
	inline void set(Map<TKey, TValue>& m, const TKey& key, const TValue& value)
	{
		typename Map<TKey, TValue>::Node node;
		node.key = key;
		node.value = value;
		node.color = map_internal::RED;
		node.left = m.m_sentinel;
		node.right = m.m_sentinel;
		node.parent = map_internal::NIL;
		uint32_t n = vector::push_back(m.m_data, node);
		uint32_t x = m.m_root;
		uint32_t y = map_internal::NIL;

		if (x == m.m_sentinel)
			m.m_root = n;
		else
		{
			while (x != m.m_sentinel)
			{
				y = x;

				if (key < m.m_data[x].key)
					x = m.m_data[x].left;
				else
					x = m.m_data[x].right;
			}

			if (key < m.m_data[y].key)
				m.m_data[y].left = n;
			else
				m.m_data[y].right = n;

			m.m_data[n].parent = y;
		}

		map_internal::insert_fixup(m, n);
		m.m_data[m.m_root].color = map_internal::BLACK;
		#ifdef RBTREE_VERIFY
			map_internal::dbg_verify(m, m.m_root);
		#endif
	}

	template <typename TKey, typename TValue>
	inline void remove(Map<TKey, TValue>& m, const TKey& key)
	{
		using namespace map_internal;

		uint32_t n = inner_find(m, key);

		if (!(m.m_data[n].key == key))
		{
			return;
		}

		uint32_t x;
		uint32_t y;

		if (left(m, n) == m.m_sentinel || right(m, n) == m.m_sentinel)
		{
			y = n;
		}
		else
		{
			y = successor(m, n);
		}

		if (left(m, y) != m.m_sentinel)
		{
			x = left(m, y);
		}
		else
		{
			x = right(m, y);
		}

		m.m_data[x].parent = parent(m, y);

		if (parent(m, y) != map_internal::NIL)
		{
			if (y == left(m, parent(m, y)))
			{
				m.m_data[parent(m, y)].left = x;
			}
			else
			{
				m.m_data[parent(m, y)].right = x;
			}
		}
		else
		{
			m.m_root = x;
		}

		if (y != n)
		{
			m.m_data[n].key = m.m_data[y].key;
			m.m_data[n].value = m.m_data[y].value;
		}

		// Do the fixup
		if (color(m, y) == map_internal::BLACK)
		{
			uint32_t y;

			while (x != m.m_root && color(m, x) == map_internal::BLACK)
			{
				if (x == left(m, parent(m, x)))
				{
					y = right(m, parent(m, x));

					if (color(m, y) == map_internal::RED)
					{
						m.m_data[y].color = map_internal::BLACK;
						m.m_data[parent(m, x)].color = map_internal::RED;
						rotate_left(m, parent(m, x));
						y = right(m, parent(m, x));
					}

					if (color(m, left(m, y)) == map_internal::BLACK && color(m, right(m, y)) == map_internal::BLACK)
					{
						m.m_data[y].color = map_internal::RED;
						x = parent(m, x);
					}
					else
					{
						if (color(m, right(m, y)) == map_internal::BLACK)
						{
							m.m_data[left(m, y)].color = map_internal::BLACK;
							m.m_data[y].color = map_internal::RED;
							rotate_right(m, y);
							y = right(m, parent(m, x));
						}

						m.m_data[y].color = color(m, parent(m, x));
						m.m_data[parent(m, x)].color = map_internal::BLACK;
						m.m_data[right(m, y)].color = map_internal::BLACK;
						rotate_left(m, parent(m, x));
						x = m.m_root;
					}
				}
				else
				{
					y = left(m, parent(m, x));

					if (color(m, y) == map_internal::RED)
					{
						m.m_data[y].color = map_internal::BLACK;
						m.m_data[parent(m, x)].color = map_internal::RED;
						rotate_right(m, parent(m, x));
						y = left(m, parent(m, x));
					}

					if (color(m, right(m, y)) == map_internal::BLACK && color(m, left(m, y)) == map_internal::BLACK)
					{
						m.m_data[y].color = map_internal::RED;
						x = parent(m, x);
					}
					else
					{
						if (color(m, left(m, y)) == map_internal::BLACK)
						{
							m.m_data[right(m, y)].color = map_internal::BLACK;
							m.m_data[y].color = map_internal::RED;
							rotate_left(m, y);
							y = left(m, parent(m, x));
						}

						m.m_data[y].color = color(m, parent(m, x));
						m.m_data[parent(m, x)].color = map_internal::BLACK;
						m.m_data[left(m, y)].color = map_internal::BLACK;
						rotate_right(m, parent(m, x));
						x = m.m_root;
					}
				}
			}

			m.m_data[x].color = map_internal::BLACK;
		}

		destroy(m, y);
	 	#ifdef RBTREE_VERIFY
			map_internal::dbg_verify(m, m.m_root);
	 	#endif
	}

	template <typename TKey, typename TValue>
	void clear(Map<TKey, TValue>& m)
	{
		vector::clear(m.m_data);

		m.m_root = 0;
		m.m_sentinel = 0;

		typename Map<TKey, TValue>::Node r;
		r.key = TKey();
		r.value = TValue();
		r.left = map_internal::NIL;
		r.right = map_internal::NIL;
		r.parent = map_internal::NIL;
		r.color = map_internal::BLACK;
		vector::push_back(m.m_data, r);
	}

	template <typename TKey, typename TValue>
	const typename Map<TKey, TValue>::Node* begin(const Map<TKey, TValue>& m)
	{
		return vector::begin(m.m_data) + 1; // Skip sentinel at index 0
	}

	template <typename TKey, typename TValue>
	const typename Map<TKey, TValue>::Node* end(const Map<TKey, TValue>& m)
	{
		return vector::end(m.m_data);
	}
} // namespace map

template <typename TKey, typename TValue>
inline Map<TKey, TValue>::Map(Allocator& a)
	: m_data(a)
{
	map::clear(*this);
}

} // namespace crown
