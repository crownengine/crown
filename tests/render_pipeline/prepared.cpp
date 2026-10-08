/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

// Production compiler/executor/material binding and native gathering helpers;
// the supporting engine APIs and GPU are recording test doubles.
#include "test_sdk.h"
#include "resource/render_config_resource.inl"
#include "render_config_accessors_under_test.h"
namespace crown { namespace render_config_resource_internal {
#include "pipeline_compiler_under_test.h"
}}
#include "device/render_pipeline.cpp"
#include "device/render_frame.cpp"
#include "device/pipeline.cpp"
#include <fstream>
#include <iostream>

using namespace crown;
static u32 checks = 0;
#define CHECK(c) do { ++checks; if (!(c)) throw std::runtime_error("CHECK failed: " #c); } while (0)

template<class F> static void fails(F function, const char *message)
{
	try { function(); } catch (const std::runtime_error &error) {
		if (std::string(error.what()).find(message) == std::string::npos)
			std::cerr << "Expected error containing '" << message << "', got '" << error.what() << "'.\n";
		CHECK(std::string(error.what()).find(message) != std::string::npos);
		return;
	}
	CHECK(false);
}

static std::vector<u8> compile(const std::string &text)
{
	Allocator allocator;
	JsonObject object(allocator);
	sjson::clear_error();
	sjson::parse(object, text.c_str());
	CompileOptions options;
	render_config_resource_internal::PipelineCompiler compiler(allocator, options);
	if (compiler.parse(object) != 0)
		throw std::runtime_error(options.last_error);
	RenderConfigResource header = {};
	header.version = RESOURCE_HEADER(RESOURCE_VERSION_RENDER_CONFIG);
	compiler.layout(header);
	options.write(header.version);
	options.write(header.render_settings);
	compiler.write(header);
	CHECK(options.bytes.size() == header.names_offset + header.names_size);
	return options.bytes;
}

static RenderSettings settings(bool cookies = true)
{
	RenderSettings value = {};
	value.flags = RenderSettingsFlags::SUN_SHADOWS | RenderSettingsFlags::LOCAL_LIGHTS_SHADOWS
		| RenderSettingsFlags::BLOOM | RenderSettingsFlags::SELECTION | RenderSettingsFlags::MSAA;
	if (cookies) value.flags |= RenderSettingsFlags::LIGHTS_COOKIE;
	value.msaa_quality = 2;
	value.sun_shadow_map_size = { 256, 256 };
	value.local_lights_shadow_map_size = { 256, 256 };
	value.lights_cookie_atlas_size = { 128, 128 };
	return value;
}

static void reset_records()
{
	bgfx::submits.clear(); bgfx::touches.clear(); bgfx::uploads.clear();
	bgfx::discard();
}

struct MaterialFixture
{
	MaterialResource resource;
	MaterialTestData data;
	TextureData textures[6];
	UniformData values[1];
	MaterialManager manager;
	Material material;
	bgfx::TextureHandle texture;
	bgfx::UniformHandle samplers[6], uniform;

	explicit MaterialFixture(const char *name = "surface") : material(default_allocator())
	{
		static const char *names[] = { "u_albedo_map", "u_normal_map", "u_metallic_map", "u_roughness_map", "u_ao_map", "u_emission_map" };
		resource.shader = StringId32(name);
		resource.num_textures = 6; resource.textures = textures;
		resource.num_uniforms = 1; resource.uniforms = values;
		texture = bgfx::createTexture2D(1, 1, false, 1, bgfx::TextureFormat::RGBA8, 0);
		material._resource = &resource;
		material._data = (char *)&data;
		material._material_manager = &manager;
		material._resource_manager = nullptr;
		material._shader = {};
		material._shader.program = { 27000 };
		material._shader.state = 123;
		material._shader.num_samplers = 6;
		for (u32 i = 0; i < 6; ++i) {
			textures[i].name = StringId32(names[i]);
			samplers[i] = bgfx::createUniform(names[i], bgfx::UniformType::Sampler);
			data.textures[i] = { samplers[i].idx, texture.idx };
			material._shader.samplers[i] = { textures[i].name._id, 100 + i, i };
		}
		uniform = bgfx::createUniform("u_material_value", bgfx::UniformType::Vec4);
		values[0].name = StringId32("u_material_value");
		data.uniforms[0].uniform_handle = uniform.idx;
		data.uniforms[0].values[0] = 19;
	}

	~MaterialFixture()
	{
		bgfx::destroy(texture);
		for (auto sampler : samplers) bgfx::destroy(sampler);
		bgfx::destroy(uniform);
	}
};

static RenderBatch mesh_batch(MaterialFixture &fixture, u32 object, bool selected = false, bool skinned = false)
{
	RenderBatch batch;
	batch.type = RenderDrawTypes::MESH;
	batch.material = &fixture.material;
	batch.vertex_buffer = { 9 }; batch.index_buffer = { 10 };
	batch.first_index = object * 3; batch.num_indices = 3;
	batch.transform = bgfx::setTransform(to_float_ptr(MATRIX4X4_IDENTITY));
	batch.object_id = object;
	batch.depth = object * 17;
	batch.flags = selected ? RenderBatchFlags::SELECTED : 0;
	batch.skinned = skinned;
	bgfx::discard();
	return batch;
}

