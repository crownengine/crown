/*
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

#include <cstdlib>
#include "Assert.h"
#include "Types.h"

namespace crown
{

enum RBTreeNodeColor { RED, BLACK };

template<typename TKey, typename TValue>
struct RBTreePair
{
public:
	RBTreePair(const TKey& k, const TValue& v):
		key(k), value(v)
	{ }

	RBTreePair(const RBTreePair& p):
		key(p.key), value(p.value)
	{ }

	TKey key;
	TValue value;
};

template<typename TKey, typename TValue>
struct RBTreeNode
{
	RBTreeNode(const TKey& key, const TValue& value):
		item(key, value)
	{
		left = NULL;
		right = NULL;
		parent = NULL;
		color = BLACK;
	}

	RBTreePair<TKey, TValue> item;
	struct RBTreeNode<TKey, TValue>* left;
	struct RBTreeNode<TKey, TValue>* right;
	struct RBTreeNode<TKey, TValue>* parent;
	enum RBTreeNodeColor color;
};

template<typename TKey, typename TValue>
class RBTree
{
protected:
	typedef RBTreeNode<TKey, TValue> Node;
	typedef RBTreePair<TKey, TValue> Pair;

public:
	RBTree();
	~RBTree();

	Pair& add(const TKey& key, const TValue& value);
	void remove(const TKey& key);
	bool contains(const TKey& key) const;
	void clear();

	inline int32_t size() const
	{
		return m_size;
	}

protected:

	Node* find_or_add(TKey key);

private:
	Node* m_root;
	Node* m_sentinel;
	int32_t m_size;

	Node* predecessor(Node* n) const;
	Node* successor(Node* n) const;
	Node* min(Node* n) const;
	Node* max(Node* n) const;
	inline void rotate_left(Node* n);
	inline void rotate_right(Node* n);

	Node* inner_find(TKey key) const;
	void add_fixup(Node* n);
	void inner_clear(Node* n);

#ifdef RBTREE_VERIFY
	int32_t dbg_verify(Node* n) const;
#endif
};

template<typename TKey, typename TValue>
RBTree<TKey, TValue>::RBTree()
{
	m_sentinel = new Node(TKey(), TValue());
	m_root = m_sentinel;
	m_size = 0;
}

template<typename TKey, typename TValue>
RBTree<TKey, TValue>::~RBTree()
{
	clear();
	delete m_sentinel;
}

template<typename TKey, typename TValue>
RBTreePair<TKey, TValue>& RBTree<TKey, TValue>::add(const TKey& key, const TValue& value)
{
	Node* n = new Node(key, value);
	n->color = RED;
	n->left = m_sentinel;
	n->right = m_sentinel;
	Pair& pair = n->item;
	Node* x = m_root;
	Node* y = NULL;

	if (x == m_sentinel)
	{
		m_root = n;
	}
	else
	{
		while (x != m_sentinel)
		{
			y = x;

			if (key < x->item.key)
			{
				x = x->left;
			}
			else
			{
				x = x->right;
			}
		}

		if (key < y->item.key)
		{
			y->left = n;
		}
		else
		{
			y->right = n;
		}

		n->parent = y;
	}

	add_fixup(n);
	m_root->color = BLACK;
	m_size++;
#ifdef RBTREE_VERIFY
	dbg_verify(m_root);
#endif
	return pair;
}

template<typename TKey, typename TValue>
void RBTree<TKey, TValue>::remove(const TKey& key)
{
	Node* n = inner_find(key);

	if (!(n->item.key == key))
	{
		return;
	}

	Node* x;
	Node* y;

	if (n->left == m_sentinel || n->right == m_sentinel)
	{
		y = n;
	}
	else
	{
		y = successor(n);
	}

	if (y->left != m_sentinel)
	{
		x = y->left;
	}
	else
	{
		x = y->right;
	}

	x->parent = y->parent;

	if (y->parent != NULL)
	{
		if (y == y->parent->left)
		{
			y->parent->left = x;
		}
		else
		{
			y->parent->right = x;
		}
	}
	else
	{
		m_root = x;
	}

	if (y != n)
	{
		n->item = y->item;
	}

	//Do the fixup
	if (y->color == BLACK)
	{
		Node* y;

		while (x != m_root && x->color == BLACK)
		{
			if (x == x->parent->left)
			{
				y = x->parent->right;

				if (y->color == RED)
				{
					y->color = BLACK;
					x->parent->color = RED;
					rotate_left(x->parent);
					y = x->parent->right;
				}

				if (y->left->color == BLACK && y->right->color == BLACK)
				{
					y->color = RED;
					x = x->parent;
				}
				else
				{
					if (y->right->color == BLACK)
					{
						y->left->color = BLACK;
						y->color = RED;
						rotate_right(y);
						y = x->parent->right;
					}

					y->color = x->parent->color;
					x->parent->color = BLACK;
					y->right->color = BLACK;
					rotate_left(x->parent);
					x = m_root;
				}
			}
			else
			{
				y = x->parent->left;

				if (y->color == RED)
				{
					y->color = BLACK;
					x->parent->color = RED;
					rotate_right(x->parent);
					y = x->parent->left;
				}

				if (y->right->color == BLACK && y->left->color == BLACK)
				{
					y->color = RED;
					x = x->parent;
				}
				else
				{
					if (y->left->color == BLACK)
					{
						y->right->color = BLACK;
						y->color = RED;
						rotate_left(y);
						y = x->parent->left;
					}

					y->color = x->parent->color;
					x->parent->color = BLACK;
					y->left->color = BLACK;
					rotate_right(x->parent);
					x = m_root;
				}
			}
		}

		x->color = BLACK;
	}

	delete y;
	m_size -= 1;
#ifdef RBTREE_VERIFY
	dbg_verify(m_root);
#endif
}

template<typename TKey, typename TValue>
RBTreeNode<TKey, TValue>* RBTree<TKey, TValue>::find_or_add(TKey key)
{
	Node* p = inner_find(key);

	if (p != m_sentinel && p->item.key == key)
	{
		return p;
	}

	Node* n = new Node(key, TValue());
	n->color = RED;
	n->left = m_sentinel;
	n->right = m_sentinel;

	if (p == m_sentinel)
	{
		m_root = n;
	}
	else
	{
		if (key < p->item.key)
		{
			p->left = n;
		}
		else
		{
			p->right = n;
		}

		n->parent = p;
	}

	add_fixup(n);
	m_root->color = BLACK;
	m_size++;
#ifdef RBTREE_VERIFY
	dbg_verify(m_root);
#endif
	return n;
}

template<typename TKey, typename TValue>
void RBTree<TKey, TValue>::clear()
{
	Node* tmp = m_root;
	m_root = m_sentinel;
	inner_clear(tmp);

	m_size = 0;
}

template<typename TKey, typename TValue>
bool RBTree<TKey, TValue>::contains(const TKey& key) const
{
	Node* n = inner_find(key);

	if (n == m_sentinel || !(n->item.key == key))
	{
		return false;
	}

	return true;
}

/* Inner utilities */
template<typename TKey, typename TValue>
RBTreeNode<TKey, TValue>* RBTree<TKey, TValue>::inner_find(TKey key) const
{
	Node* x = m_root;

	while (x != m_sentinel)
	{
		if (key > x->item.key)
		{
			if (x->right == m_sentinel)
			{
				return x;
			}

			x = x->right;
		}
		else if (key < x->item.key)
		{
			if (x->left == m_sentinel)
			{
				return x;
			}

			x = x->left;
		}
		else
		{
			break;
		}
	}

	return x;
}

