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

		_db.create(_id, OBJECT_TYPE_NODE_ANIMATION);
		_db.set_resource(_id, "name", name);
		_db.set_string(_id, "weight", weight);
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

		_db.create(_id, OBJECT_TYPE_STATE_MACHINE_NODE);
		_db.set_bool(_id, "loop", true);
		_db.set_string(_id, "speed", "1");
		_db.create_empty_set(_id, "animations");
		_db.create_empty_set(_id, "transitions");
	}

	public void add_animation(NodeAnimation anim)
	{
		_db.add_to_set(_id, "animations", anim._id);
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

		_db.create(_id, OBJECT_TYPE_STATE_MACHINE);
		add_node(initial_state);
		_db.set_reference(_id, "initial_state", initial_state._id);
		_db.create_empty_set(_id, "variables");
		_db.set_string(_id, "animation_type", animation_type);
		if (skeleton_name != null)
			_db.set_resource(_id, "skeleton_name", skeleton_name);
	}

	public StateMachineResource.mesh(Database db, Guid id, string skeleton_name)
	{
		this(db, id, OBJECT_TYPE_MESH_ANIMATION, null, skeleton_name);
	}

	public StateMachineResource.sprite(Database db, Guid id, string? initial_animation_name)
	{
		this(db, id, OBJECT_TYPE_SPRITE_ANIMATION, initial_animation_name, null);
	}

	public void add_node(StateMachineNode node)
	{
		_db.add_to_set(_id, "states", node._id);
	}

	public int save(Project project, string resource_name)
	{
		return _db.save(project.absolute_path(resource_name) + "." + OBJECT_TYPE_STATE_MACHINE, _id);
	}
}

} /* namespace Crown */
