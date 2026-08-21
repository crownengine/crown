/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class GLTFNames
{
	private GLib.GenericArray<string> _nodes;
	private GLib.GenericArray<string> _materials;
	private GLib.GenericArray<string> _images;
	private GLib.GenericArray<string> _skins;
	private GLib.GenericArray<string> _animations;
	private GLib.GenericArray<string> _animation_resources;

	private static string unique_name(string? raw
		, string kind
		, size_t index
		, GLib.HashTable<string, bool> used
		)
	{
		string name = raw != null && raw != "" ? raw : "%s_%u".printf(kind, (uint)index);
		if (used.contains(name)) {
			name = "%s_%u".printf(name, (uint)index);
			while (used.contains(name))
				name += "_%u".printf((uint)index);
		}
		used[name] = true;
		return name;
	}

	public GLTFNames(cgltf.Data data)
	{
		_nodes = new GLib.GenericArray<string>();
		_materials = new GLib.GenericArray<string>();
		_images = new GLib.GenericArray<string>();
		_skins = new GLib.GenericArray<string>();
		_animations = new GLib.GenericArray<string>();
		_animation_resources = new GLib.GenericArray<string>();

		GLib.HashTable<string, bool> used = new GLib.HashTable<string, bool>(GLib.str_hash, GLib.str_equal);
		for (size_t i = 0; i < data.nodes_count; ++i)
			_nodes.add(unique_name(data.nodes[i].name, "node", i, used));

		used.remove_all();
		for (size_t i = 0; i < data.materials_count; ++i)
			_materials.add(unique_name(data.materials[i].name, "material", i, used));

		used.remove_all();
		for (size_t i = 0; i < data.images_count; ++i)
			_images.add(unique_name(data.images[i].name, "image", i, used));

		used.remove_all();
		for (size_t i = 0; i < data.skins_count; ++i)
			_skins.add(unique_name(data.skins[i].name, "skin", i, used));

		used.remove_all();
		GLib.HashTable<string, bool> used_animation_resources = new GLib.HashTable<string, bool>(GLib.str_hash, GLib.str_equal);
		for (size_t i = 0; i < data.animations_count; ++i) {
			string animation_name = unique_name(data.animations[i].name, "animation", i, used);
			_animations.add(animation_name);
			string animation_resource = animation_name
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
			_animation_resources.add(unique_name(animation_resource, "animation", i, used_animation_resources));
		}
	}

	public unowned string node(cgltf.Data data, cgltf.Node* node)
	{
		return _nodes[(uint)cgltf.node_index(data, node)];
	}

	public unowned string material(cgltf.Data data, cgltf.Material* material)
	{
		return _materials[(uint)cgltf.material_index(data, material)];
	}

	public unowned string image(cgltf.Data data, cgltf.Image* image)
	{
		return _images[(uint)cgltf.image_index(data, image)];
	}

	public unowned string skin(cgltf.Data data, cgltf.Skin* skin)
	{
		return _skins[(uint)cgltf.skin_index(data, skin)];
	}

	public unowned string animation(cgltf.Data data, cgltf.Animation* animation)
	{
		return _animations[(uint)cgltf.animation_index(data, animation)];
	}

	public unowned string animation_resource(cgltf.Data data, cgltf.Animation* animation)
	{
		return _animation_resources[(uint)cgltf.animation_index(data, animation)];
	}
}

public class GLTFSkinGroups
{
	private int[] _groups;
	private bool[] _active;
	private GLib.GenericArray<int?> _primaries;
	private GLib.GenericArray<int?> _active_groups;

	private static bool same_joints(cgltf.Skin* a, cgltf.Skin* b)
	{
		if (a.joints.length != b.joints.length)
			return false;
		for (size_t ai = 0; ai < a.joints.length; ++ai) {
			bool found = false;
			for (size_t bi = 0; bi < b.joints.length; ++bi) {
				if (a.joints[ai] == b.joints[bi]) {
					found = true;
					break;
				}
			}
			if (!found)
				return false;
		}
		return true;
	}

	public GLTFSkinGroups(cgltf.Data data, cgltf.Scene* scene)
	{
		_groups = new int[(int)data.skins_count];
		_active = new bool[(int)data.skins_count];
		_primaries = new GLib.GenericArray<int?>();
		_active_groups = new GLib.GenericArray<int?>();
		for (size_t i = 0; i < data.skins_count; ++i)
			_groups[i] = -1;

		// A glTF may contain multiple skin objects for one armature. Group skins
		// by joint identity, independently of joint-array order, so compatible
		// skins share one skeleton and state machine.
		for (size_t i = 0; i < data.skins_count; ++i) {
			if (_groups[i] >= 0)
				continue;
			int group = (int)_primaries.length;
			_primaries.add((int)i);
			_groups[i] = group;
			for (size_t j = i + 1; j < data.skins_count; ++j) {
				if (_groups[j] < 0 && same_joints(&data.skins[i], &data.skins[j]))
					_groups[j] = group;
			}
		}

		for (size_t i = 0; i < data.nodes_count; ++i) {
			cgltf.Node* node = &data.nodes[i];
			if (node.skin == null || !GLTFImporter.node_in_scene(scene, node))
				continue;
			activate(_groups[cgltf.skin_index(data, node.skin)]);
		}
		if (_active_groups.length == 0) {
			for (int group = 0; group < (int)_primaries.length; ++group)
				activate(group);
		}
	}

	private void activate(int group)
	{
		if (!_active[group]) {
			_active[group] = true;
			_active_groups.add(group);
		}
	}

	public uint length()
	{
		return _active_groups.length;
	}

	public int group(cgltf.Data data, cgltf.Skin* skin)
	{
		return _groups[cgltf.skin_index(data, skin)];
	}

	public int active_group(uint index)
	{
		return _active_groups[index];
	}

	public cgltf.Skin* primary(cgltf.Data data, int group)
	{
		return &data.skins[(size_t)_primaries[group]];
	}
}

public class GLTFRigidAttachments
{
	private GLib.HashTable<cgltf.Node*, cgltf.Skin*> _skins;
	private GLib.HashTable<cgltf.Node*, bool> _flattened;
	public bool valid;

	private static bool ancestor_or_self(cgltf.Node* ancestor, cgltf.Node* node)
	{
		cgltf.Node* current = node;
		while (current != null) {
			if (current == ancestor)
				return true;
			current = current.parent;
		}
		return false;
	}

	private static cgltf.Node* skeleton_root(cgltf.Skin* skin)
	{
		if (skin.skeleton != null)
			return skin.skeleton;
		if (skin.joints.length == 0)
			return null;

		cgltf.Node* candidate = skin.joints[0];
		while (candidate != null) {
			bool common = true;
			for (size_t ji = 1; ji < skin.joints.length; ++ji) {
				if (!ancestor_or_self(candidate, skin.joints[ji])) {
					common = false;
					break;
				}
			}
			if (common)
				return candidate;
			candidate = candidate.parent;
		}
		return null;
	}

	private static bool node_in_skeleton(cgltf.Skin* skin, cgltf.Node* node)
	{
		cgltf.Node* root = skeleton_root(skin);
		if (root == null)
			return false;
		for (size_t ji = 0; ji < skin.joints.length; ++ji) {
			cgltf.Node* ancestor = skin.joints[ji];
			while (ancestor != null) {
				if (ancestor == node)
					return true;
				if (ancestor == root)
					break;
				ancestor = ancestor.parent;
			}
		}
		return false;
	}

