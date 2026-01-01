/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public struct SoundResource
{
	public Database _db;
	public Guid _id;

	public SoundResource(Database db
		, Guid id
		, string source_sound
		)
	{
		_db = db;
		_id = id;

		_db.create(_id, OBJECT_TYPE_SOUND);
		_db.set_string(_id, "source", source_sound);
	}

	public int save(Project project, string resource_name)
	{
		return _db.save(project.absolute_path(resource_name) + "." + OBJECT_TYPE_SOUND, _id);
	}

	public static void import(Import import_result, Database database, string destination_dir, SList<string> filenames)
	{
		Project project = database._project;

		foreach (unowned string filename_i in filenames) {
			GLib.File file_src = File.new_for_path(filename_i);

			GLib.File file_dst       = File.new_for_path(Path.build_filename(destination_dir, file_src.get_basename()));
			string resource_filename = project.resource_filename(file_dst.get_path());
			string resource_path     = ResourceId.normalize(resource_filename);
			string resource_name     = ResourceId.name(resource_path);

			try {
				file_src.copy(file_dst, FileCopyFlags.OVERWRITE);
			} catch (Error e) {
				loge(e.message);
				import_result(ImportResult.ERROR);
				return;
			}

			Database db = new Database(project);
			var sound_resource = SoundResource(db, Guid.new_guid(), resource_path);
			if (sound_resource.save(project, resource_name) != 0) {
				import_result(ImportResult.ERROR);
				return;
			}
		}

		import_result(ImportResult.SUCCESS);
	}
}

} /* namespace Crown */
