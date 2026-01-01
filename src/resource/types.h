/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/types.h"

/// @defgroup Resource Resource
namespace crown
{
struct CompileOptions;
struct DataCompiler;
struct ResourceLoader;
struct ResourceManager;
struct ResourcePackage;

struct ActorResource;
struct StateMachineResource;
struct ControllerResource;
struct FontResource;
struct JointResource;
struct LevelResource;
struct LuaResource;
struct MaterialResource;
struct MeshResource;
struct MeshSkeletonResource;
struct MeshAnimationResource;
struct PackageResource;
struct PhysicsConfigResource;
struct RenderConfigResource;
struct ShaderResource;
struct ShapeResource;
struct SoundResource;
struct SpriteAnimationResource;
struct SpriteResource;
struct TextureResource;
struct UnitResource;

struct Platform
{
	enum Enum
	{
		ANDROID,
		ANDROID_ARM64,
		HTML5,
		LINUX,
		WINDOWS,

		COUNT
	};
};

} // namespace crown

/// @addtogroup Resource
/// @{
#define RESOURCE_TYPE_STATE_MACHINE    STRING_ID_64("state_machine",    UINT64_C(0xa486d4045106165c))
#define RESOURCE_TYPE_CONFIG           STRING_ID_64("config",           UINT64_C(0x82645835e6b73232))
#define RESOURCE_TYPE_FONT             STRING_ID_64("font",             UINT64_C(0x9efe0a916aae7880))
#define RESOURCE_TYPE_UNIT             STRING_ID_64("unit",             UINT64_C(0xe0a48d0be9a7453f))
#define RESOURCE_TYPE_LEVEL            STRING_ID_64("level",            UINT64_C(0x2a690fd348fe9ac5))
#define RESOURCE_TYPE_MATERIAL         STRING_ID_64("material",         UINT64_C(0xeac0b497876adedf))
#define RESOURCE_TYPE_MESH             STRING_ID_64("mesh",             UINT64_C(0x48ff313713a997a1))
#define RESOURCE_TYPE_MESH_SKELETON    STRING_ID_64("mesh_skeleton",    UINT64_C(0x2597bb272931eded))
#define RESOURCE_TYPE_MESH_ANIMATION   STRING_ID_64("mesh_animation",   UINT64_C(0x7369558b842d5314))
#define RESOURCE_TYPE_PACKAGE          STRING_ID_64("package",          UINT64_C(0xad9c6d9ed1e5e77a))
#define RESOURCE_TYPE_PHYSICS_CONFIG   STRING_ID_64("physics_config",   UINT64_C(0x72e3cc03787a11a1))
#define RESOURCE_TYPE_RENDER_CONFIG    STRING_ID_64("render_config",    UINT64_C(0x27862fe24795319c))
#define RESOURCE_TYPE_SCRIPT           STRING_ID_64("lua",              UINT64_C(0xa14e8dfa2cd117e2))
#define RESOURCE_TYPE_SHADER           STRING_ID_64("shader",           UINT64_C(0xcce8d5b5f5ae333f))
#define RESOURCE_TYPE_SOUND            STRING_ID_64("sound",            UINT64_C(0x90641b51c98b7aac))
#define RESOURCE_TYPE_SPRITE_ANIMATION STRING_ID_64("sprite_animation", UINT64_C(0x487e78e3f87f238d))
#define RESOURCE_TYPE_SPRITE           STRING_ID_64("sprite",           UINT64_C(0x8d5871f9ebdb651c))
#define RESOURCE_TYPE_TEXTURE          STRING_ID_64("texture",          UINT64_C(0xcd4238c6a0c69e32))

#define RESOURCE_NAME_INVALID StringId64(u64(0))
#define PACKAGE_RESOURCE_NONE RESOURCE_NAME_INVALID

#define RESOURCE_FULL_REBUILD_COUNT       u32(0) //!< How many times we required a full asset rebuild?
#define RESOURCE_VERSION(ver)             (RESOURCE_FULL_REBUILD_COUNT + ver)
#define RESOURCE_VERSION_STATE_MACHINE    RESOURCE_VERSION(8)
#define RESOURCE_VERSION_CONFIG           RESOURCE_VERSION(2)
#define RESOURCE_VERSION_FONT             RESOURCE_VERSION(1)
#define RESOURCE_VERSION_UNIT             RESOURCE_VERSION(18)
#define RESOURCE_VERSION_LEVEL            (RESOURCE_VERSION_UNIT + 6) //!< Level embeds UnitResource
#define RESOURCE_VERSION_MATERIAL         RESOURCE_VERSION(6)
#define RESOURCE_VERSION_MESH             RESOURCE_VERSION(9)
#define RESOURCE_VERSION_MESH_SKELETON    RESOURCE_VERSION(1)
#define RESOURCE_VERSION_MESH_ANIMATION   RESOURCE_VERSION(2)
#define RESOURCE_VERSION_PACKAGE          RESOURCE_VERSION(9)
#define RESOURCE_VERSION_PHYSICS_CONFIG   RESOURCE_VERSION(4)
#define RESOURCE_VERSION_RENDER_CONFIG    RESOURCE_VERSION(1)
#define RESOURCE_VERSION_SCRIPT           RESOURCE_VERSION(4)
#define RESOURCE_VERSION_SHADER           RESOURCE_VERSION(16)
#define RESOURCE_VERSION_SOUND            RESOURCE_VERSION(2)
#define RESOURCE_VERSION_SPRITE_ANIMATION RESOURCE_VERSION(3)
#define RESOURCE_VERSION_SPRITE           RESOURCE_VERSION(4)
#define RESOURCE_VERSION_TEXTURE          RESOURCE_VERSION(12)

#define RESOURCE_MAGIC                    u32(0x9B) //!< Non-UTF8 to early out on file type detection
#define RESOURCE_HEADER(version)          u32((version & 0x00ffffff) << 8 | RESOURCE_MAGIC)
/// @}
