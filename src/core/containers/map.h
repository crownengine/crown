/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "container_types.h"
#include "vector.h"

// #define RBTREE_VERIFY

namespace crown
{
/// Functions to manipulate Map
///
/// @ingroup Containers
namespace map
{
	/// Returns the number of items in the map @a m.
	template <typename TKey, typename TValue> u32 size(const Map<TKey, TValue>& m);

	/// Returns whether the given @a key exists in the map @a m.
	template <typename TKey, typename TValue> bool has(const Map<TKey, TValue>& m, const TKey& key);

	/// Returns the value for the given @a key or @a deffault if
	/// the key does not exist in the map.
	template <typename TKey, typename TValue> const TValue& get(const Map<TKey, TValue>& m, const TKey& key, const TValue& deffault);

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
	const u32 BLACK = 0xB1B1B1B1u;
	const u32 RED = 0xEDEDEDEDu;
	const u32 NIL = 0xFFFFFFFFu;

	template <typename TKey, typename TValue>
	inline u32 root(const Map<TKey, TValue>& m)
	{
		return m._root;
	}

	template <typename TKey, typename TValue>
	inline u32 parent(const Map<TKey, TValue>& m, u32 n)
	{
		CE_ASSERT(n < vector::size(m._data), "Index out of bounds (size = %d, n = %d)", vector::size(m._data), n);
		return m._data[n].parent;
	}

	template <typename TKey, typename TValue>
	inline u32 left(const Map<TKey, TValue>& m, u32 n)
	{
		CE_ASSERT(n < vector::size(m._data), "Index out of bounds (size = %d, n = %d)", vector::size(m._data), n);
		return m._data[n].left;
	}

	template <typename TKey, typename TValue>
	inline u32 right(const Map<TKey, TValue>& m, u32 n)
	{
		CE_ASSERT(n < vector::size(m._data), "Index out of bounds (size = %d, n = %d)", vector::size(m._data), n);
		return m._data[n].right;
	}

	template <typename TKey, typename TValue>
	inline u32 color(const Map<TKey, TValue>& m, u32 n)
	{
		CE_ASSERT(n < vector::size(m._data), "Index out of bounds (size = %d, n = %d)", vector::size(m._data), n);
		return m._data[n].color;
	}

