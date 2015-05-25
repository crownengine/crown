/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "container_types.h"
#include "vector.h"

#define RBTREE_VERIFY

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
	const uint32_t BLACK = 0xb1b1b1b1u;
	const uint32_t RED = 0xededededu;
	const uint32_t NIL = 0xffffffff;

	template <typename TKey, typename TValue>
	inline uint32_t root(const Map<TKey, TValue>& m)
	{
		return m._root;
	}

	template <typename TKey, typename TValue>
	inline uint32_t parent(const Map<TKey, TValue>& m, uint32_t n)
	{
		CE_ASSERT(n < vector::size(m._data), "Index out of bounds (size = %d, n = %d)", vector::size(m._data), n);
		return m._data[n].parent;
	}

	template <typename TKey, typename TValue>
	inline uint32_t left(const Map<TKey, TValue>& m, uint32_t n)
	{
		CE_ASSERT(n < vector::size(m._data), "Index out of bounds (size = %d, n = %d)", vector::size(m._data), n);
		return m._data[n].left;
	}

	template <typename TKey, typename TValue>
	inline uint32_t right(const Map<TKey, TValue>& m, uint32_t n)
	{
		CE_ASSERT(n < vector::size(m._data), "Index out of bounds (size = %d, n = %d)", vector::size(m._data), n);
		return m._data[n].right;
	}

	template <typename TKey, typename TValue>
	inline uint32_t color(const Map<TKey, TValue>& m, uint32_t n)
	{
		if (n == NIL) return BLACK;
		CE_ASSERT(n < vector::size(m._data), "Index out of bounds (size = %d, n = %d)", vector::size(m._data), n);
		return m._data[n].color;
	}

