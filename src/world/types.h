/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
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
	enum Enum : u32
	{
		LOCK_TRANSLATION_X = u32(1) << 0,
		LOCK_TRANSLATION_Y = u32(1) << 1,
		LOCK_TRANSLATION_Z = u32(1) << 2,
		LOCK_ROTATION_X    = u32(1) << 3,
		LOCK_ROTATION_Y    = u32(1) << 4,
		LOCK_ROTATION_Z    = u32(1) << 5
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

/// Enumerates collision groups.
///
/// @ingroup World
struct CollisionGroup
{
	enum Enum : u32
	{
		GROUP_0  = u32(1) <<  0, // Reserved
		GROUP_1  = u32(1) <<  1,
		GROUP_2  = u32(1) <<  2,
		GROUP_3  = u32(1) <<  3,
		GROUP_4  = u32(1) <<  4,
		GROUP_5  = u32(1) <<  5,
		GROUP_6  = u32(1) <<  6,
		GROUP_7  = u32(1) <<  7,
		GROUP_8  = u32(1) <<  8,
		GROUP_9  = u32(1) <<  9,
		GROUP_10 = u32(1) << 10,
		GROUP_11 = u32(1) << 11,
		GROUP_12 = u32(1) << 12,
		GROUP_13 = u32(1) << 13,
		GROUP_14 = u32(1) << 14,
		GROUP_15 = u32(1) << 15,
		GROUP_16 = u32(1) << 16,
		GROUP_17 = u32(1) << 17,
		GROUP_18 = u32(1) << 18,
		GROUP_19 = u32(1) << 19,
		GROUP_20 = u32(1) << 20,
		GROUP_21 = u32(1) << 21,
		GROUP_22 = u32(1) << 22,
		GROUP_23 = u32(1) << 23,
		GROUP_24 = u32(1) << 24,
		GROUP_25 = u32(1) << 25,
		GROUP_26 = u32(1) << 26,
		GROUP_27 = u32(1) << 27,
		GROUP_28 = u32(1) << 28,
		GROUP_29 = u32(1) << 29,
		GROUP_30 = u32(1) << 30,
		GROUP_31 = u32(1) << 31
	};
};

/// Enumerates renderable flags.
///
/// @ingroup World
struct RenderableFlags
{
	enum Enum : u32
	{
		VISIBLE       = u32(1) << 0,
		SHADOW_CASTER = u32(1) << 1,
		LOD_LEVEL     = u32(1) << 2,

		DIRTY         = u32(1) << 31,
	};
};

/// Enumerates sprite flags.
///
/// @ingroup World
struct SpriteFlags
{
	enum Enum : u32
	{
		FLIP_X = u32(1) << 31,
		FLIP_Y = u32(1) << 30
	};
};

/// Enumerates LOD group fade modes.
///
/// @ingroup World
struct LodFadeMode
{
	enum Enum : u32
	{
		NONE,      ///< LODs are swapped immediately with no transition in-between.
		CROSSFADE, ///< Time-based crossfading.

		COUNT
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
		PHYSICS_MOVER_ACTOR_COLLISION,
		PHYSICS_MOVER_MOVER_COLLISION,
		PHYSICS_TRANSFORM,

		COUNT
	};
};

#define UNIT_INDEX_BITS 22
#define UNIT_INDEX_MASK 0x003fffff
#define UNIT_ID_BITS    8
#define UNIT_ID_MASK    0x3fc00000
CE_STATIC_ASSERT(UNIT_INDEX_BITS + UNIT_ID_BITS <= 30);

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
		return _idx >> UNIT_INDEX_BITS;
	}

	bool is_valid()
	{
		return _idx != UINT32_MAX;
	}
};

inline UnitId make_unit(u32 idx, u8 gen)
{
	UnitId unit = { idx | u32(gen) << UNIT_INDEX_BITS };
	return unit;
}

inline bool operator==(const UnitId &a, const UnitId &b)
{
	return a._idx == b._idx;
}

inline bool operator!=(const UnitId &a, const UnitId &b)
{
	return a._idx != b._idx;
}

const UnitId UNIT_INVALID = { UINT32_MAX };

template<>
struct hash<UnitId>
{
	u32 operator()(const UnitId &id) const
	{
		return id._idx;
	}
};

