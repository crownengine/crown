/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public static int get_destination_file(out GLib.File destination_file
	, string destination_dir
	, GLib.File source_file
	)
{
	string path = Path.build_filename(destination_dir, source_file.get_basename());
	destination_file = File.new_for_path(path);
	return 0;
}

public static int get_resource_path(out string resource_path
	, GLib.File destination_file
	, Project project
	)
{
	string resource_filename = project.resource_filename(destination_file.get_path());
	resource_path = ResourceId.normalize(resource_filename);
	return 0;
}

public static Vector3 vector3(ufbx.Vec3 v)
{
	return Vector3(v.x, v.y, v.z);
}

public static Quaternion quaternion(ufbx.Quat q)
{
	return Quaternion(q.x, q.y, q.z, q.w);
}

public static string light_type(ufbx.LightType ufbx_type)
{
	switch (ufbx_type) {
	case ufbx.LightType.DIRECTIONAL:
		return "directional";
	case ufbx.LightType.SPOT:
		return "spot";
	case ufbx.LightType.AREA:
		return "area";
	case ufbx.LightType.VOLUME:
		return "volume";
	case ufbx.LightType.POINT:
	default:
		return "omni";
	}
}

public static string projection_type(ufbx.ProjectionMode ufbx_mode)
{
	switch (ufbx_mode) {
	case ufbx.ProjectionMode.ORTHOGRAPHIC:
		return "orthographic";
	case ufbx.ProjectionMode.PERSPECTIVE:
	default:
		return "perspective";
	}
}

public class FBXImporter
{
	public static GLib.GenericArray<string> material_resource_names(ufbx.Scene scene)
	{
		GLib.GenericArray<string> names = new GLib.GenericArray<string>();
		GLib.HashTable<string, bool> used = new GLib.HashTable<string, bool>(GLib.str_hash, GLib.str_equal);
		for (int i = 0; i < scene.materials.data.length; ++i) {
			string raw = (string)scene.materials.data[i].name.data;
			if (raw == "")
				raw = "material_%u".printf(i);
			string name = raw
				.replace("/", "_")
				.replace("\\", "_")
				.replace(":", "_")
				.replace("*", "_")
				.replace("?", "_")
				.replace("\"", "_")
				.replace("<", "_")
				.replace(">", "_")
				.replace("|", "_")
				;
			while (used.contains(name))
				name += "_%u".printf(i);
			used[name] = true;
			names.add(name);
		}
		return names;
	}

	public static void import_material_slots(Database db
		, Guid component_id
		, ufbx.Node node
		, GLib.HashTable<unowned ufbx.Material, string> imported_materials
		, GLib.HashTable<unowned ufbx.Material, string>? imported_skinned_materials = null
		, string skinned_fallback_material = "core/fallback/fallback"
		)
	{
		bool skinned = node.mesh.skin_deformers.data.length == 1;
		GLib.HashTable<string, bool> used_slots = new GLib.HashTable<string, bool>(GLib.str_hash, GLib.str_equal);
		for (int i = 0; i < node.mesh.material_parts.data.length; ++i) {
			unowned ufbx.MeshPart part = node.mesh.material_parts.data[i];
			string suffix = "_%u".printf(part.index);
			string slot;
			if (node.mesh.materials.data.length == 0)
				slot = "default";
			else if (node.mesh.materials.data[part.index].name.data.length != 0)
				slot = (string)node.mesh.materials.data[part.index].name.data;
			else
				slot = "material" + suffix;
			while (used_slots.contains(slot))
				slot += suffix;
			used_slots[slot] = true;

			if (part.num_triangles == 0)
				continue;
			string material_name = skinned ? skinned_fallback_material : "core/fallback/fallback";
			if (part.index < node.materials.data.length) {
				unowned ufbx.Material material = node.materials.data[part.index];
				if (skinned && imported_skinned_materials != null && imported_skinned_materials.contains(material))
					material_name = imported_skinned_materials[material];
				else if (imported_materials.contains(material))
					material_name = imported_materials[material];
			}
			MeshResource.set_material_slot(db, component_id, slot, material_name);
		}
	}

	private static bool is_valid_animation_basename(string name)
	{
		return name != ""
			&& name.index_of_char('/') == -1
			&& name.index_of_char('\\') == -1
			&& name.index_of_char(':') == -1
			&& name.index_of_char('*') == -1
			&& name.index_of_char('?') == -1
			&& name.index_of_char('"') == -1
			&& name.index_of_char('<') == -1
			&& name.index_of_char('>') == -1
			&& name.index_of_char('|') == -1
			;
	}

	public static int get_or_import_texture_resource_name(out string? resource_name
		, Database db
		, Project project
		, string filename
		, GLib.File fbx_file
		, string destination_dir
		, bool create_textures_folder
		, ufbx.MaterialMap map
		, string semantic_suffix
		, MeshResource.TextureUsage usage
		, bool preserve_alpha
		, GLib.HashTable<string, string> imported_textures
		)
	{
		resource_name = null;
		if (!map.texture_enabled || map.texture == null)
			return 0;

		unowned ufbx.Texture? texture = map.texture;
		string textures_path = destination_dir;
		if (create_textures_folder) {
			GLib.File textures_file = File.new_for_path(Path.build_filename(destination_dir, "textures"));
			try {
				textures_file.make_directory();
			} catch (GLib.IOError.EXISTS e) {
				// Ignore.
			} catch (GLib.Error e) {
				loge(e.message);
				return 1;
			}

			textures_path = textures_file.get_path();
		}

		string texture_filename = MeshResource.texture_filename(texture);
		string texture_basename = texture_filename.length > 0
			? GLib.File.new_for_path(texture_filename).get_basename()
			: (string)texture.name.data + ".png"
			;
		if (texture.content.data.length > 0 && ResourceId.type(texture_basename) == null)
			texture_basename += ".png";

		string source_image_filename = Path.build_filename(textures_path, texture_basename);
		GLib.File source_image_file  = GLib.File.new_for_path(source_image_filename);
		string source_image_path     = source_image_file.get_path();

		string texture_resource_filename = project.resource_filename(source_image_path);
		string texture_resource_path     = ResourceId.normalize(texture_resource_filename);
		string? texture_source_name      = ResourceId.name(texture_resource_path);
		if (texture_source_name == null) {
			logw("'%s' references texture with no file extension '%s'".printf(filename, texture_basename));
			return 0;
		}

		string texture_resource_name     = texture_source_name + semantic_suffix;
		string? texture_resource_type    = ResourceId.type(texture_resource_path);
		string source_image              = texture_source_name + "." + (texture_resource_type != null ? texture_resource_type : "png");

		if (imported_textures.contains(texture_resource_name) && !preserve_alpha) {
			resource_name = imported_textures[texture_resource_name];
			return 0;
		}

		bool source_image_exists = false;
		// Extract embedded texture data or copy external texture files into textures_path.
		if (texture.content.data.length > 0) {
			// Extract embedded PNG data.
			FileStream fs = FileStream.open(source_image_path, "wb");
			if (fs == null) {
				loge("Failed to open texture '%s'".printf(source_image_path));
				return 1;
			}

			size_t num_written = fs.write((uint8[])texture.content.data);
			if (num_written != texture.content.data.length) {
				loge("Failed to write texture '%s'".printf(source_image_path));
				return 1;
			}

			source_image_exists = true;
		} else {
			GLib.File? texture_file = MeshResource.texture_source_file(texture, fbx_file);
			if (texture_file != null) {
				try {
					if (!texture_file.equal(source_image_file))
						texture_file.copy(source_image_file, FileCopyFlags.OVERWRITE);

					source_image_exists = source_image_file.query_exists();
				} catch (Error e) {
					logw(e.message);
				}
			}
		}

		// Only create .texture resource if source image exists.
		if (!source_image_exists) {
			logw("'%s' references non-existing texture '%s'".printf(filename, texture_basename));
			return 0;
		}
		bool has_alpha = (usage & MeshResource.TextureUsage.COLOR) != 0
			&& TextureResource.image_has_alpha(source_image_path)
			;

		// Create .texture resource.
		Guid texture_id = Guid.new_guid();
		TextureResource texture_resource;
		if ((usage & MeshResource.TextureUsage.NORMAL) != 0)
			texture_resource = TextureResource.normal_map(db, texture_id, source_image);
		else if ((usage & MeshResource.TextureUsage.DATA) != 0)
			texture_resource = TextureResource.data_map(db, texture_id, source_image);
		else if (preserve_alpha || has_alpha)
			texture_resource = TextureResource.alpha_map(db, texture_id, source_image);
		else
			texture_resource = TextureResource.color_map(db, texture_id, source_image);

		if (texture_resource.save(project, texture_resource_name) != 0)
			return 1;

		imported_textures.set(texture_resource_name, texture_resource_name);
		resource_name = texture_resource_name;
		return 0;
	}

