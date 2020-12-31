/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/functional.h"
#include "core/list.h"
#include "core/math/types.h"
#include "core/strings/string_id.h"
#include "core/types.h"

/// @defgroup World World
namespace crown
{
struct AnimationStateMachine;
struct DebugLine;
struct Gui;
struct Level;
struct Material;
struct MaterialManager;
struct PhysicsWorld;
struct RenderWorld;
struct SceneGraph;
struct ScriptWorld;
struct ShaderManager;
struct SoundWorld;
struct UnitManager;
struct World;

typedef u32 SoundInstanceId;

#define DEBUG_LINE_MARKER              UINT32_C(0xd7c17715)
#define DEBUG_GUI_MARKER               UINT32_C(0xf081a80a)
#define LEVEL_MARKER                   UINT32_C(0x1f2b43fe)
#define RENDER_WORLD_MARKER            UINT32_C(0xc82277de)
#define RESOURCE_PACKAGE_MARKER        UINT32_C(0x9a1ac68c)
#define SCENE_GRAPH_MARKER             UINT32_C(0x63a44dbf)
#define WORLD_MARKER                   UINT32_C(0xfb6ce2d3)
#define SCRIPT_WORLD_MARKER            UINT32_C(0x78486cdc)
#define SOUND_WORLD_MARKER             UINT32_C(0x44052b07)
#define PHYSICS_WORLD_MARKER           UINT32_C(0x1cf49bae)
#define ANIMATION_STATE_MACHINE_MARKER UINT32_C(0x59a1c462)

#define COMPONENT_TYPE_ACTOR                   STRING_ID_32("actor",                   UINT32_C(0x13958a55))
#define COMPONENT_TYPE_CAMERA                  STRING_ID_32("camera",                  UINT32_C(0x5005ac7a))
#define COMPONENT_TYPE_COLLIDER                STRING_ID_32("collider",                UINT32_C(0x3b9259cb))
#define COMPONENT_TYPE_LIGHT                   STRING_ID_32("light",                   UINT32_C(0x4cf76c7b))
#define COMPONENT_TYPE_MESH_RENDERER           STRING_ID_32("mesh_renderer",           UINT32_C(0x2554ca17))
#define COMPONENT_TYPE_SPRITE_RENDERER         STRING_ID_32("sprite_renderer",         UINT32_C(0x7b4af6de))
#define COMPONENT_TYPE_TRANSFORM               STRING_ID_32("transform",               UINT32_C(0xb4363995))
#define COMPONENT_TYPE_SCRIPT                  STRING_ID_32("script",                  UINT32_C(0x2cf63026))
#define COMPONENT_TYPE_ANIMATION_STATE_MACHINE STRING_ID_32("animation_state_machine", UINT32_C(0xe3970e6b))

/// Enumerates camera projection types.
///
/// @ingroup World
struct ProjectionType
{
	enum Enum
	{
		ORTHOGRAPHIC,
		PERSPECTIVE,

		COUNT
	};
};

/// Enumerates light types.
///
/// @ingroup World
struct LightType
{
	enum Enum
	{
		DIRECTIONAL,
		OMNI,
		SPOT,

		COUNT
	};
};

/// Enumerates actor types.
///
/// @ingroup World
struct ActorType
{
	enum Enum
	{
		STATIC,
		DYNAMIC_PHYSICAL,
		DYNAMIC_KINEMATIC,

		COUNT
	};
};

/// Enumerates actor flags.
///
/// @ingroup World
struct ActorFlags
{
	enum Enum
	{
		LOCK_TRANSLATION_X = 1 << 0,
		LOCK_TRANSLATION_Y = 1 << 1,
		LOCK_TRANSLATION_Z = 1 << 2,
		LOCK_ROTATION_X    = 1 << 3,
		LOCK_ROTATION_Y    = 1 << 4,
		LOCK_ROTATION_Z    = 1 << 5
	};
};

/// Enumerates collider types.
///
/// @ingroup World
struct ColliderType
{
	enum Enum
	{
		SPHERE,
		CAPSULE,
		BOX,
		CONVEX_HULL,
		MESH,
		HEIGHTFIELD,

		COUNT
	};
};

/// Enumerates joint types.
///
/// @ingroup World
struct JointType
{
	enum Enum
	{
		FIXED,
		HINGE,
		SPRING,