// Rename references by schema position, not by textual replacement of words:
// source IDs, condition names and shader uniforms are deliberately unchanged.
struct Edit { size_t begin, count; std::string value; };
static void renamed_value(std::vector<Edit> &edits, const std::string &text, const char *value)
{
	Allocator a; DynamicString name(a); sjson::parse_string(name, value);
	const char *end = value; sjson::skip(end);
	edits.push_back({size_t(value-text.c_str()), size_t(end-value), "\"renamed_" + name + "\""});
}
static void renamed_ref(std::vector<Edit> &edits, const std::string &text, const char *value)
{
	if (sjson::type(value) == JsonValueType::STRING) renamed_value(edits,text,value);
	else { Allocator a; JsonObject ref(a);sjson::parse_object(ref,value);renamed_value(edits,text,ref["resource"]); }
}
static void renamed_bindings(std::vector<Edit> &edits,const std::string &text,const JsonObject &obj)
{
	Allocator a;
	if (json_object::has(obj,"render_targets")) {
		JsonArray targets(a);sjson::parse_array(targets,obj["render_targets"]);
		for (const char *target : targets) renamed_ref(edits,text,target);
	}
	if (json_object::has(obj,"depth_stencil_target")) renamed_ref(edits,text,obj["depth_stencil_target"]);
	if (json_object::has(obj,"resource")) renamed_ref(edits,text,obj["resource"]);
	if (json_object::has(obj,"inputs")) {
		JsonArray inputs(a);sjson::parse_array(inputs,obj["inputs"]);
		for (const char *input : inputs) {
			JsonObject b(a);sjson::parse_object(b,input);renamed_ref(edits,text,b["texture"]);
			if (json_object::has(b,"fallback")) renamed_ref(edits,text,b["fallback"]);
		}
	}
}
static std::string rename_config(const std::string &text)
{
	Allocator a; JsonObject root(a);sjson::parse(root,text.c_str());std::vector<Edit> edits;
	for (const char *section : {"global_resources","layers","resource_generators"}) {
		JsonArray entries(a);sjson::parse_array(entries,root[section]);
		for (const char *entry : entries) {
			JsonObject obj(a);sjson::parse_object(obj,entry);renamed_value(edits,text,obj["name"]);
			if (std::string(section)=="layers") {
				renamed_bindings(edits,text,obj);
				if (json_object::has(obj,"resource_generator")) renamed_value(edits,text,obj["resource_generator"]);
				if (json_object::has(obj,"draw")) {JsonObject draw(a);sjson::parse_object(draw,obj["draw"]);renamed_bindings(edits,text,draw);}
			} else if (std::string(section)=="resource_generators") {
				JsonArray mods(a);sjson::parse_array(mods,obj["modifiers"]);
				for(const char *mod:mods){JsonObject m(a);sjson::parse_object(m,mod);renamed_bindings(edits,text,m);}
			}
		}
	}
	std::sort(edits.begin(),edits.end(),[](const Edit &a,const Edit &b){return a.begin>b.begin;});
	std::string result=text;for(const Edit&e:edits)result.replace(e.begin,e.count,e.value);
	return result;
}

static void standard_frame(Pipeline &pipeline, RenderFrame &frame, MaterialFixture &material)
{
	frame.clear();
	for (const char *name : {"main_camera","sun_shadows","local_shadows","light_cookies","local_shadow_stencil"})
		frame.add_source(StringId32(name));
	pipeline.prepare_local_lights_stencil(frame, 32, 2);
	pipeline.begin_light_cookie_atlas();
	u32 cookie=0;
	if(pipeline.light_cookies_enabled()) CHECK(pipeline.add_light_cookie(frame,cookie,material.texture,16,16).z>0);
	for(u32 i=0;i<4;++i) {
		RenderBatchView source(StringId32("sun_shadows"),i);
		source.camera=source.viewport=true;source.view.t.x=float(i+1);
		source.rect={float((i%2)*128),float((i/2)*128),128,128};
		u32 view=frame.add_view(source);frame.add_batch(view,mesh_batch(material,10+i));
	}
	RenderBatchView camera(StringId32("main_camera"),0);
	u32 main=frame.add_view(camera);
	frame.add_batch(main,mesh_batch(material,21,true));
	frame.add_batch(main,mesh_batch(material,22,false,true));
	RenderBatch sprite=mesh_batch(material,23,true);sprite.type=RenderDrawTypes::SPRITE;sprite.group=3;
	frame.add_batch(main,sprite);
	Vector4 parameter={1,2,3,4};
	frame.add_uniform(StringId32("forward_lighting"),pipeline._lights_num,&parameter,sizeof(parameter));
	std::array<Vector4,LIGHT_SIZE> lights={};lights[0].x=7;
	frame.add_upload(StringId32("lights"),RenderResourceFormat::RGBA32F,LIGHT_SIZE,1,lights.data(),sizeof(lights));
	lights[0].x=99; // Upload must not retain this pointer.
}

