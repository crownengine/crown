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
struct PhysicsResource;
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
#define RESOURCE_TYPE_PHYSICS          StringId64(0x5f7203c8f280dab8)
#define RESOURCE_TYPE_SCRIPT           StringId64(0xa14e8dfa2cd117e2)
#define RESOURCE_TYPE_SHADER           StringId64(0xcce8d5b5f5ae333f)
#define RESOURCE_TYPE_SOUND            StringId64(0x90641b51c98b7aac)
#define RESOURCE_TYPE_SPRITE_ANIMATION StringId64(0x487e78e3f87f238d)
#define RESOURCE_TYPE_SPRITE           StringId64(0x8d5871f9ebdb651c)
#define RESOURCE_TYPE_TEXTURE          StringId64(0xcd4238c6a0c69e32)
#define RESOURCE_TYPE_UNIT             StringId64(0xe0a48d0be9a7453f)

#define RESOURCE_VERSION_STATE_MACHINE    u32(1)
#define RESOURCE_VERSION_CONFIG           u32(1)
#define RESOURCE_VERSION_FONT             u32(1)
#define RESOURCE_VERSION_LEVEL            u32(1)
#define RESOURCE_VERSION_MATERIAL         u32(1)
#define RESOURCE_VERSION_MESH             u32(1)
#define RESOURCE_VERSION_PACKAGE          u32(1)
#define RESOURCE_VERSION_PHYSICS_CONFIG   u32(1)
#define RESOURCE_VERSION_PHYSICS          u32(1)
#define RESOURCE_VERSION_SCRIPT           u32(1)
#define RESOURCE_VERSION_SHADER           u32(1)
#define RESOURCE_VERSION_SOUND            u32(1)
#define RESOURCE_VERSION_SPRITE_ANIMATION u32(1)
#define RESOURCE_VERSION_SPRITE           u32(1)
#define RESOURCE_VERSION_TEXTURE          u32(1)
#define RESOURCE_VERSION_UNIT             u32(1)
/// @}
