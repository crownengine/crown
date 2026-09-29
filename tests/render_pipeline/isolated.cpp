#include <test_sdk.h>
#include "resource/render_config_resource.h"
#include "resource/render_config_resource.inl"
#include "render_config_accessors_under_test.h"
namespace crown { namespace render_config_resource_internal {
#include "pipeline_compiler_under_test.h"
}}
#include "device/render_pipeline.cpp"
#include "device/pipeline.cpp"
#include <fstream>
#include <iostream>
using namespace crown;
static int checks=0;
#define CHECK(c) do { ++checks; if(!(c)) throw std::runtime_error("CHECK failed: " #c); } while(0)
static std::string replaced(std::string s,const std::string&a,const std::string&b){auto p=s.find(a);CHECK(p!=std::string::npos);s.replace(p,a.size(),b);return s;}
static std::vector<uint8_t> compile(const std::string &text,bool accept=true){
 Allocator a; JsonObject obj(a); sjson::clear_error(); sjson::parse(obj,text.c_str());
 CompileOptions opts; render_config_resource_internal::PipelineCompiler compiler(a,opts);
 int rc=compiler.parse(obj); if(!accept){CHECK(rc!=0);return {};}
 if(rc)throw std::runtime_error("Compiler: "+opts.last_error);
 RenderConfigResource h{};h.version=RESOURCE_HEADER(RESOURCE_VERSION_RENDER_CONFIG);compiler.layout(h);
 opts.write(h.version);opts.write(h.render_settings);compiler.write(h);
 CHECK(opts.bytes.size()==h.names_offset+h.names_size);
 return opts.bytes;
}
static void no_leaks(){CHECK(bgfx::textures.empty());CHECK(bgfx::framebuffers.empty());CHECK(bgfx::uniforms.empty());}
static RenderSettings settings(u32 bits){RenderSettings s{};
 s.flags=(bits&1?u32(RenderSettingsFlags::BLOOM):0u)|(bits&2?u32(RenderSettingsFlags::LIGHTS_COOKIE):0u)|(bits&4?u32(RenderSettingsFlags::SELECTION):0u)|(bits&8?u32(RenderSettingsFlags::MSAA):0u);
 s.msaa_quality=2;s.sun_shadow_map_size={256,256};s.local_lights_shadow_map_size={256,256};s.lights_cookie_atlas_size={128,128};return s;}
static std::vector<std::string> expected(const RenderPipeline&p){std::vector<std::string> names;auto*r=p._resource;
 auto*l=render_config_resource::layers(r);auto*m=render_config_resource::modifiers(r);auto*g=render_config_resource::generators(r);
 for(u32 i=0;i<r->num_layers;++i)if(p.enabled(l[i].condition)&&l[i].generator!=RENDER_CONFIG_INVALID)
 for(u32 k=0;k<g[l[i].generator].num_modifiers;++k){auto&d=m[g[l[i].generator].first_modifier+k];if(p.enabled(d.condition))names.push_back(render_config_resource::name(r,d.name_offset));}
 return names;}

static std::string section(const std::string &text, const char *key, const std::string &replacement)
{
 Allocator a;
 JsonObject obj(a);
 sjson::parse(obj, text.c_str());
 const char *start = obj[key];
 const char *end = start;
 sjson::skip(end);
 return text.substr(0, size_t(start - text.c_str())) + replacement + text.substr(size_t(end - text.c_str()));
}

static std::string omit_layer(const std::string &text, const char *name)
{
 Allocator a;
 JsonObject obj(a);
 JsonArray layers(a);
 sjson::parse(obj, text.c_str());
 sjson::parse_array(layers, obj["layers"]);
 std::string result = "[\n";
 bool found = false;
 for (const char *start : layers) {
  JsonObject layer(a);
  DynamicString layer_name(a);
  sjson::parse_object(layer, start);
  sjson::parse_string(layer_name, layer["name"]);
  if (layer_name == name) { found = true; continue; }
  const char *end = start;
  sjson::skip(end);
  result.append(start, end).append("\n");
 }
 CHECK(found);
 return section(text, "layers", result + "]");
}

