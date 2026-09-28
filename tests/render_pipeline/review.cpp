/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "review_sdk.h"
#include "resource/render_config_resource.cpp"
#include "device/render_pipeline.cpp"
#include "device/render_frame.cpp"
#include "device/render_config_shaders.cpp"
#include <fstream>
#include <filesystem>
#include <iostream>
#include <sstream>

using namespace crown;
static u32 checks = 0;
#define CHECK(c) do { ++checks; if (!(c)) throw std::runtime_error("CHECK failed: " #c); } while (0)

static std::vector<u8> compile(const std::string &text, const std::map<std::string, std::string> &files = {})
{
	CompileOptions options;
	options.source = text;
	options.files = files;
	sjson::clear_error();
	if (render_config_resource_internal::compile(options) != 0)
		throw std::runtime_error("Compiler: " + options.last_error + " [source: " + text.substr(0, 160) + "]");
	const RenderConfigResource &r = *(const RenderConfigResource *)options.bytes.data();
	CHECK(r.version == RESOURCE_HEADER(RESOURCE_VERSION_RENDER_CONFIG));
	CHECK(options.bytes.size() == r.names_offset + r.names_size);
	return options.bytes;
}

static void rejects(const std::string &text, const char *message)
{
	CompileOptions options;
	options.source = text;
	options.files["base.render_config"] = "layers=[]";
	options.files["core/renderer/default.render_config"] = "layers=[]";
	sjson::clear_error();
	CHECK(render_config_resource_internal::compile(options) != 0);
	CHECK(options.last_error.find(message) != std::string::npos);
	CHECK(options.bytes.empty());
}

static void compiler_entry(const std::string &default_config)
{
	// These go through compile(), not only PipelineCompiler::parse().
	for (const char *field : { "conditions", "shader_layers", "shader_contexts" }) {
		rejects(std::string("pipeline=\"base\" ") + field + "=[]", "either an inline");
		rejects(std::string(field) + "=[]", "needs layers");
	}
	rejects(R"(pipeline="base" shader_layers=[{shader="mesh" layer="missing"}])", "either an inline");
	CompileOptions options;
	options.source = "layers=[] shaders=[]";
	sjson::clear_error();
	CHECK(render_config_resource_internal::compile(options) == 0);
	CHECK(options.reads.empty());

	const std::string base = R"(shaders=["shared" "base_only"] layers=[])";
	auto bytes = compile(R"(pipeline="base" shaders=["shared" "own_only" "shared"])", {{"base.render_config", base}});
	const auto &r = *(const RenderConfigResource *)bytes.data();
	CHECK(r.num_shader_libraries == 3);
	const u32 *libraries = render_config_resource::shader_libraries(&r);
	CHECK(std::string(render_config_resource::name(&r, libraries[0])) == "shared");
	CHECK(std::string(render_config_resource::name(&r, libraries[1])) == "own_only");
	CHECK(std::string(render_config_resource::name(&r, libraries[2])) == "base_only");
	options = CompileOptions();
	options.source = R"(pipeline="base" shaders=["shared"] )";
	options.files["base.render_config"] = base;
	sjson::clear_error();
	CHECK(render_config_resource_internal::compile(options) == 0);
	CHECK(options.requirements.size() == 2);
	CHECK(options.reads.size() == 1 && options.reads[0] == "base.render_config");

	bytes = compile("render_settings={bloom=false}", {{"core/renderer/default.render_config", default_config}});
	const auto &inherited = *(const RenderConfigResource *)bytes.data();
	CHECK(inherited.num_layers != 0 && inherited.num_shader_libraries == 4);
	CHECK(!(inherited.render_settings.flags & RenderSettingsFlags::BLOOM));
	rejects(R"(layers=[] shader_contexts=[{shader="mesh" context="shadow"}])", "missing 'program'");
	rejects(R"(layers=[] shader_contexts=[
 {shader="mesh" context="shadow" program="a"}
 {shader="mesh" context="shadow" program="b"}])", "Duplicate shader/context");
	rejects(R"(layers=[] shader_contexts=[{shader="mesh" context="shadow" program="a" layer="missing"}])", "Unknown pipeline property");
	bytes = compile(R"(layers=[] shader_contexts=[{shader="mesh" context="shadow" program="a" skinned_program="a_skin"}])");
	const auto &context_resource = *(const RenderConfigResource *)bytes.data();
	CHECK(context_resource.num_shader_contexts == 1);
	CHECK(render_config_resource::shader_contexts(&context_resource)[0].skinned_program == StringId32("a_skin"));
}

