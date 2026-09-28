/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

// Included inside render_config_resource_internal, with CROWN_CAN_COMPILE.
struct PipelineCompiler
{
	CompileOptions &opts;
	Array<RenderConditionName> conditions;
	Array<RenderResourceData> resources;
	Array<RenderLayerData> layers;
	Array<RenderGeneratorData> generators;
	Array<RenderModifierData> modifiers;
	Array<RenderInputData> inputs;
	Array<RenderUniformData> uniforms;
	Array<RenderShaderLayerData> shader_layers;
	Array<char> names;

	PipelineCompiler(Allocator &a, CompileOptions &options)
		: opts(options)
		, conditions(a), resources(a), layers(a), generators(a)
		, modifiers(a), inputs(a), uniforms(a), shader_layers(a), names(a)
	{
	}

	template<typename T>
	static u32 find(const Array<T> &values, StringId32 name)
	{
		for (u32 i = 0; i < array::size(values); ++i) {
			if (values[i].name == name)
				return i;
		}
		return RENDER_CONFIG_INVALID;
	}

	u32 add_name(const DynamicString &str)
	{
		u32 offset = array::size(names);
		array::push(names, str.c_str(), str.length());
		array::push_back(names, '\0');
		return offset;
	}

	const char *name(u32 offset) const
	{
		return array::begin(names) + offset;
	}

