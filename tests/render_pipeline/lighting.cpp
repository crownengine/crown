// Exercise the real shader loader and native lighting binder together. The
// fixture models the stock mesh: six authored states, ten compiled samplers.
#define CROWN_TEST_REAL_SHADER_MANAGER
#include "lighting_sdk.h"
#include "world/shader_manager.cpp"
#include "resource/render_config_resource.inl"
#include "render_config_accessors_under_test.h"
#include "device/render_pipeline.cpp"
#include "device/render_frame.cpp"
#include "device/pipeline.cpp"
#include <iostream>

using namespace crown;
static u32 checks = 0;
#define CHECK(condition) do { ++checks; if (!(condition)) throw std::runtime_error("CHECK failed: " #condition); } while (0)

static const char *material_names[] = {
	"u_albedo_map", "u_normal_map", "u_metallic_map",
	"u_roughness_map", "u_ao_map", "u_emission_map"
};
static const char *lighting_names[] = {
	"u_cascaded_shadow_map", "u_local_lights_shadow_map",
	"u_lights_data", "u_lights_cookie_atlas"
};

template<class T> static void write(std::vector<u8> &bytes, const T &value)
{
	const u8 *p = (const u8 *)&value;
	bytes.insert(bytes.end(), p, p + sizeof(value));
}

static std::vector<u8> resource(StringId32 name, u32 vs, u32 fs, u32 authored = 6)
{
	std::vector<u8> bytes;
	write(bytes, u32(RESOURCE_HEADER(RESOURCE_VERSION_SHADER)));
	write(bytes, u32(1));
	write(bytes, name._id);
	write(bytes, u64(BGFX_STATE_DEFAULT));
	write(bytes, u32(0));
	write(bytes, u32(0));
	write(bytes, authored);
	for (u32 i = 0; i < authored; ++i) {
		write(bytes, StringId32(material_names[i])._id);
		write(bytes, u32(0));
		write(bytes, i);
	}
	write(bytes, u32(ShaderBackend::COUNT));
	for (u32 backend = 0; backend < ShaderBackend::COUNT; ++backend) {
		write(bytes, backend);
		write(bytes, u32(sizeof(vs)));
		write(bytes, vs);
		write(bytes, u32(sizeof(fs)));
		write(bytes, fs);
	}
	return bytes;
}

static u32 fixture(std::vector<bgfx::UniformDecl> uniforms)
{
	bgfx::shader_fixtures.push_back(uniforms);
	return u32(bgfx::shader_fixtures.size() - 1);
}

static void check_bindings(Pipeline &pipeline, const ShaderData &shader,
	const bgfx::TextureHandle *textures, const bgfx::UniformHandle *samplers, u32 mask)
{
	bgfx::discard();
	CHECK(pipeline.bind_lighting(shader));
	u32 seen = 0;
	for (const auto &binding : bgfx::texture_bindings) {
		CHECK(binding.stage >= CASCADED_SHADOW_MAP_SLOT && binding.stage <= LOCAL_LIGHTS_COOKIE_ATLAS_SLOT);
		const u32 index = binding.stage - CASCADED_SHADOW_MAP_SLOT;
		CHECK((seen & (1u << index)) == 0);
		CHECK(binding.texture.idx == textures[index].idx);
		CHECK(binding.sampler.idx == samplers[index].idx);
		CHECK(binding.flags == (index == 3 ? u32(BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP) : UINT32_MAX));
		seen |= 1u << index;
	}
	if (seen != mask)
		std::cerr << "Expected native texture mask " << mask << ", got " << seen << ".\n";
	CHECK(seen == mask);
	bgfx::discard();
}

