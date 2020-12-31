/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

namespace Crown
{
public enum ActionType
{
	SPAWN_UNIT,
	DESTROY_UNIT,
	SPAWN_SOUND,
	DESTROY_SOUND,
	MOVE_OBJECTS,
	DUPLICATE_OBJECTS,
	OBJECT_SET_EDITOR_NAME,
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
	"Destroy Unit",
	"Spawn Sound",
	"Destroy Sound",
	"Move Objects",
	"Duplicate Objects",
	"Set Object Name",
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

}
