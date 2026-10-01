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

		_db.create(_id, STRING_ID_64(OBJECT_TYPE_ANIMATION_FRAME, 0x5b6893db5cc56d9e));
		_db.set_double(_id, _db.property_index(_id, STRING_ID_64("frame", 0x4285067a507a51e2)), (double)frame);
		_db.set_double(_id, _db.property_index(_id, STRING_ID_64("index", 0xb03b8bced9422c44)), (double)index);
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

		_db.create(_id, STRING_ID_64(OBJECT_TYPE_SPRITE_ANIMATION, 0x487e78e3f87f238d));
		_db.set_double(_id, _db.property_index(_id, STRING_ID_64("frames_per_second", 0x20f235a9b753a8b7)), 16.0);
		_db.create_empty_set(_id, _db.property_index(_id, STRING_ID_64("frames", 0xbb0b674305a539e7)));
	}

	public void add_frame(AnimationFrame anim)
	{
		_db.add_to_set(_id, _db.property_index(_id, STRING_ID_64("frames", 0xbb0b674305a539e7)), anim._id);
	}

	public int save(Project project, string resource_name)
	{
		return _db.save(project.absolute_path(resource_name) + "." + OBJECT_TYPE_SPRITE_ANIMATION, _id);
	}
}

} /* namespace Crown */
