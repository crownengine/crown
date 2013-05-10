local ffi = require("ffi")

local lib_path = os.getenv("LD_LIBRARY_PATH")

lib = ffi.load(lib_path .. "/libcrown.so", true)

require("vec3")
require("mat4")
require("quat")
require("math_utils")
require("camera")
require("script")

--------------------------------------------------------------
--------------------------------------------------------------
--------------------------------------------------------------
print("-- Testing Vec3 --\n")
local pos = Vec3.vec3(1.0, 1.0, 1.0)

--------------------------------------------------------------
--------------------------------------------------------------
--------------------------------------------------------------

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

--------------------------------------------------------------
--------------------------------------------------------------
--------------------------------------------------------------
print("-- Testing MathUtils --\n")



print("sin of 0 is " .. Math.sin(0.0))

--------------------------------------------------------------
--------------------------------------------------------------
--------------------------------------------------------------
print("-- Testing Camera --\n")

local cam = Camera.camera(pos, 90.0, 1.6)

print("@move forward by 1 meter")
print("x:" .. Camera.position(cam).x)
print("y:" .. Camera.position(cam).y)
print("z:" .. Camera.position(cam).z)

for i=1,10 do	
	Camera.move_forward(cam, 1.0);
	print("@move forward by 1 meter")
	print("x:" .. Camera.position(cam).x)
	print("y:" .. Camera.position(cam).y)
	print("z:" .. Camera.position(cam).z)
end

--------------------------------------------------------------
--------------------------------------------------------------
--------------------------------------------------------------
print("-- Testing Script --\n")

print(Script.vec3_used())
print(Script.mat4_used())
print(Script.quat_used())







