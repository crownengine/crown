/*
 * Copyright (c) 2012-2026 Daniele Bartolini <dbartolini@crownengine.org>
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
	[CCode (array_length_cname = "length", array_length_type = "size_t")]
	public char[] data;
}

[SimpleType]
[CCode (cname = "ufbx_blob", destroy_function = "", has_type_id = false)]
public struct Blob
{
	[CCode (array_length_cname = "size", array_length_type = "size_t")]
	public void[] data;
}

[SimpleType]
[CCode (cname = "ufbx_vec2", destroy_function = "", has_type_id = false)]
public struct Vec2
{
	public Real x;
	public Real y;
	[CCode (cname = "v")]
	public Real v[2];
}

[SimpleType]
[CCode (cname = "ufbx_vec3", destroy_function = "", has_type_id = false)]
public struct Vec3
{
	public Real x;
	public Real y;
	public Real z;
	[CCode (cname = "v")]
	public Real v[3];
}

[SimpleType]
[CCode (cname = "ufbx_vec4", destroy_function = "", has_type_id = false)]
public struct Vec4
{
	public Real x;
	public Real y;
	public Real z;
	public Real w;
	[CCode (cname = "v")]
	public Real v[3];
}

[SimpleType]
[CCode (cname = "ufbx_quat", destroy_function = "", has_type_id = false)]
public struct Quat
{
	public Real x;
	public Real y;
	public Real z;
	public Real w;
	[CCode (cname = "v")]
	public Real v[4];
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
	public Vec3 translation;
	public Quat rotation;
	public Vec3 scale;
}

[CCode (cname = "ufbx_matrix", destroy_function = "", has_type_id = false)]
public struct Matrix
{
	public Real m00;
	public Real m10;
	public Real m20;
	public Real m01;
	public Real m11;
	public Real m21;
	public Real m02;
	public Real m12;
	public Real m22;
	public Real m03;
	public Real m13;
	public Real m23;
	[CCode (cname = "cols")]
	public Vec3 cols[4];
	[CCode (cname = "v")]
	public Real v[12];
}

[CCode (cname = "ufbx_void_list", destroy_function = "", has_type_id = false)]
public struct VoidList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public void[] data;
}

[CCode (cname = "ufbx_bool_list", destroy_function = "", has_type_id = false)]
public struct BoolList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public bool[] data;
}

[CCode (cname = "ufbx_uint32_list", destroy_function = "", has_type_id = false)]
public struct Uint32List
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public uint32[] data;
}

[CCode (cname = "ufbx_real_list", destroy_function = "", has_type_id = false)]
public struct RealList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public Real[] data;
}

[CCode (cname = "ufbx_vec2_list", destroy_function = "", has_type_id = false)]
public struct Vec2List
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public Vec2[] data;
}

[CCode (cname = "ufbx_vec3_list", destroy_function = "", has_type_id = false)]
public struct Vec3List
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public Vec3[] data;
}

[CCode (cname = "ufbx_vec4_list", destroy_function = "", has_type_id = false)]
public struct Vec4List
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public Vec4[] data;
}

[CCode (cname = "ufbx_string_list", destroy_function = "", has_type_id = false)]
public struct StringList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public String[] data;
}

public const uint32 NO_INDEX;
[CCode (cname = "ufbx_dom_value_type", cprefix = "UFBX_DOM_VALUE_", has_type_id = false)]
public enum DomValueType
{
	NUMBER,
	STRING,
	BLOB,
	ARRAY_I32,
	ARRAY_I64,
	ARRAY_F32,
	ARRAY_F64,
	ARRAY_BLOB,
	ARRAY_IGNORED,
	TYPE_COUNT
}

[CCode (cname = "ufbx_dom_value", destroy_function = "", has_type_id = false)]
public struct DomValue
{
	public DomValueType type;
	public String value_str;
	public Blob value_blob;
	public int64 value_int;
	public double value_float;
}

[CCode (cname = "ufbx_dom_node_list", destroy_function = "", has_type_id = false)]
public struct DomNodeList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public DomNode[] data;
}

[CCode (cname = "ufbx_dom_value_list", destroy_function = "", has_type_id = false)]
public struct DomValueList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public DomValue[] data;
}

[Compact]
[CCode (cname = "ufbx_dom_node", destroy_function = "", has_type_id = false)]
public class DomNode
{
	public String name;
	public DomNodeList children;
	public DomValueList values;
}

[CCode (cname = "ufbx_int32_list", destroy_function = "", has_type_id = false)]
public struct Int32List
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public int32[] data;
}

[CCode (cname = "ufbx_int64_list", destroy_function = "", has_type_id = false)]
public struct Int64List
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public int64[] data;
}

[CCode (cname = "ufbx_float_list", destroy_function = "", has_type_id = false)]
public struct FloatList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public float[] data;
}

[CCode (cname = "ufbx_double_list", destroy_function = "", has_type_id = false)]
public struct DoubleList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public double[] data;
}

[CCode (cname = "ufbx_blob_list", destroy_function = "", has_type_id = false)]
public struct BlobList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public Blob[] data;
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

[Compact]
[CCode (cname = "ufbx_prop", destroy_function = "", has_type_id = false)]
public class Prop
{
	public String name;
	public uint32 _internal_key;
	public PropType type;
	public PropFlags flags;
	public String value_str;
	public Blob value_blob;
	public int64 value_int;
	[CCode (cname = "value_real_arr")]
	public Real value_real_arr[4];
	[CCode (cname = "value_real")]
	public Real value_real;
	[CCode (cname = "value_vec2")]
	public Vec2 value_vec2;
	[CCode (cname = "value_vec3")]
	public Vec3 value_vec3;
	[CCode (cname = "value_vec4")]
	public Vec4 value_vec4;
}

[CCode (cname = "ufbx_prop_list", destroy_function = "", has_type_id = false)]
public struct PropList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public Prop[] data;
}

[CCode (cname = "ufbx_props", destroy_function = "", has_type_id = false)]
public struct Props
{
	public PropList props;
	public size_t num_animated;
	public unowned Props? defaults;
}

[CCode (cname = "ufbx_element_list", destroy_function = "", has_type_id = false)]
public struct ElementList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public Element[] data;
}
[CCode (cname = "ufbx_unknown_list", destroy_function = "", has_type_id = false)]
public struct UnknownList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public Unknown[] data;
}
[CCode (cname = "ufbx_node_list", destroy_function = "", has_type_id = false)]
public struct NodeList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public (unowned Node)[] data;
}
[CCode (cname = "ufbx_mesh_list", destroy_function = "", has_type_id = false)]
public struct MeshList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public Mesh[] data;
}
[CCode (cname = "ufbx_light_list", destroy_function = "", has_type_id = false)]
public struct LightList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public Light[] data;
}
[CCode (cname = "ufbx_camera_list", destroy_function = "", has_type_id = false)]
public struct CameraList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public Camera[] data;
}
[CCode (cname = "ufbx_bone_list", destroy_function = "", has_type_id = false)]
public struct BoneList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public Bone[] data;
}
[CCode (cname = "ufbx_empty_list", destroy_function = "", has_type_id = false)]
public struct EmptyList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public Empty[] data;
}
[CCode (cname = "ufbx_line_curve_list", destroy_function = "", has_type_id = false)]
public struct LineCurveList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public LineCurve[] data;
}
[CCode (cname = "ufbx_nurbs_curve_list", destroy_function = "", has_type_id = false)]
public struct NurbsCurveList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public NurbsCurve[] data;
}
[CCode (cname = "ufbx_nurbs_surface_list", destroy_function = "", has_type_id = false)]
public struct NurbsSurfaceList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public NurbsSurface[] data;
}
[CCode (cname = "ufbx_nurbs_trim_surface_list", destroy_function = "", has_type_id = false)]
public struct NurbsTrimSurfaceList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public NurbsTrimSurface[] data;
}
[CCode (cname = "ufbx_nurbs_trim_boundary_list", destroy_function = "", has_type_id = false)]
public struct NurbsTrimBoundaryList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public NurbsTrimBoundary[] data;
}
[CCode (cname = "ufbx_procedural_geometry_list", destroy_function = "", has_type_id = false)]
public struct ProceduralGeometryList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public ProceduralGeometry[] data;
}
[CCode (cname = "ufbx_stereo_camera_list", destroy_function = "", has_type_id = false)]
public struct StereoCameraList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public StereoCamera[] data;
}
[CCode (cname = "ufbx_camera_switcher_list", destroy_function = "", has_type_id = false)]
public struct CameraSwitcherList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public CameraSwitcher[] data;
}
[CCode (cname = "ufbx_marker_list", destroy_function = "", has_type_id = false)]
public struct MarkerList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public Marker[] data;
}
[CCode (cname = "ufbx_lod_group_list", destroy_function = "", has_type_id = false)]
public struct LogGroupList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public LodGroup[] data;
}
[CCode (cname = "ufbx_skin_deformer_list", destroy_function = "", has_type_id = false)]
public struct SkinDeformerList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public SkinDeformer[] data;
}
[CCode (cname = "ufbx_skin_cluster_list", destroy_function = "", has_type_id = false)]
public struct SkinClusterList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public SkinCluster[] data;
}
[CCode (cname = "ufbx_blend_deformer_list", destroy_function = "", has_type_id = false)]
public struct BlendDeformerList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public BlendDeformer[] data;
}
[CCode (cname = "ufbx_blend_channel_list", destroy_function = "", has_type_id = false)]
public struct BlendChannelList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public BlendChannel[] data;
}
[CCode (cname = "ufbx_blend_shape_list", destroy_function = "", has_type_id = false)]
public struct BlendShapeList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public BlendShape[] data;
}
[CCode (cname = "ufbx_cache_deformer_list", destroy_function = "", has_type_id = false)]
public struct CacheDeformerList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public CacheDeformer[] data;
}
[CCode (cname = "ufbx_cache_file_list", destroy_function = "", has_type_id = false)]
public struct CacheFileList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public CacheFile[] data;
}
[CCode (cname = "ufbx_material_list", destroy_function = "", has_type_id = false)]
public struct MaterialList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public Material[] data;
}
[CCode (cname = "ufbx_texture_list", destroy_function = "", has_type_id = false)]
public struct TextureList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public Texture[] data;
}
[CCode (cname = "ufbx_video_list", destroy_function = "", has_type_id = false)]
public struct VideoList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public Video[] data;
}
[CCode (cname = "ufbx_shader_list", destroy_function = "", has_type_id = false)]
public struct ShaderList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public Shader[] data;
}
[CCode (cname = "ufbx_shader_binding_list", destroy_function = "", has_type_id = false)]
public struct ShaderBindingList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public ShaderBinding[] data;
}
[CCode (cname = "ufbx_anim_stack_list", destroy_function = "", has_type_id = false)]
public struct AnimStackList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public AnimStack[] data;
}
[CCode (cname = "ufbx_anim_layer_list", destroy_function = "", has_type_id = false)]
public struct AnimLayerList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public AnimLayer[] data;
}
[CCode (cname = "ufbx_anim_value_list", destroy_function = "", has_type_id = false)]
public struct AnimValueList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public AnimValue[] data;
}
[CCode (cname = "ufbx_anim_curve_list", destroy_function = "", has_type_id = false)]
public struct AnimCurveList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public AnimCurve[] data;
}
[CCode (cname = "ufbx_display_layer_list", destroy_function = "", has_type_id = false)]
public struct DisplayLayerList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public DisplayLayer[] data;
}
[CCode (cname = "ufbx_selection_set_list", destroy_function = "", has_type_id = false)]
public struct SelectionSetList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public SelectionSet[] data;
}
[CCode (cname = "ufbx_selection_node_list", destroy_function = "", has_type_id = false)]
public struct SelectionNodeList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public SelectionNode[] data;
}
[CCode (cname = "ufbx_character_list", destroy_function = "", has_type_id = false)]
public struct CharacterList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public Character[] data;
}
[CCode (cname = "ufbx_constraint_list", destroy_function = "", has_type_id = false)]
public struct ConstraintList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public Constraint[] data;
}
[CCode (cname = "ufbx_audio_layer_list", destroy_function = "", has_type_id = false)]
public struct AudioLayerList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public AudioLayer[] data;
}
[CCode (cname = "ufbx_audio_clip_list", destroy_function = "", has_type_id = false)]
public struct AudioClipList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public AudioClip[] data;
}
[CCode (cname = "ufbx_pose_list", destroy_function = "", has_type_id = false)]
public struct PoseList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public Pose[] data;
}
[CCode (cname = "ufbx_metadata_object_list", destroy_function = "", has_type_id = false)]
public struct MetadataObjectList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public MetadataObject[] data;
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

[SimpleType]
[CCode (cname = "ufbx_connection", destroy_function = "", has_type_id = false)]
public struct Connection
{
	public unowned Element src;
	public unowned Element dst;
	public String src_prop;
	public String dst_prop;
}

[CCode (cname = "ufbx_connection_list", destroy_function = "", has_type_id = false)]
public struct ConnectionList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public Connection[] data;
}

[Compact]
[CCode (cname = "ufbx_element", destroy_function = "", has_type_id = false)]
public class Element
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

[Compact]
[CCode (cname = "ufbx_unknown", destroy_function = "", has_type_id = false)]
public class Unknown
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	public String type;
	public String super_type;
	public String sub_type;
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
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	public unowned Node? parent;
	public NodeList children;
	public unowned Mesh? mesh;
	public unowned Light? light;
	public unowned Camera? camera;
	public unowned Bone? bone;
	public unowned Element? attrib;
	public unowned Node? geometry_transform_helper;
	public unowned Node? scale_helper;
	public ElementType attrib_type;
	public ElementList all_attribs;
	public InheritMode inherit_mode;
	public InheritMode original_inherit_mode;
	public Transform local_transform;
	public Transform geometry_transform;
	public Vec3 inherit_scale;
	public unowned Node? inherit_scale_node;
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
	public unowned Pose? bind_pose;
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
	public bool exists;
	public VoidList values;
	public Uint32List indices;
	public size_t value_reals;
	public bool unique_per_vertex;
	public RealList values_w;
}

[CCode (cname = "ufbx_vertex_real", destroy_function = "", has_type_id = false)]
public struct VertexReal
{
	public bool exists;
	public RealList values;
	public Uint32List indices;
	public size_t value_reals;
	public bool unique_per_vertex;
	public RealList values_w;
}

[CCode (cname = "ufbx_vertex_vec2", destroy_function = "", has_type_id = false)]
public struct VertexVec2
{
	public bool exists;
	public Vec2List values;
	public Uint32List indices;
	public size_t value_reals;
	public bool unique_per_vertex;
	public RealList values_w;
}

[CCode (cname = "ufbx_vertex_vec3", destroy_function = "", has_type_id = false)]
public struct VertexVec3
{
	public bool exists;
	public Vec3List values;
	public Uint32List indices;
	public size_t value_reals;
	public bool unique_per_vertex;
	public RealList values_w;
}

[CCode (cname = "ufbx_vertex_vec4", destroy_function = "", has_type_id = false)]
public struct VertexVec4
{
	public bool exists;
	public Vec4List values;
	public Uint32List indices;
	public size_t value_reals;
	public bool unique_per_vertex;
	public RealList values_w;
}

[SimpleType]
[CCode (cname = "ufbx_uv_set", destroy_function = "", has_type_id = false)]
public struct UvSet
{
	public String name;
	public uint32 index;
	public VertexVec2 vertex_uv;
	public VertexVec3 vertex_tangent;
	public VertexVec3 vertex_bitangent;
}

[SimpleType]
[CCode (cname = "ufbx_color_set", destroy_function = "", has_type_id = false)]
public struct ColorSet
{
	public String name;
	public uint32 index;
	public VertexVec4 vertex_color;
}

[CCode (cname = "ufbx_uv_set_list", destroy_function = "", has_type_id = false)]
public struct UvSetList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public UvSet[] data;
}

[CCode (cname = "ufbx_color_set_list", destroy_function = "", has_type_id = false)]
public struct ColorSetList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public ColorSet[] data;
}

[SimpleType]
[CCode (cname = "ufbx_edge", destroy_function = "", has_type_id = false)]
public struct Edge
{
	[CCode (cname = "a")]
	public uint32 a;
	[CCode (cname = "b")]
	public uint32 b;
	[CCode (cname = "indices")]
	public uint32 indices[2];
}

[CCode (cname = "ufbx_edge_list", destroy_function = "", has_type_id = false)]
public struct EdgeList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public Edge[] data;
}

[SimpleType]
[CCode (cname = "ufbx_face", destroy_function = "", has_type_id = false)]
public struct Face
{
	public uint32 index_begin;
	public uint32 num_indices;
}

[CCode (cname = "ufbx_face_list", destroy_function = "", has_type_id = false)]
public struct FaceList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public Face[] data;
}

[SimpleType]
[CCode (cname = "ufbx_mesh_part", destroy_function = "", has_type_id = false)]
public struct MeshPart
{
	public uint32 index;
	public size_t num_faces;
	public size_t num_triangles;
	public size_t num_empty_faces;
	public size_t num_point_faces;
	public size_t num_line_faces;
	public Uint32List face_indices;
}

[CCode (cname = "ufbx_mesh_part_list", destroy_function = "", has_type_id = false)]
public struct MeshPartList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public MeshPart[] data;
}

[Compact]
[CCode (cname = "ufbx_face_group", destroy_function = "", has_type_id = false)]
public struct FaceGroup
{
	public int32 id;
	public String name;
}

[CCode (cname = "ufbx_face_group_list", destroy_function = "", has_type_id = false)]
public struct FaceGroupList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public FaceGroup[] data;
}

[SimpleType]
[CCode (cname = "ufbx_subdivision_weight_range", destroy_function = "", has_type_id = false)]
public struct SubdivisionWeightRange
{
	public uint32 weight_begin;
	public uint32 num_weights;
}

[CCode (cname = "ufbx_subdivision_weight_range_list", destroy_function = "", has_type_id = false)]
public struct SubdivisionWeightRangeList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public SubdivisionWeightRange[] data;
}

[SimpleType]
[CCode (cname = "ufbx_subdivision_weight", destroy_function = "", has_type_id = false)]
public struct SubdivisionWeight
{
	public Real weight;
	public uint32 index;
}

[CCode (cname = "ufbx_subdivision_weight_list", destroy_function = "", has_type_id = false)]
public struct SubdivisionWeightList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public SubdivisionWeight[] data;
}

[CCode (cname = "ufbx_subdivision_result", destroy_function = "", has_type_id = false)]
public struct SubdivisionResult
{
	public size_t result_memory_used;
	public size_t temp_memory_used;
	public size_t result_allocs;
	public size_t temp_allocs;
	public SubdivisionWeightRangeList source_vertex_ranges;
	public SubdivisionWeightList source_vertex_weights;
	public SubdivisionWeightRangeList skin_cluster_ranges;
	public SubdivisionWeightList skin_cluster_weights;
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

[Compact]
[CCode (cname = "ufbx_mesh", destroy_function = "", has_type_id = false)]
public class Mesh
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	[CCode (cname = "instances")]
	public NodeList instances;
	public size_t num_vertices;
	public size_t num_indices;
	public size_t num_faces;
	public size_t num_triangles;
	public size_t num_edges;
	public size_t max_face_triangles;
	public size_t num_empty_faces;
	public size_t num_point_faces;
	public size_t num_line_faces;
	public FaceList faces;
	public BoolList face_smoothing;
	public Uint32List face_material;
	public Uint32List face_group;
	public BoolList face_hole;
	public EdgeList edges;
	public BoolList edge_smoothing;
	public RealList edge_crease;
	public BoolList edge_visibility;
	public Uint32List vertex_indices;
	public Vec3List vertices;
	public Uint32List vertex_first_index;
	public VertexVec3 vertex_position;
	public VertexVec3 vertex_normal;
	public VertexVec2 vertex_uv;
	public VertexVec3 vertex_tangent;
	public VertexVec3 vertex_bitangent;
	public VertexVec4 vertex_color;
	public VertexReal vertex_crease;
	public UvSetList uv_sets;
	public ColorSetList color_sets;
	public MaterialList materials;
	public FaceGroupList face_groups;
	public MeshPartList material_parts;
	public MeshPartList face_group_parts;
	public Uint32List material_part_usage_order;
	public bool skinned_is_local;
	public VertexVec3 skinned_position;
	public VertexVec3 skinned_normal;
	public SkinDeformerList skin_deformers;
	public BlendDeformerList blend_deformers;
	public CacheDeformerList cache_deformers;
	public ElementList all_deformers;
	public uint32 subdivision_preview_levels;
	public uint32 subdivision_render_levels;
	public SubdivisionDisplayMode subdivision_display_mode;
	public SubdivisionBoundary subdivision_boundary;
	public SubdivisionBoundary subdivision_uv_boundary;
	public bool reversed_winding;
	public bool generated_normals;
	public bool subdivision_evaluated;
	public unowned SubdivisionResult? subdivision_result;
	public bool from_tessellated_nurbs;
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

[Compact]
[CCode (cname = "ufbx_light", destroy_function = "", has_type_id = false)]
public class Light
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	[CCode (cname = "instances")]
	public NodeList instances;
	public Vec3 color;
	public Real intensity;
	public Vec3 local_direction;
	public LightType type;
	public LightDecay decay;
	public LightAreaShape area_shape;
	public Real inner_angle;
	public Real outer_angle;
	public bool cast_light;
	public bool cast_shadows;
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
	public CoordinateAxis right;
	public CoordinateAxis up;
	public CoordinateAxis front;

	[CCode (cname = "ufbx_axes_right_handed_y_up")]
	public const CoordinateAxes RIGHT_HANDED_Y_UP;
	[CCode (cname = "ufbx_axes_right_handed_z_up")]
	public const CoordinateAxes RIGHT_HANDED_Z_UP;
	[CCode (cname = "ufbx_axes_left_handed_y_up")]
	public const CoordinateAxes LEFT_HANDED_Y_UP;
	[CCode (cname = "ufbx_axes_left_handed_z_up")]
	public const CoordinateAxes LEFT_HANDED_Z_UP;
}

[Compact]
[CCode (cname = "ufbx_camera", destroy_function = "", has_type_id = false)]
public class Camera
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	[CCode (cname = "instances")]
	public NodeList instances;
	public ProjectionMode projection_mode;
	public bool resolution_is_pixels;
	public Vec2 resolution;
	public Vec2 field_of_view_deg;
	public Vec2 field_of_view_tan;
	public Real orthographic_extent;
	public Vec2 orthographic_size;
	public Vec2 projection_plane;
	public Real aspect_ratio;
	public Real near_plane;
	public Real far_plane;
	public CoordinateAxes projection_axes;
	public AspectMode aspect_mode;
	public ApertureMode aperture_mode;
	public GateFit gate_fit;
	public ApertureFormat aperture_format;
	public Real focal_length_mm;
	public Vec2 film_size_inch;
	public Vec2 aperture_size_inch;
	public Real squeeze_ratio;
}

[Compact]
[CCode (cname = "ufbx_bone", destroy_function = "", has_type_id = false)]
public class Bone
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	[CCode (cname = "instances")]
	public NodeList instances;
	public Real radius;
	public Real relative_length;
	public bool is_root;
}

[Compact]
[CCode (cname = "ufbx_empty", destroy_function = "", has_type_id = false)]
public class Empty
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	[CCode (cname = "instances")]
	public NodeList instances;
}

[SimpleType]
[CCode (cname = "ufbx_line_segment", destroy_function = "", has_type_id = false)]
public struct LineSegment
{
	public uint32 index_begin;
	public uint32 num_indices;
}

[CCode (cname = "ufbx_line_segment_list", destroy_function = "", has_type_id = false)]
public struct LineSegmentList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public LineSegment[] data;
}

[Compact]
[CCode (cname = "ufbx_line_curve", destroy_function = "", has_type_id = false)]
public class LineCurve
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	[CCode (cname = "instances")]
	public NodeList instances;
	public Vec3 color;
	public Vec3List control_points;
	public Uint32List point_indices;
	public LineSegmentList segments;
	public bool from_tessellated_nurbs;
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
	public uint32 order;
	public NurbsTopology topology;
	public RealList knot_vector;
	public Real t_min;
	public Real t_max;
	public RealList spans;
	public bool is_2d;
	public size_t num_wrap_control_points;
	public bool valid;
}

[Compact]
[CCode (cname = "ufbx_nurbs_curve", destroy_function = "", has_type_id = false)]
public class NurbsCurve
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	[CCode (cname = "instances")]
	public NodeList instances;
	public NurbsBasis basis;
	public Vec4List control_points;
}

[Compact]
[CCode (cname = "ufbx_nurbs_surface", destroy_function = "", has_type_id = false)]
public class NurbsSurface
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	[CCode (cname = "instances")]
	public NodeList instances;
	public NurbsBasis basis_u;
	public NurbsBasis basis_v;
	public size_t num_control_points_u;
	public size_t num_control_points_v;
	public Vec4List control_points;
	public uint32 span_subdivision_u;
	public uint32 span_subdivision_v;
	public bool flip_normals;
	public unowned Material? material;
}

[Compact]
[CCode (cname = "ufbx_nurbs_trim_surface", destroy_function = "", has_type_id = false)]
public class NurbsTrimSurface
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	[CCode (cname = "instances")]
	public NodeList instances;
}

[Compact]
[CCode (cname = "ufbx_nurbs_trim_boundary", destroy_function = "", has_type_id = false)]
public class NurbsTrimBoundary
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	[CCode (cname = "instances")]
	public NodeList instances;
}

[Compact]
[CCode (cname = "ufbx_procedural_geometry", destroy_function = "", has_type_id = false)]
public class ProceduralGeometry
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	[CCode (cname = "instances")]
	public NodeList instances;
}

[Compact]
[CCode (cname = "ufbx_stereo_camera", destroy_function = "", has_type_id = false)]
public class StereoCamera
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	[CCode (cname = "instances")]
	public NodeList instances;
	public unowned Camera? left;
	public unowned Camera? right;
}

[Compact]
[CCode (cname = "ufbx_camera_switcher", destroy_function = "", has_type_id = false)]
public class CameraSwitcher
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	[CCode (cname = "instances")]
	public NodeList instances;
}

[CCode (cname = "ufbx_marker_type", cprefix = "UFBX_MARKER_", has_type_id = false)]
public enum MarkerType
{
	UNKNOWN,
	FK_EFFECTOR,
	IK_EFFECTOR,
	TYPE_COUNT
}

[Compact]
[CCode (cname = "ufbx_marker", destroy_function = "", has_type_id = false)]
public class Marker
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	[CCode (cname = "instances")]
	public NodeList instances;
	public MarkerType type;
}

[CCode (cname = "ufbx_lod_display", cprefix = "UFBX_LOD_DISPLAY_", has_type_id = false)]
public enum LodDisplay
{
	USE_LOD,
	SHOW,
	HIDE,
	COUNT
}

[SimpleType]
[CCode (cname = "ufbx_lod_level", destroy_function = "", has_type_id = false)]
public struct LodLevel
{
	public Real distance;
	public LodDisplay display;
}

[CCode (cname = "ufbx_lod_level_list", destroy_function = "", has_type_id = false)]
public struct LodLevelList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public LodLevel[] data;
}

[Compact]
[CCode (cname = "ufbx_lod_group", destroy_function = "", has_type_id = false)]
public class LodGroup
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	[CCode (cname = "instances")]
	public NodeList instances;
	public bool relative_distances;
	public LodLevelList lod_levels;
	public bool ignore_parent_transform;
	public bool use_distance_limit;
	public Real distance_limit_min;
	public Real distance_limit_max;
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

[SimpleType]
[CCode (cname = "ufbx_skin_vertex", destroy_function = "", has_type_id = false)]
public struct SkinVertex
{
	public uint32 weight_begin;
	public uint32 num_weights;
	public Real dq_weight;
}

[CCode (cname = "ufbx_skin_vertex_list", destroy_function = "", has_type_id = false)]
public struct SkinVertexList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public SkinVertex[] data;
}

[SimpleType]
[CCode (cname = "ufbx_skin_weight", destroy_function = "", has_type_id = false)]
public struct SkinWeight
{
	public uint32 cluster_index;
	public Real weight;
}

[CCode (cname = "ufbx_skin_weight_list", destroy_function = "", has_type_id = false)]
public struct SkinWeightList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public SkinWeight[] data;
}

[Compact]
[CCode (cname = "ufbx_skin_deformer", destroy_function = "", has_type_id = false)]
public class SkinDeformer
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	public SkinningMethod skinning_method;
	public SkinClusterList clusters;
	public SkinVertexList vertices;
	public SkinWeightList weights;
	public size_t max_weights_per_vertex;
	public size_t num_dq_weights;
	public Uint32List dq_vertices;
	public RealList dq_weights;
}

[Compact]
[CCode (cname = "ufbx_skin_cluster", destroy_function = "", has_type_id = false)]
public class SkinCluster
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	public unowned Node? bone_node;
	public Matrix geometry_to_bone;
	public Matrix mesh_node_to_bone;
	public Matrix bind_to_world;
	public Matrix geometry_to_world;
	public Transform geometry_to_world_transform;
	public size_t num_weights;
	public Uint32List vertices;
	public RealList weights;
}

[Compact]
[CCode (cname = "ufbx_blend_deformer", destroy_function = "", has_type_id = false)]
public class BlendDeformer
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	public BlendChannelList channels;
}

[SimpleType]
[CCode (cname = "ufbx_blend_keyframe", destroy_function = "", has_type_id = false)]
public struct BlendKeyframe
{
	public unowned BlendShape shape;
	public Real target_weight;
	public Real effective_weight;
}

[CCode (cname = "ufbx_blend_keyframe_list", destroy_function = "", has_type_id = false)]
public struct BlendKeyframeList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public BlendKeyframe[] data;
}

[Compact]
[CCode (cname = "ufbx_blend_channel", destroy_function = "", has_type_id = false)]
public class BlendChannel
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	public Real weight;
	public BlendKeyframeList keyframes;
	public unowned BlendShape? target_shape;
}

[Compact]
[CCode (cname = "ufbx_blend_shape", destroy_function = "", has_type_id = false)]
public class BlendShape
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	public size_t num_offsets;
	public Uint32List offset_vertices;
	public Vec3List position_offsets;
	public Vec3List normal_offsets;
	public RealList offset_weights;
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

[SimpleType]
[CCode (cname = "ufbx_cache_frame", destroy_function = "", has_type_id = false)]
public struct CacheFrame
{
	public String channel;
	public double time;
	public String filename;
	public CacheFileFormat file_format;
	public MirrorAxis mirror_axis;
	public Real scale_factor;
	public CacheDataFormat data_format;
	public CacheDataEncoding data_encoding;
	public uint64 data_offset;
	public uint32 data_count;
	public uint32 data_element_bytes;
	public uint64 data_total_bytes;
}

[CCode (cname = "ufbx_cache_frame", destroy_function = "", has_type_id = false)]
public struct CacheFrameList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public CacheFrame[] data;
}

[SimpleType]
[CCode (cname = "ufbx_cache_channel", destroy_function = "", has_type_id = false)]
public struct CacheChannel
{
	public String name;
	public CacheInterpretation interpretation;
	public String interpretation_name;
	public CacheFrameList frames;
	public MirrorAxis mirror_axis;
	public Real scale_factor;
}

[CCode (cname = "ufbx_cache_channel_list", destroy_function = "", has_type_id = false)]
public struct CacheChannelList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public CacheChannel[] data;
}

[CCode (cname = "ufbx_geometry_cache", destroy_function = "", has_type_id = false)]
public struct GeometryCache
{
	public String root_filename;
	public CacheChannelList channels;
	public CacheFrameList frames;
	public StringList extra_info;
}

[Compact]
[CCode (cname = "ufbx_cache_deformer", destroy_function = "", has_type_id = false)]
public class CacheDeformer
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	public String channel;
	public unowned CacheFile? file;
	public unowned GeometryCache? external_cache;
	public unowned CacheChannel? external_channel;
}

[Compact]
[CCode (cname = "ufbx_cache_file", destroy_function = "", has_type_id = false)]
public class CacheFile
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	public String filename;
	public String absolute_filename;
	public String relative_filename;
	public Blob raw_filename;
	public Blob raw_absolute_filename;
	public Blob raw_relative_filename;
	public CacheFileFormat format;
	public unowned GeometryCache? external_cache;
}

[CCode (cname = "ufbx_material_map", destroy_function = "", has_type_id = false)]
public struct MaterialMap
{
	[CCode (cname = "value_real")]
	public Real value_real;
	[CCode (cname = "value_vec2")]
	public Vec2 value_vec2;
	[CCode (cname = "value_vec3")]
	public Vec3 value_vec3;
	[CCode (cname = "value_vec4")]
	public Vec4 value_vec4;
	public int64 value_int;
	public unowned Texture? texture;
	public bool has_value;
	public bool texture_enabled;
	public bool feature_disabled;
	public uint8 value_components;
}

[CCode (cname = "ufbx_material_feature_info", destroy_function = "", has_type_id = false)]
public struct MaterialFeatureInfo
{
	public bool enabled;
	public bool is_explicit;
}

[SimpleType]
[CCode (cname = "ufbx_material_texture", destroy_function = "", has_type_id = false)]
public struct MaterialTexture
{
	public String material_prop;
	public String shader_prop;
	public unowned Texture texture;
}

[CCode (cname = "ufbx_material_texture_list", destroy_function = "", has_type_id = false)]
public struct MaterialTextureList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public MaterialTexture[] data;
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
	OPENPBR_MATERIAL,
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
	THIN_FILM_FACTOR,
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

[CCode (cname = "ufbx_extrapolation_mode", cprefix = "UFBX_EXTRAPOLATION_", has_type_id = false)]
public enum ExtrapolationMode
{
	CONSTANT,
	REPEAT,
	MIRROR,
	SLOPE,
	REPEAT_RELATIVE,
	COUNT
}

[SimpleType]
[CCode (cname = "ufbx_extrapolation", destroy_function = "", has_type_id = false)]
public struct Extrapolation
{
	public ExtrapolationMode mode;
	public int32 repeat_count;
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
	public MaterialMap maps[MaterialFbxMap.MAP_COUNT];
	[CCode (cname = "diffuse_factor")]
	public MaterialMap diffuse_factor;
	[CCode (cname = "diffuse_color")]
	public MaterialMap diffuse_color;
	[CCode (cname = "specular_factor")]
	public MaterialMap specular_factor;
	[CCode (cname = "specular_color")]
	public MaterialMap specular_color;
	[CCode (cname = "specular_exponent")]
	public MaterialMap specular_exponent;
	[CCode (cname = "reflection_factor")]
	public MaterialMap reflection_factor;
	[CCode (cname = "reflection_color")]
	public MaterialMap reflection_color;
	[CCode (cname = "transparency_factor")]
	public MaterialMap transparency_factor;
	[CCode (cname = "transparency_color")]
	public MaterialMap transparency_color;
	[CCode (cname = "emission_factor")]
	public MaterialMap emission_factor;
	[CCode (cname = "emission_color")]
	public MaterialMap emission_color;
	[CCode (cname = "ambient_factor")]
	public MaterialMap ambient_factor;
	[CCode (cname = "ambient_color")]
	public MaterialMap ambient_color;
	[CCode (cname = "normal_map")]
	public MaterialMap normal_map;
	[CCode (cname = "bump")]
	public MaterialMap bump;
	[CCode (cname = "bump_factor")]
	public MaterialMap bump_factor;
	[CCode (cname = "displacement_factor")]
	public MaterialMap displacement_factor;
	[CCode (cname = "displacement")]
	public MaterialMap displacement;
	[CCode (cname = "vector_displacement_factor")]
	public MaterialMap vector_displacement_factor;
	[CCode (cname = "vector_displacement")]
	public MaterialMap vector_displacement;
}

[CCode (cname = "ufbx_material_pbr_maps", destroy_function = "", has_type_id = false)]
public struct MaterialPbrMaps
{
	[CCode (cname = "maps")]
	public MaterialMap maps[MaterialPbrMap.MAP_COUNT];
	[CCode (cname = "base_factor")]
	public MaterialMap base_factor;
	[CCode (cname = "base_color")]
	public MaterialMap base_color;
	[CCode (cname = "roughness")]
	public MaterialMap roughness;
	[CCode (cname = "metalness")]
	public MaterialMap metalness;
	[CCode (cname = "diffuse_roughness")]
	public MaterialMap diffuse_roughness;
	[CCode (cname = "specular_factor")]
	public MaterialMap specular_factor;
	[CCode (cname = "specular_color")]
	public MaterialMap specular_color;
	[CCode (cname = "specular_ior")]
	public MaterialMap specular_ior;
	[CCode (cname = "specular_anisotropy")]
	public MaterialMap specular_anisotropy;
	[CCode (cname = "specular_rotation")]
	public MaterialMap specular_rotation;
	[CCode (cname = "transmission_factor")]
	public MaterialMap transmission_factor;
	[CCode (cname = "transmission_color")]
	public MaterialMap transmission_color;
	[CCode (cname = "transmission_depth")]
	public MaterialMap transmission_depth;
	[CCode (cname = "transmission_scatter")]
	public MaterialMap transmission_scatter;
	[CCode (cname = "transmission_scatter_anisotropy")]
	public MaterialMap transmission_scatter_anisotropy;
	[CCode (cname = "transmission_dispersion")]
	public MaterialMap transmission_dispersion;
	[CCode (cname = "transmission_roughness")]
	public MaterialMap transmission_roughness;
	[CCode (cname = "transmission_extra_roughness")]
	public MaterialMap transmission_extra_roughness;
	[CCode (cname = "transmission_priority")]
	public MaterialMap transmission_priority;
	[CCode (cname = "transmission_enable_in_aov")]
	public MaterialMap transmission_enable_in_aov;
	[CCode (cname = "subsurface_factor")]
	public MaterialMap subsurface_factor;
	[CCode (cname = "subsurface_color")]
	public MaterialMap subsurface_color;
	[CCode (cname = "subsurface_radius")]
	public MaterialMap subsurface_radius;
	[CCode (cname = "subsurface_scale")]
	public MaterialMap subsurface_scale;
	[CCode (cname = "subsurface_anisotropy")]
	public MaterialMap subsurface_anisotropy;
	[CCode (cname = "subsurface_tint_color")]
	public MaterialMap subsurface_tint_color;
	[CCode (cname = "subsurface_type")]
	public MaterialMap subsurface_type;
	[CCode (cname = "sheen_factor")]
	public MaterialMap sheen_factor;
	[CCode (cname = "sheen_color")]
	public MaterialMap sheen_color;
	[CCode (cname = "sheen_roughness")]
	public MaterialMap sheen_roughness;
	[CCode (cname = "coat_factor")]
	public MaterialMap coat_factor;
	[CCode (cname = "coat_color")]
	public MaterialMap coat_color;
	[CCode (cname = "coat_roughness")]
	public MaterialMap coat_roughness;
	[CCode (cname = "coat_ior")]
	public MaterialMap coat_ior;
	[CCode (cname = "coat_anisotropy")]
	public MaterialMap coat_anisotropy;
	[CCode (cname = "coat_rotation")]
	public MaterialMap coat_rotation;
	[CCode (cname = "coat_normal")]
	public MaterialMap coat_normal;
	[CCode (cname = "coat_affect_base_color")]
	public MaterialMap coat_affect_base_color;
	[CCode (cname = "coat_affect_base_roughness")]
	public MaterialMap coat_affect_base_roughness;
	[CCode (cname = "thin_film_thickness")]
	public MaterialMap thin_film_thickness;
	[CCode (cname = "thin_film_ior")]
	public MaterialMap thin_film_ior;
	[CCode (cname = "emission_factor")]
	public MaterialMap emission_factor;
	[CCode (cname = "emission_color")]
	public MaterialMap emission_color;
	[CCode (cname = "opacity")]
	public MaterialMap opacity;
	[CCode (cname = "indirect_diffuse")]
	public MaterialMap indirect_diffuse;
	[CCode (cname = "indirect_specular")]
	public MaterialMap indirect_specular;
	[CCode (cname = "normal_map")]
	public MaterialMap normal_map;
	[CCode (cname = "tangent_map")]
	public MaterialMap tangent_map;
	[CCode (cname = "displacement_map")]
	public MaterialMap displacement_map;
	[CCode (cname = "matte_factor")]
	public MaterialMap matte_factor;
	[CCode (cname = "matte_color")]
	public MaterialMap matte_color;
	[CCode (cname = "ambient_occlusion")]
	public MaterialMap ambient_occlusion;
	[CCode (cname = "glossiness")]
	public MaterialMap glossiness;
	[CCode (cname = "coat_glossiness")]
	public MaterialMap coat_glossiness;
	[CCode (cname = "transmission_glossiness")]
	public MaterialMap transmission_glossiness;
}

[CCode (cname = "ufbx_material_features", destroy_function = "", has_type_id = false)]
public struct MaterialFeatures
{
	[CCode (cname = "features")]
	public MaterialFeatureInfo features[MaterialFeature.COUNT];
	[CCode (cname = "pbr")]
	public MaterialFeatureInfo pbr;
	[CCode (cname = "metalness")]
	public MaterialFeatureInfo metalness;
	[CCode (cname = "diffuse")]
	public MaterialFeatureInfo diffuse;
	[CCode (cname = "specular")]
	public MaterialFeatureInfo specular;
	[CCode (cname = "emission")]
	public MaterialFeatureInfo emission;
	[CCode (cname = "transmission")]
	public MaterialFeatureInfo transmission;
	[CCode (cname = "coat")]
	public MaterialFeatureInfo coat;
	[CCode (cname = "sheen")]
	public MaterialFeatureInfo sheen;
	[CCode (cname = "opacity")]
	public MaterialFeatureInfo opacity;
	[CCode (cname = "ambient_occlusion")]
	public MaterialFeatureInfo ambient_occlusion;
	[CCode (cname = "matte")]
	public MaterialFeatureInfo matte;
	[CCode (cname = "unlit")]
	public MaterialFeatureInfo unlit;
	[CCode (cname = "ior")]
	public MaterialFeatureInfo ior;
	[CCode (cname = "diffuse_roughness")]
	public MaterialFeatureInfo diffuse_roughness;
	[CCode (cname = "transmission_roughness")]
	public MaterialFeatureInfo transmission_roughness;
	[CCode (cname = "thin_walled")]
	public MaterialFeatureInfo thin_walled;
	[CCode (cname = "caustics")]
	public MaterialFeatureInfo caustics;
	[CCode (cname = "exit_to_background")]
	public MaterialFeatureInfo exit_to_background;
	[CCode (cname = "internal_reflections")]
	public MaterialFeatureInfo internal_reflections;
	[CCode (cname = "double_sided")]
	public MaterialFeatureInfo double_sided;
	[CCode (cname = "roughness_as_glossiness")]
	public MaterialFeatureInfo roughness_as_glossiness;
	[CCode (cname = "coat_roughness_as_glossiness")]
	public MaterialFeatureInfo coat_roughness_as_glossiness;
	[CCode (cname = "transmission_roughness_as_glossiness")]
	public MaterialFeatureInfo transmission_roughness_as_glossiness;
}

[Compact]
[CCode (cname = "ufbx_material", destroy_function = "", has_type_id = false)]
public class Material
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	public MaterialFbxMaps fbx;
	public MaterialPbrMaps pbr;
	public MaterialFeatures features;
	public ShaderType shader_type;
	public unowned Shader? shader;
	public String shading_model_name;
	public String shader_prop_prefix;
	public MaterialTextureList textures;
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

[SimpleType]
[CCode (cname = "ufbx_texture_layer", destroy_function = "", has_type_id = false)]
public struct TextureLayer
{
	public unowned Texture texture;
	public BlendMode blend_mode;
	public Real alpha;
}

[CCode (cname = "ufbx_texture_layer", destroy_function = "", has_type_id = false)]
public struct TextureLayerList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public TextureLayer[] data;
}

[CCode (cname = "ufbx_shader_texture_type", cprefix = "UFBX_SHADER_TEXTURE_", has_type_id = false)]
public enum ShaderTextureType
{
	UNKNOWN,
	SELECT_OUTPUT,
	OSL,
	TYPE_COUNT
}

[SimpleType]
[CCode (cname = "ufbx_shader_texture_input", destroy_function = "", has_type_id = false)]
public struct ShaderTextureInput
{
	public String name;
	[CCode (cname = "value_real")]
	public Real value_real;
	[CCode (cname = "value_vec2")]
	public Vec2 value_vec2;
	[CCode (cname = "value_vec3")]
	public Vec3 value_vec3;
	[CCode (cname = "value_vec4")]
	public Vec4 value_vec4;
	public int64 value_int;
	public String value_str;
	public Blob value_blob;
	public unowned Texture? texture;
	public int64 texture_output_index;
	public bool texture_enabled;
	public unowned Prop prop;
	public unowned Prop? texture_prop;
	public unowned Prop? texture_enabled_prop;
}

[CCode (cname = "ufbx_shader_texture_input_list", destroy_function = "", has_type_id = false)]
public struct ShaderTextureInputList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public ShaderTextureInput[] data;
}

[CCode (cname = "ufbx_shader_texture", destroy_function = "", has_type_id = false)]
public struct ShaderTexture
{
	public ShaderTextureType type;
	public String shader_name;
	public uint64 shader_type_id;
	public ShaderTextureInputList inputs;
	public String shader_source;
	public Blob raw_shader_source;
	public unowned Texture main_texture;
	public int64 main_texture_output_index;
	public String prop_prefix;
}

[SimpleType]
[CCode (cname = "ufbx_texture_file", destroy_function = "", has_type_id = false)]
public struct TextureFile
{
	public uint32 index;
	public String filename;
	public String absolute_filename;
	public String relative_filename;
	public Blob raw_filename;
	public Blob raw_absolute_filename;
	public Blob raw_relative_filename;
	public Blob content;
}

[CCode (cname = "ufbx_texture_file_list", destroy_function = "", has_type_id = false)]
public struct TextureFileList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public TextureFile[] data;
}

[Compact]
[CCode (cname = "ufbx_texture", destroy_function = "", has_type_id = false)]
public class Texture
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	public TextureType type;
	public String filename;
	public String absolute_filename;
	public String relative_filename;
	public Blob raw_filename;
	public Blob raw_absolute_filename;
	public Blob raw_relative_filename;
	public Blob content;
	public unowned Video? video;
	public uint32 file_index;
	public bool has_file;
	public TextureLayerList layers;
	public unowned ShaderTexture? shader;
	public TextureList file_textures;
	public String uv_set;
	public WrapMode wrap_u;
	public WrapMode wrap_v;
	public bool has_uv_transform;
	public Transform uv_transform;
	public Matrix texture_to_uv;
	public Matrix uv_to_texture;
}

[Compact]
[CCode (cname = "ufbx_video", destroy_function = "", has_type_id = false)]
public class Video
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	public String filename;
	public String absolute_filename;
	public String relative_filename;
	public Blob raw_filename;
	public Blob raw_absolute_filename;
	public Blob raw_relative_filename;
	public Blob content;
}

[Compact]
[CCode (cname = "ufbx_shader", destroy_function = "", has_type_id = false)]
public class Shader
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	public ShaderType type;
	public ShaderBindingList bindings;
}

[SimpleType]
[CCode (cname = "ufbx_shader_prop_binding", destroy_function = "", has_type_id = false)]
public struct ShaderPropBinding
{
	public String shader_prop;
	public String material_prop;
}

[CCode (cname = "ufbx_shader_prop_binding_list", destroy_function = "", has_type_id = false)]
public struct ShaderPropBindingList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public ShaderPropBinding[] data;
}

[Compact]
[CCode (cname = "ufbx_shader_binding", destroy_function = "", has_type_id = false)]
public class ShaderBinding
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	public ShaderPropBindingList prop_bindings;
}

[SimpleType]
[CCode (cname = "ufbx_prop_override", destroy_function = "", has_type_id = false)]
public struct PropOverride
{
	public uint32 element_id;
	public uint32 _internal_key;
	public String prop_name;
	public Vec4 value;
	public String value_str;
	public int64 value_int;
}

[CCode (cname = "ufbx_prop_override_list", destroy_function = "", has_type_id = false)]
public struct PropOverrideList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public PropOverride[] data;
}

[SimpleType]
[CCode (cname = "ufbx_transform_override", destroy_function = "", has_type_id = false)]
public struct TransformOverride
{
	public uint32 node_id;
	public Transform transform;
}

[CCode (cname = "ufbx_transform_override_list", destroy_function = "", has_type_id = false)]
public struct TransformOverrideList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public TransformOverride[] data;
}

[Compact]
[CCode (cname = "ufbx_anim", destroy_function = "", has_type_id = false)]
public class Anim
{
	public double time_begin;
	public double time_end;
	public AnimLayerList layers;
	public RealList override_layer_weights;
	public PropOverrideList prop_overrides;
	public TransformOverrideList transform_overrides;
	public bool ignore_connections;
	public bool custom;
}

[Compact]
[CCode (cname = "ufbx_anim_stack", destroy_function = "", has_type_id = false)]
public class AnimStack
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	public double time_begin;
	public double time_end;
	public AnimLayerList layers;
	public unowned Anim anim;
}

[SimpleType]
[CCode (cname = "ufbx_anim_prop", destroy_function = "", has_type_id = false)]
public struct AnimProp
{
	public unowned Element element;
	public uint32 _internal_key;
	public String prop_name;
	public unowned AnimValue anim_value;
}

[CCode (cname = "ufbx_anim_list", destroy_function = "", has_type_id = false)]
public struct AnimPropList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public AnimProp[] data;
}

[Compact]
[CCode (cname = "ufbx_anim_layer", destroy_function = "", has_type_id = false)]
public class AnimLayer
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	public Real weight;
	public bool weight_is_animated;
	public bool blended;
	public bool additive;
	public bool compose_rotation;
	public bool compose_scale;
	public AnimValueList anim_values;
	public AnimPropList anim_props;
	public unowned Anim anim;
	public uint32 _min_element_id;
	public uint32 _max_element_id;
	public uint32 _element_id_bitmask[4];
}

[Compact]
[CCode (cname = "ufbx_anim_value", destroy_function = "", has_type_id = false)]
public class AnimValue
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	public Vec3 default_value;
	public unowned AnimCurve? curves[3];
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
	public float dx;
	public float dy;
}

[SimpleType]
[CCode (cname = "ufbx_keyframe", destroy_function = "", has_type_id = false)]
public struct Keyframe
{
	public double time;
	public Real value;
	public Interpolation interpolation;
	public Tangent left;
	public Tangent right;
}

[CCode (cname = "ufbx_keyframe_list", destroy_function = "", has_type_id = false)]
public struct KeyframeList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public Keyframe[] data;
}

[Compact]
[CCode (cname = "ufbx_anim_curve", destroy_function = "", has_type_id = false)]
public class AnimCurve
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	public KeyframeList keyframes;
	public Extrapolation pre_extrapolation;
	public Extrapolation post_extrapolation;
	public Real min_value;
	public Real max_value;
	public double min_time;
	public double max_time;
}

[Compact]
[CCode (cname = "ufbx_display_layer", destroy_function = "", has_type_id = false)]
public class DisplayLayer
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	public NodeList nodes;
	public bool visible;
	public bool frozen;
	public Vec3 ui_color;
}

[Compact]
[CCode (cname = "ufbx_selection_set", destroy_function = "", has_type_id = false)]
public class SelectionSet
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	public SelectionNodeList nodes;
}

[Compact]
[CCode (cname = "ufbx_selection_node", destroy_function = "", has_type_id = false)]
public class SelectionNode
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	public unowned Node? target_node;
	public unowned Mesh? target_mesh;
	public bool include_node;
	public Uint32List vertices;
	public Uint32List edges;
	public Uint32List faces;
}

[Compact]
[CCode (cname = "ufbx_character", destroy_function = "", has_type_id = false)]
public class Character
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
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

[SimpleType]
[CCode (cname = "ufbx_constraint_target", destroy_function = "", has_type_id = false)]
public struct ConstraintTarget
{
	public unowned Node node;
	public Real weight;
	public Transform transform;
}

[CCode (cname = "ufbx_constraint_target_list", destroy_function = "", has_type_id = false)]
public struct ConstraintTargetList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public ConstraintTarget[] data;
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

[Compact]
[CCode (cname = "ufbx_constraint", destroy_function = "", has_type_id = false)]
public class Constraint
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	public ConstraintType type;
	public String type_name;
	public unowned Node? node;
	public ConstraintTargetList targets;
	public Real weight;
	public bool active;
	public bool constrain_translation[3];
	public bool constrain_rotation[3];
	public bool constrain_scale[3];
	public Transform transform_offset;
	public Vec3 aim_vector;
	public ConstraintAimUpType aim_up_type;
	public unowned Node? aim_up_node;
	public Vec3 aim_up_vector;
	public unowned Node? ik_effector;
	public unowned Node? ik_end_node;
	public Vec3 ik_pole_vector;
}

[Compact]
[CCode (cname = "ufbx_audio_layer", destroy_function = "", has_type_id = false)]
public class AudioLayer
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	public AudioClipList clips;
}

[Compact]
[CCode (cname = "ufbx_audio_clip", destroy_function = "", has_type_id = false)]
public class AudioClip
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	public String filename;
	public String absolute_filename;
	public String relative_filename;
	public Blob raw_filename;
	public Blob raw_absolute_filename;
	public Blob raw_relative_filename;
	public Blob content;
}

[SimpleType]
[CCode (cname = "ufbx_bone_pose", destroy_function = "", has_type_id = false)]
public struct BonePose
{
	public unowned Node bone_node;
	public Matrix bone_to_world;
	public Matrix bone_to_parent;
}

[CCode (cname = "ufbx_bone_pose_list", destroy_function = "", has_type_id = false)]
public struct BonePoseList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public BonePose[] data;
}

[Compact]
[CCode (cname = "ufbx_pose", destroy_function = "", has_type_id = false)]
public class Pose
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
	public bool is_bind_pose;
	public BonePoseList bone_poses;
}

[Compact]
[CCode (cname = "ufbx_metadata_object", destroy_function = "", has_type_id = false)]
public class MetadataObject
{
	[CCode (cname = "element")]
	public Element element;
	[CCode (cname = "name")]
	public String name;
	[CCode (cname = "props")]
	public Props props;
	[CCode (cname = "element_id")]
	public uint32 element_id;
	[CCode (cname = "typed_id")]
	public uint32 typed_id;
}

[SimpleType]
[CCode (cname = "ufbx_name_element", destroy_function = "", has_type_id = false)]
public struct NameElement
{
	public String name;
	public ElementType type;
	public uint32 _internal_key;
	public unowned Element element;
}

[CCode (cname = "ufbx_name_element_list", destroy_function = "", has_type_id = false)]
public struct NameElementList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public NameElement[] data;
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
	public String vendor;
	public String name;
	public String version;
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
	UNSUPPORTED_VERSION,
	INDEX_CLAMPED,
	BAD_UNICODE,
	BAD_BASE64_CONTENT,
	BAD_ELEMENT_CONNECTED_TO_ROOT,
	DUPLICATE_OBJECT_ID,
	EMPTY_FACE_REMOVED,
	UNKNOWN_OBJ_DIRECTIVE,
	TYPE_COUNT,
	TYPE_FIRST_DEDUPLICATED = WarningType.INDEX_CLAMPED
}

[SimpleType]
[CCode (cname = "ufbx_warning", destroy_function = "", has_type_id = false)]
public struct Warning
{
	public WarningType type;
	public String description;
	public uint32 element_id;
	public size_t count;
}

[CCode (cname = "ufbx_warning_list", destroy_function = "", has_type_id = false)]
public struct WarningList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public Warning[] data;
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
	ADJUST_TO_ROTATION_PIVOT,
	COUNT
}

[CCode (cname = "ufbx_thumbnail", destroy_function = "", has_type_id = false)]
public struct Thumbnail
{
	public Props props;
	public uint32 width;
	public uint32 height;
	public ThumbnailFormat format;
	public Blob data;
}

[CCode (cname = "ufbx_metadata", destroy_function = "", has_type_id = false)]
public struct Metadata
{
	public WarningList warnings;
	public bool ascii;
	public uint32 version;
	public FileFormat file_format;
	public bool may_contain_no_index;
	public bool may_contain_missing_vertex_position;
	public bool may_contain_broken_elements;
	public bool is_unsafe;
	public bool has_warning[WarningType.TYPE_COUNT];
	public String creator;
	public bool big_endian;
	public String filename;
	public String relative_root;
	public Blob raw_filename;
	public Blob raw_relative_root;
	public Exporter exporter;
	public uint32 exporter_version;
	public Props scene_props;
	public Application original_application;
	public Application latest_application;
	public Thumbnail thumbnail;
	public bool geometry_ignored;
	public bool animation_ignored;
	public bool embedded_ignored;
	public size_t max_face_triangles;
	public size_t result_memory_used;
	public size_t temp_memory_used;
	public size_t result_allocs;
	public size_t temp_allocs;
	public size_t element_buffer_size;
	public size_t num_shader_textures;
	public Real bone_prop_size_unit;
	public bool bone_prop_limb_length_relative;
	public Real ortho_size_unit;
	public int64 ktime_second;
	public String original_file_path;
	public Blob raw_original_file_path;
	public SpaceConversion space_conversion;
	public GeometryTransformHandling geometry_transform_handling;
	public InheritModeHandling inherit_mode_handling;
	public PivotHandling pivot_handling;
	public MirrorAxis handedness_conversion_axis;
	public Quat root_rotation;
	public Real root_scale;
	public MirrorAxis mirror_axis;
	public Real geometry_scale;
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
	public Props props;
	public CoordinateAxes axes;
	public Real unit_meters;
	public double frames_per_second;
	public Vec3 ambient_color;
	public String default_camera;
	public TimeMode time_mode;
	public TimeProtocol time_protocol;
	public SnapMode snap_mode;
	public CoordinateAxis original_axis_up;
	public Real original_unit_meters;
}

[Compact]
[CCode (cname = "ufbx_scene", free_function = "ufbx_free_scene", has_type_id = false)]
public class Scene
{
	public Metadata metadata;
	public SceneSettings settings;
	public unowned Node root_node;
	public unowned Anim anim;
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
	public unowned DomNode? dom_root;
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
	public bool valid;
	public Vec3 position;
	public Vec3 derivative;
}

[CCode (cname = "ufbx_surface_point", destroy_function = "", has_type_id = false)]
public struct SurfacePoint
{
	public bool valid;
	public Vec3 position;
	public Vec3 derivative_u;
	public Vec3 derivative_v;
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
	public uint32 index;
	public uint32 next;
	public uint32 prev;
	public uint32 twin;
	public uint32 face;
	public uint32 edge;
	public TopoFlags flags;
}

[CCode (cname = "ufbx_vertex_stream", destroy_function = "", has_type_id = false)]
public struct VertexStream
{
	[CCode (array_length_cname = "vertex_count", array_length_type = "size_t")]
	public void[] data;
	public size_t vertex_size;
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
	public AllocFn alloc_fn;
	public ReallocFn realloc_fn;
	public FreeFn free_fn;
	public FreeAllocatorFn free_allocator_fn;
	public void* user;
}

[CCode (cname = "ufbx_allocator_opts", destroy_function = "", has_type_id = false)]
public struct AllocatorOpts
{
	public Allocator allocator;
	public size_t memory_limit;
	public size_t allocation_limit;
	public size_t huge_threshold;
	public size_t max_chunk_size;
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
	public ReadFn read_fn;
	public SkipFn skip_fn;
	public SizeFn size_fn;
	public CloseFn close_fn;
	public void* user;
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
	public OpenFileContext context;
	public OpenFileType type;
	public Blob original_filename;
}

[CCode (cname = "ufbx_open_file_fn", has_target = true, delegate_target_pos = 0.9)]
public delegate bool OpenFileFn(Stream stream, string path, size_t path_len, OpenFileInfo info);
[CCode (cname = "ufbx_open_file_cb", destroy_function = "", has_type_id = false)]
public struct OpenFileCb
{
	public OpenFileFn fn;
	public void* user;
}

[CCode (cname = "ufbx_open_file_opts", destroy_function = "", has_type_id = false)]
public struct OpenFileOptions
{
	public uint32 _begin_zero;
	public AllocatorOpts allocator;
	public bool filename_null_terminated;
	public uint32 _end_zero;
}

[CCode (cname = "ufbx_close_memory_fn", has_target = true, delegate_target_pos = 0.9)]
public delegate void CloseMemoryFn(void* data, size_t data_size);
[CCode (cname = "ufbx_close_memory_cb", destroy_function = "", has_type_id = false)]
public struct CloseMemoryCb
{
	public CloseMemoryFn fn;
	public void* user;
}

[CCode (cname = "ufbx_open_memory_opts", destroy_function = "", has_type_id = false)]
public struct OpenMemoryOpts
{
	public uint32 _begin_zero;
	public AllocatorOpts allocator;
	public bool no_copy;
	public CloseMemoryCb close_cb;
	public uint32 _end_zero;
}

[CCode (cname = "ufbx_error_frame", destroy_function = "", has_type_id = false)]
public struct ErrorFrame
{
	public uint32 source_line;
	public String function;
	public String description;
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
	UNSUPPORTED_VERSION,
	TYPE_COUNT
}

[CCode (cname = "ufbx_error", destroy_function = "", has_type_id = false)]
public struct Error
{
	public ErrorType type;
	public String description;
	public uint32 stack_size;
	public ErrorFrame stack[ERROR_STACK_MAX_DEPTH];
	public size_t info_length;
	public char info[ERROR_INFO_LENGTH];
}

[CCode (cname = "ufbx_progress", destroy_function = "", has_type_id = false)]
public struct Progress
{
	public uint64 bytes_read;
	public uint64 bytes_total;
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
	public ProgressFn fn;
	public void* user;
}

[CCode (cname = "ufbx_inflate_input", destroy_function = "", has_type_id = false)]
public struct InflateInput
{
	public size_t total_size;
	public void* data;
	public size_t data_size;
	public void* buffer;
	public size_t buffer_size;
	public ReadFn read_fn;
	public void* read_user;
	public ProgressCb progress_cb;
	public uint64 progress_interval_hint;
	public uint64 progress_size_before;
	public uint64 progress_size_after;
	public bool no_header;
	public bool no_checksum;
	public size_t internal_fast_bits;
}

[CCode (cname = "ufbx_inflate_retain", destroy_function = "", has_type_id = false)]
struct InflateRetain
{
	public bool initialized;
	public uint64 data[1024];
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

[SimpleType]
[CCode (cname = "ufbx_baked_vec3", destroy_function = "", has_type_id = false)]
public struct BakedVec3
{
	public double time;
	public Vec3 value;
	public BakedKeyFlags flags;
}

[CCode (cname = "ufbx_baked_vec3_list", destroy_function = "", has_type_id = false)]
public struct BakedVec3List
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public BakedVec3[] data;
}

[SimpleType]
[CCode (cname = "ufbx_baked_quat", destroy_function = "", has_type_id = false)]
public struct BakedQuat
{
	public double time;
	public Quat value;
	public BakedKeyFlags flags;
}

[CCode (cname = "ufbx_baked_quat_list", destroy_function = "", has_type_id = false)]
public struct BakedQuatList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public BakedQuat[] data;
}

[SimpleType]
[CCode (cname = "ufbx_baked_node", destroy_function = "", has_type_id = false)]
public struct BakedNode
{
	public uint32 typed_id;
	public uint32 element_id;
	public bool constant_translation;
	public bool constant_rotation;
	public bool constant_scale;
	public BakedVec3List translation_keys;
	public BakedQuatList rotation_keys;
	public BakedVec3List scale_keys;
}

[CCode (cname = "ufbx_baked_node_list", destroy_function = "", has_type_id = false)]
public struct BakedNodeList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public BakedNode[] data;
}

[SimpleType]
[CCode (cname = "ufbx_baked_prop", destroy_function = "", has_type_id = false)]
public struct BakedProp
{
	public String name;
	public bool constant_value;
	public BakedVec3List keys;
}

[CCode (cname = "ufbx_baked_prop_list", destroy_function = "", has_type_id = false)]
public struct BakedPropList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public BakedProp[] data;
}

[SimpleType]
[CCode (cname = "ufbx_baked_element", destroy_function = "", has_type_id = false)]
public struct BakedElement
{
	public uint32 element_id;
	public BakedPropList props;
}

[CCode (cname = "ufbx_baked_element_list", destroy_function = "", has_type_id = false)]
public struct BakedElementList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public BakedElement[] data;
}

[CCode (cname = "ufbx_baked_anim_metadata", destroy_function = "", has_type_id = false)]
public struct BakedAnimMetadata
{
	public size_t result_memory_used;
	public size_t temp_memory_used;
	public size_t result_allocs;
	public size_t temp_allocs;
}

[CCode (cname = "ufbx_baked_anim", destroy_function = "", has_type_id = false)]
public struct BakedAnim
{
	public BakedNodeList nodes;
	public BakedElementList elements;
	public double playback_time_begin;
	public double playback_time_end;
	public double playback_duration;
	public double key_time_min;
	public double key_time_max;
	public BakedAnimMetadata metadata;
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
	public uint32 max_concurrent_tasks;
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
	public ThreadPoolInitFn init_fn;
	public ThreadPoolRunFn run_fn;
	public ThreadPoolWaitFn wait_fn;
	public ThreadPoolFreeFn free_fn;
	public void* user;
}

[CCode (cname = "ufbx_thread_opts", destroy_function = "", has_type_id = false)]
public struct ThreadOpts
{
	public ThreadPool pool;
	public size_t num_tasks;
	public size_t memory_limit;
}

[CCode (cname = "ufbx_evaluate_flags", cprefix = "UFBX_EVALUATE_FLAG_", has_type_id = false)]
public enum EvaluateFlags
{
	NO_EXTRAPOLATION,
}

[CCode (cname = "ufbx_load_opts", destroy_function = "", has_type_id = false)]
public struct LoadOpts
{
	public uint32 _begin_zero;
	public AllocatorOpts temp_allocator;
	public AllocatorOpts result_allocator;
	public ThreadOpts thread_opts;
	public bool ignore_geometry;
	public bool ignore_animation;
	public bool ignore_embedded;
	public bool ignore_all_content;
	public bool evaluate_skinning;
	public bool evaluate_caches;
	public bool load_external_files;
	public bool ignore_missing_external_files;
	public bool skip_skin_vertices;
	public bool skip_mesh_parts;
	public bool clean_skin_weights;
	public bool use_blender_pbr_material;
	public bool disable_quirks;
	public bool strict;
	public bool force_single_thread_ascii_parsing;
	public bool allow_unsafe;
	public IndexErrorHandling index_error_handling;
	public bool connect_broken_elements;
	public bool allow_nodes_out_of_root;
	public bool allow_missing_vertex_position;
	public bool allow_empty_faces;
	public bool generate_missing_normals;
	public bool open_main_file_with_default;
	public char path_separator;
	public uint32 node_depth_limit;
	public uint64 file_size_estimate;
	public size_t read_buffer_size;
	public String filename;
	public Blob raw_filename;
	public ProgressCb progress_cb;
	public uint64 progress_interval_hint;
	public OpenFileCb open_file_cb;
	public GeometryTransformHandling geometry_transform_handling;
	public InheritModeHandling inherit_mode_handling;
	public SpaceConversion space_conversion;
	public PivotHandling pivot_handling;
	public bool pivot_handling_retain_empties;
	public MirrorAxis handedness_conversion_axis;
	public bool handedness_conversion_retain_winding;
	public bool reverse_winding;
	public CoordinateAxes target_axes;
	public Real target_unit_meters;
	public CoordinateAxes target_camera_axes;
	public CoordinateAxes target_light_axes;
	public String geometry_transform_helper_name;
	public String scale_helper_name;
	public bool normalize_normals;
	public bool normalize_tangents;
	public bool use_root_transform;
	public Transform root_transform;
	public double key_clamp_threshold;
	public UnicodeErrorHandling unicode_error_handling;
	public bool retain_vertex_attrib_w;
	public bool retain_dom;
	public FileFormat file_format;
	public size_t file_format_lookahead;
	public bool no_format_from_content;
	public bool no_format_from_extension;
	public bool obj_search_mtl_by_filename;
	public bool obj_merge_objects;
	public bool obj_merge_groups;
	public bool obj_split_groups;
	public String obj_mtl_path;
	public Blob obj_mtl_data;
	public Real obj_unit_meters;
	public CoordinateAxes obj_axes;
	public uint32 _end_zero;
}

[CCode (cname = "ufbx_evaluate_opts", destroy_function = "", has_type_id = false)]
public struct EvaluateOpts
{
	public uint32 _begin_zero;
	public Allocator temp_allocator;
	public AllocatorOpts result_allocator;
	public bool evaluate_skinning;
	public bool evaluate_caches;
	public uint32 evaluate_flags;
	public bool load_external_files;
	public OpenFileCb open_file_cb;
	public uint32 _end_zero;
}

[CCode (cname = "ufbx_const_uint32_list", destroy_function = "", has_type_id = false)]
public struct ConstUint32List
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public uint32[] data;
}

[CCode (cname = "ufbx_const_real_list", destroy_function = "", has_type_id = false)]
public struct ConstRealList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public Real[] data;
}

[SimpleType]
[CCode (cname = "ufbx_prop_override_desc", destroy_function = "", has_type_id = false)]
public struct PropOverrideDesc
{
	public uint32 element_id;
	public String prop_name;
	public Vec4 value;
	public String value_str;
	public int64 value_int;
}

[CCode (cname = "ufbx_const_prop_override_desc_list", destroy_function = "", has_type_id = false)]
public struct ConstPropOverrideDescList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public PropOverrideDesc[] data;
}

[CCode (cname = "ufbx_const_transform_override_list", destroy_function = "", has_type_id = false)]
public struct ConstTransformOverrideList
{
	[CCode (array_length_cname = "count", array_length_type = "size_t")]
	public TransformOverride[] data;
}

[CCode (cname = "ufbx_anim_opts", destroy_function = "", has_type_id = false)]
public struct AnimOpts
{
	public uint32 _begin_zero;
	public ConstUint32List layer_ids;
	public ConstRealList override_layer_weights;
	public ConstPropOverrideDescList prop_overrides;
	public ConstTransformOverrideList transform_overrides;
	public bool ignore_connections;
	public AllocatorOpts result_allocator;
	public uint32 _end_zero;
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
	public uint32 _begin_zero;
	public AllocatorOpts temp_allocator;
	public AllocatorOpts result_allocator;
	public bool trim_start_time;
	public double resample_rate;
	public double minimum_sample_rate;
	public double maximum_sample_rate;
	public bool bake_transform_props;
	public bool skip_node_transforms;
	public bool no_resample_rotation;
	public bool ignore_layer_weight_animation;
	public size_t max_keyframe_segments;
	public BakeStepHandling step_handling;
	public double step_custom_duration;
	public double step_custom_epsilon;
	public uint32 evaluate_flags;
	public bool key_reduction_enabled;
	public bool key_reduction_rotation;
	public double key_reduction_threshold;
	public size_t key_reduction_passes;
	public uint32 _end_zero;
}

[CCode (cname = "ufbx_tessellate_curve_opts", destroy_function = "", has_type_id = false)]
public struct TessellateCurveOpts
{
	public uint32 _begin_zero;
	public AllocatorOpts temp_allocator;
	public AllocatorOpts result_allocator;
	public size_t span_subdivision;
	public uint32 _end_zero;
}

[CCode (cname = "ufbx_tessellate_surface_opts", destroy_function = "", has_type_id = false)]
public struct TessellateSurfaceOpts
{
	public uint32 _begin_zero;
	public AllocatorOpts temp_allocator;
	public AllocatorOpts result_allocator;
	public size_t span_subdivision_u;
	public size_t span_subdivision_v;
	public bool skip_mesh_parts;
	public uint32 _end_zero;
}

[CCode (cname = "ufbx_subdivide_opts", destroy_function = "", has_type_id = false)]
public struct SubdivideOpts
{
	public uint32 _begin_zero;
	public AllocatorOpts temp_allocator;
	public AllocatorOpts result_allocator;
	public SubdivisionBoundary boundary;
	public SubdivisionBoundary uv_boundary;
	public bool ignore_normals;
	public bool interpolate_normals;
	public bool interpolate_tangents;
	public bool evaluate_source_vertices;
	public size_t max_source_vertices;
	public bool evaluate_skin_weights;
	public size_t max_skin_weights;
	public size_t skin_deformer_index;
	public uint32 _end_zero;
}

[CCode (cname = "ufbx_geometry_cache_opts", destroy_function = "", has_type_id = false)]
public struct GeometryCacheOpts
{
	public uint32 _begin_zero;
	public AllocatorOpts temp_allocator;
	public AllocatorOpts result_allocator;
	public OpenFileCb open_file_cb;
	public double frames_per_second;
	public MirrorAxis mirror_axis;
	public bool use_scale_factor;
	public Real scale_factor;
	public uint32 _end_zero;
}

[CCode (cname = "ufbx_geometry_cache_data_opts", destroy_function = "", has_type_id = false)]
public struct GeometryCacheDataOpts
{
	public uint32 _begin_zero;
	public OpenFileCb open_file_cb;
	public bool additive;
	public bool use_weight;
	public Real weight;
	public bool ignore_transform;
	public uint32 _end_zero;
}

[CCode (cname = "ufbx_panic", destroy_function = "", has_type_id = false)]
public struct Panic
{
	public bool did_panic;
	public size_t message_length;
	public char message[PANIC_MESSAGE_LENGTH];
}

public bool is_thread_safe();

} /* namespace ufbx */
