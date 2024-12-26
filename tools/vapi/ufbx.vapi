/*
 * Copyright (c) 2012-2024 Daniele Bartolini <dbartolini@crownengine.org>
 * SPDX-License-Identifier: MIT
 */

[CCode (cheader_filename = "ufbx.h")]
namespace ufbx
{
public const int ERROR_STACK_MAX_DEPTH;
public const int PANIC_MESSAGE_LENGTH;
public const int ERROR_INFO_LENGTH;

[SimpleType]
[CCode (cname = "ufbx_real", has_type_id = false)]
public struct Real : double
{
}

[SimpleType]
[CCode (cname = "ufbx_string", destroy_function = "", has_type_id = false)]
public struct String
{
	char* data;
	size_t size;
}

[SimpleType]
[CCode (cname = "ufbx_blob", destroy_function = "", has_type_id = false)]
public struct Blob
{
	void* data;
	size_t size;
}

[SimpleType]
[CCode (cname = "ufbx_vec2", destroy_function = "", has_type_id = false)]
public struct Vec2
{
	Real x;
	Real y;
	[CCode (cname = "v")]
	Real v[2];
}

[SimpleType]
[CCode (cname = "ufbx_vec3", destroy_function = "", has_type_id = false)]
public struct Vec3
{
	Real x;
	Real y;
	Real z;
	[CCode (cname = "v")]
	Real v[3];
}

[SimpleType]
[CCode (cname = "ufbx_vec4", destroy_function = "", has_type_id = false)]
public struct Vec4
{
	Real x;
	Real y;
	Real z;
	Real w;
	[CCode (cname = "v")]
	Real v[3];
}

[SimpleType]
[CCode (cname = "ufbx_quat", destroy_function = "", has_type_id = false)]
public struct Quat
{
	Real x;
	Real y;
	Real z;
	Real w;
	[CCode (cname = "v")]
	Real v[4];
}

[CCode (cname = "ufbx_rotation_order", cprefix = "UFBX_ROTATION_ORDER_", has_type_id = false)]
public enum RotationOrder
{
	XYZ,
	XZY,
	YZX,
	YXZ,
	ZXY,
	ZYX,
	SPHERIC,
	COUNT
}

[SimpleType]
[CCode (cname = "ufbx_transform", destroy_function = "", has_type_id = false)]
public struct Transform
{
	Vec3 translation;
	Quat rotation;
	Vec3 scale;
}

[CCode (cname = "ufbx_matrix", destroy_function = "", has_type_id = false)]
public struct Matrix
{
	Real m00;
	Real m10;
	Real m20;
	Real m01;
	Real m11;
	Real m21;
	Real m02;
	Real m12;
	Real m22;
	Real m03;
	Real m13;
	Real m23;
	[CCode (cname = "cols")]
	Vec3 cols[4];
	[CCode (cname = "v")]
	Real v[12];
}

[CCode (cname = "ufbx_void_list", destroy_function = "", has_type_id = false)]
public struct VoidList
{
	void* data;
	size_t count;
}

[CCode (cname = "ufbx_bool_list", destroy_function = "", has_type_id = false)]
public struct BoolList
{
	bool* data;
	size_t count;
}

[CCode (cname = "ufbx_uint32_list", destroy_function = "", has_type_id = false)]
public struct Uint32List
{
	uint32* data;
	size_t count;
}

[CCode (cname = "ufbx_real_list", destroy_function = "", has_type_id = false)]
public struct RealList
{
	Real* data;
	size_t count;
}

[CCode (cname = "ufbx_vec2_list", destroy_function = "", has_type_id = false)]
public struct Vec2List
{
	Vec2* data;
	size_t count;
}

[CCode (cname = "ufbx_vec3_list", destroy_function = "", has_type_id = false)]
public struct Vec3List
{
	Vec3* data;
	size_t count;
}

[CCode (cname = "ufbx_vec4_list", destroy_function = "", has_type_id = false)]
public struct Vec4List
{
	Vec4* data;
	size_t count;
}

[CCode (cname = "ufbx_string_list", destroy_function = "", has_type_id = false)]
public struct StringList
{
	String* data;
	size_t count;
}

public const uint32 NO_INDEX;
[CCode (cname = "ufbx_dom_value_type", cprefix = "UFBX_DOM_VALUE_", has_type_id = false)]
public enum DomValueType
{
	NUMBER,
	STRING,
	ARRAY_I8,
	ARRAY_I32,
	ARRAY_I64,
	ARRAY_F32,
	ARRAY_F64,
	ARRAY_RAW_STRING,
	ARRAY_IGNORED,
	TYPE_COUNT
}

[CCode (cname = "ufbx_dom_value", destroy_function = "", has_type_id = false)]
public struct DomValue
{
	DomValueType type;
	String value_str;
	Blob value_blob;
	int64 value_int;
	double value_float;
}

[CCode (cname = "ufbx_dom_node_list", destroy_function = "", has_type_id = false)]
public struct DomNodeList
{
	DomNode** data;
	size_t count;
}

[CCode (cname = "ufbx_dom_value_list", destroy_function = "", has_type_id = false)]
public struct DomValueList
{
	DomValue* data;
	size_t count;
}

[CCode (cname = "ufbx_dom_node", destroy_function = "", has_type_id = false)]
public struct DomNode
{
	String name;
	DomNodeList children;
	DomValueList values;
}

[CCode (cname = "ufbx_prop_type", cprefix = "UFBX_PROP_", has_type_id = false)]
public enum PropType
{
	UNKNOWN,
	BOOLEAN,
	INTEGER,
	NUMBER,
	VECTOR,
	COLOR,
	COLOR_WITH_ALPHA,
	STRING,
	DATE_TIME,
	TRANSLATION,
	ROTATION,
	SCALING,
	DISTANCE,
	COMPOUND,
	BLOB,
	REFERENCE,
	TYPE_COUNT
}

[CCode (cname = "ufbx_prop_flags", cprefix = "UFBX_PROP_FLAG_", has_type_id = false)]
[Flags]
public enum PropFlags
{
	ANIMATABLE,
	USER_DEFINED,
	HIDDEN,
	LOCK_X,
	LOCK_Y,
	LOCK_Z,
	LOCK_W,
	MUTE_X,
	MUTE_Y,
	MUTE_Z,
	MUTE_W,
	SYNTHETIC,
	ANIMATED,
	NOT_FOUND,
	CONNECTED,
	NO_VALUE,
	OVERRIDDEN,
	VALUE_REAL,
	VALUE_VEC2,
	VALUE_VEC3,
	VALUE_VEC4,
	VALUE_INT,
	VALUE_STR,
	VALUE_BLOB
}

[CCode (cname = "ufbx_prop", destroy_function = "", has_type_id = false)]
public struct Prop
{
	String name;
	uint32 _internal_key;
	PropType type;
	PropFlags flags;
	String value_str;
	Blob value_blob;
	int64 value_int;
	[CCode (cname = "value_real_arr")]
	Real value_real_arr[4];
	[CCode (cname = "value_real")]
	Real value_real;
	[CCode (cname = "value_vec2")]
	Vec2 value_vec2;
	[CCode (cname = "value_vec3")]
	Vec3 value_vec3;
	[CCode (cname = "value_vec4")]
	Vec4 value_vec4;
}

[CCode (cname = "ufbx_prop_list", destroy_function = "", has_type_id = false)]
public struct PropList
{
	Prop* data;
	size_t count;
}

[CCode (cname = "ufbx_props", destroy_function = "", has_type_id = false)]
public struct Props
{
	PropList props;
	size_t num_animated;
	Props? defaults;
}

[CCode (cname = "ufbx_element_list", destroy_function = "", has_type_id = false)]
public struct ElementList
{
	Element** data;
	size_t count;
}
[CCode (cname = "ufbx_unknown_list", destroy_function = "", has_type_id = false)]
public struct UnknownList
{
	Unknown** data;
	size_t count;
}
[CCode (cname = "ufbx_node_list", destroy_function = "", has_type_id = false)]
public struct NodeList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public Node[] data;
}
[CCode (cname = "ufbx_mesh_list", destroy_function = "", has_type_id = false)]
public struct MeshList
{
	Mesh** data;
	size_t count;
}
[CCode (cname = "ufbx_light_list", destroy_function = "", has_type_id = false)]
public struct LightList
{
	Light** data;
	size_t count;
}
[CCode (cname = "ufbx_camera_list", destroy_function = "", has_type_id = false)]
public struct CameraList
{
	Camera** data;
	size_t count;
}
[CCode (cname = "ufbx_bone_list", destroy_function = "", has_type_id = false)]
public struct BoneList
{
	Bone** data;
	size_t count;
}
[CCode (cname = "ufbx_empty_list", destroy_function = "", has_type_id = false)]
public struct EmptyList
{
	Empty** data;
	size_t count;
}
[CCode (cname = "ufbx_line_curve_list", destroy_function = "", has_type_id = false)]
public struct LineCurveList
{
	LineCurve** data;
	size_t count;
}
[CCode (cname = "ufbx_nurbs_curve_list", destroy_function = "", has_type_id = false)]
public struct NurbsCurveList
{
	NurbsCurve** data;
	size_t count;
}
[CCode (cname = "ufbx_nurbs_surface_list", destroy_function = "", has_type_id = false)]
public struct NurbsSurfaceList
{
	NurbsSurface** data;
	size_t count;
}
[CCode (cname = "ufbx_nurbs_trim_surface_list", destroy_function = "", has_type_id = false)]
public struct NurbsTrimSurfaceList
{
	NurbsTrimSurface** data;
	size_t count;
}
[CCode (cname = "ufbx_nurbs_trim_boundary_list", destroy_function = "", has_type_id = false)]
public struct NurbsTrimBoundaryList
{
	NurbsTrimBoundary** data;
	size_t count;
}
[CCode (cname = "ufbx_procedural_geometry_list", destroy_function = "", has_type_id = false)]
public struct ProceduralGeometryList
{
	ProceduralGeometry** data;
	size_t count;
}
[CCode (cname = "ufbx_stereo_camera_list", destroy_function = "", has_type_id = false)]
public struct StereoCameraList
{
	StereoCamera** data;
	size_t count;
}
[CCode (cname = "ufbx_camera_switcher_list", destroy_function = "", has_type_id = false)]
public struct CameraSwitcherList
{
	CameraSwitcher** data;
	size_t count;
}
[CCode (cname = "ufbx_marker_list", destroy_function = "", has_type_id = false)]
public struct MarkerList
{
	Marker** data;
	size_t count;
}
[CCode (cname = "ufbx_lod_group_list", destroy_function = "", has_type_id = false)]
public struct LogGroupList
{
	LodGroup** data;
	size_t count;
}
[CCode (cname = "ufbx_skin_deformer_list", destroy_function = "", has_type_id = false)]
public struct SkinDeformerList
{
	SkinDeformer** data;
	size_t count;
}
[CCode (cname = "ufbx_skin_cluster_list", destroy_function = "", has_type_id = false)]
public struct SkinClusterList
{
	SkinCluster** data;
	size_t count;
}
[CCode (cname = "ufbx_blend_deformer_list", destroy_function = "", has_type_id = false)]
public struct BlendDeformerList
{
	BlendDeformer** data;
	size_t count;
}
[CCode (cname = "ufbx_blend_channel_list", destroy_function = "", has_type_id = false)]
public struct BlendChannelList
{
	BlendChannel** data;
	size_t count;
}
[CCode (cname = "ufbx_blend_shape_list", destroy_function = "", has_type_id = false)]
public struct BlendShapeList
{
	BlendShape** data;
	size_t count;
}
[CCode (cname = "ufbx_cache_deformer_list", destroy_function = "", has_type_id = false)]
public struct CacheDeformerList
{
	CacheDeformer** data;
	size_t count;
}
[CCode (cname = "ufbx_cache_file_list", destroy_function = "", has_type_id = false)]
public struct CacheFileList
{
	CacheFile** data;
	size_t count;
}
[CCode (cname = "ufbx_material_list", destroy_function = "", has_type_id = false)]
public struct MaterialList
{
	Material** data;
	size_t count;
}
[CCode (cname = "ufbx_texture_list", destroy_function = "", has_type_id = false)]
public struct TextureList
{
	Texture** data;
	size_t count;
}
[CCode (cname = "ufbx_video_list", destroy_function = "", has_type_id = false)]
public struct VideoList
{
	Video** data;
	size_t count;
}
[CCode (cname = "ufbx_shader_list", destroy_function = "", has_type_id = false)]
public struct ShaderList
{
	Shader** data;
	size_t count;
}
[CCode (cname = "ufbx_shader_binding_list", destroy_function = "", has_type_id = false)]
public struct ShaderBindingList
{
	ShaderBinding** data;
	size_t count;
}
[CCode (cname = "ufbx_anim_stack_list", destroy_function = "", has_type_id = false)]
public struct AnimStackList
{
	AnimStack** data;
	size_t count;
}
[CCode (cname = "ufbx_anim_layer_list", destroy_function = "", has_type_id = false)]
public struct AnimLayerList
{
	AnimLayer** data;
	size_t count;
}
[CCode (cname = "ufbx_anim_value_list", destroy_function = "", has_type_id = false)]
public struct AnimValueList
{
	AnimValue** data;
	size_t count;
}
[CCode (cname = "ufbx_anim_curve_list", destroy_function = "", has_type_id = false)]
public struct AnimCurveList
{
	AnimCurve** data;
	size_t count;
}
[CCode (cname = "ufbx_display_layer_list", destroy_function = "", has_type_id = false)]
public struct DisplayLayerList
{
	DisplayLayer** data;
	size_t count;
}
[CCode (cname = "ufbx_selection_set_list", destroy_function = "", has_type_id = false)]
public struct SelectionSetList
{
	SelectionSet** data;
	size_t count;
}
[CCode (cname = "ufbx_selection_node_list", destroy_function = "", has_type_id = false)]
public struct SelectionNodeList
{
	SelectionNode** data;
	size_t count;
}
[CCode (cname = "ufbx_character_list", destroy_function = "", has_type_id = false)]
public struct CharacterList
{
	Character** data;
	size_t count;
}
[CCode (cname = "ufbx_constraint_list", destroy_function = "", has_type_id = false)]
public struct ConstraintList
{
	Constraint** data;
	size_t count;
}
[CCode (cname = "ufbx_audio_layer_list", destroy_function = "", has_type_id = false)]
public struct AudioLayerList
{
	AudioLayer** data;
	size_t count;
}
[CCode (cname = "ufbx_audio_clip_list", destroy_function = "", has_type_id = false)]
public struct AudioClipList
{
	AudioClip** data;
	size_t count;
}
[CCode (cname = "ufbx_pose_list", destroy_function = "", has_type_id = false)]
public struct PoseList
{
	Pose** data;
	size_t count;
}
[CCode (cname = "ufbx_metadata_object_list", destroy_function = "", has_type_id = false)]
public struct MetadataObjectList
{
	MetadataObject** data;
	size_t count;
}