static void integration(const std::string &text)
{
	for(bool cookies:{false,true}) {
		ShaderManager shaders;
		MaterialFixture material;
		Pipeline pipeline(shaders);
		RenderFrame frame(default_allocator());
		for(const std::string &configuration : {text,rename_config(text)}) {
			auto bytes=compile(configuration);auto original=bytes;
			pipeline.create(640,480,settings(cookies),(const RenderConfigResource*)bytes.data());
			CHECK(pipeline.sun_shadows_enabled());CHECK(pipeline.local_shadows_enabled());
			CHECK(pipeline.light_cookies_enabled()==cookies);
			reset_records();standard_frame(pipeline,frame,material);
			CHECK(bgfx::submits.empty()&&bgfx::uploads.empty());
			std::vector<u8> snapshot((const u8*)array::begin(frame.batches),(const u8*)array::begin(frame.batches)+frame.batches.size()*sizeof(RenderBatch));
			pipeline.render(640,480,MATRIX4X4_IDENTITY,MATRIX4X4_IDENTITY,&frame);
			CHECK(bgfx::uploads.size()==1);CHECK(bgfx::uploads[0].texture.idx==pipeline._lights_data_texture.idx);
			float uploaded;memcpy(&uploaded,bgfx::uploads[0].bytes.data(),sizeof(uploaded));CHECK(uploaded==7);
			CHECK(bytes==original);CHECK(memcmp(snapshot.data(),array::begin(frame.batches),snapshot.size())==0);
			u32 camera_draws=0,selection_draws=0,shadow_draws=0;
			for(const auto &draw:bgfx::submits) {
				const std::string &name=bgfx::views.at(draw.view).name;
				if(name=="mesh"||name=="renamed_mesh") {
					++camera_draws;CHECK(draw.bindings.size()==10);
					for(u32 i=0;i<6;++i){CHECK(draw.bindings[i].stage==i);CHECK(draw.bindings[i].flags==100+i);}
					for(u32 i=0;i<4;++i)CHECK(draw.bindings[6+i].stage==10+i);
					CHECK(draw.bindings[6].texture.idx==pipeline._sun_shadow_map_texture.idx);
					CHECK(draw.bindings[8].texture.idx==pipeline._lights_data_texture.idx);
					CHECK(bgfx::textures.at(draw.bindings[9].texture.idx).name ==
						std::string(name == "mesh" ? "" : "renamed_") + (cookies ? "lights_cookie_atlas" : "lights_cookie_fallback"));
					CHECK(bool(draw.state.state&BGFX_STATE_MSAA));
					CHECK(draw.state.first_index==draw.depth/17*3);
					CHECK(draw.state.values.count(pipeline._lights_num.idx));
				} else if(name=="selection"||name=="renamed_selection") {
					++selection_draws;CHECK(draw.bindings.empty());
					CHECK(draw.state.values.count(pipeline._unit_id.idx));
					u32 id;memcpy(&id,draw.state.values.at(pipeline._unit_id.idx).data(),4);CHECK(id==21||id==23);
				} else if(name=="sm_cascade"||name=="renamed_sm_cascade") ++shadow_draws;
			}
			CHECK(camera_draws==2&&selection_draws==2&&shadow_draws==4);
			CHECK(material.material._shader.program.idx==27000);
			pipeline.destroy();
		}
	}
}

static void routing_and_reuse()
{
	const std::string config=R"(
conditions=["on"]
layers=[
 {name="reserved" count=260}
 {name="first" draw={type="draw_visible" source="camera" context="color" types=["mesh"]} }
 {name="second" draw={type="draw_visible" source="camera" context="depth" shader="depth" skinned_shader="depth_skin"} }
 {name="selected" if=["on"] draw={type="draw_visible" source="camera" context="selection" filter="selected" shader="id" skinned_shader="id_skin" object_id_uniform="u_id"} }
]
shader_layers=[
 {shader="surface" context="color" layer="first" program="color" skinned_program="color_skin"}
 {shader="surface" context="depth" layer="second" program="routed_depth" skinned_program="routed_skin"}
]
)";
	auto bytes=compile(config);ShaderManager shaders;Pipeline pipeline(shaders);MaterialFixture material;
	pipeline.create(320,240,settings(),(const RenderConfigResource*)bytes.data());
	pipeline._render_pipeline.set_condition(StringId32("on"),true);
	RenderFrame frame(default_allocator());u32 view=frame.add_view(RenderBatchView(StringId32("camera"),0));
	frame.add_batch(view,mesh_batch(material,42,true));
	frame.add_batch(view,mesh_batch(material,43,false,true));
	reset_records();pipeline.render(320,240,MATRIX4X4_IDENTITY,MATRIX4X4_IDENTITY,&frame);
	CHECK(bgfx::submits.size()==5);
	const char *programs[]={"color","color_skin","routed_depth","routed_skin","id"};
	for(u32 i=0;i<5;++i){CHECK(bgfx::submits[i].view>255);CHECK(bgfx::submits[i].program.idx==StringId32(programs[i])._id%60000);}
	CHECK(bgfx::submits[0].state.transform==bgfx::submits[2].state.transform);
	CHECK(bgfx::submits[1].state.transform==bgfx::submits[3].state.transform);
	CHECK(bgfx::submits[0].state.first_index==bgfx::submits[2].state.first_index);
	pipeline._render_pipeline.set_condition(StringId32("on"),false);
	reset_records();pipeline.render(320,240,MATRIX4X4_IDENTITY,MATRIX4X4_IDENTITY,&frame);CHECK(bgfx::submits.size()==4);
	pipeline.destroy();
	// A role-looking name without a draw operation does not render anything.
	bytes=compile("layers=[{name=\"mesh\"}]");pipeline.create(8,8,settings(),(const RenderConfigResource*)bytes.data());
	reset_records();pipeline.render(8,8,MATRIX4X4_IDENTITY,MATRIX4X4_IDENTITY,&frame);CHECK(bgfx::submits.empty());pipeline.destroy();
	bytes=compile("layers=[]");pipeline.create(8,8,settings(),(const RenderConfigResource*)bytes.data());
	reset_records();pipeline.render(8,8,MATRIX4X4_IDENTITY,MATRIX4X4_IDENTITY,&frame);CHECK(bgfx::submits.empty());CHECK(bgfx::touches==std::vector<u16>{0});pipeline.destroy();
}

