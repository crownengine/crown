/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include "test_sdk.h"
namespace crown {
struct TextureData {StringId32 name;};
struct UniformData {StringId32 name;};
struct TextureHandle {u32 sampler_handle,texture_handle;};
struct UniformHandle {u32 uniform_handle;float values[16];};
struct MaterialResource {StringId32 shader;u32 num_textures=0,num_uniforms=0;TextureData*textures=nullptr;UniformData*uniforms=nullptr;};
struct MaterialTestData {TextureHandle textures[16]{};UniformHandle uniforms[16]{};};
struct MaterialManager {bgfx::UniformHandle _default_samplers[MATERIAL_MAX_TEXTURE_SLOTS];bgfx::TextureHandle _default_texture;};
struct TextureResource {bgfx::TextureHandle handle;};
namespace material_resource {
inline const TextureData *texture_data_array(const MaterialResource*r){return r->textures;}
inline const UniformData *uniform_data_array(const MaterialResource*r){return r->uniforms;}
inline const TextureHandle *texture_handle(const TextureData*,u32 i,const char*data){return &((const MaterialTestData*)data)->textures[i];}
inline const UniformHandle *uniform_handle(const UniformData*,u32 i,const char*data){return &((const MaterialTestData*)data)->uniforms[i];}
}
}
#include "world/material.h"
#include "material_bindings_under_test.h"
