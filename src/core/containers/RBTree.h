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
#include <cassert>
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

	Pair& Add(const TKey& key, const TValue& value);
	void Remove(const TKey& key);
	bool Contains(const TKey& key) const;
	void Clear();

	inline int GetSize() const
	{
		return mSize;
	}

protected:

	Node* FindOrAdd(TKey key);

private:
	Node* mRoot;
	Node* mSentinel;
	int mSize;

	Node* GetPredecessor(Node* n) const;
	Node* GetSuccessor(Node* n) const;
	Node* GetMin(Node* n) const;
	Node* GetMax(Node* n) const;
	inline void RotateLeft(Node* n);
	inline void RotateRight(Node* n);

	Node* InnerFind(TKey key) const;
	void AddFixup(Node* n);
	void InnerClear(Node* n);

#ifdef RBTREE_VERIFY
	int dbgVerify(Node* n) const;
#endif
};

template<typename TKey, typename TValue>
RBTree<TKey, TValue>::RBTree()
{
	mSentinel = new Node(TKey(), TValue());
	mRoot = mSentinel;
	mSize = 0;
}

template<typename TKey, typename TValue>
RBTree<TKey, TValue>::~RBTree()
{
	Clear();
	delete mSentinel;
}

template<typename TKey, typename TValue>
RBTreePair<TKey, TValue>& RBTree<TKey, TValue>::Add(const TKey& key, const TValue& value)
{
	Node* n = new Node(key, value);
	n->color = RED;
	n->left = mSentinel;
	n->right = mSentinel;
	Pair& pair = n->item;
	Node* x = mRoot;
	Node* y = NULL;

	if (x == mSentinel)
	{
		mRoot = n;
	}
	else
	{
		while (x != mSentinel)
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

	AddFixup(n);
	mRoot->color = BLACK;
	mSize++;
#ifdef RBTREE_VERIFY
	dbgVerify(mRoot);
#endif
	return pair;
}

template<typename TKey, typename TValue>
void RBTree<TKey, TValue>::Remove(const TKey& key)
{
	Node* n = InnerFind(key);

	if (!(n->item.key == key))
	{
		return;
	}

	Node* x;
	Node* y;

	if (n->left == mSentinel || n->right == mSentinel)
	{
		y = n;
	}
	else
	{
		y = GetSuccessor(n);
	}

	if (y->left != mSentinel)
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
		mRoot = x;
	}

	if (y != n)
	{
		n->item = y->item;
	}

	//Do the fixup
	if (y->color == BLACK)
	{
		Node* y;

		while (x != mRoot && x->color == BLACK)
		{
			if (x == x->parent->left)
			{
				y = x->parent->right;

				if (y->color == RED)
				{
					y->color = BLACK;
					x->parent->color = RED;
					RotateLeft(x->parent);
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
						RotateRight(y);
						y = x->parent->right;
					}

					y->color = x->parent->color;
					x->parent->color = BLACK;
					y->right->color = BLACK;
					RotateLeft(x->parent);
					x = mRoot;
				}
			}
			else
			{
				y = x->parent->left;

				if (y->color == RED)
				{
					y->color = BLACK;
					x->parent->color = RED;
					RotateRight(x->parent);
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
						RotateLeft(y);
						y = x->parent->left;
					}

					y->color = x->parent->color;
					x->parent->color = BLACK;
					y->left->color = BLACK;
					RotateRight(x->parent);
					x = mRoot;
				}
			}
		}

		x->color = BLACK;
	}

	delete y;
	mSize -= 1;
#ifdef RBTREE_VERIFY
	dbgVerify(mRoot);
#endif
}