/// Spawn flags.
///
/// @ingroup World
struct SpawnFlags
{
	enum Enum : u32
	{
		NONE              = u32(0),
		OVERRIDE_POSITION = u32(1) << 0,
		OVERRIDE_ROTATION = u32(1) << 1,
		OVERRIDE_SCALE    = u32(1) << 2,
	};
};

/// Play sound flags.
///
/// @ingroup World
struct PlaySoundFlags
{
	enum Enum : u32
	{
		NONE               = u32(0),
		ENABLE_ATTENUATION = u32(1) << 0,
	};
};

typedef void (*UnitDestroyFunction)(UnitId unit, void *user_data);

struct UnitDestroyCallback
{
	UnitDestroyFunction destroy;
	void *user_data;
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

INSTANCE_ID(TransformId)
INSTANCE_ID(CameraId)
INSTANCE_ID(MeshId)
INSTANCE_ID(SpriteId)
INSTANCE_ID(LodGroupId)
INSTANCE_ID(LightId)
INSTANCE_ID(FogId)
INSTANCE_ID(GlobalLightingId)
INSTANCE_ID(BloomId)
INSTANCE_ID(TonemapId)
INSTANCE_ID(ColliderId)
INSTANCE_ID(ActorId)
INSTANCE_ID(MoverId)
INSTANCE_ID(JointId)
INSTANCE_ID(ScriptId)
INSTANCE_ID(StateMachineId)

#undef INSTANCE_ID

/// Mesh renderer description.
///
/// @ingroup World
struct MeshRendererDesc
{
	StringId64 mesh_resource;     ///< Name of .mesh resource.
	StringId64 material_resource; ///< Name of .material resource.
	StringId32 geometry_name;     ///< Name of geometry inside .mesh resource.
	u32 flags;                    ///< RenderableFlags::Enum
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
	u32 flags;                    ///< SpriteFlags::Enum | RenderableFlags::Enum
	char _pad[4];
};

/// LOD group component header.
///
/// @ingroup World
struct LodGroupDesc
{
	u32 num_levels; ///< Number of LodDesc entries following this header.
	u32 fade_mode;  ///< LodFadeMode::Enum.
	s32 level;      ///< Manual LOD level, or -1 for automatic selection.
};

/// LOD group level description.
///
/// @ingroup World
struct LodDesc
{
	u32 unit_index;  ///< Index of unit that owns the mesh renderer component, or UINT32_MAX.
	f32 screen_size; ///< Screen-height threshold in [0, 1].
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
	u32 type;          ///< LightType::Enum
	f32 range;         ///< In meters.
	f32 intensity;     ///<
	f32 spot_angle;    ///< In radians.
	Vector3 color;     ///< Color of the light.
	f32 shadow_bias;   ///<
	u32 flags;         ///< RenderableFlags::Enum
};

/// Global lighting description.
///
/// @ingroup World
struct GlobalLightingDesc
{
	StringId64 skydome_map; ///< Texture to use for skydome rendering.
	f32 skydome_intensity;  ///< Skydome color intensity multiplier.
	Vector3 ambient_color;  ///<
};

/// Bloom description.
///
/// @ingroup World
struct BloomDesc
{
	bool enabled;  ///< Whether the bloom effect is enabled.
	char _pad[3];  ///<
	f32 threshold; ///< The minimum luminance a pixel should have to be included in the bloom layer.
	f32 weight;    ///< The weight to use when mixing the bloom layer with the main color layer.
	f32 intensity; ///< The global intensity of the bloom effect.
};

struct TonemapType
{
	enum Enum
	{
		GAMMA,    ///< Linear- to Gamma-space conversion.
		REINHARD, ///<
		FILMIC,   ///<
		ACES,     ///<