[CCode (cname = "ufbx_element_type", cprefix = "UFBX_ELEMENT_", has_type_id = false)]
public enum ElementType
{
	UNKNOWN,
	NODE,
	MESH,
	LIGHT,
	CAMERA,
	BONE,
	EMPTY,
	LINE_CURVE,
	NURBS_CURVE,
	NURBS_SURFACE,
	NURBS_TRIM_SURFACE,
	NURBS_TRIM_BOUNDARY,
	PROCEDURAL_GEOMETRY,
	STEREO_CAMERA,
	CAMERA_SWITCHER,
	MARKER,
	LOD_GROUP,
	SKIN_DEFORMER,
	SKIN_CLUSTER,
	BLEND_DEFORMER,
	BLEND_CHANNEL,
	BLEND_SHAPE,
	CACHE_DEFORMER,
	CACHE_FILE,
	MATERIAL,
	TEXTURE,
	VIDEO,
	SHADER,
	SHADER_BINDING,
	ANIM_STACK,
	ANIM_LAYER,
	ANIM_VALUE,
	ANIM_CURVE,
	DISPLAY_LAYER,
	SELECTION_SET,
	SELECTION_NODE,
	CHARACTER,
	CONSTRAINT,
	AUDIO_LAYER,
	AUDIO_CLIP,
	POSE,
	METADATA_OBJECT,
	TYPE_COUNT,
	TYPE_FIRST_ATTRIB = MESH,
	TYPE_LAST_ATTRIB  = LOD_GROUP,
}

[CCode (cname = "ufbx_connection", destroy_function = "", has_type_id = false)]
public struct Connection
{
	Element* src;
	Element* dst;
	String src_prop;
	String dst_prop;
}

[CCode (cname = "ufbx_connection_list", destroy_function = "", has_type_id = false)]
public struct ConnectionList
{
	Connection* data;
	size_t count;
}

[CCode (cname = "ufbx_element", destroy_function = "", has_type_id = false)]
public struct Element
{
	public String name;
	public Props props;
	public uint32 element_id;
	public uint32 typed_id;
	public NodeList instances;
	public ElementType type;
	public ConnectionList connections_src;
	public ConnectionList connections_dst;
	public unowned DomNode? dom_node;
	public unowned Scene scene;
}

[CCode (cname = "ufbx_unknown", destroy_function = "", has_type_id = false)]
public struct Unknown
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	String type;
	String super_type;
	String sub_type;
}

[CCode (cname = "ufbx_inherit_mode", cprefix = "UFBX_INHERIT_MODE_", has_type_id = false)]
public enum InheritMode
{
	NORMAL,
	IGNORE_PARENT_SCALE,
	COMPONENTWISE_SCALE,
	COUNT
}

[CCode (cname = "ufbx_mirror_axis", cprefix = "UFBX_MIRROR_AXIS_", has_type_id = false)]
public enum MirrorAxis
{
	NONE,
	X,
	Y,
	Z,
	COUNT
}

[Compact]
[CCode (cname = "ufbx_node", destroy_function = "", has_type_id = false)]
public class Node
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	public Node? parent;
	public NodeList children;
	public Mesh? mesh;
	public Light? light;
	public Camera? camera;
	public Bone? bone;
	public Element? attrib;
	public Node? geometry_transform_helper;
	public Node? scale_helper;
	public ElementType attrib_type;
	public ElementList all_attribs;
	public InheritMode inherit_mode;
	public InheritMode original_inherit_mode;
	public Transform local_transform;
	public Transform geometry_transform;
	public Vec3 inherit_scale;
	public Node? inherit_scale_node;
	public RotationOrder rotation_order;
	public Vec3 euler_rotation;
	public Matrix node_to_parent;
	public Matrix node_to_world;
	public Matrix geometry_to_node;
	public Matrix geometry_to_world;
	public Matrix unscaled_node_to_world;
	public Vec3 adjust_pre_translation;
	public Quat adjust_pre_rotation;
	public Real adjust_pre_scale;
	public Quat adjust_post_rotation;
	public Real adjust_post_scale;
	public Real adjust_translation_scale;
	public MirrorAxis adjust_mirror_axis;
	public MaterialList materials;
	public Pose? bind_pose;
	public bool visible;
	public bool is_root;
	public bool has_geometry_transform;
	public bool has_adjust_transform;
	public bool has_root_adjust_transform;
	public bool is_geometry_transform_helper;
	public bool is_scale_helper;
	public bool is_scale_compensate_parent;
	public uint32 node_depth;
}

[CCode (cname = "ufbx_vertex_attrib", destroy_function = "", has_type_id = false)]
public struct VertexAttrib
{
	bool exists;
	VoidList values;
	Uint32List indices;
	size_t value_reals;
	bool unique_per_vertex;
	RealList values_w;
}

[CCode (cname = "ufbx_vertex_real", destroy_function = "", has_type_id = false)]
public struct VertexReal
{
	bool exists;
	RealList values;
	Uint32List indices;
	size_t value_reals;
	bool unique_per_vertex;
	RealList values_w;
}

[CCode (cname = "ufbx_vertex_vec2", destroy_function = "", has_type_id = false)]
public struct VertexVec2
{
	bool exists;
	Vec2List values;
	Uint32List indices;
	size_t value_reals;
	bool unique_per_vertex;
	RealList values_w;
}

[CCode (cname = "ufbx_vertex_vec3", destroy_function = "", has_type_id = false)]
public struct VertexVec3
{
	bool exists;
	Vec3List values;
	Uint32List indices;
	size_t value_reals;
	bool unique_per_vertex;
	RealList values_w;
}

[CCode (cname = "ufbx_vertex_vec4", destroy_function = "", has_type_id = false)]
public struct VertexVec4
{
	bool exists;
	Vec4List values;
	Uint32List indices;
	size_t value_reals;
	bool unique_per_vertex;
	RealList values_w;
}

[CCode (cname = "ufbx_uv_set", destroy_function = "", has_type_id = false)]
public struct UvSet
{
	String name;
	uint32 index;
	VertexVec2 vertex_uv;
	VertexVec3 vertex_tangent;
	VertexVec3 vertex_bitangent;
}

[CCode (cname = "ufbx_color_set", destroy_function = "", has_type_id = false)]
public struct ColorSet
{
	String name;
	uint32 index;
	VertexVec4 vertex_color;
}

[CCode (cname = "ufbx_uv_set_list", destroy_function = "", has_type_id = false)]
public struct UvSetList
{
	UvSet* data;
	size_t count;
}

[CCode (cname = "ufbx_color_set_list", destroy_function = "", has_type_id = false)]
public struct ColorSetList
{
	ColorSet* data;
	size_t count;
}

[CCode (cname = "ufbx_edge", destroy_function = "", has_type_id = false)]
public struct Edge
{
	[CCode (cname = "a")]
	uint32 a;
	[CCode (cname = "b")]
	uint32 b;
	[CCode (cname = "indices")]
	uint32 indices[2];
}

[CCode (cname = "ufbx_edge_list", destroy_function = "", has_type_id = false)]
public struct EdgeList
{
	Edge* data;
	size_t count;
}

[CCode (cname = "ufbx_face", destroy_function = "", has_type_id = false)]
public struct Face
{
	uint32 index_begin;
	uint32 num_indices;
}

[CCode (cname = "ufbx_face_list", destroy_function = "", has_type_id = false)]
public struct FaceList
{
	Face* data;
	size_t count;
}

[CCode (cname = "ufbx_mesh_part", destroy_function = "", has_type_id = false)]
public struct MeshPart
{
	uint32 index;
	size_t num_faces;
	size_t num_triangles;
	size_t num_empty_faces;
	size_t num_point_faces;
	size_t num_line_faces;
	Uint32List face_indices;
}

[CCode (cname = "ufbx_mesh_part_list", destroy_function = "", has_type_id = false)]
public struct MeshPartList
{
	MeshPart* data;
	size_t count;
}

[CCode (cname = "ufbx_face_group", destroy_function = "", has_type_id = false)]
public struct FaceGroup
{
	int32 id;
	String name;
}

[CCode (cname = "ufbx_face_group_list", destroy_function = "", has_type_id = false)]
public struct FaceGroupList
{
	FaceGroup* data;
	size_t count;
}

[CCode (cname = "ufbx_subdivision_weight_range", destroy_function = "", has_type_id = false)]
public struct SubdivisionWeightRange
{
	uint32 weight_begin;
	uint32 num_weights;
}

[CCode (cname = "ufbx_subdivision_weight_range_list", destroy_function = "", has_type_id = false)]
public struct SubdivisionWeightRangeList
{
	SubdivisionWeightRange* data;
	size_t count;
}

[CCode (cname = "ufbx_subdivision_weight", destroy_function = "", has_type_id = false)]
public struct SubdivisionWeight
{
	Real weight;
	uint32 index;
}

[CCode (cname = "ufbx_subdivision_weight_list", destroy_function = "", has_type_id = false)]
public struct SubdivisionWeightList
{
	SubdivisionWeight* data;
	size_t count;
}

[CCode (cname = "ufbx_subdivision_result", destroy_function = "", has_type_id = false)]
public struct SubdivisionResult
{
	size_t result_memory_used;
	size_t temp_memory_used;
	size_t result_allocs;
	size_t temp_allocs;
	SubdivisionWeightRangeList source_vertex_ranges;
	SubdivisionWeightList source_vertex_weights;
	SubdivisionWeightRangeList skin_cluster_ranges;
	SubdivisionWeightList skin_cluster_weights;
}

[CCode (cname = "ufbx_subdivision_display_mode", cprefix = "UFBX_SUBDIVISION_DISPLAY_", has_type_id = false)]
public enum SubdivisionDisplayMode
{
	DISABLED,
	HULL,
	HULL_AND_SMOOTH,
	SMOOTH,
	MODE_COUNT
}

[CCode (cname = "ufbx_subdivision_boundary", cprefix = "UFBX_SUBDIVISION_BOUNDARY_", has_type_id = false)]
public enum SubdivisionBoundary
{
	DEFAULT,
	LEGACY,
	SHARP_CORNERS,
	SHARP_NONE,
	SHARP_BOUNDARY,
	SHARP_INTERIOR,
	COUNT
}

