#ifndef CROWN_RENDER_PIPELINE_TEST_SDK_H
#define CROWN_RENDER_PIPELINE_TEST_SDK_H
// Test doubles, not Crown/bgfx headers. Used only by the isolated CPU tests.
#include <algorithm>
#include <array>
#include <cassert>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <map>
#include <string>
#include <stdexcept>
#include <vector>
#define CROWN_PLATFORM_LINUX 1
#define CROWN_PLATFORM_WINDOWS 0
#define CROWN_CAN_COMPILE 1
#ifndef CROWN_CAN_RELOAD
#define CROWN_CAN_RELOAD 1
#endif
#define CE_STATIC_ASSERT(c) static_assert(c, #c)
#define CE_UNUSED(x) (void)(x)
#define CE_UNUSED_2(x,y) (void)(x); (void)(y)
#define countof(a) (sizeof(a)/sizeof((a)[0]))
#define CE_ASSERT(c,...) do { if(!(c)) throw std::runtime_error(#c); } while(0)
#define CE_ENSURE(c) CE_ASSERT(c,"")
#define MATERIAL_MAX_TEXTURE_SLOTS 10
#define RESOURCE_VERSION_RENDER_CONFIG 11
#define BGFX_STATE_MSAA (1ull << 57)
#define RESOURCE_HEADER(v) (v)
#define BGFX_INVALID_HANDLE {UINT16_MAX}
#define BGFX_SAMPLER_U_CLAMP (1u<<0)
#define BGFX_SAMPLER_V_CLAMP (1u<<1)
#define BGFX_SAMPLER_MIN_POINT (1u<<2)
#define BGFX_SAMPLER_MAG_POINT (1u<<3)
#define BGFX_SAMPLER_MIP_POINT (1u<<4)
#define BGFX_SAMPLER_MIN_ANISOTROPIC (1u<<5)
#define BGFX_SAMPLER_MAG_ANISOTROPIC (1u<<6)
#define BGFX_SAMPLER_COMPARE_LEQUAL (1u<<7)
#define BGFX_TEXTURE_RT (1ull<<36)
#define BGFX_TEXTURE_RT_MSAA_SHIFT 36
#define BGFX_TEXTURE_RT_MSAA_MASK (7ull<<36)
#define BGFX_TEXTURE_MSAA_SAMPLE (1ull<<40)
#define BGFX_TEXTURE_RT_WRITE_ONLY (1ull<<41)
#define BGFX_CLEAR_COLOR 1
#define BGFX_CLEAR_DEPTH 2
#define BGFX_CLEAR_STENCIL 4
#define BGFX_STENCIL_TEST_ALWAYS 1
#define BGFX_STENCIL_FUNC_REF(n) (n)
#define BGFX_STENCIL_FUNC_RMASK(n) (n)
#define BGFX_STENCIL_OP_FAIL_S_REPLACE 2
#define BGFX_STENCIL_OP_FAIL_Z_REPLACE 4
#define BGFX_STENCIL_OP_PASS_Z_REPLACE 8
namespace bgfx {
struct TextureHandle {uint16_t idx;}; struct FrameBufferHandle {uint16_t idx;}; struct UniformHandle {uint16_t idx;}; struct ProgramHandle {uint16_t idx;}; struct VertexBufferHandle {uint16_t idx;}; struct IndexBufferHandle {uint16_t idx;};
template<class T> bool isValid(T t) {return t.idx!=UINT16_MAX;}
struct TextureFormat {enum Enum {RGBA8,BGRA8,RGBA16F,RGBA32F,R32U,D16,D24,D24S8,D32F};};
struct UniformType {enum Enum {Sampler,Vec4,Mat4};};
struct ViewMode {enum Enum {Default,Sequential,DepthAscending,DepthDescending};};
struct Attrib {enum Enum {Position,TexCoord0};}; struct AttribType {enum Enum {Float};};
struct VertexLayout {uint32_t m_hash=0,stride=0; void begin() {stride=0;} void add(Attrib::Enum,int n,AttribType::Enum,bool=false) {stride+=n*4;} void end(){m_hash=1;}};
struct Caps {struct Limits {uint32_t maxFBAttachments=8,maxTextureSize=16384,maxTextureSamplers=16,maxViews=512;} limits; bool homogeneousDepth=false,originBottomLeft=false;};
inline Caps caps; inline const Caps *getCaps(){return &caps;}
struct Memory {uint8_t *data; uint32_t size; bool owned;};
inline const Memory *copy(const void *p,uint32_t n) {auto m=new Memory{new uint8_t[n],n,true}; memcpy(m->data,p,n);return m;}
inline const Memory *makeRef(const void *p,uint32_t n) {return new Memory{(uint8_t*)p,n,false};}
inline void release_memory(const Memory *m) {if(m->owned)delete[]m->data;delete m;}
struct DeferredMemory {const Memory *memory;uint32_t due;std::vector<uint8_t> snapshot;};
inline uint32_t frame_number=0;
inline std::vector<DeferredMemory> deferred_memory;
inline uint32_t frame() {
 ++frame_number;
 for(auto it=deferred_memory.begin();it!=deferred_memory.end();) {
  // Model bgfx's documented two-frame borrowed-memory lifetime. Copying in
  // makeRef would hide both use-after-free and premature reuse in these tests.
  assert(std::memcmp(it->memory->data,it->snapshot.data(),it->snapshot.size())==0);
  if(it->due<=frame_number){release_memory(it->memory);it=deferred_memory.erase(it);}else ++it;
 }
 return frame_number;
}
struct TextureRecord {uint16_t w,h; TextureFormat::Enum format;uint64_t flags;std::string name;};
struct UniformRecord {std::string name;UniformType::Enum type;uint16_t count;uint32_t refs;};
struct ViewRecord {FrameBufferHandle fb=BGFX_INVALID_HANDLE;uint16_t w=0,h=0,clear=0;ViewMode::Enum mode=ViewMode::Default;std::string name;uint32_t rgba=0;float depth=1.0f;uint8_t stencil=0;uint16_t x=0,y=0;std::array<float,16> camera{},projection{};};
struct TextureBinding {uint8_t stage; UniformHandle sampler; TextureHandle texture; uint32_t flags;};
struct DrawState {
 uint64_t state = 0;
 uint32_t front = 0, back = 0, transform = UINT32_MAX;
 uint16_t num_transforms = 0;
 uint32_t first_vertex = 0, num_vertices = 0, first_index = 0, num_indices = 0;
 uint16_t vertex_buffer = UINT16_MAX, index_buffer = UINT16_MAX;
 const uint8_t *vertices = nullptr, *indices = nullptr;
 std::map<uint16_t,std::vector<uint8_t>> values;
};
struct SubmitRecord {uint16_t view; ProgramHandle program;std::vector<TextureHandle> inputs;
 std::vector<TextureBinding> bindings;DrawState state;uint32_t depth;};
inline DrawState pending;
inline std::map<uint32_t,std::vector<float>> transforms;
inline uint32_t next_transform = 0;
struct UploadRecord {TextureHandle texture;uint16_t width,height;std::vector<uint8_t> bytes;};
inline std::vector<UploadRecord> uploads;
inline std::map<uint16_t,TextureRecord> textures;
inline std::map<uint16_t,std::vector<TextureHandle>> framebuffers;
inline std::map<uint16_t,UniformRecord> uniforms;
inline std::map<uint16_t,ViewRecord> views;
inline std::vector<SubmitRecord> submits;
inline std::vector<uint16_t> touches,resets;
inline std::vector<TextureHandle> bound;
inline std::vector<TextureBinding> texture_bindings;
inline uint16_t next_texture=0,next_fb=0,next_uniform=0;
inline bool transient_available=true;
inline bool isTextureValid(uint16_t,bool,uint16_t,TextureFormat::Enum,uint64_t){return true;}
inline TextureHandle createTexture2D(uint16_t w,uint16_t h,bool,uint16_t,TextureFormat::Enum f,uint64_t flags,const Memory*m=nullptr){assert(w&&h);auto id=next_texture++;textures[id]={w,h,f,flags,{}};if(m){release_memory(m);}return {id};}
inline void setName(TextureHandle h,const char*n){textures.at(h.idx).name=n;}
inline FrameBufferHandle createFrameBuffer(uint8_t n,const TextureHandle*t,bool owns=false){assert(!owns);auto id=next_fb++;for(int i=0;i<n;++i)assert(textures.count(t[i].idx));framebuffers[id]=std::vector<TextureHandle>(t,t+n);return {id};}
inline UniformHandle createUniform(const char*n,UniformType::Enum t,uint16_t count=1){for(auto &u:uniforms)if(u.second.name==n){assert(u.second.type==t);u.second.count=std::max(u.second.count,count);++u.second.refs;return {u.first};}auto id=next_uniform++;uniforms[id]={n,t,count,1};return {id};}
inline void destroy(TextureHandle h){for(const auto &f:framebuffers)for(auto t:f.second)assert(t.idx!=h.idx);assert(textures.erase(h.idx)==1);}
inline void destroy(FrameBufferHandle h){assert(framebuffers.erase(h.idx)==1);}
inline void destroy(UniformHandle h){auto &u=uniforms.at(h.idx);if(!--u.refs)uniforms.erase(h.idx);}
inline void resetView(uint16_t v){assert(v<caps.limits.maxViews);views[v]=ViewRecord{};resets.push_back(v);}
inline void setViewName(uint16_t v,const char*n){assert(v<caps.limits.maxViews);views[v].name=n;}
inline void setViewFrameBuffer(uint16_t v,FrameBufferHandle f){assert(v<caps.limits.maxViews);views[v].fb=f;}
inline void setViewRect(uint16_t v,uint16_t x,uint16_t y,uint16_t w,uint16_t h){assert(v<caps.limits.maxViews&&w&&h);views[v].w=w;views[v].h=h;views[v].x=x;views[v].y=y;}
inline void setViewMode(uint16_t v,ViewMode::Enum m){views[v].mode=m;}
inline void setViewClear(uint16_t v,uint16_t flags,uint32_t rgba,float depth,uint8_t stencil){assert(v<caps.limits.maxViews);views[v].clear=flags;views[v].rgba=rgba;views[v].depth=depth;views[v].stencil=stencil;}
inline void setViewTransform(uint16_t v,const void*c,const void*p){assert(v<caps.limits.maxViews);if(c)memcpy(views[v].camera.data(),c,64);if(p)memcpy(views[v].projection.data(),p,64);}
inline void setViewOrder(uint16_t,uint16_t,const uint16_t*){}
inline void touch(uint16_t v){assert(v<caps.limits.maxViews);touches.push_back(v);}
inline void setUniform(UniformHandle h,const void*p,uint16_t n=1){assert(uniforms.count(h.idx));assert(n<=uniforms.at(h.idx).count);
 const auto *bytes=(const uint8_t*)p;pending.values[h.idx]=std::vector<uint8_t>(bytes,bytes+n*(uniforms.at(h.idx).type==UniformType::Mat4?64:16));}
inline void setTexture(uint8_t stage,UniformHandle u,TextureHandle t,uint32_t flags=UINT32_MAX){assert(uniforms.count(u.idx));assert(textures.count(t.idx));bound.push_back(t);texture_bindings.push_back({stage,u,t,flags});}
inline void setState(uint64_t value){pending.state=value;}
inline void setStencil(uint32_t front,uint32_t back=0){pending.front=front;pending.back=back;}
inline void discard(){bound.clear();texture_bindings.clear();pending=DrawState{};}
inline void submit(uint16_t v,ProgramHandle p,uint32_t depth=0){assert(v<caps.limits.maxViews&&isValid(p));submits.push_back({v,p,bound,texture_bindings,pending,depth});discard();}
struct TransientVertexBuffer {uint8_t*data;};struct TransientIndexBuffer {uint8_t*data;};
inline uint8_t transient[8*1024*1024];
inline size_t transient_used=0;
inline uint32_t getAvailTransientVertexBuffer(uint32_t n,const VertexLayout&layout){return transient_available?std::min<uint32_t>(n,(sizeof(transient)-transient_used)/std::max(1u,layout.stride)):0;}
inline uint32_t getAvailTransientIndexBuffer(uint32_t n){return transient_available?std::min<uint32_t>(n,(sizeof(transient)-transient_used)/2):0;}
inline void allocTransientVertexBuffer(TransientVertexBuffer*b,uint32_t n,const VertexLayout&layout){size_t bytes=n*std::max(1u,layout.stride);assert(bytes<=sizeof(transient)-transient_used);b->data=transient+transient_used;transient_used=(transient_used+bytes+15)&~size_t(15);}
inline void allocTransientIndexBuffer(TransientIndexBuffer*b,uint32_t n){assert(n*2<=sizeof(transient)-transient_used);b->data=transient+transient_used;transient_used=(transient_used+n*2+15)&~size_t(15);}
inline void setVertexBuffer(uint8_t,const TransientVertexBuffer*b,uint32_t first=0,uint32_t count=UINT32_MAX){pending.vertices=b->data;pending.first_vertex=first;pending.num_vertices=count;}
inline void setIndexBuffer(const TransientIndexBuffer*b,uint32_t first=0,uint32_t count=UINT32_MAX){pending.indices=b->data;pending.first_index=first;pending.num_indices=count;}
inline void setVertexBuffer(uint8_t,VertexBufferHandle h,uint32_t first=0,uint32_t count=UINT32_MAX){assert(isValid(h));pending.vertex_buffer=h.idx;pending.first_vertex=first;pending.num_vertices=count;}
inline void setIndexBuffer(IndexBufferHandle h,uint32_t first=0,uint32_t count=UINT32_MAX){assert(isValid(h));pending.index_buffer=h.idx;pending.first_index=first;pending.num_indices=count;}
inline uint32_t setTransform(const void*data,uint16_t count=1){assert(count);uint32_t id=next_transform;next_transform+=count;const float *p=(const float*)data;transforms[id]=std::vector<float>(p,p+16*count);pending.transform=id;pending.num_transforms=count;return id;}
inline void setTransform(uint32_t id,uint16_t count=1){assert(transforms.count(id)&&transforms.at(id).size()>=16u*count);pending.transform=id;pending.num_transforms=count;}
inline void updateTexture2D(TextureHandle texture,uint16_t,uint8_t,uint16_t,uint16_t,uint16_t w,uint16_t h,const Memory*memory) {
 assert(textures.count(texture.idx));
 assert(w>0&&h>0&&w<=textures.at(texture.idx).w&&h<=textures.at(texture.idx).h);
 uploads.push_back({texture,w,h,std::vector<uint8_t>(memory->data,memory->data+memory->size)});
 if(memory->owned)release_memory(memory);
 else deferred_memory.push_back({memory,frame_number+2,uploads.back().bytes});
}

}
namespace bx {struct Handedness {enum Enum {Left,Right};};inline void mtxOrtho(float*p,float,float,float,float,float,float,float,bool,Handedness::Enum=Handedness::Left){for(int i=0;i<16;++i)p[i]=(i%5==0)?1.0f:0.0f;} inline float abs(float x){return std::fabs(x);}}
namespace crown {
using u8=uint8_t;using u16=uint16_t;using u32=uint32_t;using u64=uint64_t;using s32=int32_t;using f32=float;
using std::max;using std::min;
template<class T>void exchange(T &a,T &b){std::swap(a,b);}
struct ScopedProfileScope {explicit ScopedProfileScope(const char*){}};
struct Allocator {void*allocate(size_t n){return malloc(n);}void deallocate(void*p){free(p);}};
inline Allocator &default_allocator(){static Allocator a;return a;}
using TempAllocator128=Allocator;using TempAllocator256=Allocator;using TempAllocator512=Allocator;using TempAllocator1024=Allocator;using TempAllocator2048=Allocator;using TempAllocator4096=Allocator;
struct StringId32 {u32 _id=0;StringId32()=default;explicit StringId32(const char*s){_id=2166136261u;while(*s){_id^=u8(*s++);_id*=16777619u;}}explicit StringId32(u32 i):_id(i){}bool operator==(StringId32 b)const{return _id==b._id;}bool operator!=(StringId32 b)const{return !(*this==b);}bool operator<(StringId32 b)const{return _id<b._id;}};
#define STRING_ID_32(s,n) crown::StringId32(s)
struct Vector2{float x,y;};struct Vector3{float x,y,z;};struct Vector4{float x,y,z,w;};struct Matrix4x4{Vector4 x,y,z,t;};
inline Vector4 VECTOR4_ZERO={0,0,0,0};inline Matrix4x4 MATRIX4X4_IDENTITY={{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};
inline const float*to_float_ptr(const Matrix4x4&m){return &m.x.x;}inline float*to_float_ptr(Matrix4x4&m){return &m.x.x;}
struct Value{};
struct BloomDesc{float enabled,threshold,weight,intensity;};struct VignetteDesc{float enabled;float pad[7];};struct ColorGradingDesc{float pad[8];};struct TonemapDesc{float pad[4];};struct GlobalLightingDesc{Vector3 ambient_color;float shadow_distance;};
struct FileBuffer;
struct CompileOptions;
struct DynamicString;
struct StringView;
struct ResourceManager;
struct ShaderManager;
struct StringId64 { u64 _id; };
using ResourceId=StringId64;
struct MaterialManager; struct MaterialResource; struct TextureResource;
template<class T> struct Vector;
} // namespace crown
#include "resource/shader_resource.h"
namespace crown {
#ifndef CROWN_TEST_REAL_SHADER_MANAGER
struct ShaderManager {
 std::vector<StringId32> lookups;
 std::map<u32,ShaderData> overrides;
 ShaderData shader(StringId32 n) {
  lookups.push_back(n);
  if(overrides.count(n._id))return overrides.at(n._id);
  ShaderData sd = {};
  sd.program = {uint16_t(n._id % 60000)};
  return sd;
 }
};
#endif
template<class T>struct Array:std::vector<T>{explicit Array(Allocator&){} };
namespace array {template<class T>void reserve(Array<T>&a,u32 n){a.reserve(n);}template<class T>void resize(Array<T>&a,u32 n){a.resize(n);}template<class T>u32 size(const Array<T>&a){return u32(a.size());}template<class T>u32 push_back(Array<T>&a,const T&t){a.push_back(t);return u32(a.size()-1);}template<class T>void push(Array<T>&a,const T*t,u32 n){a.insert(a.end(),t,t+n);}template<class T>T*begin(Array<T>&a){return a.data();}template<class T>const T*begin(const Array<T>&a){return a.data();}template<class T>void clear(Array<T>&a){a.clear();}}
template<class K,class V>struct HashMap:std::map<K,V>{explicit HashMap(Allocator&){} };
namespace hash_map {template<class K,class V>bool has(const HashMap<K,V>&m,K k){return m.count(k)!=0;}template<class K,class V>void remove(HashMap<K,V>&m,K k){m.erase(k);}template<class K,class V>V get(const HashMap<K,V>&m,K k,V d){auto i=m.find(k);return i==m.end()?d:i->second;}template<class K,class V>void set(HashMap<K,V>&m,K k,V v){m[k]=v;}template<class K,class V>void clear(HashMap<K,V>&m){m.clear();}}
struct DynamicString:std::string {using std::string::operator=;explicit DynamicString(Allocator&){} };
struct StringView:std::string {using std::string::string;};
struct JsonObject{std::vector<std::pair<StringView,const char*>> values;explicit JsonObject(Allocator&){}const char*operator[](const char*k)const{for(auto &v:values)if(v.first==k)return v.second;throw std::runtime_error("missing key");}};
using JsonArray=Array<const char*>;
struct JsonValueType {enum Enum {NIL,BOOL,NUMBER,STRING,ARRAY,OBJECT};};
namespace json_object {inline auto begin(const JsonObject&o){return o.values.begin();}inline auto end(const JsonObject&o){return o.values.end();}inline bool has(const JsonObject&o,const char*k){for(auto &v:o.values)if(v.first==k)return true;return false;}}
#define JSON_OBJECT_SKIP_HOLE(a,b)
namespace sjson {
inline bool bad=false;inline bool has_error(){return bad;}inline void clear_error(){bad=false;}
inline void ws(const char*&p){for(;;){while(isspace(*p)||*p==',')++p;if(p[0]=='/'&&p[1]=='/'){while(*p&&*p!='\n')++p;}else if(p[0]=='/'&&p[1]=='*'){p+=2;while(*p&&!(p[0]=='*'&&p[1]=='/'))++p;if(*p)p+=2;}else break;}}
inline std::string str(const char*&p){ws(p);std::string s;if(*p=='"'){++p;while(*p&&*p!='"'){if(*p=='\\'&&p[1])++p;s+=*p++;}if(*p=='"')++p;else bad=true;}else{while(isalnum(*p)||*p=='_'||*p=='-')s+=*p++;}return s;}
inline void skip(const char*&p){ws(p);if(*p=='"'){str(p);return;}if(*p=='{'||*p=='['){char e=*p++=='{'?'}':']';while(*p){ws(p);if(*p==e){++p;return;}if(*p=='='||*p==':'){++p;continue;}const char*b=p;skip(p);if(b==p){bad=true;return;}}bad=true;return;}while(*p&&!isspace(*p)&&*p!=','&&*p!=']'&&*p!='}'&&*p!='='&&*p!=':')++p;}
inline JsonValueType::Enum type(const char*p){ws(p);return *p=='{'?JsonValueType::OBJECT:*p=='['?JsonValueType::ARRAY:*p=='"'?JsonValueType::STRING:JsonValueType::NUMBER;}
inline void parse_array(JsonArray&a,const char*p){a.clear();ws(p);if(*p++!='['){bad=true;return;}for(;;){ws(p);if(*p==']')return;if(!*p){bad=true;return;}a.push_back(p);const char*b=p;skip(p);if(p==b){bad=true;return;}}}
inline void parse_object(JsonObject&o,const char*p){o.values.clear();ws(p);bool bracket=*p=='{';if(bracket)++p;for(;;){ws(p);if(!*p||*p=='}')return;auto key=str(p);ws(p);if(key.empty()||(*p!='='&&*p!=':')){bad=true;return;}++p;ws(p);o.values.emplace_back(StringView(key.data(),key.size()),p);const char*b=p;skip(p);if(p==b){bad=true;return;}}}
inline void parse(JsonObject&o,const char*p){parse_object(o,p);}
inline void parse_string(DynamicString&s,const char*p){ws(p);if(*p!='"'){bad=true;return;}s=str(p);}
inline float parse_float(const char*p){char*end;float n=strtof(p,&end);if(end==p)bad=true;return n;}
inline bool parse_bool(const char*p){ws(p);if(strncmp(p,"true",4)==0)return true;if(strncmp(p,"false",5)==0)return false;bad=true;return false;}
inline StringId32 parse_string_id(const char*p){Allocator a;DynamicString s(a);parse_string(s,p);return StringId32(s.c_str());}
inline Vector4 parse_vector4(const char*p){Allocator a;JsonArray v(a);parse_array(v,p);Vector4 r{};if(v.size()!=4){bad=true;return r;}for(int i=0;i<4;++i)(&r.x)[i]=parse_float(v[i]);return r;}
}
#define RETURN_IF_ERROR(expr) expr; do {if(sjson::has_error())return -1;}while(0)
struct CompileOptions {std::vector<uint8_t>bytes;std::string last_error;void error(int,const char*fmt,...){char b[2048];va_list a;va_start(a,fmt);vsnprintf(b,sizeof(b),fmt,a);va_end(a);last_error=b;}template<class T>void write(const T&t){write(&t,sizeof(t));}void write(const void*p,u32 n){auto c=(const uint8_t*)p;bytes.insert(bytes.end(),c,c+n);}};
#define RENDER_CONFIG_RESOURCE 0
#define RETURN_IF_FALSE(system,c,opts,...) do{if(!(c)){opts.error(system,__VA_ARGS__);return -1;}}while(0)
namespace error {[[noreturn]]inline void abort(const char*fmt,...){char b[2048];va_list a;va_start(a,fmt);vsnprintf(b,sizeof(b),fmt,a);va_end(a);throw std::runtime_error(b);}}
}
struct stbrp_context{int w=0,h=0,x=0,y=0,row=0;};struct stbrp_node{};struct stbrp_rect{int id,w,h,x,y,was_packed;};
inline void stbrp_init_target(stbrp_context*c,int w,int h,stbrp_node*,int){c->w=w;c->h=h;c->x=c->y=c->row=0;}
inline void stbrp_pack_rects(stbrp_context*c,stbrp_rect*r,int){if(c->x+r->w>c->w){c->x=0;c->y+=c->row;c->row=0;}r->x=c->x;r->y=c->y;r->was_packed=r->w<=c->w&&c->y+r->h<=c->h;if(r->was_packed){c->x+=r->w;c->row=std::max(c->row,r->h);}}

#endif // CROWN_RENDER_PIPELINE_TEST_SDK_H
