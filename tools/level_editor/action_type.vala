/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public enum ActionType
{
	CREATE_OBJECTS,
	DESTROY_OBJECTS,
	CHANGE_OBJECTS,
	OBJECT_SET_EDITOR_NAME,

	COUNT
}

public const string ActionNames[] =
{
	"Create Objects",
	"Destroy Objects",
	"Change Objects",
	"Set Object Name",
};

public enum ActionTypeFlags
{
	FROM_SERVER = 0x1
}

} /* namespace Crown */
