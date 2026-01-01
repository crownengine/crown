/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

namespace crown
{
/// Node in an intrusive linked list.
///
/// @ingroup Containers
struct ListNode
{
	ListNode *next;
	ListNode *prev;
};

} // namespace crown
