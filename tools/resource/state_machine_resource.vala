/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public struct NodeAnimation
{
	public Database _db;
	public Guid _id;

	public NodeAnimation(Database db
		, Guid id
		, string name
		, string weight
		)
	{
		_db = db;
		_id = id;

		_db.create(_id, STRING_ID_64(OBJECT_TYPE_NODE_ANIMATION, 0xdfec9ab6472a94b9));
		_db.set_resource(_id, _db.property_index(_id, STRING_ID_64("name", 0xd4c943cba60c270b)), name);
		_db.set_string(_id, _db.property_index(_id, STRING_ID_64("weight", 0xcb065493e921ff4a)), weight);
	}
}

public struct StateMachineNode
{
	public Database _db;
	public Guid _id;

	public StateMachineNode(Database db, Guid id)
	{
		_db = db;
		_id = id;

		_db.create(_id, STRING_ID_64(OBJECT_TYPE_STATE_MACHINE_NODE, 0x20aedaf23037aaf8));
		_db.set_bool(_id, _db.property_index(_id, STRING_ID_64("loop", 0x8c9c410e83f48bce)), true);
		_db.set_string(_id, _db.property_index(_id, STRING_ID_64("speed", 0x2c1c82c87303ec5f)), "1");
		_db.create_empty_set(_id, _db.property_index(_id, STRING_ID_64("animations", 0x76ebb1c25f3c34c8)));
		_db.create_empty_set(_id, _db.property_index(_id, STRING_ID_64("transitions", 0x024d73d5d9042c38)));
	}

	public void add_animation(NodeAnimation anim)
	{
		_db.add_to_set(_id, _db.property_index(_id, STRING_ID_64("animations", 0x76ebb1c25f3c34c8)), anim._id);
	}
}

public struct StateMachineResource
{
	public Database _db;
	public Guid _id;

	public StateMachineResource(Database db
		, Guid id
		, string animation_type
		, string? initial_animation_name
		, string? skeleton_name
		)
	{
		_db = db;
		_id = id;

		Guid initial_state_id = Guid.new_guid();
		StateMachineNode initial_state = StateMachineNode(db, initial_state_id);

		if (initial_animation_name != null) {
			NodeAnimation na = NodeAnimation(db, Guid.new_guid(), initial_animation_name, "1");
			initial_state.add_animation(na);
		}

		_db.create(_id, STRING_ID_64(OBJECT_TYPE_STATE_MACHINE, 0xa486d4045106165c));
		add_node(initial_state);
		_db.set_reference(_id, _db.property_index(_id, STRING_ID_64("initial_state", 0x6a1f2de0faab3a44)), initial_state._id);
		_db.create_empty_set(_id, _db.property_index(_id, STRING_ID_64("variables", 0x4fb1ab3fd540bd03)));
		_db.set_string(_id, _db.property_index(_id, STRING_ID_64("animation_type", 0x241dedaa13a493d9)), animation_type);
		if (skeleton_name != null)
			_db.set_resource(_id, _db.property_index(_id, STRING_ID_64("skeleton_name", 0x533ec079537085ed)), skeleton_name);
	}

	public StateMachineResource.mesh(Database db
		, Guid id
		, string skeleton_name
		, string? initial_animation_name = null
		)
	{
		this(db, id, OBJECT_TYPE_MESH_ANIMATION, initial_animation_name, skeleton_name);
	}

	public StateMachineResource.sprite(Database db, Guid id, string? initial_animation_name)
	{
		this(db, id, OBJECT_TYPE_SPRITE_ANIMATION, initial_animation_name, null);
	}

	public void add_node(StateMachineNode node)
	{
		_db.add_to_set(_id, _db.property_index(_id, STRING_ID_64("states", 0x87c1f74888ad323c)), node._id);
	}

	public int save(Project project, string resource_name)
	{
		return _db.save(project.absolute_path(resource_name) + "." + OBJECT_TYPE_STATE_MACHINE, _id);
	}
}

} /* namespace Crown */