static void malformed()
{
	for(const char *text : {
		"layers=[{name=\"a\" draw={type=\"draw_visible\"}}]",
		"layers=[{name=\"a\" draw={type=\"draw_visible\" source=\"x\" types=[\"light\"]}}]",
		"layers=[{name=\"a\" draw={type=\"draw_visible\" source=\"x\" types=[\"mesh\" \"mesh\"]}}]",
		"layers=[{name=\"a\" count=2 draw={type=\"draw_visible\" source=\"x\"}}]",
		"layers=[{name=\"a\" draw={type=\"draw_visible\" source=\"x\" skinned_shader=\"s\"}}]",
		"layers=[{name=\"a\" draw={type=\"upload\" source=\"x\"}}]",
		"layers=[{name=\"a\" resource_generator=\"g\" draw={type=\"draw_visible\" source=\"x\"}}] resource_generators=[{name=\"g\" modifiers=[]}]",
		"layers=[{name=\"a\"}] shader_layers=[{shader=\"s\" layer=\"a\"}]",
		"layers=[{name=\"a\" draw={type=\"external\" source=\"x\"}} {name=\"b\" draw={type=\"external\" source=\"x\"}}]"
	}) {
		bool failed=false;try{compile(text);}catch(const std::runtime_error&){failed=true;}CHECK(failed);
	}
	ShaderManager shaders;Pipeline pipeline(shaders);RenderFrame frame(default_allocator());
	auto bytes=compile("layers=[{name=\"a\" draw={type=\"draw_visible\" source=\"missing\"}}]");
	pipeline.create(8,8,settings(),(const RenderConfigResource*)bytes.data());
	fails([&]{pipeline.render(8,8,MATRIX4X4_IDENTITY,MATRIX4X4_IDENTITY,&frame);},"Unknown prepared");
	frame.add_source(StringId32("missing"));reset_records();pipeline.render(8,8,MATRIX4X4_IDENTITY,MATRIX4X4_IDENTITY,&frame);CHECK(bgfx::submits.empty());
	pipeline.destroy();
}


static void transient_reuse()
{
	// Snapshot transient buffer headers, then reuse the geometry in a second
	// context without allocating/writing it again. Subsequent allocations must
	// not overwrite either buffer (the old test SDK used one shared pointer).
	auto bytes=compile(R"(layers=[
 {name="sprites" count=2 draw={type="draw_visible" source="sprites" types=["sprite"] view_mode="groups"}}
 {name="ids" draw={type="draw_visible" source="sprites" context="selection" filter="selected" shader="selection" object_id_uniform="u_id"}}
])");
	ShaderManager shaders;Pipeline pipeline(shaders);MaterialFixture material;
	pipeline.create(64,64,settings(),(const RenderConfigResource*)bytes.data());
	RenderFrame frame(default_allocator());u32 source=frame.add_view(RenderBatchView(StringId32("sprites"),0));
	bgfx::VertexLayout layout;layout.begin();layout.add(bgfx::Attrib::Position,3,bgfx::AttribType::Float);layout.end();
	bgfx::TransientVertexBuffer vertices;bgfx::allocTransientVertexBuffer(&vertices,8,layout);
	bgfx::TransientIndexBuffer indices;bgfx::allocTransientIndexBuffer(&indices,12);
	memset(vertices.data,41,96);memset(indices.data,17,24);
	for(u32 i=0;i<2;++i) {
		RenderBatch batch=mesh_batch(material,50+i,i==1);
		batch.type=RenderDrawTypes::SPRITE;batch.transient=true;
		batch.transient_vertices=vertices;batch.transient_indices=indices;
		batch.num_vertices=8;batch.first_index=i*6;batch.num_indices=6;batch.group=i;
		frame.add_batch(source,batch);
	}
	bgfx::TransientVertexBuffer later;bgfx::allocTransientVertexBuffer(&later,8,layout);memset(later.data,0,96);
	const auto before=bgfx::transient_used;
	reset_records();pipeline.render(64,64,MATRIX4X4_IDENTITY,MATRIX4X4_IDENTITY,&frame);
	CHECK(bgfx::submits.size()==3);CHECK(bgfx::transient_used==before);
	for(const auto &draw:bgfx::submits) {
		CHECK(draw.state.vertices==vertices.data);CHECK(draw.state.indices==indices.data);
		CHECK(draw.state.vertices[0]==41&&draw.state.indices[0]==17);
		CHECK(draw.state.num_vertices==8&&draw.state.num_indices==6);
	}
	CHECK(bgfx::submits[0].state.first_index==0);CHECK(bgfx::submits[1].state.first_index==6);
	CHECK(bgfx::submits[2].state.first_index==6);
	pipeline.destroy();
}

