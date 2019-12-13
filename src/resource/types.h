/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

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
struct PackageResource;
struct PhysicsConfigResource;
struct ShaderResource;
struct ShapeResource;
struct SoundResource;
struct SpriteAnimationResource;
struct SpriteResource;
struct TextureResource;
struct UnitResource;

} // namespace crown

/// @addtogroup Resource
/// @{
#define RESOURCE_TYPE_STATE_MACHINE    StringId64(0xa486d4045106165c)
#define RESOURCE_TYPE_CONFIG           StringId64(0x82645835e6b73232)
#define RESOURCE_TYPE_FONT             StringId64(0x9efe0a916aae7880)
#define RESOURCE_TYPE_LEVEL            StringId64(0x2a690fd348fe9ac5)
#define RESOURCE_TYPE_MATERIAL         StringId64(0xeac0b497876adedf)
#define RESOURCE_TYPE_MESH             StringId64(0x48ff313713a997a1)
#define RESOURCE_TYPE_PACKAGE          StringId64(0xad9c6d9ed1e5e77a)
#define RESOURCE_TYPE_PHYSICS_CONFIG   StringId64(0x72e3cc03787a11a1)
#define RESOURCE_TYPE_SCRIPT           StringId64(0xa14e8dfa2cd117e2)
#define RESOURCE_TYPE_SHADER           StringId64(0xcce8d5b5f5ae333f)
#define RESOURCE_TYPE_SOUND            StringId64(0x90641b51c98b7aac)
#define RESOURCE_TYPE_SPRITE_ANIMATION StringId64(0x487e78e3f87f238d)
#define RESOURCE_TYPE_SPRITE           StringId64(0x8d5871f9ebdb651c)
#define RESOURCE_TYPE_TEXTURE          StringId64(0xcd4238c6a0c69e32)
#define RESOURCE_TYPE_UNIT             StringId64(0xe0a48d0be9a7453f)

#define RESOURCE_FULL_REBUILD_COUNT       u32(0) //!< How many times we required a full asset rebuild?
#define RESOURCE_VERSION(ver)             (RESOURCE_FULL_REBUILD_COUNT + ver)
#define RESOURCE_VERSION_STATE_MACHINE    RESOURCE_VERSION(2)
#define RESOURCE_VERSION_CONFIG           RESOURCE_VERSION(1)
#define RESOURCE_VERSION_FONT             RESOURCE_VERSION(1)
#define RESOURCE_VERSION_LEVEL            RESOURCE_VERSION(5)
#define RESOURCE_VERSION_MATERIAL         RESOURCE_VERSION(2)
#define RESOURCE_VERSION_MESH             RESOURCE_VERSION(1)
#define RESOURCE_VERSION_PACKAGE          RESOURCE_VERSION(3)
#define RESOURCE_VERSION_PHYSICS_CONFIG   RESOURCE_VERSION(1)
#define RESOURCE_VERSION_SCRIPT           RESOURCE_VERSION(1)
#define RESOURCE_VERSION_SHADER           RESOURCE_VERSION(1)
#define RESOURCE_VERSION_SOUND            RESOURCE_VERSION(1)
#define RESOURCE_VERSION_SPRITE_ANIMATION RESOURCE_VERSION(1)
#define RESOURCE_VERSION_SPRITE           RESOURCE_VERSION(2)
#define RESOURCE_VERSION_TEXTURE          RESOURCE_VERSION(1)
#define RESOURCE_VERSION_UNIT             RESOURCE_VERSION(3)

#define RESOURCE_MAGIC                    u32(0x9B) //!< Non-UTF8 to early out on file type detection
#define RESOURCE_HEADER(version)          u32((version & 0x00ffffff) << 8 | RESOURCE_MAGIC)
/// @}
