local ffi = require("ffi")

local lib_path = os.getenv("LD_LIBRARY_PATH")

lib = ffi.load(lib_path .. "/libcrown.so", true)

require("vec3")
require("mat4")
require("quat")
require("math_utils")
-- require("camera")
require("script")

--------------------------------------------------------------
--------------------------------------------------------------
--------------------------------------------------------------
print("-- Testing Vec3 --\n")
local pos = Vec3.vec3(1.0, 1.0, 1.0)

pos = Vec3.add(pos, Vec3.vec3(1.0, 2.0, 3.0)
)

print(pos.x)
print(pos.y)
print(pos.z)

Vec3.negate(pos)

print(pos.x)
print(pos.y)
print(pos.z)


--------------------------------------------------------------
--------------------------------------------------------------
--------------------------------------------------------------

print("-- Testing Mat4 --\n")
local m = Mat4.mat4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0)
local t = Mat4.mat4(9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0)
local trans = Vec3.vec3(1.0, 1.0, 0.0)
local scale = Vec3.vec3(10.0, 10.0, 10.0)

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

print(".. Mat4.set_translation-- \n")
Mat4.set_translation(m, trans)
Mat4.print(m)
print("\n")

print(".. Mat4.get_translation-- \n")
local tr = Mat4.get_translation(m)
print(tr.x)
print(tr.y)
print(tr.z)

print(".. Mat4.set_scale-- \n")
Mat4.set_scale(m, scale)
Mat4.print(m)
print("\n")

print(".. Mat4.get_scale-- \n")
local sc = Mat4.get_scale(m)
print(sc.x)
print(sc.y)
print(sc.z)

--------------------------------------------------------------
--------------------------------------------------------------
--------------------------------------------------------------
print("-- Testing MathUtils --\n")
print("sin of 0 is " .. Math.sin(0.0))

-- --------------------------------------------------------------
-- --------------------------------------------------------------
-- --------------------------------------------------------------
-- print("-- Testing Camera --\n")

-- local cam = Camera.camera(pos, 90.0, 1.6)

-- print("@move forward by 1 meter")
-- print("x:" .. Camera.position(cam).x)
-- print("y:" .. Camera.position(cam).y)
-- print("z:" .. Camera.position(cam).z)

-- for i=1,10 do	
-- 	Camera.move_forward(cam, 1.0);
-- 	print("@move forward by 1 meter\n")
-- 	print("x:" .. Camera.position(cam).x)
-- 	print("y:" .. Camera.position(cam).y)
-- 	print("z:" .. Camera.position(cam).z)
-- end

-- local vm = Camera.view_matrix(cam)
-- local pm = Camera.projection_matrix(cam)

-- print("@printing view matrix\n")
-- print(Mat4.print(vm))
-- print("@printing projection matrix\n")
-- print(Mat4.print(pm))

--------------------------------------------------------------
--------------------------------------------------------------
--------------------------------------------------------------
print("-- Testing Script --\n")

print(Script.vec3_used())
print(Script.mat4_used())
print(Script.quat_used())