static void create(RenderPipeline &runtime, const std::vector<u8> &bytes)
{
	runtime.create((const RenderConfigResource *)bytes.data(), 128, 128, 0, 0, NULL, 0, NULL, 0, NULL);
}

static RenderBatch procedural()
{
	RenderBatch batch;
	batch.vertex_buffer = { 1 };
	batch.index_buffer = { 2 };
	batch.num_vertices = 3;
	batch.num_indices = 3;
	return batch;
}

static void view_defaults()
{
	auto bytes = compile(R"(layers=[
 {name="orthographic" transform="ortho_unit" draw={type="draw_visible" source="source" shader="probe"}}
 {name="identity" transform="none" draw={type="draw_visible" source="source" shader="probe"}}
 {name="camera" transform="camera" draw={type="draw_visible" source="source" shader="probe"}}
 {name="immediate" manual_rect=true draw={type="external" source="imgui"}}
])");
	Allocator a;
	ShaderManager shaders;
	RenderPipeline runtime(a, shaders);
	create(runtime, bytes);
	RenderFrame frame(a);
	RenderBatchView source(StringId32("source"), 0);
	source.camera = source.viewport = true;
	source.rect = { 16, 24, 32, 40 };
	source.view.t.x = 25;
	source.projection.x.x = 3;
	frame.add_batch(frame.add_view(source), procedural());
	auto context = render_frame::context(frame);
	Matrix4x4 camera = MATRIX4X4_IDENTITY;
	Matrix4x4 projection = MATRIX4X4_IDENTITY;
	camera.t.x = 42;
	projection.x.x = 7;
	for (u32 execution = 0; execution < 6; ++execution) {
		if (execution != 0) {
			frame.clear();
			source.camera = source.viewport = (execution % 2 == 0);
			frame.add_batch(frame.add_view(source), procedural());
		}
		bgfx::setViewRect(3, 3, 4, 5, 6);
		bgfx::setViewTransform(3, to_float_ptr(source.view), to_float_ptr(source.projection));
		bgfx::submits.clear();
		runtime.render(camera, projection, &context);
		CHECK(bgfx::submits.size() == 3);
		for (u16 i = 0; i < 3; ++i) {
			const auto &v = bgfx::views[i];
			CHECK(v.x == (source.viewport ? 16 : 0));
			CHECK(v.y == (source.viewport ? 24 : 0));
			CHECK(v.w == (source.viewport ? 32 : 128));
			CHECK(v.h == (source.viewport ? 40 : 128));
			CHECK(v.camera[12] == (source.camera ? 25 : i == 2 ? 42 : 0));
			CHECK(v.projection[0] == (source.camera ? 3 : i == 2 ? 7 : 1));
		}
		CHECK(bgfx::views[3].x == 3 && bgfx::views[3].h == 6);
		CHECK(bgfx::views[3].camera[12] == 25 && bgfx::views[3].projection[0] == 3);
		bgfx::frame();
	}
	runtime.destroy();
	bytes = compile(R"(layers=[{name="manual" manual_rect=true draw={type="draw_visible" source="source" shader="probe"}}])");
	create(runtime, bytes);
	bool rejected = false;
	try { runtime.render(camera, projection, &context); }
	catch (const std::runtime_error &e) { rejected = std::string(e.what()).find("manual_rect") != std::string::npos; }
	CHECK(rejected);
	runtime.destroy();
}