static void invalid_execution()
{
	MaterialFixture material;ShaderManager shaders;Pipeline pipeline(shaders);RenderFrame frame(default_allocator());
	auto bytes=compile("layers=[{name=\"a\" draw={type=\"draw_visible\" source=\"camera\" shader=\"depth\"}}]");
	pipeline.create(8,8,settings(),(const RenderConfigResource*)bytes.data());
	u32 view=frame.add_view(RenderBatchView(StringId32("camera"),0));
	frame.add_batch(view,mesh_batch(material,1,false,true));
	fails([&]{pipeline.render(8,8,MATRIX4X4_IDENTITY,MATRIX4X4_IDENTITY,&frame);},"Skinned batch needs");
	pipeline.destroy();frame.clear();
	bytes=compile("layers=[{name=\"a\" draw={type=\"unregistered\" source=\"camera\"}}]");
	pipeline.create(8,8,settings(),(const RenderConfigResource*)bytes.data());
	fails([&]{pipeline.render(8,8,MATRIX4X4_IDENTITY,MATRIX4X4_IDENTITY,&frame);},"Unregistered frame operation");
	pipeline.destroy();
	bytes=compile(R"(global_resources=[{name="data" type="texture" format="RGBA32F" width=2 height=1}]
layers=[{name="upload" draw={type="upload" source="data" resource="data"}}])");
	pipeline.create(8,8,settings(),(const RenderConfigResource*)bytes.data());
	float pixels[8]={};frame.add_upload(StringId32("data"),RenderResourceFormat::RGBA32F,2,1,pixels,4);
	fails([&]{pipeline.render(8,8,MATRIX4X4_IDENTITY,MATRIX4X4_IDENTITY,&frame);},"byte count");
	frame.clear();frame.add_upload(StringId32("data"),RenderResourceFormat::RGBA32F,0,1,nullptr,0);
	reset_records();pipeline.render(8,8,MATRIX4X4_IDENTITY,MATRIX4X4_IDENTITY,&frame);CHECK(bgfx::uploads.empty());
	pipeline.destroy();
	for (const char *invalid : {
		R"(layers=[{name="a" draw={type="external" source="gui" shader="ignored"}}])",
		R"(layers=[{name="a" count=2 draw={type="external" source="gui"}}])",
		R"(global_resources=[{name="a" format="RGBA8"} {name="b" format="RGBA8"}]
 layers=[{name="a" render_targets=["b"] draw={type="draw_visible" source="camera" inputs=[{texture="a" fallback="b" sampler="s"}]}}])",
		R"(global_resources=[{name="a" format="RGBA8"}]
 layers=[{name="a" draw={type="draw_visible" source="camera" object_id_uniform="s" inputs=[{texture="a" sampler="s"}]}}])"
	}) {
		bool rejected=false;try{compile(invalid);}catch(const std::runtime_error&){rejected=true;}CHECK(rejected);
	}
}

static std::string remove_layer(const std::string &text, u32 index)
{
	Allocator a;JsonObject root(a);sjson::parse(root,text.c_str());JsonArray layers(a);sjson::parse_array(layers,root["layers"]);
	const char *start=layers[index],*end=start;sjson::skip(end);
	std::string result=text;result.erase(size_t(start-text.c_str()),size_t(end-start));return result;
}

static void deletion_execution(const std::string &text)
{
	ShaderManager shaders;Pipeline pipeline(shaders);MaterialFixture material;RenderFrame frame(default_allocator());
	for(u32 i=0;i<21;++i) {
		auto bytes=compile(remove_layer(text,i));
		pipeline.create(640,480,settings(),(const RenderConfigResource*)bytes.data());
		reset_records();standard_frame(pipeline,frame,material);
		CHECK(bgfx::submits.empty()&&bgfx::uploads.empty());
		pipeline.render(640,480,MATRIX4X4_IDENTITY,MATRIX4X4_IDENTITY,&frame);
		for(const auto &draw:bgfx::submits) CHECK(draw.view<pipeline._render_pipeline._views.size());
		pipeline.destroy();
	}
	// Transient failure cannot produce a cookie rectangle that points to an
	// unrendered tile; it must leave the source index unchanged.
	auto bytes=compile(text);pipeline.create(640,480,settings(),(const RenderConfigResource*)bytes.data());
	frame.clear();pipeline.begin_light_cookie_atlas();u32 cookie=0;bgfx::transient_available=false;
	const auto rectangle=pipeline.add_light_cookie(frame,cookie,material.texture,16,16);
	bgfx::transient_available=true;CHECK(rectangle.z==0&&cookie==0&&frame.batches.empty());
	pipeline.begin_light_cookie_atlas();CHECK(pipeline.add_light_cookie(frame,cookie,material.texture,16,16).z>0);CHECK(cookie==1);
	pipeline.destroy();
}