[CCode (cname = "ufbx_mesh", destroy_function = "", has_type_id = false)]
public struct Mesh
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	[CCode (cname = "instances")]
	NodeList instances;
	size_t num_vertices;
	size_t num_indices;
	size_t num_faces;
	size_t num_triangles;
	size_t num_edges;
	size_t max_face_triangles;
	size_t num_empty_faces;
	size_t num_point_faces;
	size_t num_line_faces;
	FaceList faces;
	BoolList face_smoothing;
	Uint32List face_material;
	Uint32List face_group;
	BoolList face_hole;
	EdgeList edges;
	BoolList edge_smoothing;
	RealList edge_crease;
	BoolList edge_visibility;
	Uint32List vertex_indices;
	Vec3List vertices;
	Uint32List vertex_first_index;
	VertexVec3 vertex_position;
	VertexVec3 vertex_normal;
	VertexVec2 vertex_uv;
	VertexVec3 vertex_tangent;
	VertexVec3 vertex_bitangent;
	VertexVec4 vertex_color;
	VertexReal vertex_crease;
	UvSetList uv_sets;
	ColorSetList color_sets;
	MaterialList materials;
	FaceGroupList face_groups;
	MeshPartList material_parts;
	MeshPartList face_group_parts;
	Uint32List material_part_usage_order;
	bool skinned_is_local;
	VertexVec3 skinned_position;
	VertexVec3 skinned_normal;
	SkinDeformerList skin_deformers;
	BlendDeformerList blend_deformers;
	CacheDeformerList cache_deformers;
	ElementList all_deformers;
	uint32 subdivision_preview_levels;
	uint32 subdivision_render_levels;
	SubdivisionDisplayMode subdivision_display_mode;
	SubdivisionBoundary subdivision_boundary;
	SubdivisionBoundary subdivision_uv_boundary;
	bool reversed_winding;
	bool generated_normals;
	bool subdivision_evaluated;
	SubdivisionResult? subdivision_result;
	bool from_tessellated_nurbs;
}

[CCode (cname = "ufbx_light_type", cprefix = "UFBX_LIGHT_", has_type_id = false)]
public enum LightType
{
	POINT,
	DIRECTIONAL,
	SPOT,
	AREA,
	VOLUME,
	TYPE_COUNT
}

[CCode (cname = "ufbx_light_decay", cprefix = "UFBX_LIGHT_DECAY_", has_type_id = false)]
public enum LightDecay
{
	NONE,
	LINEAR,
	QUADRATIC,
	CUBIC,
	COUNT
}

[CCode (cname = "ufbx_light_area_shape", cprefix = "UFBX_LIGHT_AREA_SHAPE_", has_type_id = false)]
public enum LightAreaShape
{
	RECTANGLE,
	SPHERE,
	COUNT
}

[CCode (cname = "ufbx_light", destroy_function = "", has_type_id = false)]
public struct Light
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	[CCode (cname = "instances")]
	NodeList instances;
	Vec3 color;
	Real intensity;
	Vec3 local_direction;
	LightType type;
	LightDecay decay;
	LightAreaShape area_shape;
	Real inner_angle;
	Real outer_angle;
	bool cast_light;
	bool cast_shadows;
}

[CCode (cname = "ufbx_projection_mode", cprefix = "UFBX_PROJECTION_MODE_", has_type_id = false)]
public enum ProjectionMode
{
	PERSPECTIVE,
	ORTHOGRAPHIC,
	COUNT
}

[CCode (cname = "ufbx_aspect_mode", cprefix = "UFBX_ASPECT_MODE_", has_type_id = false)]
public enum AspectMode
{
	WINDOW_SIZE,
	FIXED_RATIO,
	FIXED_RESOLUTION,
	FIXED_WIDTH,
	FIXED_HEIGHT,
	COUNT
}

[CCode (cname = "ufbx_aperture_mode", cprefix = "UFBX_APERTURE_MODE_", has_type_id = false)]
public enum ApertureMode
{
	HORIZONTAL_AND_VERTICAL,
	HORIZONTAL,
	VERTICAL,
	FOCAL_LENGTH,
	COUNT
}

[CCode (cname = "ufbx_gate_fit", cprefix = "UFBX_GATE_FIT_", has_type_id = false)]
public enum GateFit
{
	NONE,
	VERTICAL,
	HORIZONTAL,
	FILL,
	OVERSCAN,
	STRETCH,
	COUNT
}

[CCode (cname = "ufbx_aperture_format", cprefix = "UFBX_APERTURE_FORMAT_", has_type_id = false)]
public enum ApertureFormat
{
	CUSTOM,
	16MM_THEATRICAL,
	SUPER_16MM,
	35MM_ACADEMY,
	35MM_TV_PROJECTION,
	35MM_FULL_APERTURE,
	35MM_185_PROJECTION,
	35MM_ANAMORPHIC,
	70MM_PROJECTION,
	VISTAVISION,
	DYNAVISION,
	IMAX,
	COUNT
}

[CCode (cname = "ufbx_coordinate_axis", cprefix = "UFBX_COORDINATE_AXIS_", has_type_id = false)]
public enum CoordinateAxis
{
	POSITIVE_X,
	NEGATIVE_X,
	POSITIVE_Y,
	NEGATIVE_Y,
	POSITIVE_Z,
	NEGATIVE_Z,
	UNKNOWN,
	COUNT
}

[SimpleType]
[CCode (cname = "ufbx_coordinate_axes", has_type_id = false)]
public struct CoordinateAxes
{
	CoordinateAxis right;
	CoordinateAxis up;
	CoordinateAxis front;

	[CCode (cname = "ufbx_axes_right_handed_y_up")]
	public const CoordinateAxes RIGHT_HANDED_Y_UP;
	[CCode (cname = "ufbx_axes_right_handed_z_up")]
	public const CoordinateAxes RIGHT_HANDED_Z_UP;
	[CCode (cname = "ufbx_axes_left_handed_y_up")]
	public const CoordinateAxes LEFT_HANDED_Y_UP;
	[CCode (cname = "ufbx_axes_left_handed_z_up")]
	public const CoordinateAxes LEFT_HANDED_Z_UP;
}

[CCode (cname = "ufbx_camera", destroy_function = "", has_type_id = false)]
public struct Camera
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	[CCode (cname = "instances")]
	NodeList instances;
	ProjectionMode projection_mode;
	bool resolution_is_pixels;
	Vec2 resolution;
	Vec2 field_of_view_deg;
	Vec2 field_of_view_tan;
	Real orthographic_extent;
	Vec2 orthographic_size;
	Vec2 projection_plane;
	Real aspect_ratio;
	Real near_plane;
	Real far_plane;
	CoordinateAxes projection_axes;
	AspectMode aspect_mode;
	ApertureMode aperture_mode;
	GateFit gate_fit;
	ApertureFormat aperture_format;
	Real focal_length_mm;
	Vec2 film_size_inch;
	Vec2 aperture_size_inch;
	Real squeeze_ratio;
}

[CCode (cname = "ufbx_bone", destroy_function = "", has_type_id = false)]
public struct Bone
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	[CCode (cname = "instances")]
	NodeList instances;
	Real radius;
	Real relative_length;
	bool is_root;
}

[CCode (cname = "ufbx_empty", destroy_function = "", has_type_id = false)]
public struct Empty
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	[CCode (cname = "instances")]
	NodeList instances;
}

[CCode (cname = "ufbx_line_segment", destroy_function = "", has_type_id = false)]
public struct LineSegment
{
	uint32 index_begin;
	uint32 num_indices;
}

[CCode (cname = "ufbx_line_segment_list", destroy_function = "", has_type_id = false)]
public struct LineSegmentList
{
	LineSegment* data;
	size_t count;
}

[CCode (cname = "ufbx_line_curve", destroy_function = "", has_type_id = false)]
public struct LineCurve
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	[CCode (cname = "instances")]
	NodeList instances;
	Vec3 color;
	Vec3List control_points;
	Uint32List point_indices;
	LineSegmentList segments;
	bool from_tessellated_nurbs;
}

[CCode (cname = "ufbx_nurbs_topology", cprefix = "UFBX_NURBS_TOPOLOGY_", has_type_id = false)]
public enum NurbsTopology
{
	OPEN,
	PERIODIC,
	CLOSED,
	COUNT
}

[CCode (cname = "ufbx_nurbs_basis", destroy_function = "", has_type_id = false)]
public struct NurbsBasis
{
	uint32 order;
	NurbsTopology topology;
	RealList knot_vector;
	Real t_min;
	Real t_max;
	RealList spans;
	bool is_2d;
	size_t num_wrap_control_points;
	bool valid;
}

[CCode (cname = "ufbx_nurbs_curve", destroy_function = "", has_type_id = false)]
public struct NurbsCurve
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	[CCode (cname = "instances")]
	NodeList instances;
	NurbsBasis basis;
	Vec4List control_points;
}

[CCode (cname = "ufbx_nurbs_surface", destroy_function = "", has_type_id = false)]
public struct NurbsSurface
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	[CCode (cname = "instances")]
	NodeList instances;
	NurbsBasis basis_u;
	NurbsBasis basis_v;
	size_t num_control_points_u;
	size_t num_control_points_v;
	Vec4List control_points;
	uint32 span_subdivision_u;
	uint32 span_subdivision_v;
	bool flip_normals;
	Material? material;
}

[CCode (cname = "ufbx_nurbs_trim_surface", destroy_function = "", has_type_id = false)]
public struct NurbsTrimSurface
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	[CCode (cname = "instances")]
	NodeList instances;
}

[CCode (cname = "ufbx_nurbs_trim_boundary", destroy_function = "", has_type_id = false)]
public struct NurbsTrimBoundary
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	[CCode (cname = "instances")]
	NodeList instances;
}

[CCode (cname = "ufbx_procedural_geometry", destroy_function = "", has_type_id = false)]
public struct ProceduralGeometry
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	[CCode (cname = "instances")]
	NodeList instances;
}

[CCode (cname = "ufbx_stereo_camera", destroy_function = "", has_type_id = false)]
public struct StereoCamera
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	[CCode (cname = "instances")]
	NodeList instances;
	Camera? left;
	Camera? right;
}

[CCode (cname = "ufbx_camera_switcher", destroy_function = "", has_type_id = false)]
public struct CameraSwitcher
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	[CCode (cname = "instances")]
	NodeList instances;
}

[CCode (cname = "ufbx_marker_type", cprefix = "UFBX_MARKER_", has_type_id = false)]
public enum MarkerType
{
	UNKNOWN,
	FK_EFFECTOR,
	IK_EFFECTOR,
	TYPE_COUNT
}

[CCode (cname = "ufbx_marker", destroy_function = "", has_type_id = false)]
public struct Marker
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	[CCode (cname = "instances")]
	NodeList instances;
	MarkerType type;
}

[CCode (cname = "ufbx_lod_display", cprefix = "UFBX_LOD_DISPLAY_", has_type_id = false)]
public enum LodDisplay
{
	USE_LOD,
	SHOW,
	HIDE,
	COUNT
}

[CCode (cname = "ufbx_lod_level", destroy_function = "", has_type_id = false)]
public struct LodLevel
{
	Real distance;
	LodDisplay display;
}

[CCode (cname = "ufbx_lod_level_list", destroy_function = "", has_type_id = false)]
public struct LodLevelList
{
	LodLevel* data;
	size_t count;
}

[CCode (cname = "ufbx_lod_group", destroy_function = "", has_type_id = false)]
public struct LodGroup
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	[CCode (cname = "instances")]
	NodeList instances;
	bool relative_distances;
	LodLevelList lod_levels;
	bool ignore_parent_transform;
	bool use_distance_limit;
	Real distance_limit_min;
	Real distance_limit_max;
}

[CCode (cname = "ufbx_skinning_method", cprefix = "UFBX_SKINNING_METHOD_", has_type_id = false)]
public enum SkinningMethod
{
	LINEAR,
	RIGID,
	DUAL_QUATERNION,
	BLENDED_DQ_LINEAR,
	COUNT
}

[CCode (cname = "ufbx_skin_vertex", destroy_function = "", has_type_id = false)]
public struct SkinVertex
{
	uint32 weight_begin;
	uint32 num_weights;
	Real dq_weight;
}

[CCode (cname = "ufbx_skin_vertex_list", destroy_function = "", has_type_id = false)]
public struct SkinVertexList
{
	SkinVertex* data;
	size_t count;
}

[CCode (cname = "ufbx_skin_weight", destroy_function = "", has_type_id = false)]
public struct SkinWeight
{
	uint32 cluster_index;
	Real weight;
}

[CCode (cname = "ufbx_skin_weight_list", destroy_function = "", has_type_id = false)]
public struct SkinWeightList
{
	SkinWeight* data;
	size_t count;
}

[CCode (cname = "ufbx_skin_deformer", destroy_function = "", has_type_id = false)]
public struct SkinDeformer
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	SkinningMethod skinning_method;
	SkinClusterList clusters;
	SkinVertexList vertices;
	SkinWeightList weights;
	size_t max_weights_per_vertex;
	size_t num_dq_weights;
	Uint32List dq_vertices;
	RealList dq_weights;
}

[CCode (cname = "ufbx_skin_cluster", destroy_function = "", has_type_id = false)]
public struct SkinCluster
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	Node? bone_node;
	Matrix geometry_to_bone;
	Matrix mesh_node_to_bone;
	Matrix bind_to_world;
	Matrix geometry_to_world;
	Transform geometry_to_world_transform;
	size_t num_weights;
	Uint32List vertices;
	RealList weights;
}

[CCode (cname = "ufbx_blend_deformer", destroy_function = "", has_type_id = false)]
public struct BlendDeformer
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	BlendChannelList channels;
}

