/*
 * Copyright (c) 2012-2024 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public enum ActionType
{
	SPAWN_UNIT,
	SPAWN_SOUND,
	DESTROY_OBJECTS,
	MOVE_OBJECTS,
	DUPLICATE_OBJECTS,
	OBJECT_SET_EDITOR_NAME,
	SET_TRANSFORM,
	SET_LIGHT,
	SET_MESH,
	SET_SPRITE,
	SET_CAMERA,
	SET_COLLIDER,
	SET_ACTOR,
	SET_SCRIPT,
	SET_ANIMATION_STATE_MACHINE,
	SET_SOUND
}

public const string ActionNames[] =
{
	"Spawn Unit",
	"Spawn Sound",
	"Destroy Objects",
	"Move Objects",
	"Duplicate Objects",
	"Set Object Name",
	"Set Transform Parameter",
	"Set Light Parameter",
	"Set Mesh Parameter",
	"Set Sprite Parameter",
	"Set Camera Parameter",
	"Set Collider Parameter",
	"Set Actor Parameter",
	"Set Script Parameter",
	"Set Animation State Machine Parameter",
	"Set Sound Parameter"
};

public enum ActionTypeFlags
{
	FROM_SERVER = 0x1
}

} /* namespace Crown */