static u64 parameter_cost(u32 anonymous_count)
{
	auto bytes = compile(R"(layers=[{name="color" draw={type="draw_visible" source="source" shader="probe" parameters="lighting"}}])");
	Allocator a;
	ShaderManager shaders;
	RenderPipeline runtime(a, shaders);
	create(runtime, bytes);
	RenderFrame frame(a);
	auto handle = bgfx::createUniform("u_probe", bgfx::UniformType::Vec4);
	const Vector4 value = { 1, 2, 3, 4 };
	for (u32 i = 0; i < 7; ++i) {
		frame.add_uniform(StringId32("lighting"), handle, &value, sizeof(value));
		for (u32 j = 0; j < anonymous_count; ++j)
			frame.add_uniform(StringId32(), handle, &value, sizeof(value));
		frame.add_uniform(StringId32("other"), handle, &value, sizeof(value));
	}
	const u32 view = frame.add_view(RenderBatchView(StringId32("source"), 0));
	for (u32 i = 0; i < 100; ++i)
		frame.add_batch(view, procedural());
	const auto context = render_frame::context(frame);
	bgfx::submits.clear();
	array_index_reads = 0;
	runtime.render(MATRIX4X4_IDENTITY, MATRIX4X4_IDENTITY, &context);
	const u64 cost = array_index_reads;
	CHECK(bgfx::submits.size() == 100);
	CHECK(frame.parameter_sets.size() == 2);
	const RenderParameterSet absent = { UINT32_MAX, UINT32_MAX };
	const auto set = hash_map::get(frame.parameter_sets, StringId32("lighting"), absent);
	u32 count = 0;
	for (u32 i = set.first; i != UINT32_MAX; i = frame.uniforms[i].next) {
		CHECK(frame.uniforms[i].set == StringId32("lighting"));
		++count;
	}
	CHECK(count == 7);
	frame.clear();
	CHECK(frame.parameter_sets.empty());
	runtime.destroy();
	bgfx::destroy(handle);
	return cost;
}

static void context_programs()
{
	const std::string source = R"(layers=[
 {name="cascades" count=4 draw={type="draw_visible" source="sun" context="shadow" view_mode="views" shader="default_shadow" skinned_shader="default_skin"}}
 {name="local" draw={type="draw_visible" source="local" context="shadow" shader="default_shadow" skinned_shader="default_skin"}}
]
shader_contexts=[{shader="leaf" context="shadow" program="leaf_shadow" skinned_program="leaf_skin"}]
)";
	Allocator a;
	ShaderManager shaders;
	MaterialResource resource = {};
	resource.shader = StringId32("leaf");
	Material material(a);
	material._resource = &resource;
	material._shader = {};
	RenderFrame frame(a);
	for (u32 i = 0; i < 5; ++i) {
		const u32 v = frame.add_view(RenderBatchView(StringId32(i == 4 ? "local" : "sun"), i == 4 ? 0 : i));
		RenderBatch batch = procedural();
		batch.type = RenderDrawTypes::MESH;
		batch.material = &material;
		frame.add_batch(v, batch);
		batch.skinned = true;
		frame.add_batch(v, batch);
	}
	const auto context = render_frame::context(frame);
	auto bytes = compile(source);
	const auto original = bytes;
	RenderPipeline runtime(a, shaders);
	create(runtime, bytes);
	bgfx::submits.clear();
	runtime.render(MATRIX4X4_IDENTITY, MATRIX4X4_IDENTITY, &context);
	CHECK(bgfx::submits.size() == 10);
	for (u32 i = 0; i < 10; ++i) {
		CHECK(bgfx::submits[i].view == i / 2);
		CHECK(bgfx::submits[i].program.idx == shaders.shader(StringId32(i % 2 ? "leaf_skin" : "leaf_shadow")).program.idx);
	}
	CHECK(bytes == original);
	runtime.destroy();

	// Routing remains optional and exact; a program mapping does not imply it.
	bytes = compile(source + R"(shader_layers=[{shader="leaf" context="shadow" layer="cascades" index=2}])");
	create(runtime, bytes);
	bgfx::submits.clear();
	runtime.render(MATRIX4X4_IDENTITY, MATRIX4X4_IDENTITY, &context);
	CHECK(bgfx::submits.size() == 2);
	CHECK(bgfx::submits[0].view == 2 && bgfx::submits[1].view == 2);
	runtime.destroy();
	rejects(source + R"(shader_layers=[{shader="leaf" context="shadow" layer="cascades" program="ambiguous"}])", "not both");
}