[CCode (cname = "ufbx_blend_keyframe", destroy_function = "", has_type_id = false)]
public struct BlendKeyframe
{
	BlendShape* shape;
	Real target_weight;
	Real effective_weight;
}

[CCode (cname = "ufbx_blend_keyframe_list", destroy_function = "", has_type_id = false)]
public struct BlendKeyframeList
{
	BlendKeyframe* data;
	size_t count;
}

[CCode (cname = "ufbx_blend_channel", destroy_function = "", has_type_id = false)]
public struct BlendChannel
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	Real weight;
	BlendKeyframeList keyframes;
	BlendShape? target_shape;
}

[CCode (cname = "ufbx_blend_shape", destroy_function = "", has_type_id = false)]
public struct BlendShape
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	size_t num_offsets;
	Uint32List offset_vertices;
	Vec3List position_offsets;
	Vec3List normal_offsets;
}

[CCode (cname = "ufbx_cache_file_format", cprefix = "UFBX_CACHE_FILE_FORMAT_", has_type_id = false)]
public enum CacheFileFormat
{
	UNKNOWN,
	PC2,
	MC,
	COUNT
}

[CCode (cname = "ufbx_cache_data_format", cprefix = "UFBX_CACHE_DATA_FORMAT_", has_type_id = false)]
public enum CacheDataFormat
{
	UNKNOWN,
	REAL_FLOAT,
	VEC3_FLOAT,
	REAL_DOUBLE,
	VEC3_DOUBLE,
	COUNT
}

[CCode (cname = "ufbx_cache_data_encoding", cprefix = "UFBX_CACHE_DATA_ENCODING_", has_type_id = false)]
public enum CacheDataEncoding
{
	UNKNOWN,
	LITTLE_ENDIAN,
	BIG_ENDIAN,
	COUNT
}

[CCode (cname = "ufbx_cache_interpretation", cprefix = "UFBX_CACHE_INTERPRETATION_", has_type_id = false)]
public enum CacheInterpretation
{
	UNKNOWN,
	POINTS,
	VERTEX_POSITION,
	VERTEX_NORMAL,
	COUNT
}

[CCode (cname = "ufbx_cache_frame", destroy_function = "", has_type_id = false)]
public struct CacheFrame
{
	String channel;
	double time;
	String filename;
	CacheFileFormat file_format;
	MirrorAxis mirror_axis;
	Real scale_factor;
	CacheDataFormat data_format;
	CacheDataEncoding data_encoding;
	uint64 data_offset;
	uint32 data_count;
	uint32 data_element_bytes;
	uint64 data_total_bytes;
}

[CCode (cname = "ufbx_cache_frame", destroy_function = "", has_type_id = false)]
public struct CacheFrameList
{
	CacheFrame* data;
	size_t count;
}

[CCode (cname = "ufbx_cache_channel", destroy_function = "", has_type_id = false)]
public struct CacheChannel
{
	String name;
	CacheInterpretation interpretation;
	String interpretation_name;
	CacheFrameList frames;
	MirrorAxis mirror_axis;
	Real scale_factor;
}

[CCode (cname = "ufbx_cache_channel_list", destroy_function = "", has_type_id = false)]
public struct CacheChannelList
{
	CacheChannel* data;
	size_t count;
}

[CCode (cname = "ufbx_geometry_cache", destroy_function = "", has_type_id = false)]
public struct GeometryCache
{
	String root_filename;
	CacheChannelList channels;
	CacheFrameList frames;
	StringList extra_info;
}

[CCode (cname = "ufbx_cache_deformer", destroy_function = "", has_type_id = false)]
public struct CacheDeformer
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	String channel;
	CacheFile? file;
	GeometryCache? external_cache;
	CacheChannel? external_channel;
}

[CCode (cname = "ufbx_cache_file", destroy_function = "", has_type_id = false)]
public struct CacheFile
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	String filename;
	String absolute_filename;
	String relative_filename;
	Blob raw_filename;
	Blob raw_absolute_filename;
	Blob raw_relative_filename;
	CacheFileFormat format;
	GeometryCache? external_cache;
}

[CCode (cname = "ufbx_material_map", destroy_function = "", has_type_id = false)]
public struct MaterialMap
{
	[CCode (cname = "value_real")]
	Real value_real;
	[CCode (cname = "value_vec2")]
	Vec2 value_vec2;
	[CCode (cname = "value_vec3")]
	Vec3 value_vec3;
	[CCode (cname = "value_vec4")]
	Vec4 value_vec4;
	int64 value_int;
	Texture? texture;
	bool has_value;
	bool texture_enabled;
	bool feature_disabled;
	uint8 value_components;
}

[CCode (cname = "ufbx_material_feature_info", destroy_function = "", has_type_id = false)]
public struct MaterialFeatureInfo
{
	bool enabled;
	bool is_explicit;
}

[CCode (cname = "ufbx_material_texture", destroy_function = "", has_type_id = false)]
public struct MaterialTexture
{
	String material_prop;
	String shader_prop;
	Texture* texture;
}

[CCode (cname = "ufbx_material_texture_list", destroy_function = "", has_type_id = false)]
public struct MaterialTextureList
{
	MaterialTexture* data;
	size_t count;
}

[CCode (cname = "ufbx_shader_type", cprefix = "UFBX_SHADER_", has_type_id = false)]
public enum ShaderType
{
	UNKNOWN,
	FBX_LAMBERT,
	FBX_PHONG,
	OSL_STANDARD_SURFACE,
	ARNOLD_STANDARD_SURFACE,
	3DS_MAX_PHYSICAL_MATERIAL,
	3DS_MAX_PBR_METAL_ROUGH,
	3DS_MAX_PBR_SPEC_GLOSS,
	GLTF_MATERIAL,
	SHADERFX_GRAPH,
	BLENDER_PHONG,
	WAVEFRONT_MTL,
	TYPE_COUNT
}

[CCode (cname = "ufbx_material_fbx_map", cprefix = "UFBX_MATERIAL_FBX_", has_type_id = false)]
public enum MaterialFbxMap
{
	DIFFUSE_FACTOR,
	DIFFUSE_COLOR,
	SPECULAR_FACTOR,
	SPECULAR_COLOR,
	SPECULAR_EXPONENT,
	REFLECTION_FACTOR,
	REFLECTION_COLOR,
	TRANSPARENCY_FACTOR,
	TRANSPARENCY_COLOR,
	EMISSION_FACTOR,
	EMISSION_COLOR,
	AMBIENT_FACTOR,
	AMBIENT_COLOR,
	NORMAL_MAP,
	BUMP,
	BUMP_FACTOR,
	DISPLACEMENT_FACTOR,
	DISPLACEMENT,
	VECTOR_DISPLACEMENT_FACTOR,
	VECTOR_DISPLACEMENT,
	MAP_COUNT
}

[CCode (cname = "ufbx_material_pbr_map", cprefix = "UFBX_MATERIAL_PBR_", has_type_id = false)]
public enum MaterialPbrMap
{
	BASE_FACTOR,
	BASE_COLOR,
	ROUGHNESS,
	METALNESS,
	DIFFUSE_ROUGHNESS,
	SPECULAR_FACTOR,
	SPECULAR_COLOR,
	SPECULAR_IOR,
	SPECULAR_ANISOTROPY,
	SPECULAR_ROTATION,
	TRANSMISSION_FACTOR,
	TRANSMISSION_COLOR,
	TRANSMISSION_DEPTH,
	TRANSMISSION_SCATTER,
	TRANSMISSION_SCATTER_ANISOTROPY,
	TRANSMISSION_DISPERSION,
	TRANSMISSION_ROUGHNESS,
	TRANSMISSION_EXTRA_ROUGHNESS,
	TRANSMISSION_PRIORITY,
	TRANSMISSION_ENABLE_IN_AOV,
	SUBSURFACE_FACTOR,
	SUBSURFACE_COLOR,
	SUBSURFACE_RADIUS,
	SUBSURFACE_SCALE,
	SUBSURFACE_ANISOTROPY,
	SUBSURFACE_TINT_COLOR,
	SUBSURFACE_TYPE,
	SHEEN_FACTOR,
	SHEEN_COLOR,
	SHEEN_ROUGHNESS,
	COAT_FACTOR,
	COAT_COLOR,
	COAT_ROUGHNESS,
	COAT_IOR,
	COAT_ANISOTROPY,
	COAT_ROTATION,
	COAT_NORMAL,
	COAT_AFFECT_BASE_COLOR,
	COAT_AFFECT_BASE_ROUGHNESS,
	THIN_FILM_THICKNESS,
	THIN_FILM_IOR,
	EMISSION_FACTOR,
	EMISSION_COLOR,
	OPACITY,
	INDIRECT_DIFFUSE,
	INDIRECT_SPECULAR,
	NORMAL_MAP,
	TANGENT_MAP,
	DISPLACEMENT_MAP,
	MATTE_FACTOR,
	MATTE_COLOR,
	AMBIENT_OCCLUSION,
	GLOSSINESS,
	COAT_GLOSSINESS,
	TRANSMISSION_GLOSSINESS,
	MAP_COUNT
}

[CCode (cname = "ufbx_material_feature", cprefix = "UFBX_MATERIAL_FEATURE_", has_type_id = false)]
public enum MaterialFeature
{
	PBR,
	METALNESS,
	DIFFUSE,
	SPECULAR,
	EMISSION,
	TRANSMISSION,
	COAT,
	SHEEN,
	OPACITY,
	AMBIENT_OCCLUSION,
	MATTE,
	UNLIT,
	IOR,
	DIFFUSE_ROUGHNESS,
	TRANSMISSION_ROUGHNESS,
	THIN_WALLED,
	CAUSTICS,
	EXIT_TO_BACKGROUND,
	INTERNAL_REFLECTIONS,
	DOUBLE_SIDED,
	ROUGHNESS_AS_GLOSSINESS,
	COAT_ROUGHNESS_AS_GLOSSINESS,
	TRANSMISSION_ROUGHNESS_AS_GLOSSINESS,
	COUNT
}

[CCode (cname = "ufbx_material_fbx_maps", destroy_function = "", has_type_id = false)]
public struct MaterialFbxMaps
{
	[CCode (cname = "maps")]
	MaterialMap maps[MaterialFbxMap.MAP_COUNT];
	[CCode (cname = "diffuse_factor")]
	MaterialMap diffuse_factor;
	[CCode (cname = "diffuse_color")]
	MaterialMap diffuse_color;
	[CCode (cname = "specular_factor")]
	MaterialMap specular_factor;
	[CCode (cname = "specular_color")]
	MaterialMap specular_color;
	[CCode (cname = "specular_exponent")]
	MaterialMap specular_exponent;
	[CCode (cname = "reflection_factor")]
	MaterialMap reflection_factor;
	[CCode (cname = "reflection_color")]
	MaterialMap reflection_color;
	[CCode (cname = "transparency_factor")]
	MaterialMap transparency_factor;
	[CCode (cname = "transparency_color")]
	MaterialMap transparency_color;
	[CCode (cname = "emission_factor")]
	MaterialMap emission_factor;
	[CCode (cname = "emission_color")]
	MaterialMap emission_color;
	[CCode (cname = "ambient_factor")]
	MaterialMap ambient_factor;
	[CCode (cname = "ambient_color")]
	MaterialMap ambient_color;
	[CCode (cname = "normal_map")]
	MaterialMap normal_map;
	[CCode (cname = "bump")]
	MaterialMap bump;
	[CCode (cname = "bump_factor")]
	MaterialMap bump_factor;
	[CCode (cname = "displacement_factor")]
	MaterialMap displacement_factor;
	[CCode (cname = "displacement")]
	MaterialMap displacement;
	[CCode (cname = "vector_displacement_factor")]
	MaterialMap vector_displacement_factor;
	[CCode (cname = "vector_displacement")]
	MaterialMap vector_displacement;
}