	public static void unit_create_components(SceneImportOptions options
		, Database db
		, Guid parent_unit_id
		, Guid unit_id
		, string resource_name
		, string import_path
		, ufbx.Scene scene
		, ufbx.Node node
		, GLib.HashTable<unowned ufbx.Material, string> imported_materials
		, GLib.HashTable<unowned ufbx.Material, string> imported_skinned_materials
		, string skinned_fallback_material
		)
	{
		Vector3 pos = vector3(node.local_transform.translation);
		Quaternion rot = quaternion(node.local_transform.rotation);
		Vector3 scl = vector3(node.local_transform.scale);
		string editor_name = node.name.data.length == 0 ? OBJECT_NAME_UNNAMED : (string)node.name.data;
		Unit unit = Unit(db, unit_id);

		if ((node.light != null && !options.import_lights)
			|| (node.camera != null && !options.import_cameras)
			) {
			if (db.has_object(unit_id) && parent_unit_id != GUID_ZERO) {
				Value? children = db.get_property(parent_unit_id, db.property_index(parent_unit_id, STRING_ID_64("children", 0x6fbb13de0e1dce0d)));
				if (children != null)
					((GLib.GenericSet<Guid?>)children).remove(unit_id);
				db.destroy(unit_id);
			}
			return;
		}

		// Create mesh_renderer.
		if (node.mesh != null) {
			if (!db.has_object(unit_id))
				db.create(unit_id, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));

			// Create transform.
			{
				Guid component_id;
				if (!unit.has_component(out component_id, STRING_ID_64(OBJECT_TYPE_TRANSFORM, 0x69e14b13ad9b5315))) {
					component_id = Guid.new_guid();
					db.create(component_id, STRING_ID_64(OBJECT_TYPE_TRANSFORM, 0x69e14b13ad9b5315));
					db.add_to_set(unit_id, db.property_index(unit_id, STRING_ID_64("components", 0xe71d1687374e5a54)), component_id);
				}

				unit.set_component_vector3   (component_id, unit._db.property_index(component_id, STRING_ID_64("data.position", 0xbf17708e8bd2a30b)), pos);
				unit.set_component_quaternion(component_id, unit._db.property_index(component_id, STRING_ID_64("data.rotation", 0x3c8974411eeaf63c)), rot);
				unit.set_component_vector3   (component_id, unit._db.property_index(component_id, STRING_ID_64("data.scale", 0x16021af8bd4fea50)), scl);
				unit.set_component_string    (component_id, unit._db.property_index(component_id, STRING_ID_64("data.name", 0x2b855b7e7675517f)), editor_name);
			}

			if (node.mesh.num_triangles > 0) {
				// Create mesh_renderer.
				{
					Guid component_id;
					if (!unit.has_component(out component_id, STRING_ID_64(OBJECT_TYPE_MESH_RENDERER, 0x345b95f8df017893))) {
						component_id = Guid.new_guid();
						db.create(component_id, STRING_ID_64(OBJECT_TYPE_MESH_RENDERER, 0x345b95f8df017893));
						db.add_to_set(unit_id, db.property_index(unit_id, STRING_ID_64("components", 0xe71d1687374e5a54)), component_id);
					}

					unit.set_component_string(component_id, unit._db.property_index(component_id, STRING_ID_64("data.geometry_name", 0x040a59886a3e4a2d)), editor_name);
					import_material_slots(db, component_id, node, imported_materials, imported_skinned_materials, skinned_fallback_material);
					unit.set_component_string(component_id, unit._db.property_index(component_id, STRING_ID_64("data.mesh_resource", 0xd08afb39c15cc158)), resource_name);
					unit.set_component_bool  (component_id, unit._db.property_index(component_id, STRING_ID_64("data.visible", 0xad0490cc1fffda48)), true);
				}

				if (options.create_colliders) {
					// Create collider.
					{
						Guid component_id;
						if (!unit.has_component(out component_id, STRING_ID_64(OBJECT_TYPE_COLLIDER, 0x9a9fe4362129d74e))) {
							component_id = Guid.new_guid();
							db.create(component_id, STRING_ID_64(OBJECT_TYPE_COLLIDER, 0x9a9fe4362129d74e));
							db.add_to_set(unit_id, db.property_index(unit_id, STRING_ID_64("components", 0xe71d1687374e5a54)), component_id);
						}

						unit.set_component_string(component_id, unit._db.property_index(component_id, STRING_ID_64("data.shape", 0xd4ffa681b8480051)), "mesh");
						unit.set_component_string(component_id, unit._db.property_index(component_id, STRING_ID_64("data.scene", 0x8f8eae68a99b51cc)), resource_name);
						unit.set_component_string(component_id, unit._db.property_index(component_id, STRING_ID_64("data.name", 0x2b855b7e7675517f)), editor_name);
					}

					// Create actor.
					{
						Guid component_id;
						if (!unit.has_component(out component_id, STRING_ID_64(OBJECT_TYPE_ACTOR, 0xac2b738a374cf583))) {
							component_id = Guid.new_guid();
							db.create(component_id, STRING_ID_64(OBJECT_TYPE_ACTOR, 0xac2b738a374cf583));
							db.add_to_set(unit_id, db.property_index(unit_id, STRING_ID_64("components", 0xe71d1687374e5a54)), component_id);
						}

						unit.set_component_string(component_id, unit._db.property_index(component_id, STRING_ID_64("data.class", 0xf5345d5fdfcd4f82)), "static");
						unit.set_component_string(component_id, unit._db.property_index(component_id, STRING_ID_64("data.collision_filter", 0x493f7c3cd3e78169)), "default");
						unit.set_component_double(component_id, unit._db.property_index(component_id, STRING_ID_64("data.mass", 0xb77cc8d9b93f1d8e)), 1.0);
						unit.set_component_string(component_id, unit._db.property_index(component_id, STRING_ID_64("data.material", 0xf014ddbddc53c116)), "default");
					}
				}
			}
		} else if (node.light != null) {
			if (!options.import_lights)
				return;

			if (!db.has_object(unit_id))
				unit.create_empty();
			if (unit.set_prefab("core/units/light") != 0)
				return;
			unit.set_local_position(pos);
			unit.set_local_rotation(rot);
			unit.set_local_scale(scl);

			Guid component_id;
			if (unit.has_component(out component_id, STRING_ID_64(OBJECT_TYPE_LIGHT, 0x7dd7224fbb9f08c2))) {
				unit.set_component_string (component_id, unit._db.property_index(component_id, STRING_ID_64("data.type", 0x3d4dcce26f0f13fd)), light_type(node.light.type));
				unit.set_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.range", 0x969effdaa778c82b)), 10.0);
				unit.set_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.intensity", 0xd1e406b3aeae4d67)), (double)node.light.intensity);
				unit.set_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.spot_angle", 0xad796e4ae667dead)), 0.5 * MathUtils.rad((double)node.light.outer_angle));
				unit.set_component_vector3(component_id, unit._db.property_index(component_id, STRING_ID_64("data.color", 0xac9624a15a2891c0)), vector3(node.light.color));
				unit.set_component_double (component_id, unit._db.property_index(component_id, STRING_ID_64("data.shadow_bias", 0xa8290b921c961845)), 0.0004);
				unit.set_component_bool   (component_id, unit._db.property_index(component_id, STRING_ID_64("data.cast_shadows", 0xc2b3140260446694)), node.light.cast_shadows);
			}
		} else if (node.camera != null) {
			if (!options.import_cameras)
				return;

			if (!db.has_object(unit_id))
				unit.create_empty();
			if (unit.set_prefab("core/units/camera") != 0)
				return;
			unit.set_local_position(pos);
			unit.set_local_rotation(rot);
			unit.set_local_scale(scl);

			Guid component_id;
			if (unit.has_component(out component_id, STRING_ID_64(OBJECT_TYPE_CAMERA, 0x60ed8c3931822dc7))) {
				unit.set_component_string(component_id, unit._db.property_index(component_id, STRING_ID_64("data.projection", 0x6e0676d6db8d3734)), projection_type(node.camera.projection_mode));
				unit.set_component_double(component_id, unit._db.property_index(component_id, STRING_ID_64("data.fov", 0xae3b0c9413994b89)), MathUtils.rad((double)node.camera.field_of_view_deg.y));
				unit.set_component_double(component_id, unit._db.property_index(component_id, STRING_ID_64("data.far_range", 0x3283caec3b9511b0)), (double)node.camera.far_plane);
				unit.set_component_double(component_id, unit._db.property_index(component_id, STRING_ID_64("data.near_range", 0xbaf158a47f2c7242)), (double)node.camera.near_plane);
			}
		} else if (node.bone != null) {
			return;
		} else {
			if (!db.has_object(unit_id))
				db.create(unit_id, STRING_ID_64(OBJECT_TYPE_UNIT, 0xe0a48d0be9a7453f));

			// Create transform.
			Guid component_id;
			if (!unit.has_component(out component_id, STRING_ID_64(OBJECT_TYPE_TRANSFORM, 0x69e14b13ad9b5315))) {
				component_id = Guid.new_guid();
				db.create(component_id, STRING_ID_64(OBJECT_TYPE_TRANSFORM, 0x69e14b13ad9b5315));
				db.add_to_set(unit_id, db.property_index(unit_id, STRING_ID_64("components", 0xe71d1687374e5a54)), component_id);
			}

			unit.set_component_vector3   (component_id, unit._db.property_index(component_id, STRING_ID_64("data.position", 0xbf17708e8bd2a30b)), pos);
			unit.set_component_quaternion(component_id, unit._db.property_index(component_id, STRING_ID_64("data.rotation", 0x3c8974411eeaf63c)), rot);
			unit.set_component_vector3   (component_id, unit._db.property_index(component_id, STRING_ID_64("data.scale", 0x16021af8bd4fea50)), scl);
			unit.set_component_string    (component_id, unit._db.property_index(component_id, STRING_ID_64("data.name", 0x2b855b7e7675517f)), editor_name);
		}