		COUNT
	};
};

/// Tonemap description.
///
/// @ingroup World
struct TonemapDesc
{
	f32 type;      ///< TonemapType::Enum
	f32 unused[3]; ///<
};

/// Fog description.
///
/// @ingroup World
struct FogDesc
{
	Vector3 color; ///< Color of the fog.
	f32 density;   ///< Density of the fog.
	f32 range_min; ///< Distance from the camera's near plane where the fog starts to appear.
	f32 range_max; ///< Distance from the camera's near plane where the fog stops.
	f32 sun_blend; ///< How much the sun color blends into the fog color.
	f32 enabled;   ///< Whether the fog simulation is enabled.
};

/// Script description.
///
/// @ingroup World
struct ScriptDesc
{
	StringId64 script_resource;     ///< Name of .lua resource.
	char script_resource_name[256]; ///< Name of .lua resource (plain text).
};

/// Transform description.
///
/// @ingroup World
struct TransformDesc
{
	Vector3 position;
	Quaternion rotation;
	Vector3 scale;
	StringId32 name;
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
	SphereShape sphere;           ///<
	CapsuleShape capsule;         ///<
	BoxShape box;                 ///<
	HeightfieldShape heightfield; ///<
	u32 size;                     ///< Size of additional data.
//	char data[size]               ///< Convex Hull, Mesh, Heightfield data.
};

struct MoverDesc
{
	CapsuleShape capsule;        ///<
	f32 max_slope_angle;         ///< Maximum slope angle the mover is allowed to climb.
	f32 step_height;             ///< Maximum step height the mover is allowed to climb.
	StringId32 collision_filter; ///< Name of collision filter in global.physics_config resource.
	Vector3 center;              ///< Center of the capsule relative to the transform's position.
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
		SPHERICAL,
		LIMB,
		SPRING,
		D6,

		COUNT
	};
};
CE_STATIC_ASSERT(JointType::COUNT <= 16);

struct JointFlags
{
	enum Enum
	{
		USE_CUSTOM_POSES = u32(1) << 0,
		HINGE_USE_MOTOR  = u32(1) << 1,
		HINGE_USE_LIMITS = u32(1) << 2,
	};
};

struct D6Axis
{
	enum Enum : u32
	{
		X,
		Y,
		Z,

		COUNT
	};
};

struct D6Motion
{
	enum Enum : u32
	{
		LOCKED,
		LIMITED,
		FREE,

		COUNT
	};
};
CE_STATIC_ASSERT(D6Motion::COUNT <= 4);

struct D6MotorMode
{
	enum Enum : u32
	{
		OFF,
		VELOCITY,
		POSITION,

		COUNT
	};
};
CE_STATIC_ASSERT(D6MotorMode::COUNT <= 4);

#define JOINT_TYPE_AND_FLAGS_TYPE_SHIFT  28
#define JOINT_TYPE_AND_FLAGS_TYPE_MASK   UINT32_C(0xf0000000)
#define JOINT_TYPE_AND_FLAGS_FLAGS_SHIFT 0
#define JOINT_TYPE_AND_FLAGS_FLAGS_MASK  UINT32_C(0x0fffffff)

#define D6_JOINT_LINEAR_X_MOTION_SHIFT  0
#define D6_JOINT_LINEAR_X_MOTION_MASK   UINT32_C(0x00000003)
#define D6_JOINT_LINEAR_Y_MOTION_SHIFT  2
#define D6_JOINT_LINEAR_Y_MOTION_MASK   UINT32_C(0x0000000c)
#define D6_JOINT_LINEAR_Z_MOTION_SHIFT  4
#define D6_JOINT_LINEAR_Z_MOTION_MASK   UINT32_C(0x00000030)
#define D6_JOINT_ANGULAR_X_MOTION_SHIFT 6
#define D6_JOINT_ANGULAR_X_MOTION_MASK  UINT32_C(0x000000c0)
#define D6_JOINT_ANGULAR_Y_MOTION_SHIFT 8
#define D6_JOINT_ANGULAR_Y_MOTION_MASK  UINT32_C(0x00000300)
#define D6_JOINT_ANGULAR_Z_MOTION_SHIFT 10
#define D6_JOINT_ANGULAR_Z_MOTION_MASK  UINT32_C(0x00000c00)
#define D6_JOINT_LINEAR_X_MOTOR_SHIFT   12
#define D6_JOINT_LINEAR_X_MOTOR_MASK    UINT32_C(0x00003000)
#define D6_JOINT_LINEAR_Y_MOTOR_SHIFT   14
#define D6_JOINT_LINEAR_Y_MOTOR_MASK    UINT32_C(0x0000c000)
#define D6_JOINT_LINEAR_Z_MOTOR_SHIFT   16
#define D6_JOINT_LINEAR_Z_MOTOR_MASK    UINT32_C(0x00030000)
#define D6_JOINT_ANGULAR_X_MOTOR_SHIFT  18
#define D6_JOINT_ANGULAR_X_MOTOR_MASK   UINT32_C(0x000c0000)
#define D6_JOINT_ANGULAR_Y_MOTOR_SHIFT  20
#define D6_JOINT_ANGULAR_Y_MOTOR_MASK   UINT32_C(0x00300000)
#define D6_JOINT_ANGULAR_Z_MOTOR_SHIFT  22
#define D6_JOINT_ANGULAR_Z_MOTOR_MASK   UINT32_C(0x00c00000)