	public GLTFRigidAttachments(cgltf.Data data
		, cgltf.Scene* scene
		, GLTFSkinGroups skin_groups
		, bool enabled
		)
	{
		_skins = new GLib.HashTable<cgltf.Node*, cgltf.Skin*>(GLib.direct_hash, GLib.direct_equal);
		_flattened = new GLib.HashTable<cgltf.Node*, bool>(GLib.direct_hash, GLib.direct_equal);
		valid = true;
		if (!enabled)
			return;

		// glTF can animate an unskinned mesh through node parenting. Mesh
		// animation uses a skeleton palette, so bind each such mesh rigidly to
		// its nearest bone and synthesize its skin weights during compilation.
		for (size_t ni = 0; ni < data.nodes_count; ++ni) {
			cgltf.Node* node = &data.nodes[ni];
			if (node.mesh == null || node.skin != null || !GLTFImporter.node_in_scene(scene, node))
				continue;

			cgltf.Node* bone = node;
			while (bone != null) {
				int selected_group = -1;
				for (uint ai = 0; ai < skin_groups.length(); ++ai) {
					int group = skin_groups.active_group(ai);
					if (!node_in_skeleton(skin_groups.primary(data, group), bone))
						continue;
					if (selected_group >= 0 && selected_group != group) {
						string node_name = node.name != null ? node.name : "<unnamed>";
						string bone_name = bone.name != null ? bone.name : "<unnamed>";
						loge("glTF rigid mesh '%s' is parented to bone '%s' shared by multiple armatures"
							.printf(node_name, bone_name));
						valid = false;
						return;
					}
					selected_group = group;
				}
				if (selected_group >= 0) {
					_skins[node] = skin_groups.primary(data, selected_group);
					break;
				}
				bone = bone.parent;
			}
		}
	}

	public bool contains(cgltf.Node* node)
	{
		return _skins.contains(node);
	}

	public cgltf.Skin* skin(cgltf.Node* node)
	{
		return _skins[node];
	}

	public bool flattened(cgltf.Node* node)
	{
		return _flattened.contains(node);
	}

	public void resolve_roots(cgltf.Data data
		, cgltf.Scene* scene
		, bool single_group
		, GLib.HashTable<cgltf.Skin*, string> skin_state_machines
		, GLib.HashTable<cgltf.Node*, string> node_state_machines
		)
	{
		for (size_t ni = 0; ni < data.nodes_count; ++ni) {
			cgltf.Node* node = &data.nodes[ni];
			if (!contains(node))
				continue;

			cgltf.Node* owner = null;
			if (single_group) {
				if (scene.nodes.length == 1)
					owner = scene.nodes[0];
			} else {
				string state_machine = skin_state_machines[skin(node)];
				owner = node;
				while (owner != null) {
					if (node_state_machines.contains(owner)
						&& node_state_machines[owner] == state_machine)
						break;
					owner = owner.parent;
				}
				if (owner == null) {
					string node_name = node.name != null ? node.name : "<unnamed>";
					loge("Unable to place glTF rigid mesh '%s' under its armature".printf(node_name));
					valid = false;
					return;
				}
			}

			// AnimationStateMachine resets a mesh-owning unit to identity when it
			// attaches the skeleton, so its effective model-space root is its parent.
			cgltf.Node* root = owner != null && owner.mesh != null ? owner.parent : owner;
			cgltf.Node* flattened = node;
			while (flattened != root) {
				if (flattened == null) {
					string node_name = node.name != null ? node.name : "<unnamed>";
					loge("glTF rigid mesh root is not an ancestor of '%s'".printf(node_name));
					valid = false;
					return;
				}
				_flattened[flattened] = true;
				flattened = flattened.parent;
			}
		}
	}
}

public class GLTFImporter
{
	private static string animation_resource_name(Project project
		, string animations_path
		, string resource_basename
		, cgltf.Data data
		, GLTFNames names
		, GLTFSkinGroups skin_groups
		, cgltf.Skin* skin
		, cgltf.Animation* animation
		)
	{
		string animation_basename = data.animations_count > 1
			? resource_basename + "_" + names.animation_resource(data, animation)
			: resource_basename
			;
		if (skin_groups.length() > 1)
			animation_basename += "_" + names.skin(data, skin);
		string animation_filename = Path.build_filename(animations_path, animation_basename + "." + OBJECT_TYPE_MESH_ANIMATION);
		return ResourceId.name(ResourceId.normalize(project.resource_filename(animation_filename)));
	}

	public static bool node_in_scene(cgltf.Scene* scene, cgltf.Node* node)
	{
		while (node.parent != null)
			node = node.parent;
		for (size_t i = 0; i < scene.nodes.length; ++i) {
			if (scene.nodes[i] == node)
				return true;
		}
		return false;
	}

	public static string? decoded_relative_uri(string uri)
	{
		string? decoded = Uri.unescape_string(uri);
		if (decoded == null || decoded.index_of_char(':') >= 0 || Path.is_absolute(decoded))
			return null;
		return decoded;
	}

	public static int copy_external_buffers(cgltf.Data data
		, GLib.File source_file
		, Project project
		, string destination_dir
		)
	{
		GLib.File? source_parent = source_file.get_parent();
		if (source_parent == null)
			return 1;
		GLib.File destination_parent = File.new_for_path(destination_dir);

		for (size_t i = 0; i < data.buffers_count; ++i) {
			cgltf.Buffer* buffer = &data.buffers[i];
			if (buffer.uri == null || buffer.uri.has_prefix("data:"))
				continue;

			string? relative = decoded_relative_uri(buffer.uri);
			if (relative == null) {
				loge("Unsupported or unsafe glTF buffer URI '%s'".printf(buffer.uri));
				return 1;
			}

			GLib.File source = source_parent.resolve_relative_path(relative);
			GLib.File destination = destination_parent.resolve_relative_path(relative);
			if (!project.path_is_within_source_dir(destination.get_path())) {
				loge("glTF buffer URI escapes the project: '%s'".printf(buffer.uri));
				return 1;
			}
			GLib.File? parent = destination.get_parent();
			try {
				if (parent != null)
					parent.make_directory_with_parents();
			} catch (GLib.IOError.EXISTS e) {
				// Ignore.
			} catch (GLib.Error e) {
				loge(e.message);
				return 1;
			}

			try {
				if (!source.equal(destination))
					source.copy(destination, FileCopyFlags.OVERWRITE);
			} catch (GLib.Error e) {
				loge(e.message);
				return 1;
			}
		}
		return 0;
	}

	public static uint8[]? image_bytes(cgltf.Image* image, GLib.File source_file)
	{
		if (image.uri != null) {
			if (image.uri.has_prefix("data:")) {
				int comma = image.uri.index_of_char(',');
				if (comma < 0 || !image.uri.substring(0, comma).has_suffix(";base64")) {
					loge("Unsupported glTF image data URI");
					return null;
				}
				return Base64.decode(image.uri.substring(comma + 1));
			}

			string? relative = decoded_relative_uri(image.uri);
			GLib.File? parent = source_file.get_parent();
			if (relative == null || parent == null) {
				loge("Unsupported or unsafe glTF image URI '%s'".printf(image.uri));
				return null;
			}

			uint8[] contents;
			try {
				parent.resolve_relative_path(relative).load_contents(null, out contents, null);
				return contents;
			} catch (GLib.Error e) {
				logw(e.message);
				return null;
			}
		}

		if (image.buffer_view != null) {
			cgltf.BufferView* view = image.buffer_view;
			if (view.buffer == null || view.buffer.data == null || view.offset > view.buffer.size || view.size > view.buffer.size - view.offset) {
				loge("Embedded glTF image is out of bounds");
				return null;
			}

			uint8[] contents = new uint8[(int)view.size];
			void* source = view.data != null ? view.data : (uint8*)view.buffer.data + view.offset;
			Memory.copy(contents, source, view.size);
			return contents;
		}

		return null;
	}