// Minimal native scene storage. Gathering implementations below are extracted
// verbatim from render_world.cpp; culling/skinning mathematics remain doubles.
namespace crown {
struct UnitId {u32 _idx;};struct TransformId {u32 i;};
struct RenderableFlags {enum Enum {VISIBLE=1,SHADOW_CASTER=8,SELECTED=32};};
static Vector3 translation(const Matrix4x4&m){return {m.t.x,m.t.y,m.t.z};}
static Vector3 operator*(Vector3 v,const Matrix4x4&m){return {v.x+m.t.x,v.y+m.t.y,v.z+m.t.z};}
struct AnimationSkeletonInstance {u32 num_bones;UnitId *bone_lookup;Matrix4x4 *bones,*offsets;};
struct SceneGraph {u32 reads=0;TransformId instance(UnitId u){return {u._idx};}Matrix4x4 world_pose(TransformId t){++reads;Matrix4x4 m=MATRIX4X4_IDENTITY;m.t.x=float(t.i);return m;}};
namespace mesh_animation {static Matrix4x4 skinning_transform(const Matrix4x4&,const Matrix4x4 &world){return world;}}
struct RenderWorld {
 struct MeshManager {
  struct MaterialBinding {Material*material;const MaterialResource*resource;u32 index_offset,num_indices,next;};
  struct MeshData {bgfx::VertexBufferHandle vbh;bgfx::IndexBufferHandle ibh;};
  struct Data {UnitId *unit;MeshData *mesh;MaterialBinding *bindings;Matrix4x4 *world;const AnimationSkeletonInstance **skeleton;u32 *flags,*draw_cache;} _data;
  RenderWorld *_render_world;
  u32 set_instance_data(u32,SceneGraph&,u32=0,u32=UINT32_MAX);
 } _mesh_manager;
 struct Culling {Array<u32> id,render;explicit Culling(Allocator&a):id(a),render(a){}} _cullable_shadow_casters;
 RenderFrame _render_frame;SceneGraph *_scene_graph;Pipeline *_pipeline;
 explicit RenderWorld(SceneGraph&scene,Pipeline&pipeline):_cullable_shadow_casters(default_allocator()),_render_frame(default_allocator()),_scene_graph(&scene),_pipeline(&pipeline){_mesh_manager._render_world=this;}
 void gather_shadow_casters(const RenderBatchView&,u32=UINT32_MAX);
};
#include "native_gathering_under_test.h"
}

static void native_gathering()
{
	MaterialFixture material;
	ShaderManager shaders;Pipeline pipeline(shaders);
	auto bytes=compile(R"(layers=[
 {name="color" draw={type="draw_visible" source="camera"}}
 {name="depth" draw={type="draw_visible" source="camera" context="depth" shader="depth" skinned_shader="depth_skin"}}
 {name="cascades" count=2 manual_rect=true draw={type="draw_visible" source="shadows" context="shadow" view_mode="views" shader="shadow" skinned_shader="shadow_skin"}}
])");
	pipeline.create(64,32,settings(),(const RenderConfigResource*)bytes.data());
	pipeline.begin_frame();
	SceneGraph scene;RenderWorld world(scene,pipeline);
	UnitId units[]={{10},{20}},bone_units[]={{30},{31}};
	Matrix4x4 poses[]={MATRIX4X4_IDENTITY,MATRIX4X4_IDENTITY},bones[2],offsets[2]={MATRIX4X4_IDENTITY,MATRIX4X4_IDENTITY};
	AnimationSkeletonInstance skeleton={2,bone_units,bones,offsets};
	const AnimationSkeletonInstance *skeletons[]={&skeleton,nullptr};
	RenderWorld::MeshManager::MeshData geometry[]={{{1},{2}},{{3},{4}}};
	RenderWorld::MeshManager::MaterialBinding bindings[]={
	 {&material.material,&material.resource,0,3,2},
	 {&material.material,&material.resource,9,6,UINT32_MAX},
	 {&material.material,&material.resource,3,6,UINT32_MAX}
	};
	u32 flags[]={RenderableFlags::VISIBLE|RenderableFlags::SHADOW_CASTER,RenderableFlags::VISIBLE|RenderableFlags::SHADOW_CASTER};
	u32 cache[]={UINT32_MAX,UINT32_MAX};
	world._mesh_manager._data={units,geometry,bindings,poses,skeletons,flags,cache};
	u32 camera=world._render_frame.add_view(RenderBatchView(StringId32("camera"),0));
	gather_mesh(world._mesh_manager,0,UnitId{99},RenderableFlags::SELECTED,scene,world._render_frame,camera,MATRIX4X4_IDENTITY);
	CHECK(scene.reads==3);CHECK(world._render_frame.batches.size()==2);
	CHECK(world._render_frame.batches[0].object_id==99);CHECK(world._render_frame.batches[1].first_index==3);
	CHECK(world._render_frame.batches[0].flags==RenderBatchFlags::SELECTED);
	CHECK(pipeline._bones_row==1);CHECK(cache[0]==0);
	world._cullable_shadow_casters.id.push_back(0);world._cullable_shadow_casters.id.push_back(1);
	world._cullable_shadow_casters.render.push_back(0);
	RenderBatchView shadow(StringId32("shadows"),0);shadow.camera=shadow.viewport=true;shadow.rect={0,0,32,32};shadow.view.t.x=11;
	world.gather_shadow_casters(shadow,1234);CHECK(scene.reads==4);CHECK(pipeline._bones_row==1);
	world._cullable_shadow_casters.render[0]=1;shadow.index=1;shadow.view.t.x=22;shadow.rect.x=32;
	world.gather_shadow_casters(shadow,5678);CHECK(scene.reads==4);
	world._cullable_shadow_casters.render.clear();geometry[0].vbh.idx=77;bindings[0].index_offset=71;shadow.view.t.x=999;
	CHECK(world._render_frame.views[1].view.t.x==11);CHECK(world._render_frame.views[2].view.t.x==22);
	reset_records();pipeline.render(64,32,MATRIX4X4_IDENTITY,MATRIX4X4_IDENTITY,&world._render_frame);
	CHECK(scene.reads==4);CHECK(bgfx::submits.size()==7);
	for(u32 i=0;i<6;++i){
		const auto &submission=bgfx::submits[i];
		CHECK(submission.state.vertex_buffer==1);CHECK(submission.state.num_transforms==1);
		CHECK(submission.state.values.count(pipeline._bones_data_size.idx)==1);
		Vector4 palette;memcpy(&palette,submission.state.values.at(pipeline._bones_data_size.idx).data(),sizeof(palette));
		CHECK(palette.x==1.0f/pipeline._bones_texture_height&&palette.y==0);
		u32 found=0;for(const auto &binding:submission.bindings)if(binding.stage==BONES_DATA_SLOT){
			++found;CHECK(binding.sampler.idx==pipeline._bones_data_sampler.idx);
			CHECK(binding.texture.idx==pipeline._bones_texture.idx);
		}
		CHECK(found==1);
	}
	CHECK(bgfx::submits[0].state.first_index==0);CHECK(bgfx::submits[4].state.first_index==0);
	CHECK(bgfx::submits[4].state.front==1234);CHECK(bgfx::submits[6].state.front==5678);
	CHECK(bgfx::submits[6].state.vertex_buffer==3);
	CHECK(bgfx::submits[6].state.values.count(pipeline._bones_data_size.idx)==0);
	for(const auto &binding:bgfx::submits[6].bindings)CHECK(binding.stage!=BONES_DATA_SLOT);
	CHECK(bgfx::views[2].camera[12]==11);CHECK(bgfx::views[3].camera[12]==22);
	CHECK(bgfx::views[2].x==0&&bgfx::views[3].x==32);
	pipeline.end_frame();
	CHECK(bgfx::uploads.size()==1);CHECK(bgfx::uploads[0].width==64&&bgfx::uploads[0].height==1);
	bgfx::frame();bgfx::frame();CHECK(bgfx::deferred_memory.empty());
	pipeline.destroy();
}

