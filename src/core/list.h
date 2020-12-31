/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
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

} // namespace crown