	public static string image_extension(cgltf.Image* image)
	{
		if (image.mime_type == "image/jpeg")
			return "jpg";
		if (image.mime_type == "image/png")
			return "png";
		if (image.uri != null) {
			string lower = image.uri.down();
			if (lower.has_suffix(".jpg") || lower.has_suffix(".jpeg"))
				return "jpg";
		}
		return "png";
	}

	public static int write_bytes(string path, uint8[] bytes)
	{
		FileStream stream = FileStream.open(path, "wb");
		if (stream == null || stream.write(bytes) != bytes.length) {
			loge("Failed to write glTF image '%s'".printf(path));
			return 1;
		}
		return 0;
	}

	public static int split_channel(string source_path, string destination_path, int channel)
	{
		try {
			Gdk.Pixbuf source = new Gdk.Pixbuf.from_file(source_path);
			Gdk.Pixbuf output = new Gdk.Pixbuf(Gdk.Colorspace.RGB, false, 8, source.width, source.height);
			unowned uint8[] source_pixels = source.get_pixels();
			unowned uint8[] output_pixels = output.get_pixels();
			int source_channels = source.get_n_channels();
			int source_stride = source.get_rowstride();
			int output_stride = output.get_rowstride();
			for (int y = 0; y < source.height; ++y) {
				for (int x = 0; x < source.width; ++x) {
					int source_offset = y*source_stride + x*source_channels;
					int output_offset = y*output_stride + x*3;
					uint8 value = source_pixels[source_offset + int.min(channel, source_channels - 1)];
					output_pixels[output_offset + 0] = value;
					output_pixels[output_offset + 1] = value;
					output_pixels[output_offset + 2] = value;
				}
			}
			output.save(destination_path, "png");
			return 0;
		} catch (GLib.Error e) {
			loge(e.message);
			return 1;
		}
	}

	public static int import_texture(out string? resource_name
		, Database db
		, Project project
		, cgltf.Data data
		, GLTFNames names
		, cgltf.TextureView view
		, GLib.File source_file
		, string destination_dir
		, bool create_textures_folder
		, string semantic_suffix
		, MeshResource.TextureUsage usage
		, bool preserve_alpha
		, int channel
		, GLib.HashTable<string, string> imported_textures
		)
	{
		resource_name = null;
		if (view.texture == null || view.texture.image == null)
			return 0;

		cgltf.Image* image = view.texture.image;
		string cache_key = "%u:%s:%d".printf((uint)cgltf.image_index(data, image), semantic_suffix, channel);
		if (imported_textures.contains(cache_key) && !preserve_alpha) {
			resource_name = imported_textures[cache_key];
			return 0;
		}

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

		uint8[]? bytes = image_bytes(image, source_file);
		if (bytes == null) {
			logw("glTF image '%s' could not be imported".printf(names.image(data, image)));
			return 0;
		}

		string extension = image_extension(image);
		string image_name = names.image(data, image);
		string source_path = Path.build_filename(textures_path, image_name + "." + extension);
		if (write_bytes(source_path, bytes) != 0)
			return 1;

		string imported_path = source_path;
		if (channel >= 0) {
			imported_path = Path.build_filename(textures_path, image_name + semantic_suffix + ".png");
			if (split_channel(source_path, imported_path, channel) != 0)
				return 1;
		}

		string imported_resource_path = ResourceId.normalize(project.resource_filename(imported_path));
		string imported_source_name = ResourceId.name(imported_resource_path);
		string imported_source_type = ResourceId.type(imported_resource_path) ?? "png";
		string texture_resource_name = channel >= 0 ? imported_source_name : imported_source_name + semantic_suffix;
		string source_image = imported_source_name + "." + imported_source_type;

		TextureResource texture_resource;
		if ((usage & MeshResource.TextureUsage.NORMAL) != 0)
			texture_resource = TextureResource.normal_map(db, Guid.new_guid(), source_image);
		else if ((usage & MeshResource.TextureUsage.DATA) != 0)
			texture_resource = TextureResource.data_map(db, Guid.new_guid(), source_image);
		else if (preserve_alpha)
			texture_resource = TextureResource(db, Guid.new_guid(), source_image, TextureFormat.BC3, true, false);
		else
			texture_resource = TextureResource.color_map(db, Guid.new_guid(), source_image);
		if (texture_resource.save(project, texture_resource_name) != 0)
			return 1;

		imported_textures[cache_key] = texture_resource_name;
		resource_name = texture_resource_name;
		return 0;
	}

	public static Matrix4x4 local_transform(cgltf.Node* node)
	{
		float matrix[16];
		cgltf.node_transform_local(node, matrix);
		return Matrix4x4(
			Vector4(matrix[0], -matrix[2],  matrix[1], 0.0),
			Vector4(-matrix[8],  matrix[10], -matrix[9], 0.0),
			Vector4(matrix[4], -matrix[6],  matrix[5], 0.0),
			Vector4(matrix[12], -matrix[14], matrix[13], 1.0)
			);
	}

	public static Matrix4x4 identity_transform()
	{
		return Matrix4x4(
			Vector4(1.0, 0.0, 0.0, 0.0),
			Vector4(0.0, 1.0, 0.0, 0.0),
			Vector4(0.0, 0.0, 1.0, 0.0),
			Vector4(0.0, 0.0, 0.0, 1.0)
			);
	}

	public static string? collect_subtree_state_machines(cgltf.Node* node
		, GLib.HashTable<cgltf.Skin*, string> skin_state_machines
		, GLTFRigidAttachments rigid_attachments
		, GLib.HashTable<cgltf.Node*, string> subtree_state_machines
		, GLib.HashTable<cgltf.Node*, bool> mixed_subtrees
		)
	{
		cgltf.Skin* node_skin = node.skin != null
			? node.skin
			: (rigid_attachments.contains(node) ? rigid_attachments.skin(node) : null)
			;
		string? selected = node_skin != null && skin_state_machines.contains(node_skin)
			? skin_state_machines[node_skin]
			: null
			;
		bool mixed = false;
		for (size_t i = 0; i < node.children.length; ++i) {
			cgltf.Node* child_node = node.children[i];
			string? child = collect_subtree_state_machines(child_node, skin_state_machines, rigid_attachments
				, subtree_state_machines, mixed_subtrees
				);
			mixed = mixed || mixed_subtrees.contains(child_node);
			if (child == null)
				continue;
			if (selected != null && selected != child)
				mixed = true;
			else
				selected = child;
		}
		if (selected != null)
			subtree_state_machines[node] = selected;
		if (mixed)
			mixed_subtrees[node] = true;
		return selected;
	}

	public static void mark_state_machine_roots(cgltf.Node* node
		, string? inherited
		, GLib.HashTable<cgltf.Node*, string> subtree_state_machines
		, GLib.HashTable<cgltf.Node*, bool> mixed_subtrees
		, GLib.HashTable<cgltf.Node*, string> node_state_machines
		)
	{
		string? selected = subtree_state_machines.contains(node) ? subtree_state_machines[node] : null;
		string? covered = inherited;
		if (!mixed_subtrees.contains(node) && selected != null && selected != inherited) {
			node_state_machines[node] = selected;
			covered = selected;
		}
		for (size_t i = 0; i < node.children.length; ++i)
			mark_state_machine_roots(node.children[i], covered
				, subtree_state_machines, mixed_subtrees, node_state_machines
				);
	}