static void clear_frame_records()
{
 bgfx::submits.clear();
 bgfx::touches.clear();
 bgfx::discard();
}

static void deletion_regressions(const std::string &text)
{
 ShaderManager sm;
 Pipeline p(sm);
 RenderSettings rs = settings(15);
 rs.flags |= RenderSettingsFlags::SUN_SHADOWS | RenderSettingsFlags::LOCAL_LIGHTS_SHADOWS;
 const Matrix4x4 &identity = MATRIX4X4_IDENTITY;
 auto default_blob = compile(text);
 const RenderConfigResource *r = (const RenderConfigResource *)default_blob.data();

 // Exactly the reported experiment: keep resources and generators, remove all
 // layer declarations. No default-layer reconstruction and no native shaders.
 const std::string blank = section(text, "layers", "[\n// All layers commented out.\n]");
 for (const auto &source : std::vector<std::string>{blank, "layers = []", "layers = [{ name = \"mesh\" count = 0 }]",
      "layers = [{ name = \"post\" resource_generator = \"noop\" }] resource_generators = [{ name = \"noop\" modifiers = [] }]"}) {
  auto bytes = compile(source);
  auto *empty = (const RenderConfigResource *)bytes.data();
  sm.lookups.clear();
  p.create(320, 240, rs, empty);
  CHECK(p._render_pipeline.empty());
  CHECK(p.mesh_view() == UINT16_MAX);
  CHECK(p.sprite_view() == UINT16_MAX);
  CHECK(p.cascade_view() == UINT16_MAX);
  CHECK(p.sm_local_view() == UINT16_MAX);
  CHECK(p.cookie_atlas_view() == UINT16_MAX);
  CHECK(p.lights_view() == UINT16_MAX);
  CHECK(!p.selection_enabled());
  CHECK(sm.lookups.empty());
  clear_frame_records();
  p.begin_light_cookie_atlas();
  p.draw_local_lights_stencil(16, 1);
  CHECK(bgfx::touches.empty() && bgfx::submits.empty());
  u16 cookie_view = UINT16_MAX;
  CHECK(p.add_light_cookie(cookie_view, bgfx::TextureHandle BGFX_INVALID_HANDLE, 16, 16).z == 0);
  p.render(320, 240, identity, identity);
  CHECK(bgfx::submits.empty());
  CHECK(bgfx::touches == std::vector<uint16_t>{0});
  CHECK(bgfx::views.at(0).name == "empty_pipeline");
  CHECK(!bgfx::isValid(bgfx::views.at(0).fb));
  CHECK(bgfx::views.at(0).clear == BGFX_CLEAR_COLOR);
  CHECK(bgfx::views.at(0).rgba == 0x000000ff);
  p.destroy();
  no_leaks();
  CHECK(bgfx::views.at(0).clear == 0);
 }

 // Remove each default layer independently and exercise the native adapter as
 // well as the generic executor. In particular, a removed clear must not leave
 // its persistent bgfx state on the next layer reusing that numeric view ID.
 const RenderLayerData *layers = render_config_resource::layers(r);
 for (u32 i = 0; i < r->num_layers; ++i) {
  const char *name = render_config_resource::name(r, layers[i].name_offset);
  p.create(320, 240, rs, r);
  p.render(320, 240, identity, identity);
  p.destroy();
  auto bytes = compile(omit_layer(text, name));
  p.create(320, 240, rs, (const RenderConfigResource *)bytes.data());
  CHECK(p.view_id(StringId32(name)) == UINT16_MAX);
  CHECK(p._render_pipeline.frame_buffer(StringId32(name)).idx == UINT16_MAX);
  clear_frame_records();
  p.begin_light_cookie_atlas();
  p.draw_local_lights_stencil(16, 1);
  p.render(320, 240, identity, identity);
  for (u16 id : bgfx::touches) CHECK(bgfx::views.at(id).name != name);
  for (const auto &draw : bgfx::submits) CHECK(bgfx::views.at(draw.view).name != name);
  if (std::string(name) == "sm_cascade_clear") {
   CHECK(p.cascade_view() != UINT16_MAX);
   CHECK(p.sun_shadows_enabled());
   CHECK(bgfx::views.at(p.cascade_view()).clear == 0);
  }
  if (std::string(name) == "sm_cascade") CHECK(!p.sun_shadows_enabled());
  if (std::string(name) == "sm_local") CHECK(!p.local_shadows_enabled());
  if (std::string(name) == "sm_local_clear") CHECK(p.local_shadows_enabled());
  p.destroy();
  no_leaks();
 }

 // Modified clear values reach the backend; packing cookies is no longer a
 // hidden clear operation. Old copied configs should add touch=true explicitly.
 auto changed = compile(replaced(text, "clear = { depth = true }", "clear = { depth = true depth_value = 0.25 }"));
 p.create(320, 240, rs, (const RenderConfigResource *)changed.data());
 CHECK(bgfx::views.at(p.cascade_clear_view()).depth == 0.25f);
 clear_frame_records();
 p.begin_light_cookie_atlas();
 CHECK(bgfx::touches.empty());
 p.render(320, 240, identity, identity);
 CHECK(std::count(bgfx::touches.begin(), bgfx::touches.end(), p.cookie_atlas_clear_view()) == 1);
 p.destroy();
 no_leaks();

 auto limited = compile(replaced(replaced(text, "count = 4 manual_rect", "count = 2 manual_rect"),
     "count = 32 manual_rect", "count = 1 manual_rect"));
 p.create(320, 240, rs, (const RenderConfigResource *)limited.data());
 CHECK(p.cascade_view(1) != UINT16_MAX && p.cascade_view(2) == UINT16_MAX);
 CHECK(p._render_pipeline.geometry_view_count(StringId32("lights_cookie_atlas")) == 1);
 clear_frame_records();
 p.begin_light_cookie_atlas();
 u16 cookie_view = p.cookie_atlas_view();
 CHECK(p.add_light_cookie(cookie_view, p._color_textures[0], 16, 16).z > 0);
 CHECK(p.add_light_cookie(cookie_view, p._color_textures[0], 16, 16).z == 0);
 CHECK(bgfx::submits.size() == 1);
 p.destroy();
 no_leaks();

 // Arbitrary generic pipelines do not require the native renderer's resources.
 // If a native material actually references a missing sampler, suppress that
 // batch rather than send an invalid handle to bgfx. Unlit shaders can proceed.
 auto generic = compile("layers = [{ name = \"custom\" touch = true clear = { color = true } }]"
     "shader_layers = [{ shader = \"test_shader\" layer = \"custom\" }]");
 p.create(320, 240, rs, (const RenderConfigResource *)generic.data());
 CHECK(p.mesh_view() == UINT16_MAX);
 CHECK(p.shader_view(StringId32("test_shader"), p.mesh_view()) == 0);
 CHECK(p._render_pipeline.texture(StringId32("missing")).idx == UINT16_MAX);
 ShaderData unlit = {};
 CHECK(p.bind_lighting(unlit));
 const ShaderResource::Sampler sampler = {LIGHTS_DATA_SLOT};
 ShaderData lit = {}; lit.num_samplers = 1; lit.samplers = &sampler;
 CHECK(!p.bind_lighting(lit));
 p.destroy();
 no_leaks();

 // A scene role's shader must be cached even if its condition starts false.
 // Updating conditions before native gathering observes the new value now.
 auto conditional = compile("conditions = [\"vignette\"] layers = [{ name = \"debug\" if = [\"vignette\"] }]");
 p.create(320, 240, rs, (const RenderConfigResource *)conditional.data());
 CHECK(p.debug_view() == UINT16_MAX);
 CHECK(bgfx::isValid(p._debug_line_shader.program));
 p._vignette.enabled = 1;
 p.update_conditions();
 CHECK(p.debug_view() == 0);
 p._vignette.enabled = 0;
 p.update_conditions();
 CHECK(p.debug_view() == UINT16_MAX);
 p.destroy();
 no_leaks();

 // Reload back to the default must replace the presentation-only black clear.
 p.create(320, 240, rs, r);
 CHECK(bgfx::views.at(0).name == "color0_clear");
 CHECK(p.mesh_view() != UINT16_MAX);
 p.destroy();
 no_leaks();
 std::cout << "PASS empty layouts, zero-count layers, empty generators, all 21 single-layer removals, clear state reuse, reduced view ranges, optional resources and live conditions.\n";
}