static void test_renderer(bgfx::RendererType::Enum backend)
{
	bgfx::renderer = backend;
	ShaderManager manager(default_allocator());
	Pipeline pipeline(manager);
	bgfx::TextureHandle *aliases[] = {
		&pipeline._sun_shadow_map_texture, &pipeline._local_lights_shadow_map_texture,
		&pipeline._lights_data_texture, &pipeline._lights_cookie_atlas_texture
	};
	bgfx::TextureHandle textures[4];
	bgfx::UniformHandle samplers[4];
	for (u32 i = 0; i < 4; ++i) {
		textures[i] = bgfx::createTexture2D(1, 1, false, 1, bgfx::TextureFormat::RGBA8, 0);
		*aliases[i] = textures[i];
		samplers[i] = bgfx::createUniform(lighting_names[i], bgfx::UniformType::Sampler);
	}
	pipeline._u_cascaded_shadow_map = samplers[0];
	pipeline._u_local_lights_shadow_map = samplers[1];
	pipeline._lights_data = samplers[2];
	pipeline._u_lights_cookie_atlas = samplers[3];

	std::vector<bgfx::UniformDecl> material;
	for (const char *name : material_names)
		material.push_back({ name, bgfx::UniformType::Sampler });
	std::vector<bgfx::UniformDecl> lit = material;
	for (const char *name : lighting_names)
		lit.push_back({ name, bgfx::UniformType::Sampler });
	// Uniform count is not sampler count: do not truncate reflection to 16.
	for (u32 i = 0; i < 48; ++i)
		lit.insert(lit.begin(), { "u_test_" + std::to_string(i), bgfx::UniformType::Vec4 });
	const u32 empty = fixture({});
	const u32 vertex = fixture({ {"u_lights_data", bgfx::UniformType::Sampler} });
	const u32 lit_fragment = fixture(lit);
	const u32 unlit_fragment = fixture(material);
	const StringId32 name("lighting_regression");
	auto bytes = resource(name, vertex, lit_fragment);
	const auto original_bytes = bytes;
	manager.create_shaders(bytes.data());
	ShaderData shader = manager.shader(name);
	CHECK(shader.num_samplers == 6);
	for (u32 i = 0; i < shader.num_samplers; ++i)
		CHECK(shader.samplers[i].stage == i);
	// This is the regression: the old binder returns true but binds zero textures.
	check_bindings(pipeline, shader, textures, samplers, 15);
	const u32 reflection_queries = bgfx::reflection_queries;
	for (u32 i = 0; i < 64; ++i)
		check_bindings(pipeline, shader, textures, samplers, 15);
	CHECK(bgfx::reflection_queries == reflection_queries);
	CHECK(bytes == original_bytes);

	for (u32 i = 0; i < 4; ++i) {
		*aliases[i] = BGFX_INVALID_HANDLE;
		CHECK(!pipeline.bind_lighting(shader));
		CHECK(bgfx::bound.empty());
		*aliases[i] = textures[i];
	}
	// A second resource reference must reuse the same reflected program.
	manager.create_shaders(bytes.data());
	CHECK(bgfx::reflection_queries == reflection_queries);
	manager.destroy_shaders(bytes.data());
	check_bindings(pipeline, manager.shader(name), textures, samplers, 15);
	manager.destroy_shaders(bytes.data());

	// Reload the same name with an unlit permutation: no stale lighting inputs.
	bytes = resource(name, empty, unlit_fragment);
	manager.create_shaders(bytes.data());
	for (auto *alias : aliases)
		*alias = BGFX_INVALID_HANDLE;
	check_bindings(pipeline, manager.shader(name), textures, samplers, 0);
	manager.destroy_shaders(bytes.data());
	for (u32 i = 0; i < 4; ++i)
		*aliases[i] = textures[i];

	// Reflection works with no authored states, and with vertex-only sampling.
	bytes = resource(name, empty, lit_fragment, 0);
	manager.create_shaders(bytes.data());
	check_bindings(pipeline, manager.shader(name), textures, samplers, 15);
	manager.destroy_shaders(bytes.data());
	bytes = resource(name, vertex, empty, 0);
	manager.create_shaders(bytes.data());
	check_bindings(pipeline, manager.shader(name), textures, samplers, 4);
	manager.destroy_shaders(bytes.data());

	// A familiar name on a non-sampler uniform is not a texture dependency.
	const u32 not_sampler = fixture({ {"u_lights_data", bgfx::UniformType::Vec4} });
	// Release the application sampler while this intentionally different type exists.
	bgfx::destroy(samplers[2]);
	bytes = resource(name, empty, not_sampler, 0);
	manager.create_shaders(bytes.data());
	check_bindings(pipeline, manager.shader(name), textures, samplers, 0);
	manager.destroy_shaders(bytes.data());
	samplers[2] = bgfx::createUniform(lighting_names[2], bgfx::UniformType::Sampler);
	pipeline._lights_data = samplers[2];

	CHECK(manager._shader_map.empty() && manager._shader_ref_count.empty());
	CHECK(bgfx::programs.empty() && bgfx::shaders.empty());
	for (u32 i = 0; i < 4; ++i) {
		bgfx::destroy(textures[i]);
		bgfx::destroy(samplers[i]);
	}
	CHECK(bgfx::textures.empty() && bgfx::uniforms.empty());
}

int main()
{
	try {
		for (u32 backend = 0; backend < bgfx::RendererType::Count; ++backend)
			test_renderer(bgfx::RendererType::Enum(backend));
		std::cout << "Lighting binding regression: " << checks << " checks passed"
			<< " (CROWN_CAN_RELOAD=" << CROWN_CAN_RELOAD << ").\n";
		return 0;
	} catch (const std::exception &e) {
		std::cerr << e.what() << '\n';
		return 1;
	}
}