	public static void import_material_slots(Database db
		, Guid component_id
		, cgltf.Data data
		, GLTFNames names
		, cgltf.Mesh* mesh
		, bool skinned
		, GLib.HashTable<cgltf.Material*, string> imported_materials
		, GLib.HashTable<cgltf.Material*, string> imported_skinned_materials
		, string skinned_fallback_material
		)
	{
		GLib.HashTable<string, bool> used_slots = new GLib.HashTable<string, bool>(GLib.str_hash, GLib.str_equal);
		for (size_t i = 0; i < mesh.primitives_count; ++i) {
			cgltf.Primitive* primitive = &mesh.primitives[i];
			string suffix = "_%u".printf((uint)i);
			string slot = primitive.material != null
				? names.material(data, primitive.material)
				: "default"
				;
			while (used_slots.contains(slot))
				slot += suffix;
			used_slots[slot] = true;

			cgltf.Accessor* positions = cgltf.find_accessor(primitive, cgltf.AttributeType.POSITION, 0);
			size_t num_indices = primitive.indices != null
				? primitive.indices.count
				: (positions != null ? positions.count : 0)
				;
			if (num_indices < 3)
				continue;

			string material_name = skinned && skinned_fallback_material != ""
				? skinned_fallback_material
				: "core/fallback/fallback"
				;
			if (primitive.material != null) {
				if (skinned && imported_skinned_materials.contains(primitive.material))
					material_name = imported_skinned_materials[primitive.material];
				else if (imported_materials.contains(primitive.material))
					material_name = imported_materials[primitive.material];
			}
			MeshResource.set_material_slot(db, component_id, slot, material_name);
		}
	}

	public static bool needs_skinned_fallback(SceneImportOptions options
		, cgltf.Data data
		, GLTFRigidAttachments rigid_attachments
		)
	{
		for (size_t i = 0; i < data.nodes_count; ++i) {
			cgltf.Node* node = &data.nodes[i];
			if ((node.skin == null && !rigid_attachments.contains(node))
				|| node.mesh == null
				)
				continue;
			for (size_t pi = 0; pi < node.mesh.primitives_count; ++pi) {
				cgltf.Primitive* primitive = &node.mesh.primitives[pi];
				if (primitive.type != cgltf.PrimitiveType.TRIANGLES
					&& primitive.type != cgltf.PrimitiveType.TRIANGLE_STRIP
					&& primitive.type != cgltf.PrimitiveType.TRIANGLE_FAN
					)
					continue;
				cgltf.Accessor* positions = cgltf.find_accessor(primitive, cgltf.AttributeType.POSITION, 0);
				if (positions != null && positions.count >= 3
					&& (!options.import_materials || primitive.material == null))
					return true;
			}
		}
		return false;
	}

	public static string light_type(cgltf.LightType type)
	{
		switch (type) {
		case cgltf.LightType.DIRECTIONAL:
			return "directional";
		case cgltf.LightType.SPOT:
			return "spot";
		case cgltf.LightType.POINT:
			default:
			return "omni";
		}
	}

	public static void remove_owned_component(Database db, Unit unit, Guid unit_id, string type)
	{
		Guid component_id;
		if (unit.has_component(out component_id, type) && db.owner(component_id) == unit_id) {
			Value? components = db.get_property(unit_id, "components");
			if (components != null)
				((GLib.GenericSet<Guid?>)components).remove(component_id);
			db.destroy(component_id);
		}
	}

	public static void set_transform(Unit unit, Guid component_id, Matrix4x4 transform, string name)
	{
		unit.set_component_vector3   (component_id, "data.position", transform.t.to_vector3());
		unit.set_component_quaternion(component_id, "data.rotation", transform.rotation());
		unit.set_component_vector3   (component_id, "data.scale", transform.scale());
		unit.set_component_string    (component_id, "data.name", name);
	}

