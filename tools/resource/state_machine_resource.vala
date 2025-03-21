/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

using Gee;

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
		_db.set_property_string(_id, "name", name);
		_db.set_property_string(_id, "weight", weight);
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
		_db.set_property_bool(_id, "loop", true);
		_db.set_property_string(_id, "speed", "1");
		_db.create_empty_set(0, _id, "animations");
		_db.create_empty_set(0, _id, "transitions");
	}
}

public struct StateMachineResource
{
	public Database _db;
	public Guid _id;

	public StateMachineResource(Database db, Guid id, StateMachineNode initial_state)
	{
		_db = db;
		_id = id;

		_db.create(_id, OBJECT_TYPE_STATE_MACHINE);
		add_node(initial_state);
		_db.set_property_guid(_id, "initial_state", initial_state._id);
		_db.create_empty_set(0, _id, "variables");
	}

	public void add_node(StateMachineNode node)
	{
		_db.add_to_set(_id, "states", node._id);
	}

	public void save(Project project, string resource_name)
	{
		_db.save(project.absolute_path(resource_name) + "." + OBJECT_TYPE_STATE_MACHINE, _id);
	}
}

} /* namespace Crown */