		if (!options.create_colliders) {
			Guid component_id;
			if (unit.has_component(out component_id, STRING_ID_64(OBJECT_TYPE_COLLIDER, 0x9a9fe4362129d74e)) && db.owner(component_id) == unit_id) {
				Value? components = db.get_property(unit_id, db.property_index(unit_id, STRING_ID_64("components", 0xe71d1687374e5a54)));
				if (components != null)
					((GLib.GenericSet<Guid?>)components).remove(component_id);
				db.destroy(component_id);
			}

			if (unit.has_component(out component_id, STRING_ID_64(OBJECT_TYPE_ACTOR, 0xac2b738a374cf583)) && db.owner(component_id) == unit_id) {
				Value? components = db.get_property(unit_id, db.property_index(unit_id, STRING_ID_64("components", 0xe71d1687374e5a54)));
				if (components != null)
					((GLib.GenericSet<Guid?>)components).remove(component_id);
				db.destroy(component_id);
			}
		}

		db.set_name(unit_id, editor_name);
		// editor.import_path is importer-owned metadata, not a filesystem path.
		// It identifies the source node in the imported hierarchy so reimport
		// can reuse the same unit and preserve component GUIDs.
		db.set_string(unit_id, db.property_index(unit_id, STRING_ID_64("editor.import_path", 0xbfe7f0f8d6d0656a)), import_path);

		if (parent_unit_id != GUID_ZERO)
			db.add_to_set(parent_unit_id, db.property_index(parent_unit_id, STRING_ID_64("children", 0x6fbb13de0e1dce0d)), unit_id);