	#ifdef RBTREE_VERIFY
	template<typename TKey, typename TValue>
	inline s32 dbg_verify(Map<TKey, TValue>& m, u32 n)
	{
		if (n == m._sentinel)
		{
			return 0;
		}

		if (left(m, n) != m._sentinel)
		{
			CE_ASSERT(parent(m, left(m, n)) == n, "Bad RBTree");
			CE_ASSERT(m._data[left(m, n)].pair.first < m._data[n].pair.first, "Bad RBTree");
		}

		if (right(m, n) != m._sentinel)
		{
			CE_ASSERT(parent(m, right(m, n)) == n, "Bad RBTree");
			CE_ASSERT(m._data[n].pair.first < m._data[right(m, n)].pair.first, "Bad RBTree");
		}

		s32 bhL = dbg_verify(m, left(m, n));
		s32 bhR = dbg_verify(m, right(m, n));
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
	inline s32 dump(Map<TKey, TValue>& m)
	{
		for (u32 i = 0; i < vector::size(m._data); i++)
		{
			printf("%d = [%d, %d, %d] ", i, parent(m, i), left(m, i), right(m, i));
		}
		printf("\n");
		return 0;
	}
	#endif

	template <typename TKey, typename TValue>
	inline u32 min(const Map<TKey, TValue>& m, u32 x)
	{
		if (x == m._sentinel)
		{
			return x;
		}

		while (left(m, x) != m._sentinel)
		{
			x = left(m, x);
		}

		return x;
	}

	template <typename TKey, typename TValue>
	inline u32 max(const Map<TKey, TValue>& m, u32 x)
	{
		if (x == m._sentinel)
		{
			return x;
		}

		while (right(m, x) != m._sentinel)
		{
			x = right(m, x);
		}

		return x;
	}

	template <typename TKey, typename TValue>
	inline u32 successor(const Map<TKey, TValue>& m, u32 x)
	{
		if (right(m, x) != m._sentinel)
		{
			return min(m, right(m, x));
		}

		u32 y = parent(m, x);

		while (y != NIL && x == right(m, y))
		{
			x = y;
			y = parent(m, y);
		}

		return y;
	}

	template <typename TKey, typename TValue>
	inline u32 predecessor(const Map<TKey, TValue>& m, u32 x)
	{
		if (left(m, x) != m._sentinel)
		{
			return max(m, left(m, x));
		}

		u32 y = parent(m, x);

		while (y != NIL && x == left(m, y))
		{
			x = y;
			y = parent(m, y);
		}

		return y;
	}

	template <typename TKey, typename TValue>
	inline void rotate_left(Map<TKey, TValue>& m, u32 x)
	{
		CE_ASSERT(x < vector::size(m._data), "Index out of bounds (size = %d, n = %d)", vector::size(m._data), x);

		u32 y = right(m, x);
		m._data[x].right = left(m, y);

		if (left(m, y) != m._sentinel)
		{
			m._data[left(m, y)].parent = x;
		}

		m._data[y].parent = parent(m, x);

		if (parent(m, x) == NIL)
		{
			m._root = y;
		}
		else
		{
			if (x == left(m, parent(m, x)))
			{
				m._data[parent(m, x)].left = y;
			}
			else
			{
				m._data[parent(m, x)].right = y;
			}
		}

		m._data[y].left = x;
		m._data[x].parent = y;
	}

	template <typename TKey, typename TValue>
	inline void rotate_right(Map<TKey, TValue>& m, u32 x)
	{
		CE_ASSERT(x < vector::size(m._data), "Index out of bounds (size = %d, n = %d)", vector::size(m._data), x);

		u32 y = left(m, x);
		m._data[x].left = right(m, y);

		if (right(m, y) != m._sentinel)
		{
			m._data[right(m, y)].parent = x;
		}

		m._data[y].parent = parent(m, x);

		if (parent(m, x) == NIL)
		{
			m._root = y;
		}
		else
		{
			if (x == left(m, parent(m, x)))
			{
				m._data[parent(m, x)].left = y;
			}
			else
			{
				m._data[parent(m, x)].right = y;
			}
		}

		m._data[y].right = x;
		m._data[x].parent = y;
	}

	template <typename TKey, typename TValue>
	inline void destroy(Map<TKey, TValue>& m, u32 n)
	{
		CE_ASSERT(n < vector::size(m._data), "Index out of bounds (size = %d, n = %d)", vector::size(m._data), n);

		u32 x = vector::size(m._data) - 1;

		if (x == m._root)
		{
			m._root = n;

			if (left(m, x) != NIL)
				m._data[left(m, x)].parent = n;
			if (right(m, x) != NIL)
				m._data[right(m, x)].parent = n;

			m._data[n] = m._data[x];
		}
		else
		{
			if (x != n)
			{
				if (x == left(m, parent(m, x)))
				{
					m._data[parent(m, x)].left = n;
				}
				else if (x == right(m, parent(m, x)))
				{
					m._data[parent(m, x)].right = n;
				}

				if (left(m, x) != NIL)
					m._data[left(m, x)].parent = n;
				if (right(m, x) != NIL)
					m._data[right(m, x)].parent = n;

				m._data[n] = m._data[x];
			}
		}

		#ifdef RBTREE_VERIFY
			dbg_verify(m, m._root);
		#endif

		vector::pop_back(m._data);
	}

	template <typename TKey, typename TValue>
	inline void insert_fixup(Map<TKey, TValue>& m, u32 n)
	{
		CE_ASSERT(n < vector::size(m._data), "Index out of bounds (size = %d, n = %d)", vector::size(m._data), n);

		u32 x;
		u32 y;

		while (n != root(m) && color(m, parent(m, n)) == RED)
		{
			x = parent(m, n);

			if (x == left(m, parent(m, x)))
			{
				y = right(m, parent(m, x));

				if (color(m, y) == RED)
				{
					m._data[x].color = BLACK;
					m._data[y].color = BLACK;
					m._data[parent(m, x)].color = RED;
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

					m._data[x].color = BLACK;
					m._data[parent(m, x)].color = RED;
					rotate_right(m, parent(m, x));
				}
			}
			else
			{
				y = left(m, parent(m, x));

				if (color(m, y) == RED)
				{
					m._data[x].color = BLACK;
					m._data[y].color = BLACK;
					m._data[parent(m, x)].color = RED;
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

					m._data[x].color = BLACK;
					m._data[parent(m, x)].color = RED;
					rotate_left(m, parent(m, x));
				}
			}
		}
	}

	template <typename TKey, typename TValue>
	inline u32 inner_find(const Map<TKey, TValue>& m, const TKey& key)
	{
		u32 x = m._root;

		while (x != m._sentinel)
		{
			if (m._data[x].pair.first < key)
			{
				if (right(m, x) == m._sentinel)
				{
					return x;
				}

				x = right(m, x);
			}
			else if (key < m._data[x].pair.first)
			{
				if (left(m, x) == m._sentinel)
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
	inline u32 find_or_fail(const Map<TKey, TValue>& m, const TKey& key)
	{
		u32 p = inner_find(m, key);

		if (p != m._sentinel && m._data[p].pair.first == key)
			return p;

		return NIL;
	}

	template <typename TKey, typename TValue>
	inline u32 find_or_add(Map<TKey, TValue>& m, const TKey& key)
	{
		u32 p = inner_find(m, key);

		if (p != m._sentinel && m._data[p].pair.first == key)
		{
			printf("Found\n");
			return p;
		}

		printf("Not found, adding...\n");

		typename Map<TKey, TValue>::Node n;
		n.key = key;
		n.value = TValue();
		n.color = RED;
		n.left = m._sentinel;
		n.right = m._sentinel;
		n.parent = NIL;

		if (p == m._sentinel)
		{
			m._root = n;
		}
		else
		{
			if (key < m._data[p].pair.first)
			{
				m._data[p].left = n;
			}
			else
			{
				m._data[p].right = n;
			}

			m._data[n].parent = p;
		}

		add_fixup(m, n);
		m._data[m._root].color = BLACK;
		#ifdef RBTREE_VERIFY
			dbg_verify(m, m._root);
		#endif
		return n;
	}
} // namespace map_internal

namespace map
{
	template <typename TKey, typename TValue>
	u32 size(const Map<TKey, TValue>& m)
	{
		CE_ASSERT(vector::size(m._data) > 0, "Bad Map"); // There should be at least sentinel
		return vector::size(m._data) - 1;
	}

	template <typename TKey, typename TValue>
	inline bool has(const Map<TKey, TValue>& m, const TKey& key)
	{
		return map_internal::find_or_fail(m, key) != map_internal::NIL;
	}

	template <typename TKey, typename TValue>
	inline const TValue& get(const Map<TKey, TValue>& m, const TKey& key, const TValue& deffault)
	{
		u32 p = map_internal::inner_find(m, key);

		if (p != m._sentinel && m._data[p].pair.first == key)
		{
			return m._data[p].pair.second;
		}

		return deffault;
	}

	template <typename TKey, typename TValue>
	inline void set(Map<TKey, TValue>& m, const TKey& key, const TValue& value)
	{
		typename Map<TKey, TValue>::Node node(*m._data._allocator);
		node.pair.first = key;
		node.pair.second = value;
		node.color = map_internal::RED;
		node.left = m._sentinel;
		node.right = m._sentinel;
		node.parent = map_internal::NIL;
		u32 n = vector::push_back(m._data, node);
		u32 x = m._root;
		u32 y = map_internal::NIL;

		if (x == m._sentinel)
			m._root = n;
		else
		{
			while (x != m._sentinel)
			{
				y = x;

				if (key < m._data[x].pair.first)
					x = m._data[x].left;
				else
					x = m._data[x].right;
			}

			if (key < m._data[y].pair.first)
				m._data[y].left = n;
			else
				m._data[y].right = n;

			m._data[n].parent = y;
		}

		map_internal::insert_fixup(m, n);
		m._data[m._root].color = map_internal::BLACK;
		#ifdef RBTREE_VERIFY
			map_internal::dbg_verify(m, m._root);
		#endif
	}

	template <typename TKey, typename TValue>
	inline void remove(Map<TKey, TValue>& m, const TKey& key)
	{
		using namespace map_internal;

		u32 n = inner_find(m, key);

		if (!(m._data[n].pair.first == key))
		{
			return;
		}

		u32 x;
		u32 y;

		if (left(m, n) == m._sentinel || right(m, n) == m._sentinel)
		{
			y = n;
		}
		else
		{
			y = successor(m, n);
		}

		if (left(m, y) != m._sentinel)
		{
			x = left(m, y);
		}
		else
		{
			x = right(m, y);
		}

		m._data[x].parent = parent(m, y);

		if (parent(m, y) != map_internal::NIL)
		{
			if (y == left(m, parent(m, y)))
			{
				m._data[parent(m, y)].left = x;
			}
			else
			{
				m._data[parent(m, y)].right = x;
			}
		}
		else
		{
			m._root = x;
		}

		if (y != n)
		{
			m._data[n].pair.first = m._data[y].pair.first;
			m._data[n].pair.second = m._data[y].pair.second;
		}

		// Do the fixup
		if (color(m, y) == map_internal::BLACK)
		{
			u32 y;

			while (x != m._root && color(m, x) == map_internal::BLACK)
			{
				if (x == left(m, parent(m, x)))
				{
					y = right(m, parent(m, x));

					if (color(m, y) == map_internal::RED)
					{
						m._data[y].color = map_internal::BLACK;
						m._data[parent(m, x)].color = map_internal::RED;
						rotate_left(m, parent(m, x));
						y = right(m, parent(m, x));
					}

					if (color(m, left(m, y)) == map_internal::BLACK && color(m, right(m, y)) == map_internal::BLACK)
					{
						m._data[y].color = map_internal::RED;
						x = parent(m, x);
					}
					else
					{
						if (color(m, right(m, y)) == map_internal::BLACK)
						{
							m._data[left(m, y)].color = map_internal::BLACK;
							m._data[y].color = map_internal::RED;
							rotate_right(m, y);
							y = right(m, parent(m, x));
						}

						m._data[y].color = color(m, parent(m, x));
						m._data[parent(m, x)].color = map_internal::BLACK;
						m._data[right(m, y)].color = map_internal::BLACK;
						rotate_left(m, parent(m, x));
						x = m._root;
					}
				}
				else
				{
					y = left(m, parent(m, x));

					if (color(m, y) == map_internal::RED)
					{
						m._data[y].color = map_internal::BLACK;
						m._data[parent(m, x)].color = map_internal::RED;
						rotate_right(m, parent(m, x));
						y = left(m, parent(m, x));
					}

					if (color(m, right(m, y)) == map_internal::BLACK && color(m, left(m, y)) == map_internal::BLACK)
					{
						m._data[y].color = map_internal::RED;
						x = parent(m, x);
					}
					else
					{
						if (color(m, left(m, y)) == map_internal::BLACK)
						{
							m._data[right(m, y)].color = map_internal::BLACK;
							m._data[y].color = map_internal::RED;
							rotate_left(m, y);
							y = left(m, parent(m, x));
						}

						m._data[y].color = color(m, parent(m, x));
						m._data[parent(m, x)].color = map_internal::BLACK;
						m._data[left(m, y)].color = map_internal::BLACK;
						rotate_right(m, parent(m, x));
						x = m._root;
					}
				}
			}

			m._data[x].color = map_internal::BLACK;
		}

		destroy(m, y);
	 	#ifdef RBTREE_VERIFY
			map_internal::dbg_verify(m, m._root);
	 	#endif
	}

	template <typename TKey, typename TValue>
	void clear(Map<TKey, TValue>& m)
	{
		vector::clear(m._data);

		m._root = 0;
		m._sentinel = 0;

		typename Map<TKey, TValue>::Node r(*m._data._allocator);
		r.left = map_internal::NIL;
		r.right = map_internal::NIL;
		r.parent = map_internal::NIL;
		r.color = map_internal::BLACK;
		vector::push_back(m._data, r);
	}

	template <typename TKey, typename TValue>
	const typename Map<TKey, TValue>::Node* begin(const Map<TKey, TValue>& m)
	{
		return vector::begin(m._data) + 1; // Skip sentinel at index 0
	}

	template <typename TKey, typename TValue>
	const typename Map<TKey, TValue>::Node* end(const Map<TKey, TValue>& m)
	{
		return vector::end(m._data);
	}
} // namespace map

template <typename TKey, typename TValue>
inline Map<TKey, TValue>::Map(Allocator& a)
	: _data(a)
{
	map::clear(*this);
}

template <typename TKey, typename TValue>
inline const TValue& Map<TKey, TValue>::operator[](const TKey& key) const
{
	return map::get(*this, key, TValue());
}

} // namespace crown
