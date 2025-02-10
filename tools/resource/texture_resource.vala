/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

using Gee;

namespace Crown
{
public enum TextureFormat
{
	BC1,
	BC2,
	BC3,
	BC4,
	BC5,
	PTC14,
	RGB8,
	RGBA8,

	COUNT;

	public string to_key()
	{
		switch (this) {
		case BC1:
			return "BC1";
		case BC2:
			return "BC2";
		case BC3:
			return "BC3";
		case BC4:
			return "BC4";
		case BC5:
			return "BC5";
		case PTC14:
			return "PTC14";
		case RGB8:
			return "RGB8";
		case RGBA8:
			return "RGBA8";
		default:
			return "BC1";
		}
	}
}

public struct TextureResource
{
	public Database _db;
	public Guid _id;

	public TextureResource(Database db
		, Guid id
		, string source_image
		, TextureFormat output_format
		, bool generate_mips
		, bool is_normal_map
		)
	{
		_db = db;
		_id = id;

		_db.create(_id, "texture");

		for (int tp = 0; tp < TargetPlatform.COUNT; ++tp) {
			string platform = ((TargetPlatform)tp).to_key();
			_db.set_property_string(_id, "source", source_image);
			_db.set_property_string(_id, "output." + platform + ".format", output_format.to_key());
			_db.set_property_bool(_id, "output." + platform + ".generate_mips", generate_mips);
			_db.set_property_bool(_id, "output." + platform + ".normal_map", is_normal_map);
			_db.set_property_double(_id, "output." + platform + ".mip_skip_smallest", 0);
		}
	}

	public TextureResource.color_map(Database db, Guid texture_id, string source_image)
	{
		this(db, texture_id, source_image, TextureFormat.BC1, true, false);
	}

	public TextureResource.normal_map(Database db, Guid texture_id, string source_image)
	{
		this(db, texture_id, source_image, TextureFormat.BC5, true, true);
	}

	public TextureResource.font_atlas(Database db, Guid texture_id, string source_image)
	{
		this(db, texture_id, source_image, TextureFormat.BC3, false, false);
	}

	public TextureResource.sprite(Database db, Guid texture_id, string source_image)
	{
		this(db, texture_id, source_image, TextureFormat.RGBA8, false, false);
	}

	public void save(Project project, string resource_name)
	{
		_db.save(project.absolute_path(resource_name) + ".texture", _id);
	}

	public static ImportResult import(ProjectStore project_store, string destination_dir, SList<string> filenames)
	{
		Project project = project_store._project;

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
				return ImportResult.ERROR;
			}

			Database db = new Database(project);
			var texture_resource = TextureResource.color_map(db, Guid.new_guid(), resource_path);
			texture_resource.save(project, resource_name);
		}

		return ImportResult.SUCCESS;
	}
}

} /* namespace Crown */