static void shader_ownership()
{
	Allocator a;
	ResourceManager manager;
	const StringId64 first("first"), second("second"), third("third");
	// Startup package and an unrelated material/package already own resources.
	manager.references[first._id] = 1;
	manager.references[second._id] = 1;
	{
		RenderConfigShaders libraries(a, manager);
		auto old = compile(R"(layers=[] shaders=["first"])");
		libraries.acquire(*(const RenderConfigResource *)old.data());
		CHECK(manager.references[first._id] == 2 && manager.pending.empty());
		manager.unload(RESOURCE_TYPE_SHADER, first); // Loose boot package released.
		CHECK(manager.references[first._id] == 1);
		auto next = compile(R"(layers=[] shaders=["second" "third" "second"])");
		manager.events.clear();
		libraries.acquire(*(const RenderConfigResource *)next.data());
		CHECK(manager.references[first._id] == 1); // Old layout still protected.
		CHECK(manager.references[second._id] == 2 && manager.references[third._id] == 1);
		CHECK(manager.pending.empty()); // Safe to resolve/install new programs now.
		libraries.release_unused(*(const RenderConfigResource *)next.data());
		CHECK(manager.references[first._id] == 0);
		CHECK(manager.events.back() == "unload:" + std::to_string(first._id));
		const auto size = manager.events.size();
		for (u32 i = 0; i < 10; ++i) {
			libraries.acquire(*(const RenderConfigResource *)next.data());
			libraries.release_unused(*(const RenderConfigResource *)next.data());
		}
		CHECK(manager.events.size() == size);
		std::fill(old.begin(), old.end(), 0xa5);
		std::fill(next.begin(), next.end(), 0xa5); // No resource-blob pointers retained.
	}
	CHECK(manager.references[second._id] == 1); // Unrelated owner is intact.
	CHECK(manager.references[third._id] == 0);
	manager.unload(RESOURCE_TYPE_SHADER, second);
	CHECK(manager.references[second._id] == 0);
}

static void manual_fixtures(const std::filesystem::path &directory, const std::string &default_config)
{
	for (const char *name : { "00-empty.txt", "01-red.txt", "02-order.txt", "03-live-shader.txt", "04-offscreen.txt", "05-inherit.txt" }) {
		std::ifstream file(directory / name);
		CHECK(file.good());
		std::stringstream source;
		source << file.rdbuf();
		compile(source.str(), {{"core/renderer/default.render_config", default_config}});
	}
	for (const auto &entry : std::vector<std::pair<const char *, const char *>> {
		{"90-invalid-mixed.txt", "either an inline"},
		{"91-invalid-dangling.txt", "Unknown resource"},
		{"92-invalid-feedback.txt", "own color attachment"}
	}) {
		std::ifstream file(directory / entry.first);
		CHECK(file.good());
		std::stringstream source;
		source << file.rdbuf();
		rejects(source.str(), entry.second);
	}
}

static void empty_parameter_source()
{
	auto bytes = compile(R"(layers=[{name="empty" draw={type="draw_visible" source="source" shader="probe" parameters="not_needed" types=["mesh"]}}])");
	Allocator a;
	ShaderManager shaders;
	RenderPipeline runtime(a, shaders);
	create(runtime, bytes);
	RenderFrame frame(a);
	const u32 view = frame.add_view(RenderBatchView(StringId32("source"), 0));
	const auto context = render_frame::context(frame);
	bgfx::submits.clear();
	runtime.render(MATRIX4X4_IDENTITY, MATRIX4X4_IDENTITY, &context);
	CHECK(bgfx::submits.empty());
	frame.add_batch(view, procedural()); // Filtered out; still no set lookup.
	runtime.render(MATRIX4X4_IDENTITY, MATRIX4X4_IDENTITY, &context);
	CHECK(bgfx::submits.empty());
	runtime.destroy();
}

int main(int argc, char **argv)
{
	try {
		CHECK(argc == 3);
		std::ifstream file(argv[1]);
		CHECK(file.good());
		std::stringstream text;
		text << file.rdbuf();
		compiler_entry(text.str());
		manual_fixtures(argv[2], text.str());
		empty_parameter_source();
		view_defaults();
		const u64 small = parameter_cost(0);
		const u64 large = parameter_cost(1000);
		CHECK(small == large); // 7,000 unrelated uniforms must add no draw-time scan.
		std::cout << "Parameter lookup: " << small << " array reads in both cases.\n";
		context_programs();
		shader_ownership();
		CHECK(bgfx::textures.empty() && bgfx::framebuffers.empty() && bgfx::uniforms.empty());
		std::cout << checks << " PR-review checks passed (recording SDK).\n";
		return 0;
	} catch (const std::exception &e) {
		std::cerr << e.what() << '\n';
		return 1;
	}
}
