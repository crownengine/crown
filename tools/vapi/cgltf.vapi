/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

[CCode (cheader_filename = "cgltf.h")]
namespace cgltf
{
[CCode (cname = "cgltf_result", cprefix = "cgltf_result_", has_type_id = false)]
public enum Result
{
	[CCode (cname = "cgltf_result_success")] SUCCESS,
	[CCode (cname = "cgltf_result_data_too_short")] DATA_TOO_SHORT,
	[CCode (cname = "cgltf_result_unknown_format")] UNKNOWN_FORMAT,
	[CCode (cname = "cgltf_result_invalid_json")] INVALID_JSON,
	[CCode (cname = "cgltf_result_invalid_gltf")] INVALID_GLTF,
	[CCode (cname = "cgltf_result_invalid_options")] INVALID_OPTIONS,
	[CCode (cname = "cgltf_result_file_not_found")] FILE_NOT_FOUND,
	[CCode (cname = "cgltf_result_io_error")] IO_ERROR,
	[CCode (cname = "cgltf_result_out_of_memory")] OUT_OF_MEMORY,
	[CCode (cname = "cgltf_result_legacy_gltf")] LEGACY_GLTF
}

[CCode (cname = "cgltf_primitive_type", cprefix = "cgltf_primitive_type_", has_type_id = false)]
public enum PrimitiveType
{
	[CCode (cname = "cgltf_primitive_type_invalid")] INVALID,
	[CCode (cname = "cgltf_primitive_type_points")] POINTS,
	[CCode (cname = "cgltf_primitive_type_lines")] LINES,
	[CCode (cname = "cgltf_primitive_type_line_loop")] LINE_LOOP,
	[CCode (cname = "cgltf_primitive_type_line_strip")] LINE_STRIP,
	[CCode (cname = "cgltf_primitive_type_triangles")] TRIANGLES,
	[CCode (cname = "cgltf_primitive_type_triangle_strip")] TRIANGLE_STRIP,
	[CCode (cname = "cgltf_primitive_type_triangle_fan")] TRIANGLE_FAN
}

[CCode (cname = "cgltf_attribute_type", cprefix = "cgltf_attribute_type_", has_type_id = false)]
public enum AttributeType
{
	[CCode (cname = "cgltf_attribute_type_invalid")] INVALID,
	[CCode (cname = "cgltf_attribute_type_position")] POSITION,
	[CCode (cname = "cgltf_attribute_type_normal")] NORMAL,
	[CCode (cname = "cgltf_attribute_type_tangent")] TANGENT,
	[CCode (cname = "cgltf_attribute_type_texcoord")] TEXCOORD,
	[CCode (cname = "cgltf_attribute_type_color")] COLOR,
	[CCode (cname = "cgltf_attribute_type_joints")] JOINTS,
	[CCode (cname = "cgltf_attribute_type_weights")] WEIGHTS,
	[CCode (cname = "cgltf_attribute_type_custom")] CUSTOM
}

[CCode (cname = "cgltf_camera_type", cprefix = "cgltf_camera_type_", has_type_id = false)]
public enum CameraType
{
	[CCode (cname = "cgltf_camera_type_invalid")] INVALID,
	[CCode (cname = "cgltf_camera_type_perspective")] PERSPECTIVE,
	[CCode (cname = "cgltf_camera_type_orthographic")] ORTHOGRAPHIC
}

[CCode (cname = "cgltf_light_type", cprefix = "cgltf_light_type_", has_type_id = false)]
public enum LightType
{
	[CCode (cname = "cgltf_light_type_invalid")] INVALID,
	[CCode (cname = "cgltf_light_type_directional")] DIRECTIONAL,
	[CCode (cname = "cgltf_light_type_point")] POINT,
	[CCode (cname = "cgltf_light_type_spot")] SPOT
}

[CCode (cname = "cgltf_alpha_mode", cprefix = "cgltf_alpha_mode_", has_type_id = false)]
public enum AlphaMode
{
	[CCode (cname = "cgltf_alpha_mode_opaque")] OPAQUE,
	[CCode (cname = "cgltf_alpha_mode_mask")] MASK,
	[CCode (cname = "cgltf_alpha_mode_blend")] BLEND
}

[SimpleType]
[CCode (cname = "cgltf_options", destroy_function = "", has_type_id = false)]
public struct Options
{
}

[SimpleType]
[CCode (cname = "cgltf_buffer", destroy_function = "", has_type_id = false)]
public struct Buffer
{
	public size_t size;
	public string? uri;
	public void* data;
}

[SimpleType]
[CCode (cname = "cgltf_buffer_view", destroy_function = "", has_type_id = false)]
public struct BufferView
{
	public Buffer* buffer;
	public size_t offset;
	public size_t size;
	public void* data;
}

[SimpleType]
[CCode (cname = "cgltf_accessor", destroy_function = "", has_type_id = false)]
public struct Accessor
{
	public size_t count;
}

[SimpleType]
[CCode (cname = "cgltf_image", destroy_function = "", has_type_id = false)]
public struct Image
{
	public string? name;
	public string? uri;
	public BufferView* buffer_view;
	public string? mime_type;
}

[SimpleType]
[CCode (cname = "cgltf_texture", destroy_function = "", has_type_id = false)]
public struct Texture
{
	public Image* image;
}

[SimpleType]
[CCode (cname = "cgltf_texture_view", destroy_function = "", has_type_id = false)]
public struct TextureView
{
	public Texture* texture;
}

[SimpleType]
[CCode (cname = "cgltf_pbr_metallic_roughness", destroy_function = "", has_type_id = false)]
public struct PbrMetallicRoughness
{
	public TextureView base_color_texture;
	public TextureView metallic_roughness_texture;
	public float base_color_factor[4];
	public float metallic_factor;
	public float roughness_factor;
}

[SimpleType]
[CCode (cname = "cgltf_emissive_strength", destroy_function = "", has_type_id = false)]
public struct EmissiveStrength
{
	public float emissive_strength;
}

[SimpleType]
[CCode (cname = "cgltf_material", destroy_function = "", has_type_id = false)]
public struct Material
{
	public string? name;
	public bool has_pbr_metallic_roughness;
	public bool has_emissive_strength;
	public PbrMetallicRoughness pbr_metallic_roughness;
	public EmissiveStrength emissive_strength;
	public TextureView normal_texture;
	public TextureView occlusion_texture;
	public TextureView emissive_texture;
	public float emissive_factor[3];
	public AlphaMode alpha_mode;
	public float alpha_cutoff;
}

[SimpleType]
[CCode (cname = "cgltf_primitive", destroy_function = "", has_type_id = false)]
public struct Primitive
{
	public PrimitiveType type;
	public Accessor* indices;
	public Material* material;
}

[SimpleType]
[CCode (cname = "cgltf_mesh", destroy_function = "", has_type_id = false)]
public struct Mesh
{
	public size_t primitives_count;
	[CCode (array_length_cname = "primitives_count", array_length_type = "size_t")]
	public Primitive[] primitives;
}

[SimpleType]
[CCode (cname = "cgltf_skin", destroy_function = "", has_type_id = false)]
public struct Skin
{
	public string? name;
	[CCode (array_length_cname = "joints_count", array_length_type = "size_t")]
	public Node*[] joints;
	public Node* skeleton;
}

[SimpleType]
[CCode (cname = "cgltf_camera_perspective", destroy_function = "", has_type_id = false)]
public struct CameraPerspective
{
	public float yfov;
	public bool has_zfar;
	public float zfar;
	public float znear;
}

[SimpleType]
[CCode (cname = "cgltf_camera_orthographic", destroy_function = "", has_type_id = false)]
public struct CameraOrthographic
{
	public float ymag;
	public float zfar;
	public float znear;
}

[SimpleType]
[CCode (cname = "cgltf_camera", destroy_function = "", has_type_id = false)]
public struct Camera
{
	public CameraType type;
	[CCode (cname = "data.perspective")]
	public CameraPerspective perspective;
	[CCode (cname = "data.orthographic")]
	public CameraOrthographic orthographic;
}

[SimpleType]
[CCode (cname = "cgltf_light", destroy_function = "", has_type_id = false)]
public struct Light
{
	public float color[3];
	public float intensity;
	public LightType type;
	public float range;
	public float spot_outer_cone_angle;
}

[SimpleType]
[CCode (cname = "cgltf_node", destroy_function = "", has_type_id = false)]
public struct Node
{
	public string? name;
	public Node* parent;
	[CCode (array_length_cname = "children_count", array_length_type = "size_t")]
	public Node*[] children;
	public Skin* skin;
	public Mesh* mesh;
	public Camera* camera;
	public Light* light;
}

[SimpleType]
[CCode (cname = "cgltf_scene", destroy_function = "", has_type_id = false)]
public struct Scene
{
	[CCode (array_length_cname = "nodes_count", array_length_type = "size_t")]
	public Node*[] nodes;
}

[SimpleType]
[CCode (cname = "cgltf_animation", destroy_function = "", has_type_id = false)]
public struct Animation
{
	public string? name;
}

[Compact]
[CCode (cname = "cgltf_data", free_function = "cgltf_free", has_type_id = false)]
public class Data
{
	public size_t materials_count;
	[CCode (array_length_cname = "materials_count", array_length_type = "size_t")]
	public Material[] materials;
	public size_t buffers_count;
	[CCode (array_length_cname = "buffers_count", array_length_type = "size_t")]
	public Buffer[] buffers;
	public size_t images_count;
	[CCode (array_length_cname = "images_count", array_length_type = "size_t")]
	public Image[] images;
	public size_t skins_count;
	[CCode (array_length_cname = "skins_count", array_length_type = "size_t")]
	public Skin[] skins;
	public size_t nodes_count;
	[CCode (array_length_cname = "nodes_count", array_length_type = "size_t")]
	public Node[] nodes;
	public size_t scenes_count;
	[CCode (array_length_cname = "scenes_count", array_length_type = "size_t")]
	public Scene[] scenes;
	public Scene* scene;
	public size_t animations_count;
	[CCode (array_length_cname = "animations_count", array_length_type = "size_t")]
	public Animation[] animations;
	[CCode (array_length_cname = "extensions_required_count", array_length_type = "size_t")]
	public string[] extensions_required;
}

[CCode (cname = "cgltf_parse_file")]
public Result parse_file(ref Options options, string path, out Data data);

[CCode (cname = "cgltf_load_buffers")]
public Result load_buffers(ref Options options, Data data, string path);

[CCode (cname = "cgltf_node_transform_local")]
public void node_transform_local(Node* node, [CCode (array_length = false)] float[] matrix);

[CCode (cname = "cgltf_node_index")]
public size_t node_index(Data data, Node* node);

[CCode (cname = "cgltf_material_index")]
public size_t material_index(Data data, Material* material);

[CCode (cname = "cgltf_image_index")]
public size_t image_index(Data data, Image* image);

[CCode (cname = "cgltf_skin_index")]
public size_t skin_index(Data data, Skin* skin);

[CCode (cname = "cgltf_animation_index")]
public size_t animation_index(Data data, Animation* animation);

[CCode (cname = "cgltf_find_accessor")]
public Accessor* find_accessor(Primitive* primitive, AttributeType type, int index);

} /* namespace cgltf */
