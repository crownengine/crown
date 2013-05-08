local ffi = require("ffi")

local lib_path = os.getenv("LD_LIBRARY_PATH")

lib = ffi.load(lib_path .. "/libcrown.so", true)

require("vec3")
require("mat4")
require("quat")

print("-- Testing Mat4 --\n")
local m = Mat4.mat4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0)
local t = Mat4.mat4(9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0)

Mat4.print(m)
print("\n")

print("-- Mat4.add --\n")

m = Mat4.add(m, t)
Mat4.print(m)
print("\n")

print("-- Mat4.subtract --\n")

m = Mat4.subtract(m, t)
Mat4.print(m)
print("\n")