		COUNT
	};
};

/// Enumerates collision groups.
///
/// @ingroup World
struct CollisionGroup
{
	enum Enum
	{
		GROUP_0  = 1 <<  0, // Reserved
		GROUP_1  = 1 <<  1,
		GROUP_2  = 1 <<  2,
		GROUP_3  = 1 <<  3,
		GROUP_4  = 1 <<  4,
		GROUP_5  = 1 <<  5,
		GROUP_6  = 1 <<  6,
		GROUP_7  = 1 <<  7,
		GROUP_8  = 1 <<  8,
		GROUP_9  = 1 <<  9,
		GROUP_10 = 1 << 10,
		GROUP_11 = 1 << 11,
		GROUP_12 = 1 << 12,
		GROUP_13 = 1 << 13,
		GROUP_14 = 1 << 14,
		GROUP_15 = 1 << 15,
		GROUP_16 = 1 << 16,
		GROUP_17 = 1 << 17,
		GROUP_18 = 1 << 18,
		GROUP_19 = 1 << 19,
		GROUP_20 = 1 << 20,
		GROUP_21 = 1 << 21,
		GROUP_22 = 1 << 22,
		GROUP_23 = 1 << 23,
		GROUP_24 = 1 << 24,
		GROUP_25 = 1 << 25,
		GROUP_26 = 1 << 26,
		GROUP_27 = 1 << 27,
		GROUP_28 = 1 << 28,
		GROUP_29 = 1 << 29,
		GROUP_30 = 1 << 30,
		GROUP_31 = 1 << 31
	};
};

/// Enumerates world event types.
///
/// @ingroup World
struct EventType
{
	enum Enum
	{
		UNIT_SPAWNED,
		UNIT_DESTROYED,

		LEVEL_LOADED,

		PHYSICS_COLLISION,
		PHYSICS_TRIGGER,
		PHYSICS_TRANSFORM,

		COUNT
	};
};

#define UNIT_INDEX_BITS 22
#define UNIT_INDEX_MASK 0x003fffff
#define UNIT_ID_BITS    8
#define UNIT_ID_MASK    0x3fc00000

/// Unit id.
///
/// @ingroup World
struct UnitId
{
	u32 _idx;

	u32 index() const
	{
		return _idx & UNIT_INDEX_MASK;
	}

	u32 id() const
	{
		return (_idx >> UNIT_INDEX_BITS) & UNIT_ID_MASK;
	}

