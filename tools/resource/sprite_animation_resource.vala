/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public struct AnimationFrame
{
	public Database _db;
	public Guid _id;

	public AnimationFrame(Database db
		, Guid id
		, int frame
		, int index
		)
	{
		_db = db;
		_id = id;

		_db.create(_id, OBJECT_TYPE_ANIMATION_FRAME);
		_db.set_double(_id, "frame", (double)frame);
		_db.set_double(_id, "index", (double)index);
	}
}

public struct SpriteAnimation
{
	public Database _db;
	public Guid _id;

	public SpriteAnimation(Database db, Guid id)
	{
		_db = db;
		_id = id;

		_db.create(_id, OBJECT_TYPE_SPRITE_ANIMATION);
		_db.set_double(_id, "frames_per_second", 16.0);
		_db.create_empty_set(_id, "frames");
	}

	public void add_frame(AnimationFrame anim)
	{
		_db.add_to_set(_id, "frames", anim._id);
	}

	public int save(Project project, string resource_name)
	{
		return _db.save(project.absolute_path(resource_name) + "." + OBJECT_TYPE_SPRITE_ANIMATION, _id);
	}
}

} /* namespace Crown */