[CCode (cname = "ufbx_material_pbr_maps", destroy_function = "", has_type_id = false)]
public struct MaterialPbrMaps
{
	[CCode (cname = "maps")]
	MaterialMap maps[MaterialPbrMap.MAP_COUNT];
	[CCode (cname = "base_factor")]
	MaterialMap base_factor;
	[CCode (cname = "base_color")]
	MaterialMap base_color;
	[CCode (cname = "roughness")]
	MaterialMap roughness;
	[CCode (cname = "metalness")]
	MaterialMap metalness;
	[CCode (cname = "diffuse_roughness")]
	MaterialMap diffuse_roughness;
	[CCode (cname = "specular_factor")]
	MaterialMap specular_factor;
	[CCode (cname = "specular_color")]
	MaterialMap specular_color;
	[CCode (cname = "specular_ior")]
	MaterialMap specular_ior;
	[CCode (cname = "specular_anisotropy")]
	MaterialMap specular_anisotropy;
	[CCode (cname = "specular_rotation")]
	MaterialMap specular_rotation;
	[CCode (cname = "transmission_factor")]
	MaterialMap transmission_factor;
	[CCode (cname = "transmission_color")]
	MaterialMap transmission_color;
	[CCode (cname = "transmission_depth")]
	MaterialMap transmission_depth;
	[CCode (cname = "transmission_scatter")]
	MaterialMap transmission_scatter;
	[CCode (cname = "transmission_scatter_anisotropy")]
	MaterialMap transmission_scatter_anisotropy;
	[CCode (cname = "transmission_dispersion")]
	MaterialMap transmission_dispersion;
	[CCode (cname = "transmission_roughness")]
	MaterialMap transmission_roughness;
	[CCode (cname = "transmission_extra_roughness")]
	MaterialMap transmission_extra_roughness;
	[CCode (cname = "transmission_priority")]
	MaterialMap transmission_priority;
	[CCode (cname = "transmission_enable_in_aov")]
	MaterialMap transmission_enable_in_aov;
	[CCode (cname = "subsurface_factor")]
	MaterialMap subsurface_factor;
	[CCode (cname = "subsurface_color")]
	MaterialMap subsurface_color;
	[CCode (cname = "subsurface_radius")]
	MaterialMap subsurface_radius;
	[CCode (cname = "subsurface_scale")]
	MaterialMap subsurface_scale;
	[CCode (cname = "subsurface_anisotropy")]
	MaterialMap subsurface_anisotropy;
	[CCode (cname = "subsurface_tint_color")]
	MaterialMap subsurface_tint_color;
	[CCode (cname = "subsurface_type")]
	MaterialMap subsurface_type;
	[CCode (cname = "sheen_factor")]
	MaterialMap sheen_factor;
	[CCode (cname = "sheen_color")]
	MaterialMap sheen_color;
	[CCode (cname = "sheen_roughness")]
	MaterialMap sheen_roughness;
	[CCode (cname = "coat_factor")]
	MaterialMap coat_factor;
	[CCode (cname = "coat_color")]
	MaterialMap coat_color;
	[CCode (cname = "coat_roughness")]
	MaterialMap coat_roughness;
	[CCode (cname = "coat_ior")]
	MaterialMap coat_ior;
	[CCode (cname = "coat_anisotropy")]
	MaterialMap coat_anisotropy;
	[CCode (cname = "coat_rotation")]
	MaterialMap coat_rotation;
	[CCode (cname = "coat_normal")]
	MaterialMap coat_normal;
	[CCode (cname = "coat_affect_base_color")]
	MaterialMap coat_affect_base_color;
	[CCode (cname = "coat_affect_base_roughness")]
	MaterialMap coat_affect_base_roughness;
	[CCode (cname = "thin_film_thickness")]
	MaterialMap thin_film_thickness;
	[CCode (cname = "thin_film_ior")]
	MaterialMap thin_film_ior;
	[CCode (cname = "emission_factor")]
	MaterialMap emission_factor;
	[CCode (cname = "emission_color")]
	MaterialMap emission_color;
	[CCode (cname = "opacity")]
	MaterialMap opacity;
	[CCode (cname = "indirect_diffuse")]
	MaterialMap indirect_diffuse;
	[CCode (cname = "indirect_specular")]
	MaterialMap indirect_specular;
	[CCode (cname = "normal_map")]
	MaterialMap normal_map;
	[CCode (cname = "tangent_map")]
	MaterialMap tangent_map;
	[CCode (cname = "displacement_map")]
	MaterialMap displacement_map;
	[CCode (cname = "matte_factor")]
	MaterialMap matte_factor;
	[CCode (cname = "matte_color")]
	MaterialMap matte_color;
	[CCode (cname = "ambient_occlusion")]
	MaterialMap ambient_occlusion;
	[CCode (cname = "glossiness")]
	MaterialMap glossiness;
	[CCode (cname = "coat_glossiness")]
	MaterialMap coat_glossiness;
	[CCode (cname = "transmission_glossiness")]
	MaterialMap transmission_glossiness;
}

[CCode (cname = "ufbx_material_features", destroy_function = "", has_type_id = false)]
public struct MaterialFeatures
{
	[CCode (cname = "features")]
	MaterialFeatureInfo features[MaterialFeature.COUNT];
	[CCode (cname = "pbr")]
	MaterialFeatureInfo pbr;
	[CCode (cname = "metalness")]
	MaterialFeatureInfo metalness;
	[CCode (cname = "diffuse")]
	MaterialFeatureInfo diffuse;
	[CCode (cname = "specular")]
	MaterialFeatureInfo specular;
	[CCode (cname = "emission")]
	MaterialFeatureInfo emission;
	[CCode (cname = "transmission")]
	MaterialFeatureInfo transmission;
	[CCode (cname = "coat")]
	MaterialFeatureInfo coat;
	[CCode (cname = "sheen")]
	MaterialFeatureInfo sheen;
	[CCode (cname = "opacity")]
	MaterialFeatureInfo opacity;
	[CCode (cname = "ambient_occlusion")]
	MaterialFeatureInfo ambient_occlusion;
	[CCode (cname = "matte")]
	MaterialFeatureInfo matte;
	[CCode (cname = "unlit")]
	MaterialFeatureInfo unlit;
	[CCode (cname = "ior")]
	MaterialFeatureInfo ior;
	[CCode (cname = "diffuse_roughness")]
	MaterialFeatureInfo diffuse_roughness;
	[CCode (cname = "transmission_roughness")]
	MaterialFeatureInfo transmission_roughness;
	[CCode (cname = "thin_walled")]
	MaterialFeatureInfo thin_walled;
	[CCode (cname = "caustics")]
	MaterialFeatureInfo caustics;
	[CCode (cname = "exit_to_background")]
	MaterialFeatureInfo exit_to_background;
	[CCode (cname = "internal_reflections")]
	MaterialFeatureInfo internal_reflections;
	[CCode (cname = "double_sided")]
	MaterialFeatureInfo double_sided;
	[CCode (cname = "roughness_as_glossiness")]
	MaterialFeatureInfo roughness_as_glossiness;
	[CCode (cname = "coat_roughness_as_glossiness")]
	MaterialFeatureInfo coat_roughness_as_glossiness;
	[CCode (cname = "transmission_roughness_as_glossiness")]
	MaterialFeatureInfo transmission_roughness_as_glossiness;
}

[CCode (cname = "ufbx_material", destroy_function = "", has_type_id = false)]
public struct Material
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	MaterialFbxMaps fbx;
	MaterialPbrMaps pbr;
	MaterialFeatures features;
	ShaderType shader_type;
	Shader? shader;
	String shading_model_name;
	String shader_prop_prefix;
	MaterialTextureList textures;
}

[CCode (cname = "ufbx_texture_type", cprefix = "UFBX_TEXTURE_", has_type_id = false)]
public enum TextureType
{
	FILE,
	LAYERED,
	PROCEDURAL,
	SHADER,
	TYPE_COUNT
}

[CCode (cname = "ufbx_blend_mode", cprefix = "UFBX_BLEND_", has_type_id = false)]
public enum BlendMode
{
	TRANSLUCENT,
	ADDITIVE,
	MULTIPLY,
	MULTIPLY_2X,
	OVER,
	REPLACE,
	DISSOLVE,
	DARKEN,
	COLOR_BURN,
	LINEAR_BURN,
	DARKER_COLOR,
	LIGHTEN,
	SCREEN,
	COLOR_DODGE,
	LINEAR_DODGE,
	LIGHTER_COLOR,
	SOFT_LIGHT,
	HARD_LIGHT,
	VIVID_LIGHT,
	LINEAR_LIGHT,
	PIN_LIGHT,
	HARD_MIX,
	DIFFERENCE,
	EXCLUSION,
	SUBTRACT,
	DIVIDE,
	HUE,
	SATURATION,
	COLOR,
	LUMINOSITY,
	OVERLAY,
	COUNT
}

[CCode (cname = "ufbx_wrap_mode", cprefix = "UFBX_WRAP_", has_type_id = false)]
public enum WrapMode
{
	REPEAT,
	CLAMP,
	COUNT
}

[CCode (cname = "ufbx_texture_layer", destroy_function = "", has_type_id = false)]
public struct TextureLayer
{
	Texture* texture;
	BlendMode blend_mode;
	Real alpha;
}

[CCode (cname = "ufbx_texture_layer", destroy_function = "", has_type_id = false)]
public struct TextureLayerList
{
	TextureLayer* data;
	size_t count;
}

[CCode (cname = "ufbx_shader_texture_type", cprefix = "UFBX_SHADER_TEXTURE_", has_type_id = false)]
public enum ShaderTextureType
{
	UNKNOWN,
	SELECT_OUTPUT,
	OSL,
	TYPE_COUNT
}

[CCode (cname = "ufbx_shader_texture_input", destroy_function = "", has_type_id = false)]
public struct ShaderTextureInput
{
	String name;
	[CCode (cname = "value_real")]
	Real value_real;
	[CCode (cname = "value_vec2")]
	Vec2 value_vec2;
	[CCode (cname = "value_vec3")]
	Vec3 value_vec3;
	[CCode (cname = "value_vec4")]
	Vec4 value_vec4;
	int64 value_int;
	String value_str;
	Blob value_blob;
	Texture? texture;
	int64 texture_output_index;
	bool texture_enabled;
	Prop* prop;
	Prop? texture_prop;
	Prop? texture_enabled_prop;
}

[CCode (cname = "ufbx_shader_texture_input_list", destroy_function = "", has_type_id = false)]
public struct ShaderTextureInputList
{
	ShaderTextureInput* data;
	size_t size;
}

[CCode (cname = "ufbx_shader_texture", destroy_function = "", has_type_id = false)]
public struct ShaderTexture
{
	ShaderTextureType type;
	String shader_name;
	uint64 shader_type_id;
	ShaderTextureInputList inputs;
	String shader_source;
	Blob raw_shader_source;
	Texture* main_texture;
	int64 main_texture_output_index;
	String prop_prefix;
}

[CCode (cname = "ufbx_texture_file", destroy_function = "", has_type_id = false)]
public struct TextureFile
{
	uint32 index;
	String filename;
	String absolute_filename;
	String relative_filename;
	Blob raw_filename;
	Blob raw_absolute_filename;
	Blob raw_relative_filename;
	Blob content;
}

[CCode (cname = "ufbx_texture_file_list", destroy_function = "", has_type_id = false)]
public struct TextureFileList
{
	TextureFile* data;
	size_t count;
}

[CCode (cname = "ufbx_texture", destroy_function = "", has_type_id = false)]
public struct Texture
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	TextureType type;
	String filename;
	String absolute_filename;
	String relative_filename;
	Blob raw_filename;
	Blob raw_absolute_filename;
	Blob raw_relative_filename;
	Blob content;
	Video? video;
	uint32 file_index;
	bool has_file;
	TextureLayerList layers;
	ShaderTexture? shader;
	TextureList file_textures;
	String uv_set;
	WrapMode wrap_u;
	WrapMode wrap_v;
	bool has_uv_transform;
	Transform uv_transform;
	Matrix texture_to_uv;
	Matrix uv_to_texture;
}

[CCode (cname = "ufbx_video", destroy_function = "", has_type_id = false)]
public struct Video
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	String filename;
	String absolute_filename;
	String relative_filename;
	Blob raw_filename;
	Blob raw_absolute_filename;
	Blob raw_relative_filename;
	Blob content;
}

[CCode (cname = "ufbx_shader", destroy_function = "", has_type_id = false)]
public struct Shader
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	ShaderType type;
	ShaderBindingList bindings;
}

[CCode (cname = "ufbx_shader_prop_binding", destroy_function = "", has_type_id = false)]
public struct ShaderPropBinding
{
	String shader_prop;
	String material_prop;
}

[CCode (cname = "ufbx_shader_prop_binding_list", destroy_function = "", has_type_id = false)]
public struct ShaderPropBindingList
{
	ShaderPropBinding* data;
	size_t count;
}

[CCode (cname = "ufbx_shader_binding", destroy_function = "", has_type_id = false)]
public struct ShaderBinding
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	ShaderPropBindingList prop_bindings;
}

[CCode (cname = "ufbx_prop_override", destroy_function = "", has_type_id = false)]
public struct PropOverride
{
	uint32 element_id;
	uint32 _internal_key;
	String prop_name;
	Vec4 value;
	String value_str;
	int64 value_int;
}

[CCode (cname = "ufbx_prop_override_list", destroy_function = "", has_type_id = false)]
public struct PropOverrideList
{
	PropOverride* data;
	size_t count;
}

[CCode (cname = "ufbx_transform_override", destroy_function = "", has_type_id = false)]
public struct TransformOverride
{
	uint32 node_id;
	Transform transform;
}

[CCode (cname = "ufbx_transform_override_list", destroy_function = "", has_type_id = false)]
public struct TransformOverrideList
{
	TransformOverride* data;
	size_t count;
}

[CCode (cname = "ufbx_anim", destroy_function = "", has_type_id = false)]
public struct Anim
{
	double time_begin;
	double time_end;
	AnimLayerList layers;
	RealList override_layer_weights;
	PropOverrideList prop_overrides;
	TransformOverrideList transform_overrides;
	bool ignore_connections;
	bool custom;
}