	bool is_valid()
	{
		return _idx != UINT32_MAX;
	}
};

inline bool operator==(const UnitId& a, const UnitId& b)
{
	return a._idx == b._idx;
}

inline bool operator!=(const UnitId& a, const UnitId& b)
{
	return a._idx != b._idx;
}

const UnitId UNIT_INVALID = { UINT32_MAX };

template <>
struct hash<UnitId>
{
	u32 operator()(const UnitId& id) const
	{
		return id._idx;
	}
};

typedef void (*UnitDestroyFunction)(UnitId unit, void* user_data);

struct UnitDestroyCallback
{
	UnitDestroyFunction destroy;
	void* user_data;
	ListNode node;
};

#define INSTANCE_ID(name)            \
	struct name                      \
	{                                \
		u32 i;                       \
	};                               \
	inline bool is_valid(name inst)  \
	{                                \
		return inst.i != UINT32_MAX; \
	}

INSTANCE_ID(TransformInstance)
INSTANCE_ID(CameraInstance)
INSTANCE_ID(MeshInstance)
INSTANCE_ID(SpriteInstance)
INSTANCE_ID(LightInstance)
INSTANCE_ID(ColliderInstance)
INSTANCE_ID(ActorInstance)
INSTANCE_ID(JointInstance)
INSTANCE_ID(ScriptInstance)
INSTANCE_ID(StateMachineInstance)

#undef INSTANCE_ID

/// Mesh renderer description.
///
/// @ingroup World
struct MeshRendererDesc
{
	StringId64 mesh_resource;     ///< Name of .mesh resource.
	StringId64 material_resource; ///< Name of .material resource.
	StringId32 geometry_name;     ///< Name of geometry inside .mesh resource.
	bool visible;                 ///< Whether mesh is visible.
	char _pad0[3];
};

/// Sprite renderer description.
///
/// @ingroup World
struct SpriteRendererDesc
{
	StringId64 sprite_resource;   ///< Name of .sprite resource.
	StringId64 material_resource; ///< Name of .material resource.
	u32 layer;                    ///< Sort layer
	u32 depth;                    ///< Depth in layer
	bool visible;                 ///< Whether sprite is visible.
	char _pad0[3];
	char _pad1[4];
};

/// Animation state machine description.
///
/// @ingroup World
struct AnimationStateMachineDesc
{
	StringId64 state_machine_resource; ///< Name of .state_machine resource.
};

/// Light description.
///
/// @ingroup World
struct LightDesc
{
	u32 type;       ///< LightType::Enum
	f32 range;      ///< In meters.
	f32 intensity;
	f32 spot_angle; ///< In radians.
	Vector3 color;  ///< Color of the light.
};

/// Script description.
///
/// @ingroup World
struct ScriptDesc
{
	StringId64 script_resource; ///< Name of .lua resource.
};

/// Transform description.
///
/// @ingroup World
struct TransformDesc
{
	Vector3 position;
	Quaternion rotation;
	Vector3 scale;
};

/// Camera description.
///
/// @ingroup World
struct CameraDesc
{
	u32 type;       ///< ProjectionType::Enum
	f32 fov;        ///< Vertical FOV
	f32 near_range; ///< Near clipping plane distance
	f32 far_range;  ///< Far clipping plane distance
};

/// Actor resource.
///
/// @ingroup World
struct ActorResource
{
	StringId32 actor_class;      ///< Name of actor in global.physics_config resource.
	f32 mass;                    ///< Mass of the actor.
	u32 flags;                   ///< ActorFlags::Enum
	StringId32 collision_filter; ///< Name of collision filter in global.physics_config resource.
	StringId32 material;         ///< Name of material in global.physics_config resource.
};

struct SphereShape
{
	f32 radius;
};

struct CapsuleShape
{
	f32 radius;
	f32 height;
};

struct BoxShape
{
	Vector3 half_size;
};

struct HeightfieldShape
{
	u32 width;
	u32 length;
	f32 height_scale;
	f32 height_min;
	f32 height_max;
};

struct ColliderDesc
{
	u32 type;                     ///< ShapeType::Enum
	Matrix4x4 local_tm;           ///< In actor-space
	SphereShape sphere;
	CapsuleShape capsule;
	BoxShape box;
	HeightfieldShape heightfield;
	u32 size;                     ///< Size of additional data.
//	char data[size]               ///< Convex Hull, Mesh, Heightfield data.
};

struct HingeJoint
{
	Vector3 axis;

	bool use_motor;
	f32 target_velocity;
	f32 max_motor_impulse;

	bool use_limits;
	f32 lower_limit;
	f32 upper_limit;
	f32 bounciness;
};

struct JointDesc
{
	u32 type;         ///< JointType::Enum
	Vector3 anchor_0;
	Vector3 anchor_1;

	bool breakable;
	char _pad[3];
	f32 break_force;

	HingeJoint hinge;
};

struct RaycastHit
{
	Vector3 position;    ///< In world-space.
	Vector3 normal;      ///< In world-space.
	f32 time;            ///< Time of impact in [0..1].
	UnitId unit;         ///< The unit that was hit.
	ActorInstance actor; ///< The actor that was hit.
};

struct UnitSpawnedEvent
{
	UnitId unit; ///< The unit spawned.
};

struct UnitDestroyedEvent
{
	UnitId unit; ///< The unit destroyed.
};

struct LevelLoadedEvent
{
};

struct PhysicsCollisionEvent
{
	enum Type { TOUCH_BEGIN, TOUCHING, TOUCH_END } type;
	UnitId units[2];
	ActorInstance actors[2];
	Vector3 position;        ///< In world-space.
	Vector3 normal;          ///< In world-space.
	float distance;          ///< Separation distance
};

struct PhysicsTriggerEvent
{
	enum Type { TOUCH_BEGIN, TOUCHING, TOUCH_END } type;
	ActorInstance trigger;
	ActorInstance other;
};

struct PhysicsTransformEvent
{
	UnitId unit_id;
	Matrix4x4 world;
};

} // namespace crown