#define LIMB_JOINT_TWIST_MOTION_SHIFT   0
#define LIMB_JOINT_TWIST_MOTION_MASK    UINT32_C(0x00000003)
#define LIMB_JOINT_SWING_Y_MOTION_SHIFT 2
#define LIMB_JOINT_SWING_Y_MOTION_MASK  UINT32_C(0x0000000c)
#define LIMB_JOINT_SWING_Z_MOTION_SHIFT 4
#define LIMB_JOINT_SWING_Z_MOTION_MASK  UINT32_C(0x00000030)

struct HingeJoint
{
	Vector3 axis;
	f32 target_velocity;
	f32 max_motor_impulse;
	f32 lower_limit;
	f32 upper_limit;
	f32 bounciness;
};

struct SpringJoint
{
	f32 stiffness;
	f32 damping;
};

struct LimbJoint
{
	u32 motion;            ///< Packed. See LIMB_JOINT_TWIST_* and LIMB_JOINT_SWING_*.
	f32 twist_lower_limit; ///<
	f32 twist_upper_limit; ///<
	f32 swing_y_limit;     ///<
	f32 swing_z_limit;     ///<
};

struct D6Joint
{
	u32 motion_and_motor;            ///< Packed. See D6_JOINT_LINEAR_* and D6_JOINT_ANGULAR_*.
	Vector3 linear_lower_limit;      ///<
	Vector3 linear_upper_limit;      ///<
	Vector3 angular_lower_limit;     ///<
	Vector3 angular_upper_limit;     ///<
	Vector3 linear_target_velocity;  ///<
	Vector3 linear_target_position;  ///<
	Vector3 linear_max_motor_force;  ///<
	Vector3 angular_target_velocity; ///<
	Vector3 angular_target_position; ///<
	Vector3 angular_max_motor_force; ///<
};

struct JointDesc
{
	u32 type_and_flags;         ///< Packed. See JOINT_TYPE_AND_FLAGS_*.
	u32 other_actor_unit_index; ///< Index of unit that owns the other actor component, or UINT32_MAX.
	Matrix4x4 pose;             ///< Joint pose in owner actor local space.
	Matrix4x4 other_pose;       ///< Joint pose in other actor local space, or world space when no other actor is set.
	f32 break_force;            ///< Breaking threshold. Use FLT_MAX to disable breaking.
};

struct RaycastHit
{
	Vector3 position;    ///< In world-space.
	Vector3 normal;      ///< In world-space.
	f32 time;            ///< Time of impact in [0..1].
	UnitId unit;         ///< The unit that was hit.
	ActorId actor; ///< The actor that was hit.
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
	UnitId units[2];   ///<
	ActorId actors[2]; ///<
	Vector3 position;  ///< In world-space.
	Vector3 normal;    ///< In world-space.
	float distance;    ///< Separation distance
};

struct PhysicsMoverActorCollisionEvent
{
	UnitId mover_unit;      ///< The mover unit that produced the collision.
	UnitId actor_unit;      ///< The unit that holds the hit actor.
	ActorId actor;          ///< The actor that was hit.
	Vector3 normal;         ///< Collision normal from the mover sweep hit.
	Vector3 position;       ///< Mover position at collision time.
	Vector3 direction;      ///< Requested mover direction for this move.
	float direction_length; ///< Length of requested mover direction for this move.
};

struct PhysicsMoverMoverCollisionEvent
{
	UnitId mover_unit;       ///< The mover unit that produced the collision.
	UnitId other_mover_unit; ///< The other mover unit that was hit.
	MoverId mover;           ///< The mover instance that produced the collision.
	MoverId other_mover;     ///< The other mover instance that was hit.
};

struct PhysicsTriggerEvent
{
	enum Type { ENTER, LEAVE } type;
	UnitId trigger_unit; ///< The trigger unit that has been touched (i.e. the one that receives the event).
	UnitId other_unit;   ///< The other unit involved in the collision.
};

struct PhysicsTransformEvent
{
	UnitId unit_id;
	Matrix4x4 world;
};

typedef u32 AnimationId;

struct AnimationSkeletonInstance
{
	u32 num_bones;
	const Matrix4x4 *offsets;
	UnitId *bone_lookup;
	Matrix4x4 *bones;
};

} // namespace crown
