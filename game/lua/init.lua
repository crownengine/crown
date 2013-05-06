require("vec3")
require("mat4")
require("quat")


local ffi = require("ffi")

local lib_path = os.getenv("LD_LIBRARY_PATH")

lib = ffi.load(lib_path .. "libcrown.so", true)

local v = lib.vec3(1.0, 1.0, 1.0)
local q = lib.quat(10.0, v)
local m = lib.mat4(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0)

lib.mat4_print(m)