[CCode (cname = "ufbx_anim_stack", destroy_function = "", has_type_id = false)]
public struct AnimStack
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	double time_begin;
	double time_end;
	AnimLayerList layers;
	Anim* anim;
}

[CCode (cname = "ufbx_anim_prop", destroy_function = "", has_type_id = false)]
public struct AnimProp
{
	Element* element;
	uint32 _internal_key;
	String prop_name;
	AnimValue* anim_value;
}

[CCode (cname = "ufbx_anim_list", destroy_function = "", has_type_id = false)]
public struct AnimPropList
{
	AnimProp* data;
	size_t count;
}

[CCode (cname = "ufbx_anim_layer", destroy_function = "", has_type_id = false)]
public struct AnimLayer
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	Real weight;
	bool weight_is_animated;
	bool blended;
	bool additive;
	bool compose_rotation;
	bool compose_scale;
	AnimValueList anim_values;
	AnimPropList anim_props;
	Anim* anim;
	uint32 _min_element_id;
	uint32 _max_element_id;
	uint32 _element_id_bitmask[4];
}

[CCode (cname = "ufbx_anim_value", destroy_function = "", has_type_id = false)]
public struct AnimValue
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	Vec3 default_value;
	AnimCurve? curves[3];
}

[CCode (cname = "ufbx_interpolation", cprefix = "UFBX_INTERPOLATION_", has_type_id = false)]
public enum Interpolation
{
	CONSTANT_PREV,
	CONSTANT_NEXT,
	LINEAR,
	CUBIC,
	COUNT
}

[CCode (cname = "ufbx_tangent", destroy_function = "", has_type_id = false)]
public struct Tangent
{
	float dx;
	float dy;
}

[CCode (cname = "ufbx_keyframe", destroy_function = "", has_type_id = false)]
public struct Keyframe
{
	double time;
	Real value;
	Interpolation interpolation;
	Tangent left;
	Tangent right;
}

[CCode (cname = "ufbx_keyframe_list", destroy_function = "", has_type_id = false)]
public struct KeyframeList
{
	Keyframe* data;
	size_t count;
}

[CCode (cname = "ufbx_anim_curve", destroy_function = "", has_type_id = false)]
public struct AnimCurve
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	KeyframeList keyframes;
	Real min_value;
	Real max_value;
}

[CCode (cname = "ufbx_display_layer", destroy_function = "", has_type_id = false)]
public struct DisplayLayer
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	NodeList nodes;
	bool visible;
	bool frozen;
	Vec3 ui_color;
}

[CCode (cname = "ufbx_selection_set", destroy_function = "", has_type_id = false)]
public struct SelectionSet
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	SelectionNodeList nodes;
}

[CCode (cname = "ufbx_selection_node", destroy_function = "", has_type_id = false)]
public struct SelectionNode
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	Node? target_node;
	Mesh? target_mesh;
	bool include_node;
	Uint32List vertices;
	Uint32List edges;
	Uint32List faces;
}

[CCode (cname = "ufbx_character", destroy_function = "", has_type_id = false)]
public struct Character
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
}

[CCode (cname = "ufbx_constraint_type", cprefix = "UFBX_CONSTRAINT_", has_type_id = false)]
public enum ConstraintType
{
	UNKNOWN,
	AIM,
	PARENT,
	POSITION,
	ROTATION,
	SCALE,
	SINGLE_CHAIN_IK,
	TYPE_COUNT
}

[CCode (cname = "ufbx_constraint_target", destroy_function = "", has_type_id = false)]
public struct ConstraintTarget
{
	Node* node;
	Real weight;
	Transform transform;
}

[CCode (cname = "ufbx_constraint_target_list", destroy_function = "", has_type_id = false)]
public struct ConstraintTargetList
{
	ConstraintTarget* data;
	size_t count;
}

[CCode (cname = "ufbx_constraint_aim_up_type", cprefix = "UFBX_CONSTRAINT_AIM_UP_", has_type_id = false)]
public enum ConstraintAimUpType
{
	SCENE,
	TO_NODE,
	ALIGN_NODE,
	VECTOR,
	NONE,
	TYPE_COUNT
}

[CCode (cname = "ufbx_constraint_ik_pole_type", cprefix = "UFBX_CONSTRAINT_IK_POLE_", has_type_id = false)]
public enum ConstraintIkPoleType
{
	VECTOR,
	NODE,
	TYPE_COUNT
}

[CCode (cname = "ufbx_constraint", destroy_function = "", has_type_id = false)]
public struct Constraint
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	ConstraintType type;
	String type_name;
	Node? node;
	ConstraintTargetList targets;
	Real weight;
	bool active;
	bool constrain_translation[3];
	bool constrain_rotation[3];
	bool constrain_scale[3];
	Transform transform_offset;
	Vec3 aim_vector;
	ConstraintAimUpType aim_up_type;
	Node? aim_up_node;
	Vec3 aim_up_vector;
	Node? ik_effector;
	Node? ik_end_node;
	Vec3 ik_pole_vector;
}

[CCode (cname = "ufbx_audio_layer", destroy_function = "", has_type_id = false)]
public struct AudioLayer
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	AudioClipList clips;
}

[CCode (cname = "ufbx_audio_clip", destroy_function = "", has_type_id = false)]
public struct AudioClip
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	String filename;
	String absolute_filename;
	String relative_filename;
	Blob raw_filename;
	Blob raw_absolute_filename;
	Blob raw_relative_filename;
	Blob content;
}

[CCode (cname = "ufbx_bone_pose", destroy_function = "", has_type_id = false)]
public struct BonePose
{
	Node* bone_node;
	Matrix bone_to_world;
	Matrix bone_to_parent;
}

[CCode (cname = "ufbx_bone_pose_list", destroy_function = "", has_type_id = false)]
public struct BonePoseList
{
	BonePose* data;
	size_t count;
}

[CCode (cname = "ufbx_pose", destroy_function = "", has_type_id = false)]
public struct Pose
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
	bool is_bind_pose;
	BonePoseList bone_poses;
}

[CCode (cname = "ufbx_metadata_object", destroy_function = "", has_type_id = false)]
public struct MetadataObject
{
	[CCode (cname = "element")]
	Element element;
	[CCode (cname = "name")]
	String name;
	[CCode (cname = "props")]
	Props props;
	[CCode (cname = "element_id")]
	uint32 element_id;
	[CCode (cname = "typed_id")]
	uint32 typed_id;
}

[CCode (cname = "ufbx_name_element", destroy_function = "", has_type_id = false)]
public struct NameElement
{
	String name;
	ElementType type;
	uint32 _internal_key;
	Element* element;
}

[CCode (cname = "ufbx_name_element_list", destroy_function = "", has_type_id = false)]
public struct NameElementList
{
	NameElement* data;
	size_t count;
}

[CCode (cname = "ufbx_exporter", cprefix = "UFBX_EXPORTER_", has_type_id = false)]
public enum Exporter
{
	UNKNOWN,
	FBX_SDK,
	BLENDER_BINARY,
	BLENDER_ASCII,
	MOTION_BUILDER,
	COUNT
}

[CCode (cname = "ufbx_application", destroy_function = "", has_type_id = false)]
public struct Application
{
	String vendor;
	String name;
	String version;
}

[CCode (cname = "ufbx_file_format", cprefix = "UFBX_FILE_FORMAT_", has_type_id = false)]
public enum FileFormat
{
	UNKNOWN,
	FBX,
	OBJ,
	MTL,
	COUNT
}

[CCode (cname = "ufbx_warning_type", cprefix = "UFBX_WARNING_", has_type_id = false)]
public enum WarningType
{
	MISSING_EXTERNAL_FILE,
	IMPLICIT_MTL,
	TRUNCATED_ARRAY,
	MISSING_GEOMETRY_DATA,
	DUPLICATE_CONNECTION,
	BAD_VERTEX_W_ATTRIBUTE,
	MISSING_POLYGON_MAPPING,
	INDEX_CLAMPED,
	BAD_UNICODE,
	BAD_ELEMENT_CONNECTED_TO_ROOT,
	DUPLICATE_OBJECT_ID,
	EMPTY_FACE_REMOVED,
	UNKNOWN_OBJ_DIRECTIVE,
	TYPE_COUNT,
	TYPE_FIRST_DEDUPLICATED = WarningType.INDEX_CLAMPED
}

[CCode (cname = "ufbx_warning", destroy_function = "", has_type_id = false)]
public struct Warning
{
	WarningType type;
	String description;
	uint32 element_id;
	size_t count;
}

[CCode (cname = "ufbx_warning_list", destroy_function = "", has_type_id = false)]
public struct WarningList
{
	Warning* data;
	size_t count;
}

[CCode (cname = "ufbx_thumbnail_format", cprefix = "UFBX_THUMBNAIL_FORMAT_", has_type_id = false)]
public enum ThumbnailFormat
{
	UNKNOWN,
	RGB_24,
	RGBA_32,
	COUNT
}

[CCode (cname = "ufbx_space_conversion", cprefix = "UFBX_SPACE_CONVERSION_", has_type_id = false)]
public enum SpaceConversion
{
	TRANSFORM_ROOT,
	ADJUST_TRANSFORMS,
	MODIFY_GEOMETRY,
	COUNT
}

[CCode (cname = "ufbx_thumbnail", destroy_function = "", has_type_id = false)]
public struct Thumbnail
{
	Props props;
	uint32 width;
	uint32 height;
	ThumbnailFormat format;
	Blob data;
}

[CCode (cname = "ufbx_metadata", destroy_function = "", has_type_id = false)]
public struct Metadata
{
	WarningList warnings;
	bool ascii;
	uint32 version;
	FileFormat file_format;
	bool may_contain_no_index;
	bool may_contain_missing_vertex_position;
	bool may_contain_broken_elements;
	bool is_unsafe;
	bool has_warning[WarningType.TYPE_COUNT];
	String creator;
	bool big_endian;
	String filename;
	String relative_root;
	Blob raw_filename;
	Blob raw_relative_root;
	Exporter exporter;
	uint32 exporter_version;
	Props scene_props;
	Application original_application;
	Application latest_application;
	Thumbnail thumbnail;
	bool geometry_ignored;
	bool animation_ignored;
	bool embedded_ignored;
	size_t max_face_triangles;
	size_t result_memory_used;
	size_t temp_memory_used;
	size_t result_allocs;
	size_t temp_allocs;
	size_t element_buffer_size;
	size_t num_shader_textures;
	Real bone_prop_size_unit;
	bool bone_prop_limb_length_relative;
	Real ortho_size_unit;
	int64 ktime_second;
	String original_file_path;
	Blob raw_original_file_path;
	SpaceConversion space_conversion;
	Quat root_rotation;
	Real root_scale;
	MirrorAxis mirror_axis;
	Real geometry_scale;
}

[CCode (cname = "ufbx_time_mode", cprefix = "UFBX_TIME_MODE_", has_type_id = false)]
public enum TimeMode
{
	DEFAULT,
	120_FPS,
	100_FPS,
	60_FPS,
	50_FPS,
	48_FPS,
	30_FPS,
	30_FPS_DROP,
	NTSC_DROP_FRAME,
	NTSC_FULL_FRAME,
	PAL,
	24_FPS,
	1000_FPS,
	FILM_FULL_FRAME,
	CUSTOM,
	96_FPS,
	72_FPS,
	59_94_FPS,
	COUNT
}

[CCode (cname = "ufbx_time_protocol", cprefix = "UFBX_TIME_PROTOCOL_", has_type_id = false)]
public enum TimeProtocol
{
	SMPTE,
	FRAME_COUNT,
	DEFAULT,
	COUNT
}

[CCode (cname = "ufbx_snap_mode", cprefix = "UFBX_SNAP_MODE_", has_type_id = false)]
public enum SnapMode
{
	NONE,
	SNAP,
	PLAY,
	SNAP_AND_PLAY,
	COUNT
}

[CCode (cname = "ufbx_scene_settings", destroy_function = "", has_type_id = false)]
public struct SceneSettings
{
	Props props;
	CoordinateAxes axes;
	Real unit_meters;
	double frames_per_second;
	Vec3 ambient_color;
	String default_camera;
	TimeMode time_mode;
	TimeProtocol time_protocol;
	SnapMode snap_mode;
	CoordinateAxis original_axis_up;
	Real original_unit_meters;
}