static void bone_atlas_merge_regression()
{
	MaterialFixture material;ShaderManager shaders;Pipeline pipeline(shaders);
	auto bytes=compile(R"(layers=[
 {name="color_renamed" draw={type="draw_visible" source="camera"}}
 {name="depth_renamed" draw={type="draw_visible" source="camera" context="depth" shader="depth" skinned_shader="depth_skin"}}
 {name="selected_renamed" draw={type="draw_visible" source="camera" context="selection" filter="selected" shader="select" skinned_shader="select_skin" object_id_uniform="u_unit_id"}}
 {name="shadow_renamed" draw={type="draw_visible" source="shadows" context="shadow" shader="shadow" skinned_shader="shadow_skin"}}
])");
	pipeline.create(128,128,settings(),(const RenderConfigResource*)bytes.data());
	pipeline.begin_frame();
	CHECK(!bgfx::isValid(pipeline._bones_texture)); // Lazy allocation.
	SceneGraph scene;RenderWorld world(scene,pipeline);
	UnitId units[]={{10},{20}};
	std::vector<UnitId> bone_units(193);
	std::vector<Matrix4x4> bones_a(17),bones_b(193),offsets(193,MATRIX4X4_IDENTITY);
	for(u32 i=0;i<bone_units.size();++i)bone_units[i]={100+i};
	AnimationSkeletonInstance a={17,bone_units.data(),bones_a.data(),offsets.data()};
	AnimationSkeletonInstance b={193,bone_units.data(),bones_b.data(),offsets.data()};
	const AnimationSkeletonInstance *skeletons[]={&a,&b};
	Matrix4x4 poses[]={MATRIX4X4_IDENTITY,MATRIX4X4_IDENTITY};
	RenderWorld::MeshManager::MeshData geometry[]={{{1},{2}},{{3},{4}}};
	RenderWorld::MeshManager::MaterialBinding bindings[]={
	 {&material.material,&material.resource,0,3,UINT32_MAX},
	 {&material.material,&material.resource,3,6,UINT32_MAX}
	};
	u32 flags[]={RenderableFlags::VISIBLE|RenderableFlags::SELECTED|RenderableFlags::SHADOW_CASTER,
		RenderableFlags::VISIBLE|RenderableFlags::SELECTED|RenderableFlags::SHADOW_CASTER};
	u32 cache[]={UINT32_MAX,UINT32_MAX};
	world._mesh_manager._data={units,geometry,bindings,poses,skeletons,flags,cache};
	u32 camera=world._render_frame.add_view(RenderBatchView(StringId32("camera"),0));
	for(u32 i=0;i<2;++i)gather_mesh(world._mesh_manager,i,units[i],flags[i],scene,world._render_frame,camera,MATRIX4X4_IDENTITY);
	CHECK(cache[0]==0&&cache[1]==2);CHECK(pipeline._bones_row==15);
	CHECK(scene.reads==212); // Two owner transforms plus the two palettes.
	world._cullable_shadow_casters.id.push_back(1);world._cullable_shadow_casters.id.push_back(0);
	world._cullable_shadow_casters.render.push_back(0);world._cullable_shadow_casters.render.push_back(1);
	world.gather_shadow_casters(RenderBatchView(StringId32("shadows"),0));
	CHECK(scene.reads==214);CHECK(pipeline._bones_row==15);
	// Mutating source arrays cannot change the atlas copy or per-batch row.
	std::fill(bones_a.begin(),bones_a.end(),MATRIX4X4_IDENTITY);
	std::fill(bones_b.begin(),bones_b.end(),MATRIX4X4_IDENTITY);
	cache[0]=cache[1]=999;
	reset_records();pipeline.render(128,128,MATRIX4X4_IDENTITY,MATRIX4X4_IDENTITY,&world._render_frame);
	CHECK(bgfx::submits.size()==8);CHECK(scene.reads==214);
	for(const auto &submission:bgfx::submits){
		const bool first=submission.state.vertex_buffer==1;
		CHECK(first||submission.state.vertex_buffer==3);CHECK(submission.state.num_transforms==1);
		Vector4 palette;memcpy(&palette,submission.state.values.at(pipeline._bones_data_size.idx).data(),sizeof(palette));
		CHECK(palette.x==1.0f/pipeline._bones_texture_height);CHECK(palette.y==(first?0.0f:2.0f));
		CHECK(bgfx::transforms.at(submission.state.transform)[12]==(first?10.0f:20.0f));
		u32 found=0;for(const auto &binding:submission.bindings)if(binding.stage==14){
			++found;CHECK(binding.sampler.idx==pipeline._bones_data_sampler.idx&&binding.texture.idx==pipeline._bones_texture.idx);
		}
		CHECK(found==1);
	}
	pipeline.end_frame();
	CHECK(bgfx::uploads.size()==1);const auto upload=bgfx::uploads.back();
	CHECK(upload.width==64&&upload.height==15);CHECK(upload.bytes.size()==15*16*sizeof(Matrix4x4));
	Matrix4x4 matrix;memcpy(&matrix,upload.bytes.data()+sizeof(Matrix4x4),sizeof(matrix));CHECK(matrix.t.x==101);
	memcpy(&matrix,upload.bytes.data()+32*sizeof(Matrix4x4),sizeof(matrix));CHECK(matrix.t.x==20);
	for(u32 i=17*sizeof(Matrix4x4);i<32*sizeof(Matrix4x4);++i)CHECK(upload.bytes[i]==0);
	for(u32 i=(32+193)*sizeof(Matrix4x4);i<upload.bytes.size();++i)CHECK(upload.bytes[i]==0);
	CHECK(bgfx::deferred_memory.size()==1);
	const uint8_t *borrowed=bgfx::deferred_memory[0].memory->data;
	bgfx::frame();pipeline.begin_frame();
	u32 row;Matrix4x4 next=MATRIX4X4_IDENTITY;next.t.x=999;
	pipeline.add_bones_data(row,&next,1);CHECK(row==0);
	CHECK(memcmp(borrowed,upload.bytes.data(),upload.bytes.size())==0);
	pipeline.end_frame();bgfx::frame(); // First buffer can only be reused now.
	pipeline.begin_frame();next.t.x=777;pipeline.add_bones_data(row,&next,1);pipeline.end_frame();bgfx::frame();
	pipeline.begin_frame();reset_records();pipeline.end_frame();CHECK(bgfx::uploads.empty());
	bgfx::frame();CHECK(bgfx::deferred_memory.empty());
	// The app-wide end_frame path must not resurrect a disabled light upload.
	CHECK(pipeline._bones_row==0);
	fails([&]{pipeline.add_bones_data(row,&next,0);},"Skeleton must contain bones");
	fails([&]{pipeline.add_bones_data(row,nullptr,1);},"Skeleton must contain bones");
	pipeline._bones_row=pipeline._bones_texture_height;
	fails([&]{pipeline.add_bones_data(row,&next,1);},"Bone texture capacity exceeded");
	pipeline.begin_frame();
	// Malformed batch ranges fail without dereferencing outside frame storage.
	world._render_frame.batches[0].num_uniforms=UINT32_MAX;
	fails([&]{pipeline.render(128,128,MATRIX4X4_IDENTITY,MATRIX4X4_IDENTITY,&world._render_frame);},"Invalid batch uniform range");
	pipeline.destroy();CHECK(bgfx::deferred_memory.empty());
}

int main(int argc,char **argv)
{
	try {
		CHECK(argc==2);std::ifstream file(argv[1]);std::string text((std::istreambuf_iterator<char>(file)),{});CHECK(!text.empty());
		integration(text);routing_and_reuse();malformed();transient_reuse();invalid_execution();deletion_execution(text);native_gathering();bone_atlas_merge_regression();
		CHECK(bgfx::textures.empty()&&bgfx::framebuffers.empty()&&bgfx::uniforms.empty());
		std::cout<<"Prepared batches: "<<checks<<" checks passed; renamed layers/resources, material bindings, contexts, conditional selection, high view IDs, owned uploads, actual native gathering, bone-atlas row snapshots and two-frame upload lifetime.\n";
		return 0;
	} catch(const std::exception &error){std::cerr<<"FAIL: "<<error.what()<<"\n";return 1;}
}