template<typename TKey, typename TValue>
RBTreeNode<TKey, TValue>* RBTree<TKey, TValue>::FindOrAdd(TKey key)
{
	Node* p = InnerFind(key);

	if (p != mSentinel && p->item.key == key)
	{
		return p;
	}

	Node* n = new Node(key, TValue());
	n->color = RED;
	n->left = mSentinel;
	n->right = mSentinel;

	if (p == mSentinel)
	{
		mRoot = n;
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

	AddFixup(n);
	mRoot->color = BLACK;
	mSize++;
#ifdef RBTREE_VERIFY
	dbgVerify(mRoot);
#endif
	return n;
}

template<typename TKey, typename TValue>
void RBTree<TKey, TValue>::Clear()
{
	Node* tmp = mRoot;
	mRoot = mSentinel;
	InnerClear(tmp);

	mSize = 0;
}

template<typename TKey, typename TValue>
bool RBTree<TKey, TValue>::Contains(const TKey& key) const
{
	Node* n = InnerFind(key);

	if (n == mSentinel || !(n->item.key == key))
	{
		return false;
	}

	return true;
}

/* Inner utilities */
template<typename TKey, typename TValue>
RBTreeNode<TKey, TValue>* RBTree<TKey, TValue>::InnerFind(TKey key) const
{
	Node* x = mRoot;

	while (x != mSentinel)
	{
		if (key > x->item.key)
		{
			if (x->right == mSentinel)
			{
				return x;
			}

			x = x->right;
		}
		else if (key < x->item.key)
		{
			if (x->left == mSentinel)
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
void RBTree<TKey, TValue>::AddFixup(Node* n)
{
	Node* x;
	Node* y;

	while (n!=mRoot && n->parent->color==RED)
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
					RotateLeft(n);
					x = n->parent;
				}

				x->color = BLACK;
				x->parent->color = RED;
				RotateRight(x->parent);
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
					RotateRight(n);
					x = n->parent;
				}

				x->color = BLACK;
				x->parent->color = RED;
				RotateLeft(x->parent);
			}
		}
	}
}

template<typename TKey, typename TValue>
void RBTree<TKey, TValue>::InnerClear(Node* n)
{
	if (n == mSentinel)
	{
		return;
	}

	Node* tmp;
	
	tmp = n->left;
	n->left = NULL;
	InnerClear(tmp);

	tmp = n->right;
	n->right = NULL;
	InnerClear(tmp);

	delete n;
}

template<typename TKey, typename TValue>
RBTreeNode<TKey, TValue>* RBTree<TKey, TValue>::GetPredecessor(Node* x) const
{
	if (x->left != mSentinel)
	{
		return GetMax(x->left);
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
RBTreeNode<TKey, TValue>* RBTree<TKey, TValue>::GetSuccessor(Node* x) const
{
	if (x->right != mSentinel)
	{
		return GetMin(x->right);
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
RBTreeNode<TKey, TValue>* RBTree<TKey, TValue>::GetMin(Node* x) const
{
	if (x == mSentinel)
	{
		return x;
	}

	while (x->left != mSentinel)
	{
		x = x->left;
	}

	return x;
}

template<typename TKey, typename TValue>
RBTreeNode<TKey, TValue>* RBTree<TKey, TValue>::GetMax(Node* x) const
{
	if (x == mSentinel)
	{
		return x;
	}

	while (x->right != mSentinel)
	{
		x = x->right;
	}

	return x;
}

template<typename TKey, typename TValue>
inline void RBTree<TKey, TValue>::RotateLeft(Node* x)
{
	Node* y = x->right;
	x->right = y->left;

	if (y->left != mSentinel)
	{
		y->left->parent = x;
	}

	y->parent = x->parent;

	if (x->parent == NULL)
	{
		mRoot = y;
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
inline void RBTree<TKey, TValue>::RotateRight(Node* x)
{
	Node* y = x->left;
	x->left = y->right;

	if (y->right != mSentinel)
	{
		y->right->parent = x;
	}

	y->parent = x->parent;

	if (x->parent == NULL)
	{
		mRoot = y;
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
int RBTree<TKey, TValue>::dbgVerify(Node* n) const
{
	if (n == mSentinel)
	{
		return 0;
	}

	if (n->left != mSentinel)
	{
		assert(n->left->parent == n);
		assert(n->item.key > n->left->item.key);
	}

	if (n->right != mSentinel)
	{
		assert(n->right->parent == n);
		assert(n->item.key < n->right->item.key);
	}

	int bhL = dbgVerify(n->left);
	int bhR = dbgVerify(n->right);
	assert(bhL == bhR);

	if (n->color == BLACK)
	{
		bhL += 1;
	}
	else
	{
		if (n->parent != NULL && n->parent->color == RED)
		{
			assert(false);
		}
	}

	return bhL;
}
#endif

} // namespace crown