	public static bool unit_create_components(SceneImportOptions options
		, Database db
		, Guid parent_unit_id
		, Guid unit_id
		, string resource_name
		, string import_path
		, cgltf.Data data
		, GLTFNames names
		, cgltf.Node* node
		, GLib.HashTable<cgltf.Material*, string> imported_materials
		, GLib.HashTable<cgltf.Material*, string> imported_skinned_materials
		, string skinned_fallback_material
		, GLTFRigidAttachments rigid_attachments
		, bool attach_state_machine
		, GLib.HashTable<cgltf.Node*, string> state_machines
		)
	{
		if ((node.light != null && !options.import_lights)
			|| (node.camera != null && !options.import_cameras)
			)
			return false;

		string editor_name = names.node(data, node);
		Matrix4x4 transform = rigid_attachments.flattened(node)
			? identity_transform()
			: local_transform(node)
			;
		Unit unit = Unit(db, unit_id);

		if (node.mesh != null) {
			if (!db.has_object(unit_id))
				db.create(unit_id, OBJECT_TYPE_UNIT);

			Guid transform_id;
			if (!unit.has_component(out transform_id, OBJECT_TYPE_TRANSFORM)) {
				transform_id = Guid.new_guid();
				db.create(transform_id, OBJECT_TYPE_TRANSFORM);
				db.add_to_set(unit_id, "components", transform_id);
			}
			set_transform(unit, transform_id, transform, editor_name);

			bool has_triangles = false;
			cgltf.Material* material = null;
			for (size_t i = 0; i < node.mesh.primitives_count && (!has_triangles || material == null); ++i) {
				cgltf.Primitive* primitive = &node.mesh.primitives[i];
				if (primitive.type != cgltf.PrimitiveType.TRIANGLES
					&& primitive.type != cgltf.PrimitiveType.TRIANGLE_STRIP
					&& primitive.type != cgltf.PrimitiveType.TRIANGLE_FAN
					)
					continue;
				if (material == null)
					material = primitive.material;
				cgltf.Accessor* positions = cgltf.find_accessor(primitive, cgltf.AttributeType.POSITION, 0);
				has_triangles = has_triangles || (positions != null && positions.count >= 3);
			}

			if (has_triangles) {
				Guid component_id;
				if (!unit.has_component(out component_id, OBJECT_TYPE_MESH_RENDERER)) {
					component_id = Guid.new_guid();
					db.create(component_id, OBJECT_TYPE_MESH_RENDERER);
					db.add_to_set(unit_id, "components", component_id);
				}

				bool skinned = node.skin != null || rigid_attachments.contains(node);
				string material_name = skinned && skinned_fallback_material != ""
					? skinned_fallback_material
					: "core/fallback/fallback"
					;
				if (material != null) {
					if (skinned
						&& imported_skinned_materials.contains(material))
						material_name = imported_skinned_materials[material];
					else if (imported_materials.contains(material))
						material_name = imported_materials[material];
				}
				unit.set_component_string(component_id, "data.geometry_name", editor_name);
				unit.set_component_string(component_id, "data.material", material_name);
				import_material_slots(db, component_id, data, names, node.mesh, skinned
					, imported_materials, imported_skinned_materials, skinned_fallback_material
					);
				unit.set_component_string(component_id, "data.mesh_resource", resource_name);
				unit.set_component_bool  (component_id, "data.visible", true);

				if (options.create_colliders) {
					Guid collider_id;
					if (!unit.has_component(out collider_id, OBJECT_TYPE_COLLIDER)) {
						collider_id = Guid.new_guid();
						db.create(collider_id, OBJECT_TYPE_COLLIDER);
						db.add_to_set(unit_id, "components", collider_id);
					}
					unit.set_component_string(collider_id, "data.shape", "mesh");
					unit.set_component_string(collider_id, "data.scene", resource_name);
					unit.set_component_string(collider_id, "data.name", editor_name);

					Guid actor_id;
					if (!unit.has_component(out actor_id, OBJECT_TYPE_ACTOR)) {
						actor_id = Guid.new_guid();
						db.create(actor_id, OBJECT_TYPE_ACTOR);
						db.add_to_set(unit_id, "components", actor_id);
					}
					unit.set_component_string(actor_id, "data.class", "static");
					unit.set_component_string(actor_id, "data.collision_filter", "default");
					unit.set_component_double(actor_id, "data.mass", 1.0);
					unit.set_component_string(actor_id, "data.material", "default");
				}
			}
		} else if (node.light != null) {
			if (!db.has_object(unit_id))
				unit.create_empty();
			if (unit.set_prefab("core/units/light") != 0)
				return false;
			unit.set_local_position(transform.t.to_vector3());
			unit.set_local_rotation(transform.rotation());
			unit.set_local_scale(transform.scale());

			Guid component_id;
			if (unit.has_component(out component_id, OBJECT_TYPE_LIGHT)) {
				unit.set_component_string (component_id, "data.type", light_type(node.light.type));
				unit.set_component_double (component_id, "data.range", node.light.range > 0.0f ? node.light.range : 10.0);
				unit.set_component_double (component_id, "data.intensity", node.light.intensity);
				unit.set_component_double (component_id, "data.spot_angle", node.light.spot_outer_cone_angle);
				unit.set_component_vector3(component_id, "data.color", Vector3(node.light.color[0], node.light.color[1], node.light.color[2]));
				unit.set_component_double (component_id, "data.shadow_bias", 0.0001);
				unit.set_component_bool   (component_id, "data.cast_shadows", false);
			}
		} else if (node.camera != null) {
			if (!db.has_object(unit_id))
				unit.create_empty();
			if (unit.set_prefab("core/units/camera") != 0)
				return false;
			unit.set_local_position(transform.t.to_vector3());
			unit.set_local_rotation(transform.rotation());
			unit.set_local_scale(transform.scale());

			Guid component_id;
			if (unit.has_component(out component_id, OBJECT_TYPE_CAMERA)) {
				bool orthographic = node.camera.type == cgltf.CameraType.ORTHOGRAPHIC;
				unit.set_component_string(component_id, "data.projection", orthographic ? "orthographic" : "perspective");
				if (orthographic) {
					unit.set_component_double(component_id, "data.orthographic_size", node.camera.orthographic.ymag);
					unit.set_component_double(component_id, "data.near_range", node.camera.orthographic.znear);
					unit.set_component_double(component_id, "data.far_range", node.camera.orthographic.zfar);
				} else {
					unit.set_component_double(component_id, "data.fov", node.camera.perspective.yfov);
					unit.set_component_double(component_id, "data.near_range", node.camera.perspective.znear);
					unit.set_component_double(component_id, "data.far_range", node.camera.perspective.has_zfar ? node.camera.perspective.zfar : 1000.0);
				}
			}
		} else {
			if (!db.has_object(unit_id))
				db.create(unit_id, OBJECT_TYPE_UNIT);
			Guid component_id;
			if (!unit.has_component(out component_id, OBJECT_TYPE_TRANSFORM)) {
				component_id = Guid.new_guid();
				db.create(component_id, OBJECT_TYPE_TRANSFORM);
				db.add_to_set(unit_id, "components", component_id);
			}
			set_transform(unit, component_id, transform, editor_name);
		}

		if (!options.create_colliders) {
			remove_owned_component(db, unit, unit_id, OBJECT_TYPE_COLLIDER);
			remove_owned_component(db, unit, unit_id, OBJECT_TYPE_ACTOR);
		}

		if (attach_state_machine && state_machines.contains(node)) {
			Guid component_id;
			if (!unit.has_component(out component_id, OBJECT_TYPE_ANIMATION_STATE_MACHINE)) {
				component_id = Guid.new_guid();
				db.create(component_id, OBJECT_TYPE_ANIMATION_STATE_MACHINE);
				db.add_to_set(unit_id, "components", component_id);
			}
			unit.set_component_string(component_id, "data.state_machine_resource", state_machines[node]);
		} else if (attach_state_machine || parent_unit_id != GUID_ZERO) {
			remove_owned_component(db, unit, unit_id, OBJECT_TYPE_ANIMATION_STATE_MACHINE);
		}

		db.set_name(unit_id, editor_name);
		db.set_string(unit_id, "editor.import_path", import_path);
		if (parent_unit_id != GUID_ZERO)
			db.add_to_set(parent_unit_id, "children", unit_id);

		GLib.GenericSet<Guid?> matched_children = new GLib.GenericSet<Guid?>(Guid.hash_func, Guid.equal_func);
		Guid?[] old_children = db.get_set(unit_id, "children");
		GLib.GenericArray<Guid?> child_unit_ids = new GLib.GenericArray<Guid?>();
		int[] child_indices = new int[(int)node.children.length];
		int num_child_indices = 0;
		for (size_t i = 0; i < node.children.length; ++i) {
			cgltf.Node* child = node.children[i];
			string child_name = names.node(data, child);
			string child_path = import_path + "/" + ((uint)i).to_string() + ":" + child_name;
			Guid child_id = GUID_ZERO;
			foreach (Guid? old_id in old_children) {
				if (!matched_children.contains(old_id)
					&& db.is_alive(old_id)
					&& db.get_string(old_id, "editor.import_path", "") == child_path
					) {
					child_id = old_id;
					break;
				}
			}
			if (child_id == GUID_ZERO) {
				foreach (Guid? old_id in old_children) {
					if (!matched_children.contains(old_id) && db.is_alive(old_id) && db.name(old_id) == child_name) {
						child_id = old_id;
						break;
					}
				}
			}
			if (child_id == GUID_ZERO)
				child_id = Guid.new_guid();

			if (unit_create_components(options, db, unit_id, child_id, resource_name, child_path, data, names, child
				, imported_materials, imported_skinned_materials, skinned_fallback_material
				, rigid_attachments, attach_state_machine, state_machines)) {
				matched_children.add(child_id);
				child_unit_ids.add(child_id);
				child_indices[num_child_indices++] = (int)i;
			}
		}

		if (options.import_lods) {
			for (int i = 0; i < num_child_indices; ++i) {
				cgltf.Node* child = node.children[child_indices[i]];
				string base_name;
				int first_lod;
				string name = names.node(data, child);
				if (!Mesh.parse_lod_name(out base_name, out first_lod, name))
					continue;
				GLib.HashTable<int, Guid?> lod_units = new GLib.HashTable<int, Guid?>(GLib.direct_hash, GLib.direct_equal);
				for (int ci = 0; ci < num_child_indices; ++ci) {
					cgltf.Node* candidate = node.children[child_indices[ci]];
					string candidate_base;
					int candidate_lod;
					if (Mesh.parse_lod_name(out candidate_base, out candidate_lod, names.node(data, candidate))
						&& candidate_base.down() == base_name.down()
						) {
						lod_units[candidate_lod] = child_unit_ids[ci];
						first_lod = int.min(first_lod, candidate_lod);
					}
				}

				Guid component_id;
				if (!unit.has_component(out component_id, OBJECT_TYPE_LOD_GROUP)) {
					component_id = Guid.new_guid();
					db.create(component_id, OBJECT_TYPE_LOD_GROUP);
					db.add_to_set(unit_id, "components", component_id);
				}
				unit.set_component_string(component_id, "data.fade_mode", "none");
				unit.set_component_double(component_id, "data.level", -1.0);
				db.create_empty_set(component_id, "data.lod_levels");
				double screen_size = 1.0;
				for (int lod = first_lod; lod_units.contains(lod); ++lod) {
					Guid level_id = Guid.new_guid();
					db.create(level_id, OBJECT_TYPE_LOD_LEVEL);
					db.set_reference(level_id, "data.mesh_renderer", lod_units[lod]);
					db.set_double(level_id, "data.screen_size", screen_size);
					db.add_to_set(component_id, "data.lod_levels", level_id);
					screen_size *= 0.5;
					if (lod == int.MAX)
						break;
				}
				break;
			}
		} else {
			remove_owned_component(db, unit, unit_id, OBJECT_TYPE_LOD_GROUP);
		}

		return true;
	}