	// Reject misspelled fields instead of silently accepting an ineffective edit.
	s32 keys(const JsonObject &obj, const char *allowed)
	{
		for (auto it = json_object::begin(obj); it != json_object::end(obj); ++it) {
			JSON_OBJECT_SKIP_HOLE(obj, it);
			bool found = false;
			const char *begin = allowed;
			while (*begin) {
				const char *end = begin;
				while (*end && *end != ' ') ++end;
				found |= it->first == StringView(begin, u32(end - begin));
				begin = *end ? end + 1 : end;
			}
			RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, found, opts
				, "Unknown pipeline property '%.*s'", it->first.length(), it->first.data());
		}
		return 0;
	}

	s32 string(DynamicString &value, const char *json)
	{
		RETURN_IF_ERROR(sjson::parse_string(value, json));
		RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, value.length() > 0 && value.length() <= 255, opts
			, "Pipeline names must contain 1..255 bytes");
		RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, StringId32(value.c_str()) != StringId32(), opts
			, "Pipeline name hashes to the reserved zero identifier");
		return 0;
	}

	s32 identifier(StringId32 &id, u32 &offset, const JsonObject &obj, const char *key)
	{
		RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, json_object::has(obj, key), opts
			, "Pipeline object is missing '%s'", key);
		TempAllocator256 ta;
		DynamicString value(ta);
		if (string(value, obj[key]) != 0) return -1;
		id = StringId32(value.c_str());
		offset = add_name(value);
		return 0;
	}

	s32 integer(u32 &value, const JsonObject &obj, const char *key, u32 low, u32 high)
	{
		if (!json_object::has(obj, key)) return 0;
		f32 number = RETURN_IF_ERROR(sjson::parse_float(obj[key]));
		RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE
			, std::isfinite(number) && number >= low && number <= high && floorf(number) == number
			, opts, "'%s' must be an integer in [%u, %u]", key, low, high);
		value = u32(number);
		return 0;
	}

	s32 real(f32 &value, const JsonObject &obj, const char *key, f32 low, f32 high)
	{
		if (!json_object::has(obj, key)) return 0;
		value = RETURN_IF_ERROR(sjson::parse_float(obj[key]));
		RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, std::isfinite(value) && value >= low && value <= high
			, opts, "'%s' is outside its valid range", key);
		return 0;
	}

	s32 boolean(u32 &value, const JsonObject &obj, const char *key)
	{
		if (json_object::has(obj, key)) {
			value = RETURN_IF_ERROR(sjson::parse_bool(obj[key]));
		}
		return 0;
	}

	s32 hex(u32 &value, const char *json)
	{
		TempAllocator128 ta;
		DynamicString str(ta);
		RETURN_IF_ERROR(sjson::parse_string(str, json));
		RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, str.length() == 8, opts
			, "Expected exactly eight hexadecimal digits");
		value = 0;
		for (u32 i = 0; i < 8; ++i) {
			const char c = str.c_str()[i];
			u32 digit = c >= '0' && c <= '9' ? u32(c - '0')
				: c >= 'a' && c <= 'f' ? u32(c - 'a' + 10)
				: c >= 'A' && c <= 'F' ? u32(c - 'A' + 10) : 16;
			RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, digit < 16, opts, "Invalid hexadecimal color");
			value = (value << 4) | digit;
		}
		return 0;
	}

	s32 choice(u32 &value, const JsonObject &obj, const char *key, const char *const *choices, u32 count)
	{
		if (!json_object::has(obj, key)) return 0;
		TempAllocator128 ta;
		DynamicString str(ta);
		RETURN_IF_ERROR(sjson::parse_string(str, obj[key]));
		for (u32 i = 0; i < count; ++i) {
			if (str == choices[i]) { value = i; return 0; }
		}
		RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, false, opts, "Unknown %s '%s'", key, str.c_str());
	}

	s32 parse_conditions(const char *json)
	{
		TempAllocator1024 ta;
		JsonArray arr(ta);
		RETURN_IF_ERROR(sjson::parse_array(arr, json));
		RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, array::size(arr) <= 32, opts, "At most 32 pipeline conditions are supported");
		for (u32 i = 0; i < array::size(arr); ++i) {
			DynamicString str(ta);
			if (string(str, arr[i]) != 0) return -1;
			RenderConditionName c = { StringId32(str.c_str()), add_name(str) };
			RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, find(conditions, c.name) == RENDER_CONFIG_INVALID
				, opts, "Duplicate condition name or hash collision: '%s'", str.c_str());
			array::push_back(conditions, c);
		}
		return 0;
	}

	s32 mask(u32 &bits, const JsonObject &obj, const char *key)
	{
		bits = 0;
		if (!json_object::has(obj, key)) return 0;
		TempAllocator512 ta;
		JsonArray arr(ta);
		RETURN_IF_ERROR(sjson::parse_array(arr, obj[key]));
		for (u32 i = 0; i < array::size(arr); ++i) {
			StringId32 id = RETURN_IF_ERROR(sjson::parse_string_id(arr[i]));
			u32 index = find(conditions, id);
			RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, index != RENDER_CONFIG_INVALID, opts, "Unknown pipeline condition in '%s'", key);
			bits |= 1u << index;
		}
		return 0;
	}

	s32 condition(RenderCondition &c, const JsonObject &obj)
	{
		if (mask(c.required, obj, "if") != 0 || mask(c.excluded, obj, "unless") != 0) return -1;
		RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, (c.required & c.excluded) == 0, opts, "Contradictory pipeline conditions");
		return 0;
	}

	s32 flags(u32 &bits, const JsonObject &obj, bool resource)
	{
		if (!json_object::has(obj, "flags")) return 0;
		TempAllocator512 ta;
		JsonArray arr(ta);
		RETURN_IF_ERROR(sjson::parse_array(arr, obj["flags"]));
		for (u32 i = 0; i < array::size(arr); ++i) {
			DynamicString str(ta);
			RETURN_IF_ERROR(sjson::parse_string(str, arr[i]));
			if (str == "clamp") bits |= RenderResourceFlags::CLAMP;
			else if (str == "point") bits |= RenderResourceFlags::POINT;
			else if (str == "anisotropic") bits |= RenderResourceFlags::ANISOTROPIC;
			else if (str == "compare") bits |= RenderResourceFlags::COMPARE;
			else if (resource && str == "msaa") bits |= RenderResourceFlags::MSAA;
			else RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, false, opts, "Unknown texture flag '%s'", str.c_str());
		}
		RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE
			, (bits & (RenderResourceFlags::POINT | RenderResourceFlags::ANISOTROPIC)) != (RenderResourceFlags::POINT | RenderResourceFlags::ANISOTROPIC)
			, opts, "Point and anisotropic filtering are mutually exclusive");
		return 0;
	}

	static bool depth_format(u32 format) { return format >= RenderResourceFormat::D16; }

	s32 parse_resources(const char *json)
	{
		static const char *const formats[] = { "RGBA8", "BGRA8", "RGBA16F", "RGBA32F", "R32U", "D16", "D24", "D24S8", "D32F" };
		static const char *const types[] = { "render_target", "texture" };
		TempAllocator4096 ta;
		JsonArray arr(ta);
		RETURN_IF_ERROR(sjson::parse_array(arr, json));
		RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, array::size(arr) <= 1024, opts, "Too many global resources");
		for (u32 i = 0; i < array::size(arr); ++i) {
			TempAllocator2048 entry_allocator;
			JsonObject obj(entry_allocator);
			RETURN_IF_ERROR(sjson::parse_object(obj, arr[i]));
			if (keys(obj, "name type format size width height w_scale h_scale count flags if unless initial_value") != 0) return -1;
			RenderResourceData d = {};
			d.size_source = StringId32("backbuffer");
			d.w_scale = d.h_scale = 1.0f;
			d.count = 1;
			if (identifier(d.name, d.name_offset, obj, "name") != 0) return -1;
			RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, d.name != StringId32("backbuffer") && find(resources, d.name) == RENDER_CONFIG_INVALID
				, opts, "Reserved/duplicate resource name or hash collision: '%s'", name(d.name_offset));
			RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, json_object::has(obj, "format"), opts, "Resource '%s' needs a format", name(d.name_offset));
			u32 type = 0;
			if (choice(d.format, obj, "format", formats, countof(formats)) != 0 || choice(type, obj, "type", types, countof(types)) != 0) return -1;
			if (type == 0) d.flags |= RenderResourceFlags::RENDER_TARGET;
			if (json_object::has(obj, "size")) {
				d.size_source = RETURN_IF_ERROR(sjson::parse_string_id(obj["size"]));
			}
			if (integer(d.width, obj, "width", 1, 65535) != 0 || integer(d.height, obj, "height", 1, 65535) != 0
				|| integer(d.count, obj, "count", 1, 16) != 0
				|| real(d.w_scale, obj, "w_scale", 0.000001f, 65535.0f) != 0 || real(d.h_scale, obj, "h_scale", 0.000001f, 65535.0f) != 0
				|| flags(d.flags, obj, true) != 0 || condition(d.condition, obj) != 0) return -1;
			RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, (d.width == 0) == (d.height == 0) && !(d.width && json_object::has(obj, "size"))
				, opts, "Use either width+height or a named size for '%s'", name(d.name_offset));
			RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, !(d.flags & RenderResourceFlags::COMPARE) || depth_format(d.format)
				, opts, "Comparison sampling requires a depth format");
			RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, !(d.flags & RenderResourceFlags::MSAA) || (d.flags & RenderResourceFlags::RENDER_TARGET)
				, opts, "MSAA requires a render target");
			if (json_object::has(obj, "initial_value")) {
				d.has_initial_value = 1;
				if (hex(d.initial_value, obj["initial_value"]) != 0) return -1;
				RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, type == 1 && d.format == RenderResourceFormat::RGBA8 && d.width == 1 && d.height == 1
					&& d.count == 1 && d.w_scale == 1.0f && d.h_scale == 1.0f
					, opts, "initial_value is supported for a single 1x1 RGBA8 texture");
			}
			array::push_back(resources, d);
		}
		return 0;
	}

	s32 reference(RenderResourceRef &ref, const char *json)
	{
		TempAllocator512 ta;
		DynamicString str(ta);
		ref.index = 0;
		if (sjson::type(json) == JsonValueType::OBJECT) {
			JsonObject obj(ta);
			RETURN_IF_ERROR(sjson::parse_object(obj, json));
			if (keys(obj, "resource index") != 0) return -1;
			RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, json_object::has(obj, "resource"), opts, "Missing resource name");
			if (string(str, obj["resource"]) != 0 || integer(ref.index, obj, "index", 0, 15) != 0) return -1;
		} else if (string(str, json) != 0) return -1;
		ref.resource = find(resources, StringId32(str.c_str()));
		RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, ref.resource != RENDER_CONFIG_INVALID, opts, "Unknown resource '%s'", str.c_str());
		RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, ref.index < resources[ref.resource].count, opts, "Resource index out of range: '%s'", str.c_str());
		return 0;
	}

	static bool same(RenderResourceRef a, RenderResourceRef b) { return a.resource == b.resource && a.index == b.index; }

	s32 target(RenderTargetData &d, const JsonObject &obj, bool fullscreen)
	{
		memset(&d, 0, sizeof(d));
		d.depth.resource = RENDER_CONFIG_INVALID;
		d.clear_depth = 1.0f;
		d.clear_rgba = 0x000000ff;
		d.transform = fullscreen ? RenderLayerTransform::ORTHO_UNIT : RenderLayerTransform::NONE;
		d.touch = fullscreen;
		if (json_object::has(obj, "render_targets")) {
			TempAllocator512 ta;
			JsonArray arr(ta);
			RETURN_IF_ERROR(sjson::parse_array(arr, obj["render_targets"]));
			RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, array::size(arr) <= RENDER_CONFIG_MAX_TARGETS, opts, "Too many color attachments");
			for (u32 i = 0; i < array::size(arr); ++i) {
				if (reference(d.colors[i], arr[i]) != 0) return -1;
				const RenderResourceData &r = resources[d.colors[i].resource];
				RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, (r.flags & RenderResourceFlags::RENDER_TARGET) && !depth_format(r.format)
					, opts, "Color attachment '%s' is not a color render target", name(r.name_offset));
				for (u32 j = 0; j < i; ++j)
					RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, !same(d.colors[i], d.colors[j]), opts, "Duplicate framebuffer attachment");
			}
			d.num_colors = array::size(arr);
		}
		if (json_object::has(obj, "depth_stencil_target")) {
			if (reference(d.depth, obj["depth_stencil_target"]) != 0) return -1;
			const RenderResourceData &r = resources[d.depth.resource];
			RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, (r.flags & RenderResourceFlags::RENDER_TARGET) && depth_format(r.format)
				, opts, "Depth attachment is not a depth render target");
		}
		RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, d.num_colors + (d.depth.resource != RENDER_CONFIG_INVALID) <= RENDER_CONFIG_MAX_TARGETS
			, opts, "Too many framebuffer attachments");
		static const char *const sorts[] = { "default", "sequential", "front_to_back", "back_to_front" };
		static const char *const transforms[] = { "none", "camera", "ortho_unit", "ortho_screen", "ortho_graph", "ortho_unit_flipped" };
		if (choice(d.sort, obj, "sort", sorts, countof(sorts)) != 0 || choice(d.transform, obj, "transform", transforms, countof(transforms)) != 0
			|| boolean(d.touch, obj, "touch") != 0 || boolean(d.manual_rect, obj, "manual_rect") != 0) return -1;
		if (json_object::has(obj, "clear")) {
			TempAllocator512 ta;
			JsonObject clear(ta);
			RETURN_IF_ERROR(sjson::parse_object(clear, obj["clear"]));
			if (keys(clear, "color depth stencil rgba depth_value stencil_value") != 0) return -1;
			const char *const flags[] = { "color", "depth", "stencil" };
			for (u32 i = 0; i < countof(flags); ++i) {
				u32 enabled = 0;
				if (boolean(enabled, clear, flags[i]) != 0) return -1;
				if (enabled) d.clear_flags |= 1u << i;
			}
			if (json_object::has(clear, "rgba") && hex(d.clear_rgba, clear["rgba"]) != 0) return -1;
			if (real(d.clear_depth, clear, "depth_value", 0.0f, 1.0f) != 0 || integer(d.clear_stencil, clear, "stencil_value", 0, 255) != 0) return -1;
		}
		return 0;
	}

	s32 parse_inputs(RenderModifierData &d, const JsonObject &obj)
	{
		d.first_input = array::size(inputs);
		if (!json_object::has(obj, "inputs")) return 0;
		TempAllocator2048 ta;
		JsonArray arr(ta);
		RETURN_IF_ERROR(sjson::parse_array(arr, obj["inputs"]));
		RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, array::size(arr) <= 16, opts, "Too many modifier inputs");
		for (u32 i = 0; i < array::size(arr); ++i) {
			JsonObject input(ta);
			RETURN_IF_ERROR(sjson::parse_object(input, arr[i]));
			if (keys(input, "texture sampler stage flags") != 0) return -1;
			RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, json_object::has(input, "texture"), opts, "Input needs a texture");
			RenderInputData b = {};
			b.stage = i;
			b.flags = RENDER_CONFIG_INVALID;
			StringId32 sampler;
			if (reference(b.texture, input["texture"]) != 0 || identifier(sampler, b.sampler_name_offset, input, "sampler") != 0
				|| integer(b.stage, input, "stage", 0, 15) != 0) return -1;
			if (json_object::has(input, "flags")) { b.flags = 0; if (flags(b.flags, input, false) != 0) return -1; }
			for (u32 j = d.first_input; j < array::size(inputs); ++j) {
				RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, inputs[j].stage != b.stage && strcmp(name(inputs[j].sampler_name_offset), name(b.sampler_name_offset)) != 0
					, opts, "Duplicate sampler or texture stage in a modifier");
			}
			for (u32 j = 0; j < d.target.num_colors; ++j)
				RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, !same(b.texture, d.target.colors[j]), opts, "A modifier cannot sample its own color attachment");
			RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, !same(b.texture, d.target.depth), opts, "A modifier cannot sample its own depth attachment");
			array::push_back(inputs, b);
		}
		d.num_inputs = array::size(inputs) - d.first_input;
		return 0;
	}

	s32 parse_uniforms(RenderModifierData &d, const JsonObject &obj)
	{
		d.first_uniform = array::size(uniforms);
		if (json_object::has(obj, "pixel_size_uniform")) {
			StringId32 id;
			if (identifier(id, d.pixel_size_uniform, obj, "pixel_size_uniform") != 0) return -1;
		}
		if (!json_object::has(obj, "uniforms")) return 0;
		TempAllocator2048 ta;
		JsonArray arr(ta);
		RETURN_IF_ERROR(sjson::parse_array(arr, obj["uniforms"]));
		RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, array::size(arr) <= 64, opts, "Too many modifier uniforms");
		for (u32 i = 0; i < array::size(arr); ++i) {
			JsonObject uniform(ta);
			RETURN_IF_ERROR(sjson::parse_object(uniform, arr[i]));
			if (keys(uniform, "name value") != 0) return -1;
			RenderUniformData u;
			StringId32 id;
			if (identifier(id, u.name_offset, uniform, "name") != 0) return -1;
			RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, json_object::has(uniform, "value"), opts, "Uniform needs a vec4 value");
			u.value = RETURN_IF_ERROR(sjson::parse_vector4(uniform["value"]));
			RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, std::isfinite(u.value.x) && std::isfinite(u.value.y) && std::isfinite(u.value.z) && std::isfinite(u.value.w)
				, opts, "Uniform values must be finite");
			for (u32 j = d.first_uniform; j < array::size(uniforms); ++j)
				RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, strcmp(name(uniforms[j].name_offset), name(u.name_offset)) != 0, opts, "Duplicate modifier uniform");
			array::push_back(uniforms, u);
		}
		d.num_uniforms = array::size(uniforms) - d.first_uniform;
		return 0;
	}

	s32 parse_generators(const char *json)
	{
		TempAllocator4096 ta;
		JsonArray arr(ta);
		RETURN_IF_ERROR(sjson::parse_array(arr, json));
		for (u32 i = 0; i < array::size(arr); ++i) {
			TempAllocator2048 ga;
			JsonObject obj(ga);
			RETURN_IF_ERROR(sjson::parse_object(obj, arr[i]));
			if (keys(obj, "name modifiers") != 0) return -1;
			RenderGeneratorData g;
			if (identifier(g.name, g.name_offset, obj, "name") != 0) return -1;
			RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, find(generators, g.name) == RENDER_CONFIG_INVALID, opts, "Duplicate generator name/hash");
			RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, json_object::has(obj, "modifiers"), opts, "A resource generator needs a modifiers array");
			JsonArray mods(ga);
			RETURN_IF_ERROR(sjson::parse_array(mods, obj["modifiers"]));
			RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, array::size(mods) > 0 && array::size(modifiers) + array::size(mods) <= 4096, opts, "Invalid modifier count");
			g.first_modifier = array::size(modifiers);
			g.num_modifiers = array::size(mods);
			for (u32 j = 0; j < array::size(mods); ++j) {
				TempAllocator4096 ma;
				JsonObject obj(ma);
				RETURN_IF_ERROR(sjson::parse_object(obj, mods[j]));
				if (keys(obj, "type shader profiling_scope resource if unless render_targets depth_stencil_target sort transform touch manual_rect clear inputs uniforms pixel_size_uniform") != 0) return -1;
				RenderModifierData d = {};
				d.resource = d.pixel_size_uniform = RENDER_CONFIG_INVALID;
				if (identifier(d.type, d.name_offset, obj, "type") != 0) return -1;
				if (json_object::has(obj, "profiling_scope")) {
					StringId32 ignored;
					if (identifier(ignored, d.name_offset, obj, "profiling_scope") != 0) return -1;
				}
				if (json_object::has(obj, "shader")) {
					d.shader = RETURN_IF_ERROR(sjson::parse_string_id(obj["shader"]));
				}
				RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, d.type != StringId32("fullscreen_pass") || d.shader != StringId32()
					, opts, "fullscreen_pass needs a shader program name");
				// Programs live in the .shader libraries listed in 'shaders'. A program
				// such as 'blit+BLEND_ENABLED' is not a file dependency.
				if (json_object::has(obj, "resource")) {
					StringId32 id = RETURN_IF_ERROR(sjson::parse_string_id(obj["resource"]));
					d.resource = find(resources, id);
					RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, d.resource != RENDER_CONFIG_INVALID, opts, "Unknown modifier resource context");
				}
				if (condition(d.condition, obj) != 0 || target(d.target, obj, true) != 0
					|| parse_inputs(d, obj) != 0 || parse_uniforms(d, obj) != 0) return -1;
				array::push_back(modifiers, d);
			}
			array::push_back(generators, g);
		}
		return 0;
	}

	s32 parse_layers(const char *json)
	{
		TempAllocator4096 ta;
		JsonArray arr(ta);
		RETURN_IF_ERROR(sjson::parse_array(arr, json));
		RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, array::size(arr) > 0 && array::size(arr) <= 1024, opts, "Invalid layer count");
		u32 views = 0;
		for (u32 i = 0; i < array::size(arr); ++i) {
			TempAllocator2048 ea;
			JsonObject obj(ea);
			RETURN_IF_ERROR(sjson::parse_object(obj, arr[i]));
			if (keys(obj, "name profiling_scope if unless resource_generator count render_targets depth_stencil_target sort transform touch manual_rect clear") != 0) return -1;
			RenderLayerData d = {};
			d.count = 1;
			d.generator = RENDER_CONFIG_INVALID;
			if (identifier(d.name, d.name_offset, obj, "name") != 0) return -1;
			RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, find(layers, d.name) == RENDER_CONFIG_INVALID, opts, "Duplicate layer name/hash");
			if (json_object::has(obj, "profiling_scope")) {
				StringId32 ignored;
				if (identifier(ignored, d.name_offset, obj, "profiling_scope") != 0) return -1;
			}
			if (condition(d.condition, obj) != 0 || target(d.target, obj, false) != 0 || integer(d.count, obj, "count", 1, 65535) != 0) return -1;
			if (json_object::has(obj, "resource_generator")) {
				StringId32 id = RETURN_IF_ERROR(sjson::parse_string_id(obj["resource_generator"]));
				d.generator = find(generators, id);
				RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, d.generator != RENDER_CONFIG_INVALID, opts, "Unknown resource generator");
				RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, !json_object::has(obj, "count") && d.target.num_colors == 0 && d.target.depth.resource == RENDER_CONFIG_INVALID
					&& d.target.clear_flags == 0, opts, "Generator layers reserve one view per modifier; specify outputs/clears on the modifiers");
				d.count = generators[d.generator].num_modifiers;
			}
			views += d.count;
			RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, views <= 65535, opts, "Pipeline exceeds the 16-bit view-ID range");
			array::push_back(layers, d);
		}
		return 0;
	}

	s32 parse_shader_layers(const char *json)
	{
		TempAllocator2048 ta;
		JsonArray arr(ta);
		RETURN_IF_ERROR(sjson::parse_array(arr, json));
		for (u32 i = 0; i < array::size(arr); ++i) {
			JsonObject obj(ta);
			RETURN_IF_ERROR(sjson::parse_object(obj, arr[i]));
			if (keys(obj, "shader layer index") != 0) return -1;
			RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, json_object::has(obj, "shader") && json_object::has(obj, "layer"), opts, "shader_layers entries need shader and layer");
			RenderShaderLayerData d;
			d.shader = RETURN_IF_ERROR(sjson::parse_string_id(obj["shader"]));
			StringId32 layer = RETURN_IF_ERROR(sjson::parse_string_id(obj["layer"]));
			d.layer = find(layers, layer);
			d.index = 0;
			if (integer(d.index, obj, "index", 0, 65534) != 0) return -1;
			RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, d.layer != RENDER_CONFIG_INVALID && d.index < layers[d.layer].count
				&& layers[d.layer].generator == RENDER_CONFIG_INVALID, opts, "Shader must select an existing geometry layer/view");
			for (u32 j = 0; j < array::size(shader_layers); ++j)
				RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, shader_layers[j].shader != d.shader, opts, "Duplicate shader layer mapping");
			array::push_back(shader_layers, d);
		}
		return 0;
	}

	s32 available(u32 resource, RenderCondition consumer)
	{
		if (resource == RENDER_CONFIG_INVALID) return 0;
		const RenderResourceData &r = resources[resource];
		RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, (consumer.required & r.condition.required) == r.condition.required
			&& (consumer.excluded & r.condition.excluded) == r.condition.excluded
			, opts, "Consumer of '%s' must include its resource's if/unless conditions", name(r.name_offset));
		return 0;
	}

	s32 available(const RenderTargetData &t, RenderCondition consumer)
	{
		for (u32 i = 0; i < t.num_colors; ++i)
			if (available(t.colors[i].resource, consumer) != 0) return -1;
		return available(t.depth.resource, consumer);
	}

	s32 parse(const JsonObject &obj)
	{
		RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, json_object::has(obj, "global_resources") && json_object::has(obj, "layers") && json_object::has(obj, "resource_generators")
			, opts, "A pipeline needs global_resources, layers and resource_generators");
		if (json_object::has(obj, "conditions") && parse_conditions(obj["conditions"]) != 0) return -1;
		if (parse_resources(obj["global_resources"]) != 0 || parse_generators(obj["resource_generators"]) != 0 || parse_layers(obj["layers"]) != 0) return -1;
		if (json_object::has(obj, "shader_layers") && parse_shader_layers(obj["shader_layers"]) != 0) return -1;
		for (u32 i = 0; i < array::size(layers); ++i) {
			const RenderLayerData &l = layers[i];
			if (l.generator == RENDER_CONFIG_INVALID) { if (available(l.target, l.condition) != 0) return -1; continue; }
			const RenderGeneratorData &g = generators[l.generator];
			for (u32 j = 0; j < g.num_modifiers; ++j) {
				const RenderModifierData &m = modifiers[g.first_modifier + j];
				RenderCondition c = { l.condition.required | m.condition.required, l.condition.excluded | m.condition.excluded };
				RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, !(c.required & c.excluded), opts, "Layer and modifier conditions contradict each other");
				if (available(m.target, c) != 0 || available(m.resource, c) != 0) return -1;
				for (u32 k = 0; k < m.num_inputs; ++k)
					if (available(inputs[m.first_input + k].texture.resource, c) != 0) return -1;
			}
		}
		for (u32 i = 0; i < array::size(inputs); ++i) {
			for (u32 j = 0; j < array::size(uniforms); ++j)
				RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, strcmp(name(inputs[i].sampler_name_offset), name(uniforms[j].name_offset)) != 0
					, opts, "Uniform cannot be both a sampler and a vec4");
			for (u32 j = 0; j < array::size(modifiers); ++j)
				RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, modifiers[j].pixel_size_uniform == RENDER_CONFIG_INVALID
					|| strcmp(name(inputs[i].sampler_name_offset), name(modifiers[j].pixel_size_uniform)) != 0
					, opts, "Pixel-size uniform cannot also be a sampler");
		}
		for (u32 i = 0; i < array::size(modifiers); ++i) {
			const RenderModifierData &m = modifiers[i];
			for (u32 j = m.first_uniform; j < m.first_uniform + m.num_uniforms; ++j)
				RETURN_IF_FALSE(RENDER_CONFIG_RESOURCE, m.pixel_size_uniform == RENDER_CONFIG_INVALID
					|| strcmp(name(uniforms[j].name_offset), name(m.pixel_size_uniform)) != 0
					, opts, "Constant and automatic pixel-size uniforms must have different names");
		}
		return 0;
	}

	void layout(RenderConfigResource &r)
	{
		u32 offset = sizeof(r);
#define PIPELINE_TABLE(field, type) \
		r.num_##field = array::size(field); \
		r.field##_offset = offset; \
		offset += r.num_##field * sizeof(type)
		PIPELINE_TABLE(conditions, RenderConditionName);
		PIPELINE_TABLE(resources, RenderResourceData);
		PIPELINE_TABLE(layers, RenderLayerData);
		PIPELINE_TABLE(generators, RenderGeneratorData);
		PIPELINE_TABLE(modifiers, RenderModifierData);
		PIPELINE_TABLE(inputs, RenderInputData);
		PIPELINE_TABLE(uniforms, RenderUniformData);
		PIPELINE_TABLE(shader_layers, RenderShaderLayerData);
#undef PIPELINE_TABLE
		r.names_size = array::size(names);
		r.names_offset = offset;
	}

	template<typename T>
	void write_array(const Array<T> &a)
	{
		if (array::size(a)) opts.write(array::begin(a), array::size(a) * sizeof(T));
	}

	void write(const RenderConfigResource &r)
	{
#define PIPELINE_HEADER(field) opts.write(r.num_##field); opts.write(r.field##_offset)
		PIPELINE_HEADER(conditions); PIPELINE_HEADER(resources); PIPELINE_HEADER(layers);
		PIPELINE_HEADER(generators); PIPELINE_HEADER(modifiers); PIPELINE_HEADER(inputs);
		PIPELINE_HEADER(uniforms); PIPELINE_HEADER(shader_layers);
#undef PIPELINE_HEADER
		opts.write(r.names_size); opts.write(r.names_offset);
		write_array(conditions); write_array(resources); write_array(layers);
		write_array(generators); write_array(modifiers); write_array(inputs);
		write_array(uniforms); write_array(shader_layers); write_array(names);
	}
};
