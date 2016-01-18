/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "math_types.h"
#include "string_id.h"
#include "types.h"

namespace crown
{

class Level;
class MaterialManager;
class PhysicsWorld;
class RenderWorld;
class ShaderManager;
class SoundWorld;
class UnitManager;
class World;
struct DebugLine;
struct Gui;
struct Material;
struct SceneGraph;

typedef uint32_t SoundInstanceId;

/// Enumerates camera projection types.
///
/// @ingroup Graphics
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
/// @ingroup Graphics
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

struct ActorFlags
{
	enum Enum
	{
		LOCK_TRANSLATION_X = (1 << 0),
		LOCK_TRANSLATION_Y = (1 << 1),
		LOCK_TRANSLATION_Z = (1 << 2),
		LOCK_ROTATION_X    = (1 << 3),
		LOCK_ROTATION_Y    = (1 << 4),
		LOCK_ROTATION_Z    = (1 << 5)
	};
};

struct ShapeType
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

struct CollisionGroup
{
	enum Enum
	{
		GROUP_0  = (1<<0), // Reserved
		GROUP_1  = (1<<1),
		GROUP_2  = (1<<2),
		GROUP_3  = (1<<3),
		GROUP_4  = (1<<4),
		GROUP_5  = (1<<5),
		GROUP_6  = (1<<6),
		GROUP_7  = (1<<7),
		GROUP_8  = (1<<8),
		GROUP_9  = (1<<9),
		GROUP_10 = (1<<10),
		GROUP_11 = (1<<11),
		GROUP_12 = (1<<12),
		GROUP_13 = (1<<13),
		GROUP_14 = (1<<14),
		GROUP_15 = (1<<15),
		GROUP_16 = (1<<16),
		GROUP_17 = (1<<17),
		GROUP_18 = (1<<18),
		GROUP_19 = (1<<19),
		GROUP_20 = (1<<20),
		GROUP_21 = (1<<21),
		GROUP_22 = (1<<22),
		GROUP_23 = (1<<23),
		GROUP_24 = (1<<24),
		GROUP_25 = (1<<25),
		GROUP_26 = (1<<26),
		GROUP_27 = (1<<27),
		GROUP_28 = (1<<28),
		GROUP_29 = (1<<29),
		GROUP_30 = (1<<30),
		GROUP_31 = (1<<31)
	};
};

struct RaycastMode
{
	enum Enum
	{
		CLOSEST,
		ALL,

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
		PHYSICS_TRANSFORM,

		COUNT
	};
};

#define UNIT_INDEX_BITS 22
#define UNIT_INDEX_MASK 0x003fffff
#define UNIT_ID_BITS    8
#define UNIT_ID_MASK    0x3fc00000

struct UnitId
{
	uint32_t idx;

	uint32_t index() const
	{
		return idx & UNIT_INDEX_MASK;
	}

	uint32_t id() const
	{
		return (idx >> UNIT_INDEX_BITS) & UNIT_ID_MASK;
	}

	uint32_t encode() const
	{
		return idx;
	}

	void decode(uint32_t id)
	{
		idx = id;
	}

	bool is_valid()
	{
		return idx != UINT32_MAX;
	}
};

inline UnitId INVALID_UNIT() { UnitId id = { UINT32_MAX }; return id; }

struct TransformInstance
{
	uint32_t i;
};

struct CameraInstance
{
	uint32_t i;
};

struct MeshInstance
{
	uint32_t i;
};

struct SpriteInstance
{
	uint32_t i;
};

struct LightInstance
{
	uint32_t i;
};

struct ColliderInstance
{
	uint32_t i;
};

struct ActorInstance
{
	uint32_t i;
};

struct ControllerInstance
{
	uint32_t i;
};

struct JointInstance
{
	uint32_t i;
};

struct MeshRendererDesc
{
	StringId64 mesh_resource;
	StringId32 mesh_name;
	StringId64 material_resource; // FIXME
	bool visible;
	char _pad[3];
};

struct SpriteRendererDesc
{
	StringId64 sprite_resource;
	StringId64 material_resource; // FIXME
	bool visible;
	char _pad[3];
	char _pad1[4];
};

struct LightDesc
{
	uint32_t type;    // LightType::Enum
	float range;      // In meters
	float intensity;
	float spot_angle; // In radians
	Vector3 color;
};

struct TransformDesc
{
	Vector3 position;
	Quaternion rotation;
	Vector3 scale;
};

struct CameraDesc
{
	uint32_t type; // ProjectionType::Enum
	float fov;
	float near_range;
	float far_range;
};

struct ControllerDesc
{
	float height;                // Height of the capsule
	float radius;                // Radius of the capsule
	float slope_limit;           // The maximum slope which the character can walk up in radians.
	float step_offset;           // Maximum height of an obstacle which the character can climb.
	float contact_offset;        // Skin around the object within which contacts will be generated. Use it to avoid numerical precision issues.
	StringId32 collision_filter; // Collision filter from global.physics_config
};

struct ActorResource
{
	StringId32 actor_class;      // Actor from global.physics
	float mass;                  // Mass of the actor
	uint32_t flags;              // ActorFlags
	StringId32 collision_filter; // Collision filter from global.physics_config
};

struct SphereShape
{
	float radius;
};

struct CapsuleShape
{
	float radius;
	float height;
};

struct BoxShape
{
	Vector3 half_size;
};

struct HeightfieldShape
{
	uint32_t width;
	uint32_t length;
	float height_scale;
	float min_height;
	float max_height;
};

struct ShapeDesc
{
	StringId32 shape_class;       // Shape class from global.physics_config
	uint32_t type;                // ShapeType
	StringId32 material;          // Material from global.physics_config
	Matrix4x4 local_tm;           // In actor-space
	SphereShape sphere;
	CapsuleShape capsule;
	BoxShape box;
	HeightfieldShape heightfield;
	// dynamic data               // Mesh, Heightfield
};

struct HingeJoint
{
	Vector3 axis;

	bool use_motor;
	float target_velocity;
	float max_motor_impulse;

	bool use_limits;
	float lower_limit;
	float upper_limit;
	float bounciness;
};

struct JointDesc
{
	uint32_t type;    // JointType::Enum
	Vector3 anchor_0;
	Vector3 anchor_1;

	bool breakable;
	char _pad[3];
	float break_force;

	HingeJoint hinge;
};

struct RaycastHit
{
	ActorInstance actor;
	Vector3 position;    // In world-space
	Vector3 normal;      // In world-space
};

struct UnitSpawnedEvent
{
	/// The unit spawned
	UnitId unit;
};

struct UnitDestroyedEvent
{
	/// The unit destroyed
	UnitId unit;
};

struct LevelLoadedEvent
{
};

struct PhysicsCollisionEvent
{
	enum Type { BEGIN_TOUCH, END_TOUCH } type;
	ActorInstance actors[2];
	Vector3 where;
	Vector3 normal;
};

struct PhysicsTriggerEvent
{
	enum Type { BEGIN_TOUCH, END_TOUCH } type;
	ActorInstance trigger;
	ActorInstance other;
};

struct PhysicsTransformEvent
{
	UnitId unit_id;
	Matrix4x4 world_tm;
};

} // namespace crown