template<typename TKey, typename TValue>
void RBTree<TKey, TValue>::add_fixup(Node* n)
{
	Node* x;
	Node* y;

	while (n!=m_root && n->parent->color==RED)
	{
		x = n->parent;

		if (x == x->parent->left)
		{
			y = x->parent->right;

			if (y->color == RED)
			{
				x->color = BLACK;
				y->color = BLACK;
				x->parent->color = RED;
				n = x->parent;
				continue;
			}
			else
			{
				if (n == x->right)
				{
					n = x;
					rotate_left(n);
					x = n->parent;
				}

				x->color = BLACK;
				x->parent->color = RED;
				rotate_right(x->parent);
			}
		}
		else
		{
			y = x->parent->left;

			if (y->color == RED)
			{
				x->color = BLACK;
				y->color = BLACK;
				x->parent->color = RED;
				n = x->parent;
				continue;
			}
			else
			{
				if (n == x->left)
				{
					n = x;
					rotate_right(n);
					x = n->parent;
				}

				x->color = BLACK;
				x->parent->color = RED;
				rotate_left(x->parent);
			}
		}
	}
}

template<typename TKey, typename TValue>
void RBTree<TKey, TValue>::inner_clear(Node* n)
{
	if (n == m_sentinel)
	{
		return;
	}

	Node* tmp;
	
	tmp = n->left;
	n->left = NULL;
	inner_clear(tmp);

	tmp = n->right;
	n->right = NULL;
	inner_clear(tmp);

	delete n;
}