	public static void mark_bones(cgltf.Skin* skin, GLib.HashTable<cgltf.Node*, bool> marked)
	{
		for (size_t i = 0; i < skin.joints.length; ++i) {
			cgltf.Node* node = skin.joints[i];
			while (node != null) {
				marked[node] = true;
				node = node.parent;
			}
		}
	}

	public static void import_bone(Database db
		, cgltf.Data data
		, GLTFNames names
		, GLib.HashTable<cgltf.Node*, bool> marked
		, Guid parent_bone_id
		, Guid bone_id
		, cgltf.Node* node
		)
	{
		db.create(bone_id, OBJECT_TYPE_MESH_BONE);
		db.set_string(bone_id, "name", names.node(data, node));
		if (parent_bone_id != GUID_ZERO)
			db.add_to_set(parent_bone_id, "children", bone_id);
		for (size_t i = 0; i < node.children.length; ++i) {
			cgltf.Node* child = node.children[i];
			if (marked.contains(child))
				import_bone(db, data, names, marked, bone_id, Guid.new_guid(), child);
		}
	}

	public static int import_materials(SceneImportOptions options
		, Database db
		, Project project
		, cgltf.Data data
		, GLTFNames names
		, GLib.File source_file
		, string destination_dir
		, bool has_state_machine
		, GLib.HashTable<cgltf.Material*, string> imported_materials
		, GLib.HashTable<cgltf.Material*, string> imported_skinned_materials
		, GLTFRigidAttachments rigid_attachments
		)
	{
		if (!options.import_units || !options.import_materials)
			return 0;

		string materials_path = destination_dir;
		if (options.create_materials_folder && data.materials_count != 0) {
			GLib.File materials_file = File.new_for_path(Path.build_filename(destination_dir, "materials"));
			try {
				materials_file.make_directory();
			} catch (GLib.IOError.EXISTS e) {
				// Ignore.
			} catch (GLib.Error e) {
				loge(e.message);
				return 1;
			}
			materials_path = materials_file.get_path();
		}

		GLib.HashTable<string, string> imported_textures = new GLib.HashTable<string, string>(GLib.str_hash, GLib.str_equal);
		GLib.HashTable<string, bool> used_material_resources = new GLib.HashTable<string, bool>(GLib.str_hash, GLib.str_equal);
		for (size_t i = 0; i < data.materials_count; ++i) {
			cgltf.Material* material = &data.materials[i];
			string filename = Path.build_filename(materials_path, names.material(data, material) + ".material");
			used_material_resources[ResourceId.name(ResourceId.normalize(project.resource_filename(filename)))] = true;
		}
		for (size_t i = 0; i < data.materials_count; ++i) {
			cgltf.Material* material = &data.materials[i];
			string material_filename = Path.build_filename(materials_path, names.material(data, material) + ".png");
			string material_resource_name = ResourceId.name(ResourceId.normalize(project.resource_filename(material_filename)));

			Vector3 albedo = Vector3(1.0, 1.0, 1.0);
			double metallic = 0.0;
			double roughness = 1.0;
			Vector3 emission_color = Vector3(material.emissive_factor[0], material.emissive_factor[1], material.emissive_factor[2]);
			double emission_intensity = material.has_emissive_strength
				? material.emissive_strength.emissive_strength
				: 1.0
				;
			string? albedo_map = null;
			string? normal_map = null;
			string? metallic_map = null;
			string? roughness_map = null;
			string? ao_map = null;
			string? emission_map = null;
			bool masking = material.alpha_mode == cgltf.AlphaMode.MASK;

			if (material.has_pbr_metallic_roughness) {
				albedo = Vector3(material.pbr_metallic_roughness.base_color_factor[0]
					, material.pbr_metallic_roughness.base_color_factor[1]
					, material.pbr_metallic_roughness.base_color_factor[2]
					);
				metallic = material.pbr_metallic_roughness.metallic_factor;
				roughness = material.pbr_metallic_roughness.roughness_factor;
			}

			if (material.alpha_mode == cgltf.AlphaMode.BLEND)
				logw("glTF material blending is not supported; '%s' will be imported as opaque".printf(names.material(data, material)));
			if (masking && !MathUtils.equal(material.alpha_cutoff, 0.5f))
				logw("glTF alpha cutoffs other than 0.5 are not supported");

			if (options.import_textures) {
				if (material.has_pbr_metallic_roughness) {
					if (import_texture(out albedo_map, db, project, data, names
						, material.pbr_metallic_roughness.base_color_texture
						, source_file, destination_dir, options.create_textures_folder
						, "_df", MeshResource.TextureUsage.COLOR, masking, -1, imported_textures) != 0)
						return 1;
					if (import_texture(out metallic_map, db, project, data, names
						, material.pbr_metallic_roughness.metallic_roughness_texture
						, source_file, destination_dir, options.create_textures_folder
						, "_mt", MeshResource.TextureUsage.DATA, false, 2, imported_textures) != 0)
						return 1;
					if (import_texture(out roughness_map, db, project, data, names
						, material.pbr_metallic_roughness.metallic_roughness_texture
						, source_file, destination_dir, options.create_textures_folder
						, "_rg", MeshResource.TextureUsage.DATA, false, 1, imported_textures) != 0)
						return 1;
				}
				if (import_texture(out normal_map, db, project, data, names, material.normal_texture
					, source_file, destination_dir, options.create_textures_folder
					, "_nr", MeshResource.TextureUsage.NORMAL, false, -1, imported_textures) != 0)
					return 1;
				if (import_texture(out ao_map, db, project, data, names, material.occlusion_texture
					, source_file, destination_dir, options.create_textures_folder
					, "_ao", MeshResource.TextureUsage.DATA, false, -1, imported_textures) != 0)
					return 1;
				if (import_texture(out emission_map, db, project, data, names, material.emissive_texture
					, source_file, destination_dir, options.create_textures_folder
					, "_em", MeshResource.TextureUsage.COLOR, false, -1, imported_textures) != 0)
					return 1;
			}

			bool uses_skinning = false;
			bool uses_static_mesh = false;
			if (has_state_machine) {
				for (size_t ni = 0; ni < data.nodes_count && (!uses_skinning || !uses_static_mesh); ++ni) {
					cgltf.Node* node = &data.nodes[ni];
					if (node.mesh == null)
						continue;
					bool skinned = node.skin != null || rigid_attachments.contains(node);
					for (size_t pi = 0; pi < node.mesh.primitives_count; ++pi) {
						if (node.mesh.primitives[pi].material != material)
							continue;
						uses_skinning = uses_skinning || skinned;
						uses_static_mesh = uses_static_mesh || !skinned;
					}
				}
			}
			string shader = uses_skinning && !uses_static_mesh ? "mesh+SKINNING" : "mesh";
			masking = masking && albedo_map != null;
			MaterialResource resource = MaterialResource.mesh(db
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
			if (resource.save(project, material_resource_name) != 0)
				return 1;
			imported_materials[material] = material_resource_name;

			if (uses_skinning && uses_static_mesh) {
				string skinned_material_resource_name = material_resource_name + "_skinned";
				while (used_material_resources.contains(skinned_material_resource_name))
					skinned_material_resource_name += "_skinned";
				used_material_resources[skinned_material_resource_name] = true;
				MaterialResource skinned_resource = MaterialResource.mesh(db
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
					, "mesh+SKINNING"
					, masking
					);
				if (skinned_resource.save(project, skinned_material_resource_name) != 0)
					return 1;
				imported_skinned_materials[material] = skinned_material_resource_name;
			} else if (uses_skinning) {
				imported_skinned_materials[material] = material_resource_name;
			}
		}
		return 0;
	}

	public static ImportResult do_import(SceneImportOptions options
		, Project project
		, string destination_dir
		, GLib.GenericArray<string> filenames
		)
	{
		try {
			File.new_for_path(destination_dir).make_directory_with_parents();
		} catch (GLib.IOError.EXISTS e) {
			// Ignore.
		} catch (GLib.Error e) {
			loge(e.message);
			return ImportResult.ERROR;
		}

		for (int fi = 0; fi < filenames.length; ++fi) {
			string filename = filenames[fi];
			GLib.File file_src = File.new_for_path(filename);
			GLib.File file_dst;
			string resource_path;
			if (get_destination_file(out file_dst, destination_dir, file_src) != 0)
				return ImportResult.ERROR;
			if (get_resource_path(out resource_path, file_dst, project) != 0)
				return ImportResult.ERROR;
			string resource_name = ResourceId.name(resource_path);
			string resource_basename = GLib.File.new_for_path(resource_name).get_basename();

			cgltf.Options load_options = {};
			cgltf.Data data;
			cgltf.Result load_result = cgltf.parse_file(ref load_options, filename, out data);
			if (load_result != cgltf.Result.SUCCESS) {
				loge("cgltf: failed to parse '%s' (%d)".printf(filename, load_result));
				return ImportResult.ERROR;
			}

			foreach (unowned string extension in data.extensions_required) {
				if (extension == "KHR_draco_mesh_compression"
					|| extension == "EXT_meshopt_compression"
					|| extension == "KHR_texture_basisu"
					) {
					loge("Unsupported required glTF extension '%s'".printf(extension));
					return ImportResult.ERROR;
				}
			}

			if (copy_external_buffers(data, file_src, project, destination_dir) != 0)
				return ImportResult.ERROR;
			load_result = cgltf.load_buffers(ref load_options, data, filename);
			if (load_result != cgltf.Result.SUCCESS) {
				loge("cgltf: failed to load buffers for '%s' (%d)".printf(filename, load_result));
				return ImportResult.ERROR;
			}
			cgltf.Scene* active_scene = data.scene != null
				? data.scene
				: (data.scenes_count != 0 ? &data.scenes[0] : null)
				;
			if (active_scene == null) {
				loge("glTF contains no scene");
				return ImportResult.ERROR;
			}
			GLTFNames names = new GLTFNames(data);
			GLTFSkinGroups skin_groups = new GLTFSkinGroups(data, active_scene);
			uint num_skin_groups = skin_groups.length();
			if (options.import_animation && !options.new_skeleton && num_skin_groups > 1) {
				loge("Importing multiple glTF armatures requires creating new skeletons");
				return ImportResult.ERROR;
			}
			bool import_clips = options.import_animation
				&& options.import_clips
				&& data.animations_count != 0
				&& num_skin_groups != 0
				;

			try {
				if (!file_src.equal(file_dst))
					file_src.copy(file_dst, FileCopyFlags.OVERWRITE);
			} catch (GLib.Error e) {
				loge(e.message);
				return ImportResult.ERROR;
			}

			Database db = new Database(project);
			GLib.HashTable<cgltf.Skin*, string> skin_state_machines = new GLib.HashTable<cgltf.Skin*, string>(GLib.direct_hash, GLib.direct_equal);
			GLib.GenericArray<string> target_skeletons = new GLib.GenericArray<string>();
			bool has_state_machine = false;
			string single_state_machine = "";
			string animations_path = destination_dir;
			if (import_clips && options.create_animations_folder) {
				GLib.File animations_file = File.new_for_path(Path.build_filename(destination_dir, "animations"));
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

			for (uint ai = 0; ai < num_skin_groups; ++ai) {
				int group = skin_groups.active_group(ai);
				cgltf.Skin* skin = skin_groups.primary(data, group);
				string group_resource_name = num_skin_groups == 1
					? resource_name
					: resource_name + "_" + names.skin(data, skin)
					;
				string initial_animation_name = "";
				if (import_clips) {
					initial_animation_name = animation_resource_name(project
						, animations_path
						, resource_basename
						, data
						, names
						, skin_groups
						, skin
						, &data.animations[0]
						);
				}
				string target_skeleton = options.target_skeleton;

				if (options.import_animation && options.new_skeleton) {
					GLib.HashTable<cgltf.Node*, bool> marked = new GLib.HashTable<cgltf.Node*, bool>(GLib.direct_hash, GLib.direct_equal);
					mark_bones(skin, marked);
					Guid animation_skeleton_id = Guid.new_guid();
					db.create(animation_skeleton_id, OBJECT_TYPE_MESH_SKELETON);
					db.set_string(animation_skeleton_id, "mesh_resource", resource_name);
					db.set_string(animation_skeleton_id, "skin_name", names.skin(data, skin));
					for (size_t ni = 0; ni < data.nodes_count; ++ni) {
						cgltf.Node* node = &data.nodes[ni];
						if (marked.contains(node) && (node.parent == null || !marked.contains(node.parent))) {
							Guid root_bone_id = Guid.new_guid();
							import_bone(db, data, names, marked, GUID_ZERO, root_bone_id, node);
							db.add_to_set(animation_skeleton_id, "skeleton", root_bone_id);
						}
					}
					if (db.save(project.absolute_path(group_resource_name) + "." + OBJECT_TYPE_MESH_SKELETON, animation_skeleton_id) != 0)
						return ImportResult.ERROR;
					target_skeleton = group_resource_name;

					StateMachineResource state_machine = StateMachineResource.mesh(db
						, Guid.new_guid()
						, target_skeleton
						, initial_animation_name != "" ? initial_animation_name : null
						);
					if (state_machine.save(project, group_resource_name) != 0)
						return ImportResult.ERROR;
					has_state_machine = true;
					if (num_skin_groups == 1)
						single_state_machine = group_resource_name;
					for (size_t si = 0; si < data.skins_count; ++si) {
						cgltf.Skin* member = &data.skins[si];
						if (skin_groups.group(data, member) == group)
							skin_state_machines[member] = group_resource_name;
					}
				}
				target_skeletons.add(target_skeleton);
			}

			if (import_clips) {
				for (size_t animation_i = 0; animation_i < data.animations_count; ++animation_i) {
					cgltf.Animation* animation = &data.animations[animation_i];
					for (uint ai = 0; ai < num_skin_groups; ++ai) {
						string target_skeleton = target_skeletons[ai];
						if (target_skeleton == "") {
							logw("Animation must have a target skeleton. Animation clips won't be imported.");
							continue;
						}
						int group = skin_groups.active_group(ai);
						cgltf.Skin* skin = skin_groups.primary(data, group);
						string clip_resource_name = animation_resource_name(project
							, animations_path
							, resource_basename
							, data
							, names
							, skin_groups
							, skin
							, animation
							);
						Guid animation_id = Guid.new_guid();
						db.create(animation_id, OBJECT_TYPE_MESH_ANIMATION);
						db.set_string(animation_id, "source", resource_path);
						db.set_string(animation_id, "target_skeleton", target_skeleton);
						db.set_string(animation_id, "stack_name", names.animation(data, animation));
						db.set_string(animation_id, "skin_name", names.skin(data, skin));
						if (db.save(project.absolute_path(clip_resource_name) + "." + OBJECT_TYPE_MESH_ANIMATION, animation_id) != 0)
							return ImportResult.ERROR;
					}
				}
			}

			GLTFRigidAttachments rigid_attachments = new GLTFRigidAttachments(data
				, active_scene
				, skin_groups
				, has_state_machine
				);
			if (!rigid_attachments.valid)
				return ImportResult.ERROR;

			GLib.HashTable<cgltf.Node*, string> node_state_machines = new GLib.HashTable<cgltf.Node*, string>(GLib.direct_hash, GLib.direct_equal);
			if (has_state_machine && num_skin_groups > 1) {
				GLib.HashTable<cgltf.Node*, string> subtree_state_machines = new GLib.HashTable<cgltf.Node*, string>(GLib.direct_hash, GLib.direct_equal);
				GLib.HashTable<cgltf.Node*, bool> mixed_subtrees = new GLib.HashTable<cgltf.Node*, bool>(GLib.direct_hash, GLib.direct_equal);
				for (size_t i = 0; i < active_scene.nodes.length; ++i) {
					collect_subtree_state_machines(active_scene.nodes[i], skin_state_machines, rigid_attachments
						, subtree_state_machines, mixed_subtrees
						);
				}
				for (size_t i = 0; i < active_scene.nodes.length; ++i) {
					mark_state_machine_roots(active_scene.nodes[i], null
						, subtree_state_machines, mixed_subtrees, node_state_machines
						);
				}
			}
			rigid_attachments.resolve_roots(data, active_scene, num_skin_groups == 1
				, skin_state_machines, node_state_machines
				);
			if (!rigid_attachments.valid)
				return ImportResult.ERROR;

			GLib.HashTable<cgltf.Material*, string> imported_materials = new GLib.HashTable<cgltf.Material*, string>(GLib.direct_hash, GLib.direct_equal);
			GLib.HashTable<cgltf.Material*, string> imported_skinned_materials = new GLib.HashTable<cgltf.Material*, string>(GLib.direct_hash, GLib.direct_equal);
			if (import_materials(options, db, project, data, names, file_src, destination_dir, has_state_machine
				, imported_materials, imported_skinned_materials, rigid_attachments) != 0)
				return ImportResult.ERROR;
			string skinned_fallback_material = "";
			if (options.import_units
				&& has_state_machine
				&& needs_skinned_fallback(options, data, rigid_attachments)
				) {
				skinned_fallback_material = resource_name + "_gltf_skinned_fallback";
				MaterialResource fallback = MaterialResource.mesh(db
					, Guid.new_guid()
					, null
					, null
					, null
					, null
					, null
					, null
					, Vector3(1.0, 1.0, 1.0)
					, 0.0
					, 1.0
					, Vector3(0.0, 0.0, 0.0)
					, 1.0
					, "mesh+SKINNING"
					, false
					);
				if (fallback.save(project, skinned_fallback_material) != 0)
					return ImportResult.ERROR;
			}

			if (options.import_units) {
				create_object_types(db);
				Guid unit_id;
				if (db.add_from_resource_path(out unit_id, resource_name + ".unit") != 0)
					unit_id = Guid.new_guid();
				bool attach_state_machine = has_state_machine && num_skin_groups > 1;

				if (active_scene.nodes.length == 1) {
					unit_create_components(options, db, GUID_ZERO, unit_id, resource_name, "root", data, names, active_scene.nodes[0]
						, imported_materials, imported_skinned_materials, skinned_fallback_material
						, rigid_attachments, attach_state_machine, node_state_machines
						);
				} else {
					Unit root = Unit(db, unit_id);
					if (!db.has_object(unit_id))
						root.create_empty();
					Guid transform_id;
					if (!root.has_component(out transform_id, OBJECT_TYPE_TRANSFORM)) {
						transform_id = Guid.new_guid();
						db.create(transform_id, OBJECT_TYPE_TRANSFORM);
						db.add_to_set(unit_id, "components", transform_id);
					}
					set_transform(root, transform_id, identity_transform(), resource_basename);
					db.set_name(unit_id, resource_basename);
					db.set_string(unit_id, "editor.import_path", "root");
					Guid?[] old_children = db.get_set(unit_id, "children");
					GLib.GenericSet<Guid?> matched = new GLib.GenericSet<Guid?>(Guid.hash_func, Guid.equal_func);
					for (size_t i = 0; i < active_scene.nodes.length; ++i) {
						cgltf.Node* node = active_scene.nodes[i];
						string node_name = names.node(data, node);
						string node_path = "root/" + ((uint)i).to_string() + ":" + node_name;
						Guid child_id = GUID_ZERO;
						foreach (Guid? old_id in old_children) {
							if (!matched.contains(old_id) && db.is_alive(old_id)
								&& db.get_string(old_id, "editor.import_path", "") == node_path) {
								child_id = old_id;
								break;
							}
						}
						if (child_id == GUID_ZERO)
							child_id = Guid.new_guid();
						if (unit_create_components(options, db, unit_id, child_id, resource_name, node_path, data, names, node
							, imported_materials, imported_skinned_materials, skinned_fallback_material
							, rigid_attachments, attach_state_machine, node_state_machines))
							matched.add(child_id);
					}
				}

				Unit unit = Unit(db, unit_id);
				if (single_state_machine != "") {
					Guid component_id;
					if (!unit.has_component(out component_id, OBJECT_TYPE_ANIMATION_STATE_MACHINE)) {
						component_id = Guid.new_guid();
						db.create(component_id, OBJECT_TYPE_ANIMATION_STATE_MACHINE);
						db.add_to_set(unit_id, "components", component_id);
					}
					unit.set_component_string(component_id, "data.state_machine_resource", single_state_machine);
				} else if (!attach_state_machine || active_scene.nodes.length != 1) {
					remove_owned_component(db, unit, unit_id, OBJECT_TYPE_ANIMATION_STATE_MACHINE);
				}

				if (db.save(project.absolute_path(resource_name) + ".unit", unit_id) != 0)
					return ImportResult.ERROR;
				Guid mesh_id = Guid.new_guid();
				db.create(mesh_id, OBJECT_TYPE_MESH);
				db.set_string(mesh_id, "source", resource_path);
				// The compiler derives rigid nodes, bones and roots from the glTF.
				// Only the generated unit's animation mode is not present in the source.
				db.set_bool(mesh_id, "rigid_skinning", has_state_machine);
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
		ImportResult result = do_import(options, project, destination_dir, filenames);
		if (result == ImportResult.SUCCESS && options_path != null) {
			try {
				SJSON.save(options.encode(), options_path);
			} catch (JsonWriteError e) {
				result = ImportResult.ERROR;
			}
		}
		import_result(result, primary_resource_path(project, destination_dir, filenames, result));
	}

	public static void import(Import import_result
		, Database database
		, string destination_dir
		, GLib.SList<string> filenames
		, Gtk.Window? parent_window
		)
	{
		GLib.GenericArray<string> gltf_filenames = new GLib.GenericArray<string>();
		foreach (unowned string filename in filenames)
			gltf_filenames.add(filename);

		SceneImportOptions options = new SceneImportOptions(SceneImportFlags.LIGHTS_AND_CAMERAS | SceneImportFlags.ANIMATIONS);
		GLib.File file_dst;
		string resource_path;
		get_destination_file(out file_dst, destination_dir, File.new_for_path(gltf_filenames[0]));
		get_resource_path(out resource_path, file_dst, database._project);
		string resource_name = ResourceId.name(resource_path);
		string options_path = database._project.absolute_path(resource_name) + ".importer_settings";
		try {
			options.decode(SJSON.load_from_path(options_path));
		} catch (JsonSyntaxError e) {
			// No-op.
		}

		if (parent_window == null) {
			import_with_options(import_result, options, database._project, destination_dir, gltf_filenames, options_path);
		} else {
			SceneImportDialog dialog = new SceneImportDialog(database
				, destination_dir
				, filenames
				, import_result
				, (owned)options
				, options_path
				, _("Import glTF...")
				, GLTFImporter.import_with_options
				);
			dialog.set_transient_for(parent_window);
			dialog.set_modal(true);
#if CROWN_GTK3
			dialog.show_all();
#else
			dialog.show();
#endif
			dialog.present();
		}
	}
}

} /* namespace Crown */
