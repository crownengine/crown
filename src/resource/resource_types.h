/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#define CONFIG_EXTENSION           "config"
#define FONT_EXTENSION             "font"
#define LEVEL_EXTENSION            "level"
#define MATERIAL_EXTENSION         "material"
#define MESH_EXTENSION             "mesh"
#define PACKAGE_EXTENSION          "package"
#define PHYSICS_CONFIG_EXTENSION   "physics_config"
#define PHYSICS_EXTENSION          "physics"
#define SCRIPT_EXTENSION           "lua"
#define SHADER_EXTENSION           "shader"
#define SOUND_EXTENSION            "sound"
#define SPRITE_ANIMATION_EXTENSION "sprite_animation"
#define SPRITE_EXTENSION           "sprite"
#define TEXTURE_EXTENSION          "texture"
#define UNIT_EXTENSION             "unit"

#define CONFIG_TYPE                StringId64(0x82645835e6b73232)
#define FONT_TYPE                  StringId64(0x9efe0a916aae7880)
#define LEVEL_TYPE                 StringId64(0x2a690fd348fe9ac5)
#define MATERIAL_TYPE              StringId64(0xeac0b497876adedf)
#define MESH_TYPE                  StringId64(0x48ff313713a997a1)
#define PACKAGE_TYPE               StringId64(0xad9c6d9ed1e5e77a)
#define PHYSICS_CONFIG_TYPE        StringId64(0x72e3cc03787a11a1)
#define PHYSICS_TYPE               StringId64(0x5f7203c8f280dab8)
#define SCRIPT_TYPE                StringId64(0xa14e8dfa2cd117e2)
#define SHADER_TYPE                StringId64(0xcce8d5b5f5ae333f)
#define SOUND_TYPE                 StringId64(0x90641b51c98b7aac)
#define SPRITE_ANIMATION_TYPE      StringId64(0x487e78e3f87f238d)
#define SPRITE_TYPE                StringId64(0x8d5871f9ebdb651c)
#define TEXTURE_TYPE               StringId64(0xcd4238c6a0c69e32)
#define UNIT_TYPE                  StringId64(0xe0a48d0be9a7453f)

#define CONFIG_VERSION             uint32_t(1)
#define FONT_VERSION               uint32_t(1)
#define LEVEL_VERSION              uint32_t(1)
#define MATERIAL_VERSION           uint32_t(1)
#define MESH_VERSION               uint32_t(1)
#define PACKAGE_VERSION            uint32_t(1)
#define PHYSICS_CONFIG_VERSION     uint32_t(1)
#define PHYSICS_VERSION            uint32_t(1)
#define SCRIPT_VERSION             uint32_t(1)
#define SHADER_VERSION             uint32_t(1)
#define SOUND_VERSION              uint32_t(1)
#define SPRITE_ANIMATION_VERSION   uint32_t(1)
#define SPRITE_VERSION             uint32_t(1)
#define TEXTURE_VERSION            uint32_t(1)
#define UNIT_VERSION               uint32_t(1)

namespace crown
{
class ResourceLoader;
class ResourceManager;
struct ResourcePackage;

struct ActorResource;
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
