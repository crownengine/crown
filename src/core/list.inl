/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/list.h"

namespace crown
{
#define LIST_INIT_HEAD(head) { &(head), &(head) }

namespace list
{
	inline void init_head(ListNode &head)
	{
		head.next = &head;
		head.prev = &head;
	}

	inline void insert(ListNode &node, ListNode &prev, ListNode &next)
	{
		// Kernel
		next.prev = &node;
		node.next = &next;
		node.prev = &prev;
		prev.next = &node;
	}

	inline void add(ListNode &node, ListNode &head)
	{
		insert(node, head, *head.next);
	}

	inline void remove(ListNode &node)
	{
		node.prev->next = node.next;
		node.next->prev = node.prev;
	}

} // namespace list

} // namespace crown

#define list_for_each(cur, head) \
	for (cur = (head)->next; cur != (head); cur = cur->next)

#define list_for_each_safe(cur, tmp, head)    \
	for ( cur = (head)->next, tmp = cur->next \
		; cur != (head)                       \
		; cur = tmp, tmp = cur->next          \
		)