		// Reuse only children that existed before this import, and assign each
		// existing child to at most one imported node.
		GLib.GenericSet<Guid?> matched_children = new GLib.GenericSet<Guid?>(Guid.hash_func, Guid.equal_func);
		Guid?[] old_children = db.get_set(unit_id, db.property_index(unit_id, STRING_ID_64("children", 0x6fbb13de0e1dce0d)));
		GLib.HashTable<string, GLib.Queue<Guid?>> old_children_by_import_path = new GLib.HashTable<string, GLib.Queue<Guid?>>(GLib.str_hash, GLib.str_equal);
		GLib.HashTable<string, GLib.Queue<Guid?>> old_children_by_name = new GLib.HashTable<string, GLib.Queue<Guid?>>(GLib.str_hash, GLib.str_equal);
		foreach (Guid? child_id in old_children) {
			string old_import_path = db.get_string(child_id, db.property_index(child_id, STRING_ID_64("editor.import_path", 0xbfe7f0f8d6d0656a)), "");
			if (old_import_path != "") {
				unowned GLib.Queue<Guid?>? children = old_children_by_import_path[old_import_path];
				if (children == null) {
					old_children_by_import_path[old_import_path] = new GLib.Queue<Guid?>();
					children = old_children_by_import_path[old_import_path];
				}
				children.push_tail(child_id);
			}

			string old_name = db.name(child_id);
			unowned GLib.Queue<Guid?>? children = old_children_by_name[old_name];
			if (children == null) {
				old_children_by_name[old_name] = new GLib.Queue<Guid?>();
				children = old_children_by_name[old_name];
			}
			children.push_tail(child_id);
		}
		GLib.GenericArray<Guid?> child_unit_ids = new GLib.GenericArray<Guid?>();

		for (size_t i = 0; i < node.children.data.length; ++i) {
			unowned ufbx.Node child_node = node.children.data[i];
			string child_editor_name = child_node.name.data.length == 0 ? OBJECT_NAME_UNNAMED : (string)child_node.name.data;
			string child_import_path = import_path
				+ "/"
				+ ((uint)i).to_string()
				+ ":"
				+ child_editor_name
				;
			Guid child_unit_id = GUID_ZERO;

			unowned GLib.Queue<Guid?>? children = old_children_by_import_path[child_import_path];
			while (children != null && !children.is_empty()) {
				Guid? child_id = children.pop_head();
				if (!matched_children.contains(child_id)) {
					child_unit_id = (!)child_id;
					break;
				}
			}

			if (child_unit_id == GUID_ZERO) {
				children = old_children_by_name[child_editor_name];
				while (children != null && !children.is_empty()) {
					Guid? child_id = children.pop_head();
					if (!matched_children.contains(child_id)) {
						child_unit_id = (!)child_id;
						break;
					}
				}
			}

			if (child_unit_id == GUID_ZERO)
				child_unit_id = Guid.new_guid();
			matched_children.add(child_unit_id);
			child_unit_ids.add(child_unit_id);

			unit_create_components(options
				, db
				, unit_id
				, child_unit_id
				, resource_name
				, child_import_path
				, scene
				, child_node
				, imported_materials
				, imported_skinned_materials
				, skinned_fallback_material
				);
		}