[Compact]
[CCode (cname = "ufbx_scene", free_function = "ufbx_free_scene", has_type_id = false)]
public class Scene
{
	public Metadata metadata;
	public SceneSettings settings;
	public Node* root_node;
	public Anim* anim;
	[CCode (cname = "unknowns")]
	public UnknownList unknowns;
	[CCode (cname = "nodes")]
	public NodeList nodes;
	[CCode (cname = "meshes")]
	public MeshList meshes;
	[CCode (cname = "lights")]
	public LightList lights;
	[CCode (cname = "cameras")]
	public CameraList cameras;
	[CCode (cname = "bones")]
	public BoneList bones;
	[CCode (cname = "empties")]
	public EmptyList empties;
	[CCode (cname = "line_curves")]
	public LineCurveList line_curves;
	[CCode (cname = "nurbs_curves")]
	public NurbsCurveList nurbs_curves;
	[CCode (cname = "nurbs_surfaces")]
	public NurbsSurfaceList nurbs_surfaces;
	[CCode (cname = "nurbs_trim_surfaces")]
	public NurbsTrimSurfaceList nurbs_trim_surfaces;
	[CCode (cname = "nurbs_trim_boundaries")]
	public NurbsTrimBoundaryList nurbs_trim_boundaries;
	[CCode (cname = "procedural_geometries")]
	public ProceduralGeometryList procedural_geometries;
	[CCode (cname = "stereo_cameras")]
	public StereoCameraList stereo_cameras;
	[CCode (cname = "camera_switchers")]
	public CameraSwitcherList camera_switchers;
	[CCode (cname = "markers")]
	public MarkerList markers;
	[CCode (cname = "lod_groups")]
	public LogGroupList lod_groups;
	[CCode (cname = "skin_deformers")]
	public SkinDeformerList skin_deformers;
	[CCode (cname = "skin_clusters")]
	public SkinClusterList skin_clusters;
	[CCode (cname = "blend_deformers")]
	public BlendDeformerList blend_deformers;
	[CCode (cname = "blend_channels")]
	public BlendChannelList blend_channels;
	[CCode (cname = "blend_shapes")]
	public BlendShapeList blend_shapes;
	[CCode (cname = "cache_deformers")]
	public CacheDeformerList cache_deformers;
	[CCode (cname = "cache_files")]
	public CacheFileList cache_files;
	[CCode (cname = "materials")]
	public MaterialList materials;
	[CCode (cname = "textures")]
	public TextureList textures;
	[CCode (cname = "videos")]
	public VideoList videos;
	[CCode (cname = "shaders")]
	public ShaderList shaders;
	[CCode (cname = "shader_bindings")]
	public ShaderBindingList shader_bindings;
	[CCode (cname = "anim_stacks")]
	public AnimStackList anim_stacks;
	[CCode (cname = "anim_layers")]
	public AnimLayerList anim_layers;
	[CCode (cname = "anim_values")]
	public AnimValueList anim_values;
	[CCode (cname = "anim_curves")]
	public AnimCurveList anim_curves;
	[CCode (cname = "display_layers")]
	public DisplayLayerList display_layers;
	[CCode (cname = "selection_sets")]
	public SelectionSetList selection_sets;
	[CCode (cname = "selection_nodes")]
	public SelectionNodeList selection_nodes;
	[CCode (cname = "characters")]
	public CharacterList characters;
	[CCode (cname = "constraints")]
	public ConstraintList constraints;
	[CCode (cname = "audio_layers")]
	public AudioLayerList audio_layers;
	[CCode (cname = "audio_clips")]
	public AudioClipList audio_clips;
	[CCode (cname = "poses")]
	public PoseList poses;
	[CCode (cname = "metadata_objects")]
	public MetadataObjectList metadata_objects;
	[CCode (cname = "elements_by_type")]
	public ElementList elements_by_type[ElementType.TYPE_COUNT];
	public TextureFileList texture_files;
	public ElementList elements;
	public ConnectionList connections_src;
	public ConnectionList connections_dst;
	public NameElementList elements_by_name;
	public DomNode? dom_root;
	[CCode (cname = "ufbx_load_memory")]
	public static Scene? load_memory(void* data, size_t data_size, LoadOpts opts, ref Error error);
	[CCode (cname = "ufbx_load_file")]
	public static Scene? load_file(string filename, LoadOpts opts, ref Error error);
	[CCode (cname = "ufbx_load_file_len")]
	public static Scene? load_file_len(string filename, size_t filename_len, LoadOpts opts, ref Error error);
}

[CCode (cname = "ufbx_curve_point", destroy_function = "", has_type_id = false)]
public struct CurvePoint
{
	bool valid;
	Vec3 position;
	Vec3 derivative;
}

[CCode (cname = "ufbx_surface_point", destroy_function = "", has_type_id = false)]
public struct SurfacePoint
{
	bool valid;
	Vec3 position;
	Vec3 derivative_u;
	Vec3 derivative_v;
}

[CCode (cname = "ufbx_topo_flags", cprefix = "UFBX_TOPO_", has_type_id = false)]
[Flags]
public enum TopoFlags
{
	NON_MANIFOLD
}

[CCode (cname = "ufbx_topo_edge", destroy_function = "", has_type_id = false)]
public struct TopoEdge
{
	uint32 index;
	uint32 next;
	uint32 prev;
	uint32 twin;
	uint32 face;
	uint32 edge;
	TopoFlags flags;
}

[CCode (cname = "ufbx_vertex_stream", destroy_function = "", has_type_id = false)]
public struct VertexStream
{
	void* data;
	size_t vertex_count;
	size_t vertex_size;
}

[CCode (cname = "ufbx_alloc_fn", has_target = true, delegate_target_pos = 0.9)]
public delegate void* AllocFn(size_t size);
[CCode (cname = "ufbx_realloc_fn", has_target = true, delegate_target_pos = 0.9)]
public delegate void* ReallocFn(void* old_ptr, size_t old_size, size_t new_size);
[CCode (cname = "ufbx_free_fn", has_target = true, delegate_target_pos = 0.9)]
public delegate void FreeFn(void* ptr, size_t size);
[CCode (cname = "ufbx_free_allocator_fn", has_target = true, delegate_target_pos = 0.9)]
public delegate void FreeAllocatorFn();
[CCode (cname = "ufbx_allocator", destroy_function = "", has_type_id = false)]
public struct Allocator
{
	AllocFn alloc_fn;
	ReallocFn realloc_fn;
	FreeFn free_fn;
	FreeAllocatorFn free_allocator_fn;
	void* user;
}

[CCode (cname = "ufbx_allocator_opts", destroy_function = "", has_type_id = false)]
public struct AllocatorOpts
{
	Allocator allocator;
	size_t memory_limit;
	size_t allocation_limit;
	size_t huge_threshold;
	size_t max_chunk_size;
}

[CCode (cname = "ufbx_read_fn", has_target = true, delegate_target_pos = 0.9)]
public delegate size_t ReadFn(void* data, size_t size);
[CCode (cname = "ufbx_skip_fn", has_target = true, delegate_target_pos = 0.9)]
public delegate bool SkipFn(size_t size);
[CCode (cname = "ufbx_size_fn", has_target = true, delegate_target_pos = 0.9)]
public delegate uint64 SizeFn();
[CCode (cname = "ufbx_close_fn", has_target = true, delegate_target_pos = 0.9)]
public delegate void CloseFn();
[CCode (cname = "ufbx_stream", destroy_function = "", has_type_id = false)]
public struct Stream
{
	ReadFn read_fn;
	SkipFn skip_fn;
	SizeFn size_fn;
	CloseFn close_fn;
	void* user;
}

[CCode (cname = "ufbx_open_file_type", cprefix = "UFBX_OPEN_FILE_", has_type_id = false)]
public enum OpenFileType
{
	MAIN_MODEL,
	GEOMETRY_CACHE,
	OBJ_MTL,
	TYPE_COUNT
}

[SimpleType]
[CCode (cname = "ufbx_open_file_context", has_type_id = false)]
public struct OpenFileContext : uintptr
{
}

[CCode (cname = "ufbx_open_file_info", destroy_function = "", has_type_id = false)]
public struct OpenFileInfo
{
	OpenFileContext context;
	OpenFileType type;
	Blob original_filename;
}

[CCode (cname = "ufbx_open_file_fn", has_target = true, delegate_target_pos = 0.9)]
public delegate bool OpenFileFn(Stream stream, string path, size_t path_len, OpenFileInfo info);
[CCode (cname = "ufbx_open_file_cb", destroy_function = "", has_type_id = false)]
public struct OpenFileCb
{
	OpenFileFn fn;
	void* user;
}

[CCode (cname = "ufbx_open_file_opts", destroy_function = "", has_type_id = false)]
public struct OpenFileOptions
{
	uint32 _begin_zero;
	AllocatorOpts allocator;
	bool filename_null_terminated;
	uint32 _end_zero;
}

[CCode (cname = "ufbx_close_memory_fn", has_target = true, delegate_target_pos = 0.9)]
public delegate void CloseMemoryFn(void* data, size_t data_size);
[CCode (cname = "ufbx_close_memory_cb", destroy_function = "", has_type_id = false)]
public struct CloseMemoryCb
{
	CloseMemoryFn fn;
	void* user;
}

[CCode (cname = "ufbx_open_memory_opts", destroy_function = "", has_type_id = false)]
public struct OpenMemoryOpts
{
	uint32 _begin_zero;
	AllocatorOpts allocator;
	bool no_copy;
	CloseMemoryCb close_cb;
	uint32 _end_zero;
}

[CCode (cname = "ufbx_error_frame", destroy_function = "", has_type_id = false)]
public struct ErrorFrame
{
	uint32 source_line;
	String function;
	String description;
}

[CCode (cname = "ufbx_error_type", cprefix = "UFBX_ERROR_", has_type_id = false)]
public enum ErrorType
{
	NONE,
	UNKNOWN,
	FILE_NOT_FOUND,
	EMPTY_FILE,
	EXTERNAL_FILE_NOT_FOUND,
	OUT_OF_MEMORY,
	MEMORY_LIMIT,
	ALLOCATION_LIMIT,
	TRUNCATED_FILE,
	IO,
	CANCELLED,
	UNRECOGNIZED_FILE_FORMAT,
	UNINITIALIZED_OPTIONS,
	ZERO_VERTEX_SIZE,
	TRUNCATED_VERTEX_STREAM,
	INVALID_UTF8,
	FEATURE_DISABLED,
	BAD_NURBS,
	BAD_INDEX,
	NODE_DEPTH_LIMIT,
	THREADED_ASCII_PARSE,
	UNSAFE_OPTIONS,
	DUPLICATE_OVERRIDE,
	TYPE_COUNT
}

[CCode (cname = "ufbx_error", destroy_function = "", has_type_id = false)]
public struct Error
{
	ErrorType type;
	String description;
	uint32 stack_size;
	ErrorFrame stack[ERROR_STACK_MAX_DEPTH];
	size_t info_length;
	char info[ERROR_INFO_LENGTH];
}

[CCode (cname = "ufbx_progress", destroy_function = "", has_type_id = false)]
public struct Progress
{
	uint64 bytes_read;
	uint64 bytes_total;
}

[CCode (cname = "ufbx_progress_result", cprefix = "UFBX_PROGRESS_", has_type_id = false)]
[Flags]
public enum ProgressResult
{
	CONTINUE,
	CANCEL
}

[CCode (cname = "ufbx_progress_fn", destroy_function = "", has_type_id = false)]
public delegate ProgressResult ProgressFn(Progress progress);
[CCode (cname = "ufbx_progress_cb", destroy_function = "", has_type_id = false)]
public struct ProgressCb
{
	ProgressFn fn;
	void* user;
}

[CCode (cname = "ufbx_inflate_input", destroy_function = "", has_type_id = false)]
public struct InflateInput
{
	size_t total_size;
	void* data;
	size_t data_size;
	void* buffer;
	size_t buffer_size;
	ReadFn read_fn;
	void* read_user;
	ProgressCb progress_cb;
	uint64 progress_interval_hint;
	uint64 progress_size_before;
	uint64 progress_size_after;
	bool no_header;
	bool no_checksum;
	size_t internal_fast_bits;
}

[CCode (cname = "ufbx_inflate_retain", destroy_function = "", has_type_id = false)]
struct InflateRetain
{
	bool initialized;
	uint64 data[1024];
}

[CCode (cname = "ufbx_index_error_handling", cprefix = "UFBX_INDEX_ERROR_HANDLING_", has_type_id = false)]
public enum IndexErrorHandling
{
	CLAMP,
	NO_INDEX,
	ABORT_LOADING,
	UNSAFE_IGNORE,
	COUNT
}

[CCode (cname = "ufbx_unicode_error_handling", cprefix = "UFBX_UNICODE_ERROR_HANDLING_", has_type_id = false)]
public enum UnicodeErrorHandling
{
	REPLACEMENT_CHARACTER,
	UNDERSCORE,
	QUESTION_MARK,
	REMOVE,
	ABORT_LOADING,
	UNSAFE_IGNORE,
	COUNT
}

[CCode (cname = "ufbx_geometry_transform_handling", cprefix = "UFBX_GEOMETRY_TRANSFORM_HANDLING_", has_type_id = false)]
public enum GeometryTransformHandling
{
	PRESERVE,
	HELPER_NODES,
	MODIFY_GEOMETRY,
	MODIFY_GEOMETRY_NO_FALLBACK,
	COUNT
}

