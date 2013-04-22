# Copyright (C) 2010 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := crown
LOCAL_SRC_FILES :=\
	core/bv/Circle.cpp\
	core/bv/Frustum.cpp\
	core/bv/Rect.cpp\
	core/containers/Generic.cpp\
	core/containers/Str.cpp\
	core/math/Color4.cpp\
	core/math/Mat3.cpp\
	core/math/Mat4.cpp\
	core/math/MathUtils.cpp\
	core/math/Plane.cpp\
	core/math/Quat.cpp\
	core/math/Shape.cpp\
	core/math/Vec2.cpp\
	core/math/Vec3.cpp\
	core/math/Vec4.cpp\
	core/mem/MallocAllocator.cpp\
	core/streams/FileStream.cpp\
	core/streams/MemoryStream.cpp\
	core/streams/Stream.cpp\
\
	input/EventDispatcher.cpp\
	input/InputManager.cpp\
\
	loaders/BMPImageLoader.cpp\
	loaders/TGAImageLoader.cpp\
\
	os/OS.cpp\
	os/android/AndroidOS.cpp\
	os/android/AndroidInput.cpp\
	os/android/AndroidDevice.cpp\
	os/android/File.cpp\
\
	Filesystem.cpp\
\
	renderers/gles/GLESIndexBuffer.cpp\
	renderers/gles/GLESRenderer.cpp\
	renderers/gles/GLESTexture.cpp\
	renderers/gles/GLESTextureManager.cpp\
	renderers/gles/GLESUtils.cpp\
	renderers/gles/GLESVertexBuffer.cpp\
\
	Camera.cpp\
	Device.cpp\
	Font.cpp\
	FontManager.cpp\
	Image.cpp\
	ImageLoader.cpp\
	Log.cpp\
	Material.cpp\
	MaterialManager.cpp\
	MeshChunk.cpp\
	Mesh.cpp\
	MeshManager.cpp\
	MovableCamera.cpp\
	Pixel.cpp\
	Renderer.cpp\
	ResourceManager.cpp\
	Skybox.cpp\
	FPSSystem.cpp\
\
	samples/android/triangle.cpp\
\

LOCAL_C_INCLUDES	:=\
	$(LOCAL_PATH)/core\
	$(LOCAL_PATH)/core/math\
	$(LOCAL_PATH)/core/compressors\
	$(LOCAL_PATH)/core/containers\
	$(LOCAL_PATH)/core/bv\
	$(LOCAL_PATH)/core/mem\
	$(LOCAL_PATH)/core/streams\
	$(LOCAL_PATH)/loaders\
	$(LOCAL_PATH)/os\
	$(LOCAL_PATH)/os/android\
	$(LOCAL_PATH)/renderers\
	$(LOCAL_PATH)/things\
	$(LOCAL_PATH)/filesystem\
	$(LOCAL_PATH)/renderers/gl\
	$(LOCAL_PATH)/renderers/gles\
	$(LOCAL_PATH)/renderers/gles/egl\
	$(LOCAL_PATH)/input\
	$(LOCAL_PATH)/samples\


LOCAL_CPPFLAGS	:= -g -fexceptions
LOCAL_LDLIBS	:= -llog -landroid -lGLESv1_CM
include $(BUILD_SHARED_LIBRARY)
#(call import-module, android/native_app_glue)