template<typename TKey, typename TValue>
RBTreeNode<TKey, TValue>* RBTree<TKey, TValue>::predecessor(Node* x) const
{
	if (x->left != m_sentinel)
	{
		return max(x->left);
	}

	Node* y = x->parent;

	while (y != NULL && x == y->left)
	{
		x = y;
		y = y->parent;
	}

	return y;
}

template<typename TKey, typename TValue>
RBTreeNode<TKey, TValue>* RBTree<TKey, TValue>::successor(Node* x) const
{
	if (x->right != m_sentinel)
	{
		return min(x->right);
	}

	Node* y = x->parent;

	while (y != NULL && x == y->right)
	{
		x = y;
		y = y->parent;
	}

	return y;
}

template<typename TKey, typename TValue>
RBTreeNode<TKey, TValue>* RBTree<TKey, TValue>::min(Node* x) const
{
	if (x == m_sentinel)
	{
		return x;
	}

	while (x->left != m_sentinel)
	{
		x = x->left;
	}

	return x;
}

template<typename TKey, typename TValue>
RBTreeNode<TKey, TValue>* RBTree<TKey, TValue>::max(Node* x) const
{
	if (x == m_sentinel)
	{
		return x;
	}

	while (x->right != m_sentinel)
	{
		x = x->right;
	}

	return x;
}

template<typename TKey, typename TValue>
inline void RBTree<TKey, TValue>::rotate_left(Node* x)
{
	Node* y = x->right;
	x->right = y->left;

	if (y->left != m_sentinel)
	{
		y->left->parent = x;
	}

	y->parent = x->parent;

	if (x->parent == NULL)
	{
		m_root = y;
	}
	else
	{
		if (x == x->parent->left)
		{
			x->parent->left = y;
		}
		else
		{
			x->parent->right = y;
		}
	}

	y->left = x;
	x->parent = y;
}

template<typename TKey, typename TValue>
inline void RBTree<TKey, TValue>::rotate_right(Node* x)
{
	Node* y = x->left;
	x->left = y->right;

	if (y->right != m_sentinel)
	{
		y->right->parent = x;
	}

	y->parent = x->parent;

	if (x->parent == NULL)
	{
		m_root = y;
	}
	else
	{
		if (x == x->parent->left)
		{
			x->parent->left = y;
		}
		else
		{
			x->parent->right = y;
		}
	}

	y->right = x;
	x->parent = y;
}

#ifdef RBTREE_VERIFY
template<typename TKey, typename TValue>
int32_t RBTree<TKey, TValue>::dbg_verify(Node* n) const
{
	if (n == m_sentinel)
	{
		return 0;
	}

	if (n->left != m_sentinel)
	{
		CE_ASSERT(n->left->parent == n);
		CE_ASSERT(n->item.key > n->left->item.key);
	}

	if (n->right != m_sentinel)
	{
		CE_ASSERT(n->right->parent == n);
		CE_ASSERT(n->item.key < n->right->item.key);
	}

	int32_t bhL = dbg_verify(n->left);
	int32_t bhR = dbg_verify(n->right);
	CE_ASSERT(bhL == bhR);

	if (n->color == BLACK)
	{
		bhL += 1;
	}
	else
	{
		if (n->parent != NULL && n->parent->color == RED)
		{
			CE_ASSERT(false);
		}
	}

	return bhL;
}
#endif

} // namespace crown