[CCode (cname = "ufbx_inherit_mode_handling", cprefix = "UFBX_INHERIT_MODE_HANDLING_", has_type_id = false)]
public enum InheritModeHandling
{
	PRESERVE,
	HELPER_NODES,
	COMPENSATE,
	COMPENSATE_NO_FALLBACK,
	IGNORE,
	COUNT
}

[CCode (cname = "ufbx_pivot_handling", cprefix = "UFBX_PIVOT_HANDLING_", has_type_id = false)]
public enum PivotHandling
{
	RETAIN,
	ADJUST_TO_PIVOT,
	COUNT
}

[CCode (cname = "ufbx_baked_key_flags", cprefix = "UFBX_BAKED_KEY_", has_type_id = false)]
[Flags]
public enum BakedKeyFlags
{
	STEP_LEFT,
	STEP_RIGHT,
	STEP_KEY,
	KEYFRAME,
	REDUCED
}

[CCode (cname = "ufbx_baked_vec3", destroy_function = "", has_type_id = false)]
public struct BakedVec3
{
	double time;
	Vec3 value;
	BakedKeyFlags flags;
}

[CCode (cname = "ufbx_baked_vec3_list", destroy_function = "", has_type_id = false)]
public struct BakedVec3List
{
	BakedVec3* data;
	size_t count;
}

[CCode (cname = "ufbx_baked_quat", destroy_function = "", has_type_id = false)]
public struct BakedQuat
{
	double time;
	Quat value;
	BakedKeyFlags flags;
}

[CCode (cname = "ufbx_baked_quat_list", destroy_function = "", has_type_id = false)]
public struct BakedQuatList
{
	BakedQuat* data;
	size_t count;
}

[CCode (cname = "ufbx_baked_node", destroy_function = "", has_type_id = false)]
public struct BakedNode
{
	uint32 typed_id;
	uint32 element_id;
	bool constant_translation;
	bool constant_rotation;
	bool constant_scale;
	BakedVec3List translation_keys;
	BakedQuatList rotation_keys;
	BakedVec3List scale_keys;
}

[CCode (cname = "ufbx_baked_node_list", destroy_function = "", has_type_id = false)]
public struct BakedNodeList
{
	BakedNode* data;
	size_t count;
}

[CCode (cname = "ufbx_baked_prop", destroy_function = "", has_type_id = false)]
public struct BakedProp
{
	String name;
	bool constant_value;
	BakedVec3List keys;
}

[CCode (cname = "ufbx_baked_prop_list", destroy_function = "", has_type_id = false)]
public struct BakedPropList
{
	BakedProp* data;
	size_t count;
}

[CCode (cname = "ufbx_baked_element", destroy_function = "", has_type_id = false)]
public struct BakedElement
{
	uint32 element_id;
	BakedPropList props;
}

[CCode (cname = "ufbx_baked_element_list", destroy_function = "", has_type_id = false)]
public struct BakedElementList
{
	BakedElement* data;
	size_t count;
}

[CCode (cname = "ufbx_baked_anim_metadata", destroy_function = "", has_type_id = false)]
public struct BakedAnimMetadata
{
	size_t result_memory_used;
	size_t temp_memory_used;
	size_t result_allocs;
	size_t temp_allocs;
}

[CCode (cname = "ufbx_baked_anim", destroy_function = "", has_type_id = false)]
public struct BakedAnim
{
	BakedNodeList nodes;
	BakedElementList elements;
	double playback_time_begin;
	double playback_time_end;
	double playback_duration;
	double key_time_min;
	double key_time_max;
	BakedAnimMetadata metadata;
}

//

[CCode (cname = "ufbx_thread_pool_context", has_type_id = false)]
[SimpleType]
public struct ThreadPoolContext : uintptr
{
}

[CCode (cname = "ufbx_thread_pool_info", destroy_function = "", has_type_id = false)]
public struct ThreadPoolInfo
{
	uint32 max_concurrent_tasks;
}

[CCode (cname = "ufbx_thread_pool_init_fn", has_target = true, delegate_target_pos = 0.9)]
public delegate bool ThreadPoolInitFn(ThreadPoolContext ctx, ThreadPoolInfo info);
[CCode (cname = "ufbx_thread_pool_run_fn", has_target = true, delegate_target_pos = 0.9)]
public delegate bool ThreadPoolRunFn(ThreadPoolContext ctx, uint32 group, uint32 start_index, uint32 count);
[CCode (cname = "ufbx_thread_pool_wait_fn", has_target = true, delegate_target_pos = 0.9)]
public delegate bool ThreadPoolWaitFn(ThreadPoolContext ctx, uint32 group, uint32 max_index);
[CCode (cname = "ufbx_thread_pool_free_fn", has_target = true, delegate_target_pos = 0.9)]
public delegate void ThreadPoolFreeFn(ThreadPoolContext ctx);
[CCode (cname = "ufbx_thread_pool", destroy_function = "", has_type_id = false)]
public struct ThreadPool
{
	ThreadPoolInitFn init_fn;
	ThreadPoolRunFn run_fn;
	ThreadPoolWaitFn wait_fn;
	ThreadPoolFreeFn free_fn;
	void* user;
}

[CCode (cname = "ufbx_thread_opts", destroy_function = "", has_type_id = false)]
public struct ThreadOpts
{
	ThreadPool pool;
	size_t num_tasks;
	size_t memory_limit;
}

[CCode (cname = "ufbx_load_opts", destroy_function = "", has_type_id = false)]
public struct LoadOpts
{
	uint32 _begin_zero;
	AllocatorOpts temp_allocator;
	AllocatorOpts result_allocator;
	ThreadOpts thread_opts;
	bool ignore_geometry;
	bool ignore_animation;
	bool ignore_embedded;
	bool ignore_all_content;
	bool evaluate_skinning;
	bool evaluate_caches;
	bool load_external_files;
	bool ignore_missing_external_files;
	bool skip_skin_vertices;
	bool skip_mesh_parts;
	bool clean_skin_weights;
	bool use_blender_pbr_material;
	bool disable_quirks;
	bool strict;
	bool force_single_thread_ascii_parsing;
	bool allow_unsafe;
	IndexErrorHandling index_error_handling;
	bool connect_broken_elements;
	bool allow_nodes_out_of_root;
	bool allow_missing_vertex_position;
	bool allow_empty_faces;
	bool generate_missing_normals;
	bool open_main_file_with_default;
	char path_separator;
	uint32 node_depth_limit;
	uint64 file_size_estimate;
	size_t read_buffer_size;
	String filename;
	Blob raw_filename;
	ProgressCb progress_cb;
	uint64 progress_interval_hint;
	OpenFileCb open_file_cb;
	GeometryTransformHandling geometry_transform_handling;
	InheritModeHandling inherit_mode_handling;
	PivotHandling pivot_handling;
	SpaceConversion space_conversion;
	MirrorAxis handedness_conversion_axis;
	bool handedness_conversion_retain_winding;
	bool reverse_winding;
	CoordinateAxes target_axes;
	Real target_unit_meters;
	CoordinateAxes target_camera_axes;
	CoordinateAxes target_light_axes;
	String geometry_transform_helper_name;
	String scale_helper_name;
	bool normalize_normals;
	bool normalize_tangents;
	bool use_root_transform;
	Transform root_transform;
	double key_clamp_threshold;
	UnicodeErrorHandling unicode_error_handling;
	bool retain_vertex_attrib_w;
	bool retain_dom;
	FileFormat file_format;
	size_t file_format_lookahead;
	bool no_format_from_content;
	bool no_format_from_extension;
	bool obj_search_mtl_by_filename;
	bool obj_merge_objects;
	bool obj_merge_groups;
	bool obj_split_groups;
	String obj_mtl_path;
	Blob obj_mtl_data;
	Real obj_unit_meters;
	CoordinateAxes obj_axes;
	uint32 _end_zero;
}

[CCode (cname = "ufbx_evaluate_opts", destroy_function = "", has_type_id = false)]
public struct EvaluateOpts
{
	uint32 _begin_zero;
	Allocator temp_allocator;
	AllocatorOpts result_allocator;
	bool evaluate_skinning;
	bool evaluate_caches;
	bool load_external_files;
	OpenFileCb open_file_cb;
	uint32 _end_zero;
}

[CCode (cname = "ufbx_const_uint32_list", destroy_function = "", has_type_id = false)]
public struct ConstUint32List
{
	uint32* data;
	size_t count;
}

[CCode (cname = "ufbx_const_real_list", destroy_function = "", has_type_id = false)]
public struct ConstRealList
{
	Real* data;
	size_t count;
}

[CCode (cname = "ufbx_prop_override_desc", destroy_function = "", has_type_id = false)]
public struct PropOverrideDesc
{
	uint32 element_id;
	String prop_name;
	Vec4 value;
	String value_str;
	int64 value_int;
}

[CCode (cname = "ufbx_const_prop_override_desc_list", destroy_function = "", has_type_id = false)]
public struct ConstPropOverrideDescList
{
	PropOverrideDesc* data;
	size_t count;
}

[CCode (cname = "ufbx_const_transform_override_list", destroy_function = "", has_type_id = false)]
public struct ConstTransformOverrideList
{
	TransformOverride* data;
	size_t count;
}

[CCode (cname = "ufbx_anim_opts", destroy_function = "", has_type_id = false)]
public struct AnimOpts
{
	uint32 _begin_zero;
	ConstUint32List layer_ids;
	ConstRealList override_layer_weights;
	ConstPropOverrideDescList prop_overrides;
	ConstTransformOverrideList transform_overrides;
	bool ignore_connections;
	AllocatorOpts result_allocator;
	uint32 _end_zero;
}

[CCode (cname = "ufbx_bake_step_handling", cprefix = "UFBX_BAKE_STEP_HANDLING_", has_type_id = false)]
public enum BakeStepHandling
{
	DEFAULT,
	CUSTOM_DURATION,
	IDENTICAL_TIME,
	ADJACENT_DOUBLE,
	IGNORE,
	COUNT
}

[CCode (cname = "ufbx_bake_opts", destroy_function = "", has_type_id = false)]
public struct BakeOpts
{
	uint32 _begin_zero;
	AllocatorOpts temp_allocator;
	AllocatorOpts result_allocator;
	bool trim_start_time;
	double resample_rate;
	double minimum_sample_rate;
	double maximum_sample_rate;
	bool bake_transform_props;
	bool skip_node_transforms;
	bool no_resample_rotation;
	bool ignore_layer_weight_animation;
	size_t max_keyframe_segments;
	BakeStepHandling step_handling;
	double step_custom_duration;
	double step_custom_epsilon;
	bool key_reduction_enabled;
	bool key_reduction_rotation;
	double key_reduction_threshold;
	size_t key_reduction_passes;
	uint32 _end_zero;
}

[CCode (cname = "ufbx_tessellate_curve_opts", destroy_function = "", has_type_id = false)]
public struct TessellateCurveOpts
{
	uint32 _begin_zero;
	AllocatorOpts temp_allocator;
	AllocatorOpts result_allocator;
	size_t span_subdivision;
	uint32 _end_zero;
}

[CCode (cname = "ufbx_tessellate_surface_opts", destroy_function = "", has_type_id = false)]
public struct TessellateSurfaceOpts
{
	uint32 _begin_zero;
	AllocatorOpts temp_allocator;
	AllocatorOpts result_allocator;
	size_t span_subdivision_u;
	size_t span_subdivision_v;
	bool skip_mesh_parts;
	uint32 _end_zero;
}

[CCode (cname = "ufbx_subdivide_opts", destroy_function = "", has_type_id = false)]
public struct SubdivideOpts
{
	uint32 _begin_zero;
	AllocatorOpts temp_allocator;
	AllocatorOpts result_allocator;
	SubdivisionBoundary boundary;
	SubdivisionBoundary uv_boundary;
	bool ignore_normals;
	bool interpolate_normals;
	bool interpolate_tangents;
	bool evaluate_source_vertices;
	size_t max_source_vertices;
	bool evaluate_skin_weights;
	size_t max_skin_weights;
	size_t skin_deformer_index;
	uint32 _end_zero;
}

[CCode (cname = "ufbx_geometry_cache_opts", destroy_function = "", has_type_id = false)]
public struct GeometryCacheOpts
{
	uint32 _begin_zero;
	AllocatorOpts temp_allocator;
	AllocatorOpts result_allocator;
	OpenFileCb open_file_cb;
	double frames_per_second;
	MirrorAxis mirror_axis;
	bool use_scale_factor;
	Real scale_factor;
	uint32 _end_zero;
}

[CCode (cname = "ufbx_geometry_cache_data_opts", destroy_function = "", has_type_id = false)]
public struct GeometryCacheDataOpts
{
	uint32 _begin_zero;
	OpenFileCb open_file_cb;
	bool additive;
	bool use_weight;
	Real weight;
	bool ignore_transform;
	uint32 _end_zero;
}

[CCode (cname = "ufbx_panic", destroy_function = "", has_type_id = false)]
public struct Panic
{
	bool did_panic;
	size_t message_length;
	char message[PANIC_MESSAGE_LENGTH];
}

public bool is_thread_safe();

} /* namespace ufbx */