		if (options.import_lods) {
			// Use LOD groups authored in the FBX file.
			for (size_t gi = 0; gi < scene.lod_groups.data.length; ++gi) {
				unowned ufbx.LodGroup lod_group = scene.lod_groups.data[gi];
				for (size_t ii = 0; ii < lod_group.instances.data.length; ++ii) {
					if (lod_group.instances.data[ii] != node)
						continue;

					Guid component_id;
					if (!unit.has_component(out component_id, STRING_ID_64(OBJECT_TYPE_LOD_GROUP, 0x97993ef522a1a9f0))) {
						component_id = Guid.new_guid();
						db.create(component_id, STRING_ID_64(OBJECT_TYPE_LOD_GROUP, 0x97993ef522a1a9f0));
						db.add_to_set(unit_id, db.property_index(unit_id, STRING_ID_64("components", 0xe71d1687374e5a54)), component_id);
					}

					unit.set_component_string(component_id, unit._db.property_index(component_id, STRING_ID_64("data.fade_mode", 0xd1e695426829933b)), "none");
					unit.set_component_double(component_id, unit._db.property_index(component_id, STRING_ID_64("data.level", 0xd70ddfec12f72a7b)), -1.0);
					db.create_empty_set(component_id, db.property_index(component_id, STRING_ID_64("data.lod_levels", 0xf8b66d06ee19128f)));

					// Add levels in source order, using FBX distances when available.
					for (size_t li = 0; li < lod_group.lod_levels.data.length && (int)li < child_unit_ids.length; ++li) {
						double screen_size = lod_group.relative_distances
							? (double)lod_group.lod_levels.data[li].distance / 100.0
							: 1.0 / (1 << (int)li)
							;

						Guid level_id = Guid.new_guid();
						db.create(level_id, STRING_ID_64(OBJECT_TYPE_LOD_LEVEL, 0x5aeafa4cb5acd79a));
						db.set_reference(level_id, db.property_index(level_id, STRING_ID_64("data.mesh_renderer", 0xba4d878d19255e65)), child_unit_ids[(int)li]);
						db.set_double(level_id, db.property_index(level_id, STRING_ID_64("data.screen_size", 0xb0affbb45037b116)), screen_size);
						db.add_to_set(component_id, db.property_index(component_id, STRING_ID_64("data.lod_levels", 0xf8b66d06ee19128f)), level_id);
					}
					return;
				}
			}

			if (scene.lod_groups.data.length > 0)
				return;

			// Fall back to sibling meshes named *_LOD<n>.
			for (int i = 0; i < child_unit_ids.length; ++i) {
				unowned ufbx.Node child_node = node.children.data[i];
				if (child_node.name.data.length == 0)
					continue;

				string name = (string)child_node.name.data;
				string base_name;
				int first_lod;
				if (!Mesh.parse_lod_name(out base_name, out first_lod, name))
					continue;
				string base_name_lower = base_name.down();
				GLib.HashTable<int, Guid?> lod_units = new GLib.HashTable<int, Guid?>(GLib.direct_hash, GLib.direct_equal);

				// Index matching LODs and find the lowest index, regardless of sibling order.
				for (int ci = 0; ci < child_unit_ids.length; ++ci) {
					unowned ufbx.Node n = node.children.data[ci];
					if (n.name.data.length == 0)
						continue;

					string child_base_name;
					int child_lod;
					string child_name = (string)n.name.data;
					if (!Mesh.parse_lod_name(out child_base_name, out child_lod, child_name)
						|| child_base_name.down() != base_name_lower
						)
						continue;

					if (!lod_units.contains(child_lod))
						lod_units[child_lod] = child_unit_ids[ci];
					if (child_lod < first_lod)
						first_lod = child_lod;
				}

				Guid component_id;
				if (!unit.has_component(out component_id, STRING_ID_64(OBJECT_TYPE_LOD_GROUP, 0x97993ef522a1a9f0))) {
					component_id = Guid.new_guid();
					db.create(component_id, STRING_ID_64(OBJECT_TYPE_LOD_GROUP, 0x97993ef522a1a9f0));
					db.add_to_set(unit_id, db.property_index(unit_id, STRING_ID_64("components", 0xe71d1687374e5a54)), component_id);
				}

				unit.set_component_string(component_id, unit._db.property_index(component_id, STRING_ID_64("data.fade_mode", 0xd1e695426829933b)), "none");
				unit.set_component_double(component_id, unit._db.property_index(component_id, STRING_ID_64("data.level", 0xd70ddfec12f72a7b)), -1.0);
				db.create_empty_set(component_id, db.property_index(component_id, STRING_ID_64("data.lod_levels", 0xf8b66d06ee19128f)));

				double screen_size = 1.0;
				// Add levels by suffix until the next LOD mesh is missing.
				int lod_i = first_lod;
				for (;;) {
					if (!lod_units.contains(lod_i))
						break;
					Guid lod_unit_id = lod_units[lod_i];

					Guid level_id = Guid.new_guid();
					db.create(level_id, STRING_ID_64(OBJECT_TYPE_LOD_LEVEL, 0x5aeafa4cb5acd79a));
					db.set_reference(level_id, db.property_index(level_id, STRING_ID_64("data.mesh_renderer", 0xba4d878d19255e65)), lod_unit_id);
					db.set_double(level_id, db.property_index(level_id, STRING_ID_64("data.screen_size", 0xb0affbb45037b116)), screen_size);
					db.add_to_set(component_id, db.property_index(component_id, STRING_ID_64("data.lod_levels", 0xf8b66d06ee19128f)), level_id);
					screen_size *= 0.5;

					if (lod_i == int.MAX)
						break;
					++lod_i;
				}
				return;
			}
		} else {
			Guid component_id;
			if (unit.has_component(out component_id, STRING_ID_64(OBJECT_TYPE_LOD_GROUP, 0x97993ef522a1a9f0)) && db.owner(component_id) == unit_id) {
				Value? components = db.get_property(unit_id, db.property_index(unit_id, STRING_ID_64("components", 0xe71d1687374e5a54)));
				if (components != null)
					((GLib.GenericSet<Guid?>)components).remove(component_id);
				db.destroy(component_id);
			}
		}
	}

	public static unowned ufbx.Node? find_first_non_bone_parent(ufbx.Node? bone_node)
	{
		assert(bone_node != null);

		while (bone_node.bone != null)
			bone_node = bone_node.parent;

		return bone_node;
	}

	public static unowned ufbx.Node? find_skeleton_root(ufbx.Node? node)
	{
		if (node.bone != null)
			return node;

		for (size_t i = 0; i < node.children.data.length; ++i) {
			unowned ufbx.Node? n = find_skeleton_root(node.children.data[i]);
			if (n != null)
				return find_first_non_bone_parent(n);
		}

		return null;
	}

	public static void import_skeleton(SceneImportOptions options
		, Database db
		, Guid parent_bone_id
		, Guid bone_id
		, ufbx.Node node
		)
	{
		db.create(bone_id, STRING_ID_64(OBJECT_TYPE_MESH_BONE, 0x0e2e3f571084be90));
		db.set_string(bone_id, db.property_index(bone_id, STRING_ID_64("name", 0xd4c943cba60c270b)), (string)node.name.data);
		if (parent_bone_id != GUID_ZERO)
			db.add_to_set(parent_bone_id, db.property_index(parent_bone_id, STRING_ID_64("children", 0x6fbb13de0e1dce0d)), bone_id);

		for (size_t i = 0; i < node.children.data.length; ++i) {
			unowned ufbx.Node child_bone = node.children.data[i];

			if (child_bone.bone == null)
				continue; // Skip non-bone children.

			import_skeleton(options
				, db
				, bone_id
				, Guid.new_guid()
				, child_bone
				);
		}
	}

	public static bool material_uses_skinning(ufbx.Scene scene, ufbx.Material material)
	{
		for (size_t i = 0; i < scene.nodes.data.length; ++i) {
			unowned ufbx.Node node = scene.nodes.data[i];
			if (node.mesh == null || node.mesh.skin_deformers.data.length != 1 || node.materials.data.length == 0)
				continue;

			for (int j = 0; j < node.mesh.material_parts.data.length; ++j) {
				unowned ufbx.MeshPart part = node.mesh.material_parts.data[j];
				if (part.num_triangles != 0 && part.index < node.materials.data.length
					&& node.materials.data[part.index] == material)
					return true;
			}
		}

		return false;
	}

	private static bool material_uses_static_mesh(ufbx.Scene scene, ufbx.Material material)
	{
		for (int i = 0; i < scene.nodes.data.length; ++i) {
			unowned ufbx.Node node = scene.nodes.data[i];
			if (node.mesh == null || node.mesh.skin_deformers.data.length == 1)
				continue;
			for (int j = 0; j < node.mesh.material_parts.data.length; ++j) {
				unowned ufbx.MeshPart part = node.mesh.material_parts.data[j];
				if (part.num_triangles != 0 && part.index < node.materials.data.length
					&& node.materials.data[part.index] == material)
					return true;
			}
		}
		return false;
	}

	private static bool needs_skinned_fallback(ufbx.Scene scene, GLib.HashTable<unowned ufbx.Material, string> materials)
	{
		for (int i = 0; i < scene.nodes.data.length; ++i) {
			unowned ufbx.Node node = scene.nodes.data[i];
			if (node.mesh == null || node.mesh.skin_deformers.data.length != 1)
				continue;
			for (int j = 0; j < node.mesh.material_parts.data.length; ++j) {
				unowned ufbx.MeshPart part = node.mesh.material_parts.data[j];
				if (part.num_triangles != 0
					&& (part.index >= node.materials.data.length || !materials.contains(node.materials.data[part.index])))
					return true;
			}
		}
		return false;
	}

	public static ImportResult do_import(SceneImportOptions options, Project project, string destination_dir, GLib.GenericArray<string> filenames)
	{
		for (int fi = 0; fi < filenames.length; ++fi) {
			string filename_i = filenames[fi];
			string resource_path;
			GLib.File file_dst;
			GLib.File file_src = File.new_for_path(filename_i);
			if (get_destination_file(out file_dst, destination_dir, file_src) != 0)
				return ImportResult.ERROR;
			if (get_resource_path(out resource_path, file_dst, project) != 0)
				return ImportResult.ERROR;
			string resource_name = ResourceId.name(resource_path);
			string resource_basename = GLib.File.new_for_path(resource_name).get_basename();

			// Copy FBX file.
			try {
				file_src.copy(file_dst, FileCopyFlags.OVERWRITE);
			} catch (Error e) {
				loge(e.message);
				return ImportResult.ERROR;
			}

			// Keep in sync with fbx_document.cpp!
			ufbx.LoadOpts load_opts = {};
			load_opts.use_blender_pbr_material = true;
			load_opts.target_camera_axes =
			{
				ufbx.CoordinateAxis.POSITIVE_X,
				ufbx.CoordinateAxis.POSITIVE_Z,
				ufbx.CoordinateAxis.NEGATIVE_Y
			};
			load_opts.target_light_axes =
			{
				ufbx.CoordinateAxis.POSITIVE_X,
				ufbx.CoordinateAxis.POSITIVE_Y,
				ufbx.CoordinateAxis.POSITIVE_Z
			};
			load_opts.target_axes = ufbx.CoordinateAxes.RIGHT_HANDED_Z_UP;
			load_opts.target_unit_meters = 1.0f;
			load_opts.space_conversion = ufbx.SpaceConversion.TRANSFORM_ROOT;

			// Load FBX file.
			ufbx.Error error = {};
			ufbx.Scene? scene = ufbx.Scene.load_file(filename_i, load_opts, ref error);

			Database db = new Database(project);
			create_object_types(db);
			GLib.HashTable<string, string> imported_textures = new GLib.HashTable<string, string>(GLib.str_hash, GLib.str_equal);
			GLib.HashTable<unowned ufbx.Material, string> imported_materials = new GLib.HashTable<unowned ufbx.Material, string>(GLib.direct_hash, GLib.direct_equal);
			GLib.HashTable<unowned ufbx.Material, string> imported_skinned_materials = new GLib.HashTable<unowned ufbx.Material, string>(GLib.direct_hash, GLib.direct_equal);

			// Import animations.
			StateMachineResource? smr = null;
			bool create_state_machine = false;
			string? initial_animation_name = null;

			if (options.import_animation) {
				string target_skeleton = options.target_skeleton;

				// Import animation skeleton.
				if (options.new_skeleton) {
					// Create .animation_skeleton resource.
					unowned ufbx.Node? skeleton_root_node = find_skeleton_root(scene.root_node);
					if (skeleton_root_node != null) {
						Guid skeleton_hierarchy_id = Guid.new_guid();
						import_skeleton(options
							, db
							, GUID_ZERO
							, skeleton_hierarchy_id
							, skeleton_root_node
							);

						Guid animation_skeleton_id = Guid.new_guid();
						db.create(animation_skeleton_id, STRING_ID_64(OBJECT_TYPE_MESH_SKELETON, 0x2597bb272931eded));
						db.set_string(animation_skeleton_id, db.property_index(animation_skeleton_id, STRING_ID_64("mesh_resource", 0x4576d9c12bc9cb7a)), resource_name);
						db.add_to_set(animation_skeleton_id, db.property_index(animation_skeleton_id, STRING_ID_64("skeleton", 0x975cebbda510e575)), skeleton_hierarchy_id);
						if (db.save(project.absolute_path(resource_name) + "." + OBJECT_TYPE_MESH_SKELETON, animation_skeleton_id) != 0)
							return ImportResult.ERROR;

						target_skeleton = resource_name;
						create_state_machine = true;
					}
				}

				// Import animation clip.
				if (options.import_clips) {
					if (target_skeleton == "") {
						logw("Animation must have a target skeleton. Animation clips won't be imported.");
					} else {
						// Create 'animations' folder.
						string directory_name = "animations";
						string animations_path = destination_dir;
						if (options.create_animations_folder && scene.anim_stacks.data.length != 0) {
							GLib.File animations_file = File.new_for_path(Path.build_filename(destination_dir, directory_name));
							try {
								animations_file.make_directory();
							} catch (GLib.IOError.EXISTS e) {
								// Ignore.
							} catch (GLib.Error e) {
								loge(e.message);
								return ImportResult.ERROR;
							}

							animations_path = animations_file.get_path();
						}

						// Extract clips.
						GLib.HashTable<string, bool> used_stack_names = new GLib.HashTable<string, bool>(GLib.str_hash, GLib.str_equal);
						for (size_t anim_i = 0; anim_i < scene.anim_stacks.data.length; ++anim_i) {
							unowned ufbx.AnimStack anim_stack = scene.anim_stacks.data[anim_i];
							string stack_name = (string)anim_stack.name.data;
							string animation_basename = resource_basename;
							if (scene.anim_stacks.data.length > 1) {
								if (stack_name == "") {
									loge("Animation stack %u has no name".printf((uint)anim_i));
									return ImportResult.ERROR;
								}
								if (used_stack_names.contains(stack_name)) {
									loge("Animation stack name '%s' is not unique".printf(stack_name));
									return ImportResult.ERROR;
								}
								used_stack_names[stack_name] = true;

								if (!is_valid_animation_basename(stack_name)) {
									loge("Animation stack name '%s' cannot be used as a resource filename".printf(stack_name));
									return ImportResult.ERROR;
								}
								animation_basename = resource_basename + "_" + stack_name;
							}

							string anim_filename = Path.build_filename(animations_path, animation_basename + "." + OBJECT_TYPE_MESH_ANIMATION);
							GLib.File anim_file  = GLib.File.new_for_path(anim_filename);
							string anim_path     = anim_file.get_path();

							string anim_resource_filename = project.resource_filename(anim_path);
							string anim_resource_path     = ResourceId.normalize(anim_resource_filename);
							string anim_resource_name     = ResourceId.name(anim_resource_path);

							// Create .mesh_animation resource.
							Guid anim_id = Guid.new_guid();
							db.create(anim_id, STRING_ID_64(OBJECT_TYPE_MESH_ANIMATION, 0x7369558b842d5314));
							db.set_string(anim_id, db.property_index(anim_id, STRING_ID_64("source", 0x921f1370045bad6e)), resource_path);
							db.set_string(anim_id, db.property_index(anim_id, STRING_ID_64("target_skeleton", 0xf3bffa3b8de7b210)), target_skeleton);
							db.set_string(anim_id, db.property_index(anim_id, STRING_ID_64("stack_name", 0x18834943532142f2)), stack_name);
							if (db.save(project.absolute_path(anim_resource_name) + "." + OBJECT_TYPE_MESH_ANIMATION, anim_id) != 0)
								return ImportResult.ERROR;
							if (initial_animation_name == null)
								initial_animation_name = anim_resource_name;
						}
					}
				}

				if (create_state_machine) {
					// Create .state_machine resource to drive the skeleton.
					Guid state_machine_id = Guid.new_guid();
					smr = StateMachineResource.mesh(db
						, state_machine_id
						, target_skeleton
						, initial_animation_name
						);
					if (smr.save(project, resource_name) != 0)
						return ImportResult.ERROR;
				}
			}

			// Import materials.
			if (options.import_units && options.import_materials) {
				// Create 'materials' folder.
				string directory_name = "materials";
				string materials_path = destination_dir;
				if (options.create_materials_folder && scene.materials.data.length != 0) {
					GLib.File materials_file = File.new_for_path(Path.build_filename(destination_dir, directory_name));
					try {
						materials_file.make_directory();
					} catch (GLib.IOError.EXISTS e) {
						// Ignore.
					} catch (GLib.Error e) {
						loge(e.message);
						return ImportResult.ERROR;
					}

					materials_path = materials_file.get_path();
				}

				GLib.GenericArray<string> material_names = material_resource_names(scene);
				GLib.HashTable<string, bool> used_material_resources = new GLib.HashTable<string, bool>(GLib.str_hash, GLib.str_equal);
				for (int i = 0; i < material_names.length; ++i) {
					string filename = Path.build_filename(materials_path, material_names[i] + ".material");
					used_material_resources[ResourceId.name(ResourceId.normalize(project.resource_filename(filename)))] = true;
				}
				// Extract materials.
				for (size_t i = 0; i < scene.materials.data.length; ++i) {
					unowned ufbx.Material material = scene.materials.data[i];

					string material_filename = Path.build_filename(materials_path, material_names[(uint)i] + ".png");
					GLib.File material_file  = GLib.File.new_for_path(material_filename);
					string material_path     = material_file.get_path();

					string material_resource_filename = project.resource_filename(material_path);
					string material_resource_path     = ResourceId.normalize(material_resource_filename);
					string material_resource_name     = ResourceId.name(material_resource_path);

					string shader = "mesh";
					Vector3 albedo = Vector3(1, 1, 1);
					double metallic = 0.0;
					double roughness = 1.0;
					Vector3 emission_color = Vector3(0, 0, 0);
					double emission_intensity = 1.0;
					string? albedo_map = null;
					string? normal_map = null;
					string? metallic_map = null;
					string? roughness_map = null;
					string? ao_map = null;
					string? emission_map = null;
					bool masking = false;

					for (int mm = 0; mm < ufbx.MaterialPbrMap.MAP_COUNT; ++mm) {
						unowned ufbx.MaterialMap map = material.pbr.maps[mm];

						switch (mm) {
						case ufbx.MaterialPbrMap.BASE_COLOR: {
							if (map.texture_enabled && map.texture != null)
								albedo = Vector3(1, 1, 1);
							else if (map.has_value)
								albedo = vector3(map.value_vec3);

							unowned ufbx.MaterialMap opacity = material.pbr.opacity;
							masking = map.texture_enabled
								&& opacity.texture_enabled
								&& map.texture != null
								&& opacity.texture != null
								;
							if (masking) {
								string color_texture_filename = MeshResource.texture_filename(map.texture);
								string opacity_texture_filename = MeshResource.texture_filename(opacity.texture);
								masking = map.texture == opacity.texture
									|| (color_texture_filename.length > 0 && color_texture_filename == opacity_texture_filename)
									;
							}

							if (!masking) {
								unowned ufbx.MaterialMap transparency = material.fbx.transparency_factor;
								masking = map.texture_enabled
									&& transparency.texture_enabled
									&& map.texture != null
									&& transparency.texture != null
									;
								if (masking) {
									string color_texture_filename = MeshResource.texture_filename(map.texture);
									string transparency_texture_filename = MeshResource.texture_filename(transparency.texture);
									masking = map.texture == transparency.texture
										|| (color_texture_filename.length > 0 && color_texture_filename == transparency_texture_filename)
										;
								}
							}

							if (!masking) {
								unowned ufbx.MaterialMap transparency = material.fbx.transparency_color;
								masking = map.texture_enabled
									&& transparency.texture_enabled
									&& map.texture != null
									&& transparency.texture != null
									;
								if (masking) {
									string color_texture_filename = MeshResource.texture_filename(map.texture);
									string transparency_texture_filename = MeshResource.texture_filename(transparency.texture);
									masking = map.texture == transparency.texture
										|| (color_texture_filename.length > 0 && color_texture_filename == transparency_texture_filename)
										;
								}
							}

							if (options.import_textures
								&& get_or_import_texture_resource_name(out albedo_map
									, db
									, project
									, filename_i
									, file_src
									, destination_dir
									, options.create_textures_folder
									, map
									, "_df"
									, MeshResource.TextureUsage.COLOR
									, masking
									, imported_textures
									) != 0)
								return ImportResult.ERROR;
							break;
						}

						case ufbx.MaterialPbrMap.NORMAL_MAP: {
							if (options.import_textures
								&& get_or_import_texture_resource_name(out normal_map
									, db
									, project
									, filename_i
									, file_src
									, destination_dir
									, options.create_textures_folder
									, map
									, "_nr"
									, MeshResource.TextureUsage.NORMAL
									, false
									, imported_textures
									) != 0)
								return ImportResult.ERROR;
							break;
						}

						case ufbx.MaterialPbrMap.METALNESS: {
							if (map.has_value)
								metallic = map.value_real;

							if (options.import_textures
								&& get_or_import_texture_resource_name(out metallic_map
									, db
									, project
									, filename_i
									, file_src
									, destination_dir
									, options.create_textures_folder
									, map
									, "_mt"
									, MeshResource.TextureUsage.DATA
									, false
									, imported_textures
									) != 0)
								return ImportResult.ERROR;
							break;
						}

						case ufbx.MaterialPbrMap.ROUGHNESS: {
							if (map.has_value)
								roughness = map.value_real;

							if (options.import_textures
								&& get_or_import_texture_resource_name(out roughness_map
									, db
									, project
									, filename_i
									, file_src
									, destination_dir
									, options.create_textures_folder
									, map
									, "_rg"
									, MeshResource.TextureUsage.DATA
									, false
									, imported_textures
									) != 0)
								return ImportResult.ERROR;
							break;
						}

						case ufbx.MaterialPbrMap.AMBIENT_OCCLUSION: {
							if (options.import_textures
								&& get_or_import_texture_resource_name(out ao_map
									, db
									, project
									, filename_i
									, file_src
									, destination_dir
									, options.create_textures_folder
									, map
									, "_ao"
									, MeshResource.TextureUsage.DATA
									, false
									, imported_textures
									) != 0)
								return ImportResult.ERROR;
							break;
						}

						case ufbx.MaterialPbrMap.EMISSION_COLOR: {
							if (map.has_value)
								emission_color = vector3(map.value_vec3);

							if (options.import_textures
								&& get_or_import_texture_resource_name(out emission_map
									, db
									, project
									, filename_i
									, file_src
									, destination_dir
									, options.create_textures_folder
									, map
									, "_em"
									, MeshResource.TextureUsage.COLOR
									, false
									, imported_textures
									) != 0)
								return ImportResult.ERROR;
							break;
						}

						case ufbx.MaterialPbrMap.EMISSION_FACTOR:
							if (map.has_value)
								emission_intensity = (double)map.value_real;
							break;

						default:
							break;
						}
					}

					bool uses_skinning = smr != null && material_uses_skinning(scene, material);
					bool uses_static_mesh = material_uses_static_mesh(scene, material);
					if (uses_skinning && !uses_static_mesh)
						shader += "+SKINNING";

					masking = masking && albedo_map != null;

					// Create .material resource.
					MaterialResource material_resource = MaterialResource.mesh(db
						, Guid.new_guid()
						, albedo_map
						, normal_map
						, metallic_map
						, roughness_map
						, ao_map
						, emission_map
						, albedo
						, metallic
						, roughness
						, emission_color
						, emission_intensity
						, shader
						, masking
						);
					if (material_resource.save(project, material_resource_name) != 0)
						return ImportResult.ERROR;

					imported_materials.set(material, material_resource_name);
					if (uses_skinning && uses_static_mesh) {
						string skinned_name = material_resource_name + "_skinned";
						while (used_material_resources.contains(skinned_name))
							skinned_name += "_skinned";
						used_material_resources[skinned_name] = true;
						MaterialResource skinned = MaterialResource.mesh(db, Guid.new_guid()
							, albedo_map, normal_map, metallic_map, roughness_map, ao_map, emission_map
							, albedo, metallic, roughness, emission_color, emission_intensity
							, "mesh+SKINNING", masking);
						if (skinned.save(project, skinned_name) != 0)
							return ImportResult.ERROR;
						imported_skinned_materials[material] = skinned_name;
					} else if (uses_skinning) {
						imported_skinned_materials[material] = material_resource_name;
					}
				}
			}

			if (options.import_units) {
				string skinned_fallback_material = "core/fallback/fallback";
				if (smr != null && needs_skinned_fallback(scene, imported_skinned_materials)) {
					skinned_fallback_material = resource_name + "_fbx_skinned_fallback";
					MaterialResource fallback = MaterialResource.mesh(db, Guid.new_guid());
					db.set_string(fallback._id, db.property_index(fallback._id, STRING_ID_64("shader", 0xcce8d5b5f5ae333f)), "mesh+SKINNING");
					if (fallback.save(project, skinned_fallback_material) != 0)
						return ImportResult.ERROR;
				}
				// Generate or modify existing .unit.
				Guid unit_id;
				if (db.add_from_resource_path(out unit_id, resource_name + ".unit") != 0)
					unit_id = Guid.new_guid();
				unit_create_components(options
					, db
					, GUID_ZERO
					, unit_id
					, resource_name
					, "root"
					, scene
					, scene.root_node
					, imported_materials
					, imported_skinned_materials
					, skinned_fallback_material
					);

				if (options.import_animation && options.new_skeleton && smr != null) {
					// Create animation_state_machine component.
					Unit unit = Unit(db, unit_id);

					Guid component_id;
					if (!unit.has_component(out component_id, STRING_ID_64(OBJECT_TYPE_ANIMATION_STATE_MACHINE, 0x0d694773e87992ac))) {
						component_id = Guid.new_guid();
						db.create(component_id, STRING_ID_64(OBJECT_TYPE_ANIMATION_STATE_MACHINE, 0x0d694773e87992ac));
						db.add_to_set(unit_id, db.property_index(unit_id, STRING_ID_64("components", 0xe71d1687374e5a54)), component_id);
					}

					unit.set_component_string(component_id, unit._db.property_index(component_id, STRING_ID_64("data.state_machine_resource", 0xd98f8b4d76314483)), resource_name);
				}

				if (db.save(project.absolute_path(resource_name) + ".unit", unit_id) != 0)
					return ImportResult.ERROR;

				Guid mesh_id = Guid.new_guid();
				db.create(mesh_id, STRING_ID_64(OBJECT_TYPE_MESH, 0x48ff313713a997a1));
				db.set_string(mesh_id, db.property_index(mesh_id, STRING_ID_64("source", 0x921f1370045bad6e)), resource_path);
				if (db.save(project.absolute_path(resource_name) + ".mesh", mesh_id) != 0)
					return ImportResult.ERROR;
			}
		}

		return ImportResult.SUCCESS;
	}

	public static string? primary_resource_path(Project project, string destination_dir, GLib.GenericArray<string> filenames, ImportResult result)
	{
		if (result != ImportResult.SUCCESS || filenames.length == 0)
			return null;

		GLib.File file_dst;
		string resource_path;

		get_destination_file(out file_dst, destination_dir, File.new_for_path(filenames[0]));
		get_resource_path(out resource_path, file_dst, project);

		return ResourceId.path(OBJECT_TYPE_UNIT, ResourceId.name(resource_path));
	}

	public static void import_with_options(Import import_result
		, SceneImportOptions options
		, Project project
		, string destination_dir
		, GLib.GenericArray<string> filenames
		, string? options_path = null
		)
	{
		ImportResult result = FBXImporter.do_import(options, project, destination_dir, filenames);
		if (result == ImportResult.SUCCESS && options_path != null) {
			try {
				SJSON.save(options.encode(), options_path);
			} catch (JsonWriteError e) {
				result = ImportResult.ERROR;
			}
		}

		import_result(result, FBXImporter.primary_resource_path(project, destination_dir, filenames, result));
	}

	public static void import(Import import_result, Database database, string destination_dir, GLib.SList<string> filenames, Gtk.Window? parent_window)
	{
		GLib.GenericArray<string> fbx_filenames = new GLib.GenericArray<string>();
		foreach (unowned string filename in filenames)
			fbx_filenames.add(filename);

		SceneImportOptions options = new SceneImportOptions(SceneImportFlags.LIGHTS_AND_CAMERAS | SceneImportFlags.ANIMATIONS);

		GLib.File file_dst;
		string resource_path;
		get_destination_file(out file_dst, destination_dir, File.new_for_path(fbx_filenames[0]));
		get_resource_path(out resource_path, file_dst, database._project);
		string resource_name = ResourceId.name(resource_path);
		string options_path = database._project.absolute_path(resource_name) + ".importer_settings";
		try {
			options.decode(SJSON.load_from_path(options_path));
		} catch (JsonSyntaxError e) {
			// No-op.
		}

		if (parent_window == null) {
			FBXImporter.import_with_options(import_result
				, options
				, database._project
				, destination_dir
				, fbx_filenames
				, options_path
				);
		} else {
			SceneImportDialog dialog = new SceneImportDialog(database
				, destination_dir
				, filenames
				, import_result
				, (owned)options
				, options_path
				, _("Import FBX...")
				, FBXImporter.import_with_options
				);
			dialog.set_transient_for(parent_window);
			dialog.set_modal(true);
#if CROWN_GTK3
			dialog.show_all();
#endif
			dialog.present();
		}
	}
}

} /* namespace Crown */