int main(int argc,char**argv){try {
 CHECK(argc==2);std::ifstream in(argv[1]);std::string text((std::istreambuf_iterator<char>(in)),{});CHECK(!text.empty());
 auto blob=compile(text),original=blob;auto*r=(RenderConfigResource*)blob.data();
 CHECK(r->num_resources==13&&r->num_layers==21&&r->num_modifiers==20);
 ShaderManager sm;Pipeline p(sm);
 // All feature-flag combinations, each with both scene bloom/vignette states.
 for(u32 bits=0;bits<16;++bits){
   p.create(640,480,settings(bits),r);CHECK(p._render_pipeline._views.size()==141);
   const auto stable=p.mesh_view();CHECK((p.selection_view()!=UINT16_MAX)==bool(bits&4));
   CHECK((p.cookie_atlas_view(0)!=UINT16_MAX)==bool(bits&2));
   for(u32 scene=0;scene<4;++scene){
     p._bloom={float(scene&1),1,1,.5f};p._vignette.enabled=float((scene>>1)&1);
     bgfx::submits.clear();bgfx::touches.clear();p.render(640,480,MATRIX4X4_IDENTITY,MATRIX4X4_IDENTITY);
     auto names=expected(p._render_pipeline);CHECK(bgfx::submits.size()==names.size());
     const bool bloom=(bits&1)&&(scene&1);CHECK(names.size()==size_t((bloom?15:4)+(bits&4?2:0)));
     std::vector<std::string> emitted;for(auto &s:bgfx::submits)emitted.push_back(bgfx::views.at(s.view).name);
     CHECK(names==emitted);CHECK(p.mesh_view()==stable);
     CHECK(std::count(names.begin(),names.end(),"bloom_combine")==int(bloom));
     CHECK(std::count(names.begin(),names.end(),"bloom_bypass")==int(!bloom));
   }
   for(auto size:std::vector<std::pair<uint16_t,uint16_t>>{{1,1},{3,5},{0,0},{1280,720}}){
     p.reset(size.first,size.second);p._bloom={1,1,1,.5f};p.render(size.first,size.second,MATRIX4X4_IDENTITY,MATRIX4X4_IDENTITY);
     for(auto &t:p._render_pipeline._textures)CHECK(t.width>=1&&t.height>=1);
     CHECK(p.mesh_view()==stable);CHECK(p._render_settings.sun_shadow_map_size.x==256);
   }
   bgfx::submits.clear();bgfx::transient_available=false;p.render(1,1,MATRIX4X4_IDENTITY,MATRIX4X4_IDENTITY);CHECK(bgfx::submits.empty());bgfx::transient_available=true;
   p.destroy();no_leaks();CHECK(blob==original);
 }
 // A reload that reorders IDs across the old 8-bit boundary, routes a shader,
 // adds a resource and exercises native view wrappers without recreating p.
 std::string expanded=replaced(text,"layers = [", "layers = [\n { name = \"custom\" count = 260 transform = \"camera\" }");
 expanded=replaced(expanded,"shader_layers = []","shader_layers = [ { shader = \"test_shader\" layer = \"custom\" index = 259 } ]");
 expanded=replaced(expanded,"global_resources = [","global_resources = [\n { name = \"half_resolution\" format = \"RGBA8\" w_scale = 0.5 h_scale = 0.5 }");
 auto large=compile(expanded);p.create(640,480,settings(15),(RenderConfigResource*)large.data());CHECK(p.mesh_view()>255);CHECK(p.shader_view(StringId32("test_shader"),0)==259);
 auto half=p._render_pipeline.texture_state({p._render_pipeline.resource_index(StringId32("half_resolution")),0});CHECK(half.width==320&&half.height==240);
 p._bloom={1,1,1,.5f};p.render(640,480,MATRIX4X4_IDENTITY,MATRIX4X4_IDENTITY);p.destroy();no_leaks();
 CHECK(bgfx::views.at(400).clear==0);p.create(640,480,settings(15),r);CHECK(p.mesh_view()<255);p.destroy();no_leaks();
 // External size scaling cannot compound over successive resets.
 auto scaled=compile(replaced(text,"format = \"D32F\" size = \"sun_shadow_map_size\"", "format = \"D32F\" size = \"sun_shadow_map_size\" w_scale = 0.5 h_scale = 0.5"));
 p.create(640,480,settings(15),(RenderConfigResource*)scaled.data());CHECK(p._render_settings.sun_shadow_map_size.x==128);p.reset(800,600);CHECK(p._render_settings.sun_shadow_map_size.x==128);p.destroy();no_leaks();
 // Rejected authoring errors (actual PipelineCompiler, test SJSON frontend).
 const std::vector<std::pair<std::string,std::string>> invalid={
 {"count = 6","count = -1"},{"count = 6","count = 0"},{"count = 6","count = 1.5"},{"count = 6","count = 17"},
 {"format = \"RGBA16F\"","format = \"NOT_A_FORMAT\""},{"name = \"color1\"","name = \"color0\""},
 {"name = \"color1\"","name = \"backbuffer\""},{"flags = [ \"clamp\" \"msaa\" ]","flags = [ \"bogus\" ]"},
 {"w_scale = 0.5","w_scale = -1"},
 {"render_targets = [ \"color0\" ]","render_targets = [ \"missing\" ]"},
 {"render_targets = [ \"color0\" ]","render_targets = [ \"depth\" ]"},
 {"depth_stencil_target = \"depth\"","depth_stencil_target = \"color0\""},
 {"index = 5","index = 6"},{"stage = 0","stage = -1"},
 {"if = [ \"selection\" ]","if = [ \"missing\" ]"},
 {"if = [ \"selection\" ]","if = [ \"selection\" ] unless = [ \"selection\" ]"},
 {"resource_generator = \"post_processing\"","resource_generator = \"missing\""},
 {"name = \"color0\"","name = \"color0\" typo = 1"},
 {"pixel_size_uniform = \"u_map_pixel_size\"","pixel_size_uniform = \"s_color_map\""},
 {"texture = \"color_sdr\"","texture = \"selection_color\""}
 };
 for(auto &m:invalid){std::string base=m.first=="w_scale = 0.5"?expanded:text;compile(replaced(base,m.first,m.second),false);}
 // An in-place fullscreen feedback loop must be rejected.
 compile(replaced(text,"texture = \"color0\" sampler = \"s_color_map\" stage = 0 flags = [ \"anisotropic\" \"clamp\" ]", "texture = { resource = \"bloom\" index = 0 } sampler = \"s_color_map\" stage = 0"),false);
 // Capability errors must fail in release as well, not index a truncated ID.
 bgfx::caps.limits.maxViews=128;bool failed=false;try{p.create(640,480,settings(15),r);}catch(const std::runtime_error&){failed=true;}CHECK(failed);p.destroy();no_leaks();bgfx::caps.limits.maxViews=512;
 p.create(640,480,settings(15),r);failed=false;try{p._render_pipeline.set_condition(StringId32("bloom_allocated"),false);}catch(const std::runtime_error&){failed=true;}CHECK(failed);p.destroy();no_leaks();
 deletion_regressions(text);
 std::cout<<"PASS "<<checks<<" assertions; 64 feature/scene combinations; 64 resize cycles; 21 invalid configurations; reload, ownership, view widths, allocation guards.\n";
 return 0;
}catch(const std::exception&e){std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;}}
