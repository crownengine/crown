/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

namespace crown
{
/// Node in an intrusive linked list.
///
/// @ingroup Containers
struct ListNode
{
	ListNode* next;
	ListNode* prev;
};

namespace list
{
	inline void init_head(ListNode& head)
	{
		head.next = &head;
		head.prev = &head;
	}

	inline void insert(ListNode& node, ListNode& prev, ListNode& next)
	{
		// Kernel
		next.prev = &node;
		node.next = &next;
		node.prev = &prev;
		prev.next = &node;
	}

	inline void add(ListNode& node, ListNode& head)
	{
		insert(node, head, *head.next);
	}

	inline void remove(ListNode& node)
	{
		node.prev->next = node.next;
		node.next->prev = node.prev;
	}

} // namespace list

#define list_for_each(cur, head)                             \
	for (cur = (head)->next; cur != (head); cur = cur->next)

#define list_for_each_safe(cur, tmp, head)    \
	for ( cur = (head)->next, tmp = cur->next \
		; cur != (head)                       \
		; cur = tmp, tmp = cur->next          \
		)

} // namespace crown
