/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public enum ActionType
{
	CREATE_OBJECTS,
	DESTROY_OBJECTS,
	CHANGE_OBJECTS,

	COUNT
}

public const string ActionNames[] =
{
	N_("Create Objects"),
	N_("Destroy Objects"),
	N_("Change Objects"),
};

public enum ActionTypeFlags
{
	FROM_SERVER = 0x1
}

} /* namespace Crown */