#ifdef RBTREE_VERIFY
	template<typename TKey, typename TValue>
	inline int32_t dbg_verify(Map<TKey, TValue>& m, uint32_t n)
	{
		if (n == NIL)
		{
			return 0;
		}

		if (left(m, n) != NIL)
		{
			CE_ASSERT(parent(m, left(m, n)) == n, "Bad RBTree");
			CE_ASSERT(m._data[left(m, n)].key < m._data[n].key, "Bad RBTree");
		}

		if (right(m, n) != NIL)
		{
			CE_ASSERT(parent(m, right(m, n)) == n, "Bad RBTree");
			CE_ASSERT(m._data[n].key < m._data[right(m, n)].key, "Bad RBTree");
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
		for (uint32_t i = 0; i < vector::size(m._data); i++)
		{
			printf("%d = [%d, %d, %d] ", i, parent(m, i), left(m, i), right(m, i));
		}
		printf("\n");
		return 0;
	}
#endif // RBTREE_VERIFY

	template <typename TKey, typename TValue>
	inline uint32_t min(const Map<TKey, TValue>& m, uint32_t x)
	{
		if (x == NIL)
			return x;

		while (left(m, x) != NIL)
			x = left(m, x);

		return x;
	}

	template <typename TKey, typename TValue>
	inline uint32_t max(const Map<TKey, TValue>& m, uint32_t x)
	{
		if (x == NIL)
			return x;

		while (right(m, x) != NIL)
			x = right(m, x);

		return x;
	}

	template <typename TKey, typename TValue>
	inline uint32_t successor(const Map<TKey, TValue>& m, uint32_t x)
	{
		if (right(m, x) != NIL)
			return min(m, right(m, x));

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
		if (left(m, x) != NIL)
			return max(m, left(m, x));

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
		CE_ASSERT(x < vector::size(m._data), "Index out of bounds (size = %d, n = %d)", vector::size(m._data), x);

		uint32_t y = right(m, x);
		m._data[x].right = left(m, y);

		if (left(m, y) != NIL)
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
	inline void rotate_right(Map<TKey, TValue>& m, uint32_t x)
	{
		CE_ASSERT(x < vector::size(m._data), "Index out of bounds (size = %d, n = %d)", vector::size(m._data), x);

		uint32_t y = left(m, x);
		m._data[x].left = right(m, y);

		if (right(m, y) != NIL)
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
	inline void destroy(Map<TKey, TValue>& m, uint32_t n)
	{
		CE_ASSERT(n < vector::size(m._data), "Index out of bounds (size = %d, n = %d)", vector::size(m._data), n);

		uint32_t x = vector::size(m._data) - 1;

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
	inline void insert_fixup(Map<TKey, TValue>& m, uint32_t n)
	{
		CE_ASSERT(n < vector::size(m._data), "Index out of bounds (size = %d, n = %d)", vector::size(m._data), n);

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
	inline uint32_t inner_find(const Map<TKey, TValue>& m, const TKey& key)
	{
		uint32_t x = m._root;

		while (x != NIL)
		{
			if (m._data[x].key < key)
			{
				if (right(m, x) == NIL)
					return x;

				x = right(m, x);
			}
			else if (key < m._data[x].key)
			{
				if (left(m, x) == NIL)
					return x;

				x = left(m, x);
			}
			else
				break;
		}

		return x;
	}

	template <typename TKey, typename TValue>
	inline uint32_t find_or_fail(const Map<TKey, TValue>& m, const TKey& key)
	{
		uint32_t p = inner_find(m, key);

		if (p != NIL && m._data[p].key == key)
			return p;

		return NIL;
	}

	template <typename TKey, typename TValue>
	inline uint32_t find_or_add(Map<TKey, TValue>& m, const TKey& key)
	{
		uint32_t p = inner_find(m, key);

		if (p != NIL && m._data[p].key == key)
			return p;

		typename Map<TKey, TValue>::Node n;
		n.key = key;
		n.value = TValue();
		n.color = RED;
		n.left = NIL;
		n.right = NIL;
		n.parent = NIL;

		if (p == NIL)
			m._root = n;
		else
		{
			if (key < m._data[p].key)
				m._data[p].left = n;
			else
				m._data[p].right = n;

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
	uint32_t size(const Map<TKey, TValue>& m)
	{
		return vector::size(m._data);
	}

	template <typename TKey, typename TValue>
	inline bool has(const Map<TKey, TValue>& m, const TKey& key)
	{
		return map_internal::find_or_fail(m, key) != map_internal::NIL;
	}

	template <typename TKey, typename TValue>
	inline const TValue& get(const Map<TKey, TValue>& m, const TKey& key, const TValue& deffault)
	{
		uint32_t p = map_internal::inner_find(m, key);

		if (p != map_internal::NIL && m._data[p].key == key)
			return m._data[p].value;

		return deffault;
	}

	template <typename TKey, typename TValue>
	inline void set(Map<TKey, TValue>& m, const TKey& key, const TValue& value)
	{
		using namespace map_internal;

		typename Map<TKey, TValue>::Node node;
		node.key = key;
		node.value = value;
		node.color = RED;
		node.left = NIL;
		node.right = NIL;
		node.parent = NIL;
		uint32_t n = vector::push_back(m._data, node);
		uint32_t x = m._root;
		uint32_t y = NIL;

		if (x == NIL)
			m._root = n;
		else
		{
			while (x != NIL)
			{
				y = x;

				if (key < m._data[x].key)
					x = m._data[x].left;
				else
					x = m._data[x].right;
			}

			if (key < m._data[y].key)
				m._data[y].left = n;
			else
				m._data[y].right = n;

			m._data[n].parent = y;
		}

		insert_fixup(m, n);
		m._data[m._root].color = BLACK;
#ifdef RBTREE_VERIFY
		dbg_verify(m, m._root);
#endif
	}

	template <typename TKey, typename TValue>
	inline void remove(Map<TKey, TValue>& m, const TKey& key)
	{
		using namespace map_internal;

		uint32_t n = inner_find(m, key);

		if (!(m._data[n].key == key))
			return;

		uint32_t x;
		uint32_t y;

		if (left(m, n) == NIL || right(m, n) == NIL)
			y = n;
		else
			y = successor(m, n);

		if (left(m, y) != NIL)
			x = left(m, y);
		else
			x = right(m, y);

		m._data[x].parent = parent(m, y);

		if (parent(m, y) != NIL)
		{
			if (y == left(m, parent(m, y)))
				m._data[parent(m, y)].left = x;
			else
				m._data[parent(m, y)].right = x;
		}
		else
			m._root = x;

		if (y != n)
		{
			m._data[n].key = m._data[y].key;
			m._data[n].value = m._data[y].value;
		}

		// Do the fixup
		if (color(m, y) == BLACK)
		{
			uint32_t y;

			while (x != m._root && color(m, x) == BLACK)
			{
				if (x == left(m, parent(m, x)))
				{
					y = right(m, parent(m, x));

					if (color(m, y) == RED)
					{
						m._data[y].color = BLACK;
						m._data[parent(m, x)].color = RED;
						rotate_left(m, parent(m, x));
						y = right(m, parent(m, x));
					}

					if (color(m, left(m, y)) == BLACK && color(m, right(m, y)) == BLACK)
					{
						m._data[y].color = RED;
						x = parent(m, x);
					}
					else
					{
						if (color(m, right(m, y)) == BLACK)
						{
							m._data[left(m, y)].color = BLACK;
							m._data[y].color = RED;
							rotate_right(m, y);
							y = right(m, parent(m, x));
						}

						m._data[y].color = color(m, parent(m, x));
						m._data[parent(m, x)].color = BLACK;
						m._data[right(m, y)].color = BLACK;
						rotate_left(m, parent(m, x));
						x = m._root;
					}
				}
				else
				{
					y = left(m, parent(m, x));

					if (color(m, y) == RED)
					{
						m._data[y].color = BLACK;
						m._data[parent(m, x)].color = RED;
						rotate_right(m, parent(m, x));
						y = left(m, parent(m, x));
					}

					if (color(m, right(m, y)) == BLACK && color(m, left(m, y)) == BLACK)
					{
						m._data[y].color = RED;
						x = parent(m, x);
					}
					else
					{
						if (color(m, left(m, y)) == BLACK)
						{
							m._data[right(m, y)].color = BLACK;
							m._data[y].color = RED;
							rotate_left(m, y);
							y = left(m, parent(m, x));
						}

						m._data[y].color = color(m, parent(m, x));
						m._data[parent(m, x)].color = BLACK;
						m._data[left(m, y)].color = BLACK;
						rotate_right(m, parent(m, x));
						x = m._root;
					}
				}
			}

			m._data[x].color = BLACK;
		}

		destroy(m, y);
#ifdef RBTREE_VERIFY
		dbg_verify(m, m._root);
#endif
	}

	template <typename TKey, typename TValue>
	void clear(Map<TKey, TValue>& m)
	{
		vector::clear(m._data);
		m._root = map_internal::NIL;
	}

	template <typename TKey, typename TValue>
	const typename Map<TKey, TValue>::Node* begin(const Map<TKey, TValue>& m)
	{
		return vector::begin(m._data);
	}

	template <typename TKey, typename TValue>
	const typename Map<TKey, TValue>::Node* end(const Map<TKey, TValue>& m)
	{
		return vector::end(m._data);
	}
} // namespace map

template <typename TKey, typename TValue>
inline Map<TKey, TValue>::Map(Allocator& a)
	: _root(map_internal::NIL)
	, _data(a)
{
}

template <typename TKey, typename TValue>
inline const TValue& Map<TKey, TValue>::operator[](const TKey& key) const
{
	return map::get(*this, key, TValue());
}

} // namespace crown
