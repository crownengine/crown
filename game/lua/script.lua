local ffi = require("ffi")

ffi.cdef
[[
	uint32_t script_system_vec3_used();

	uint32_t script_system_mat4_used();

	uint32_t script_system_quat_used();
]]

Script = {}

Script.vec3_used = lib.script_system_vec3_used
Script.mat4_used = lib.script_system_mat4_used
Script.quat_used = lib.script_system_quat_used