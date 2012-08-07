#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/869353830/XMLReader.o \
	${OBJECTDIR}/_ext/1386528437/SceneNode.o \
	${OBJECTDIR}/_ext/1386528437/Scene.o \
	${OBJECTDIR}/_ext/1602971067/Filesystem.o \
	${OBJECTDIR}/_ext/869353830/BMPImageLoader.o \
	${OBJECTDIR}/_ext/1386528437/Device.o \
	${OBJECTDIR}/_ext/1981702495/LunaTheme.o \
	${OBJECTDIR}/_ext/613309208/Widget.o \
	${OBJECTDIR}/_ext/613309208/Bind.o \
	${OBJECTDIR}/_ext/845317374/GLXRenderWindow.o \
	${OBJECTDIR}/_ext/1386528437/MeshChunk.o \
	${OBJECTDIR}/_ext/1134220929/ManagedObject.o \
	${OBJECTDIR}/_ext/1386528437/Font.o \
	${OBJECTDIR}/_ext/613309208/Themes.o \
	${OBJECTDIR}/_ext/1487432554/Frustum.o \
	${OBJECTDIR}/_ext/1386528437/WindowEventHandler.o \
	${OBJECTDIR}/_ext/801113954/Angles.o \
	${OBJECTDIR}/_ext/1550003496/StringUtils.o \
	${OBJECTDIR}/_ext/1386528437/RenderWindow.o \
	${OBJECTDIR}/_ext/801113954/Vec4.o \
	${OBJECTDIR}/_ext/1386528437/Log.o \
	${OBJECTDIR}/_ext/2049961054/GLRenderer.o \
	${OBJECTDIR}/_ext/801113954/Mat4.o \
	${OBJECTDIR}/_ext/2049961054/GLTextRenderer.o \
	${OBJECTDIR}/_ext/613309208/DragArea.o \
	${OBJECTDIR}/_ext/869353830/XWMLReader.o \
	${OBJECTDIR}/_ext/1386528437/ResourceManager.o \
	${OBJECTDIR}/_ext/1386528437/Timer.o \
	${OBJECTDIR}/_ext/801113954/Point2.o \
	${OBJECTDIR}/_ext/801113954/Vec2.o \
	${OBJECTDIR}/_ext/1386528437/Camera.o \
	${OBJECTDIR}/_ext/1386528437/FontManager.o \
	${OBJECTDIR}/_ext/1386528437/MeshManager.o \
	${OBJECTDIR}/_ext/1575338083/FileStream.o \
	${OBJECTDIR}/_ext/1550003496/Generic.o \
	${OBJECTDIR}/_ext/4811410/StackLayout.o \
	${OBJECTDIR}/_ext/801113954/Color.o \
	${OBJECTDIR}/_ext/1386528437/Frame.o \
	${OBJECTDIR}/_ext/1386528437/XCursorControl.o \
	${OBJECTDIR}/_ext/1386528437/App.o \
	${OBJECTDIR}/_ext/801113954/Vec3.o \
	${OBJECTDIR}/_ext/869353830/TGAImageLoader.o \
	${OBJECTDIR}/_ext/2049961054/GLVertexBuffer.o \
	${OBJECTDIR}/_ext/1386528437/Image.o \
	${OBJECTDIR}/_ext/801113954/Plane.o \
	${OBJECTDIR}/_ext/1386528437/MovableCamera.o \
	${OBJECTDIR}/_ext/1134220929/GarbageBin.o \
	${OBJECTDIR}/_ext/1386528437/Pixel.o \
	${OBJECTDIR}/_ext/1386528437/Light.o \
	${OBJECTDIR}/_ext/845302165/WGLRenderWindow.o \
	${OBJECTDIR}/_ext/1602971067/Path.o \
	${OBJECTDIR}/_ext/613309208/Window.o \
	${OBJECTDIR}/_ext/608447161/Observable.o \
	${OBJECTDIR}/_ext/613309208/ListView.o \
	${OBJECTDIR}/_ext/1386528437/RenderTarget.o \
	${OBJECTDIR}/_ext/845302165/WGLGLSupport.o \
	${OBJECTDIR}/_ext/801113954/Mat3.o \
	${OBJECTDIR}/_ext/2049961054/GLTextureManager.o \
	${OBJECTDIR}/_ext/1386528437/Material.o \
	${OBJECTDIR}/_ext/4811410/CanvasLayout.o \
	${OBJECTDIR}/_ext/2049961054/GLIndexBuffer.o \
	${OBJECTDIR}/_ext/613309208/WindowsManager.o \
	${OBJECTDIR}/_ext/613309208/TextBox.o \
	${OBJECTDIR}/_ext/1386528437/Entity.o \
	${OBJECTDIR}/_ext/608447161/Object.o \
	${OBJECTDIR}/_ext/1575338083/Stream.o \
	${OBJECTDIR}/_ext/613309208/Label.o \
	${OBJECTDIR}/_ext/2049961054/GLTexture.o \
	${OBJECTDIR}/_ext/801113954/Quat.o \
	${OBJECTDIR}/_ext/1093628612/TextInputWindow.o \
	${OBJECTDIR}/_ext/2049961054/GLOcclusionQuery.o \
	${OBJECTDIR}/_ext/801113954/MathUtils.o \
	${OBJECTDIR}/_ext/1575338083/FileSubStream.o \
	${OBJECTDIR}/_ext/1386528437/Sprite.o \
	${OBJECTDIR}/_ext/845317374/GLXGLSupport.o \
	${OBJECTDIR}/_ext/869353830/CRWDecoder.o \
	${OBJECTDIR}/_ext/1386528437/Mesh.o \
	${OBJECTDIR}/_ext/1386528437/Skybox.o \
	${OBJECTDIR}/_ext/2049961054/GLSupport.o \
	${OBJECTDIR}/_ext/1093628612/MessageWindow.o \
	${OBJECTDIR}/_ext/613309208/ScrollArea.o \
	${OBJECTDIR}/_ext/613309208/Button.o \
	${OBJECTDIR}/_ext/1386528437/WinCursorControl.o \
	${OBJECTDIR}/_ext/1386528437/SceneManager.o \
	${OBJECTDIR}/_ext/845317374/GLXRenderContext.o \
	${OBJECTDIR}/_ext/845302165/WGLRenderContext.o \
	${OBJECTDIR}/_ext/1386528437/Viewport.o \
	${OBJECTDIR}/_ext/1575338083/MemoryStream.o \
	${OBJECTDIR}/_ext/1386528437/LogManager.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-ansi -W -Wall -Wextra -pedantic -Wno-long-long
CXXFLAGS=-ansi -W -Wall -Wextra -pedantic -Wno-long-long

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libcrown.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libcrown.a: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libcrown.a
	${AR} -rv ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libcrown.a ${OBJECTFILES} 
	$(RANLIB) ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libcrown.a

${OBJECTDIR}/_ext/869353830/XMLReader.o: ../../../src/loaders/XMLReader.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/869353830
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/869353830/XMLReader.o ../../../src/loaders/XMLReader.cpp

${OBJECTDIR}/_ext/1386528437/SceneNode.o: ../../../src/SceneNode.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/SceneNode.o ../../../src/SceneNode.cpp

${OBJECTDIR}/_ext/1386528437/Scene.o: ../../../src/Scene.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/Scene.o ../../../src/Scene.cpp

${OBJECTDIR}/_ext/1602971067/Filesystem.o: ../../../src/filesystem/Filesystem.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1602971067
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1602971067/Filesystem.o ../../../src/filesystem/Filesystem.cpp

${OBJECTDIR}/_ext/869353830/BMPImageLoader.o: ../../../src/loaders/BMPImageLoader.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/869353830
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/869353830/BMPImageLoader.o ../../../src/loaders/BMPImageLoader.cpp

${OBJECTDIR}/_ext/1386528437/Device.o: ../../../src/Device.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/Device.o ../../../src/Device.cpp

${OBJECTDIR}/_ext/1981702495/LunaTheme.o: ../../../src/windowing/themes/LunaTheme.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1981702495
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1981702495/LunaTheme.o ../../../src/windowing/themes/LunaTheme.cpp

${OBJECTDIR}/_ext/613309208/Widget.o: ../../../src/windowing/Widget.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/613309208
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/613309208/Widget.o ../../../src/windowing/Widget.cpp

${OBJECTDIR}/_ext/613309208/Bind.o: ../../../src/windowing/Bind.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/613309208
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/613309208/Bind.o ../../../src/windowing/Bind.cpp

${OBJECTDIR}/_ext/845317374/GLXRenderWindow.o: ../../../src/renderers/gl/glx/GLXRenderWindow.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/845317374
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/845317374/GLXRenderWindow.o ../../../src/renderers/gl/glx/GLXRenderWindow.cpp

${OBJECTDIR}/_ext/1386528437/MeshChunk.o: ../../../src/MeshChunk.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/MeshChunk.o ../../../src/MeshChunk.cpp

${OBJECTDIR}/_ext/1134220929/ManagedObject.o: ../../../src/core/mem/ManagedObject.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1134220929
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1134220929/ManagedObject.o ../../../src/core/mem/ManagedObject.cpp

${OBJECTDIR}/_ext/1386528437/Font.o: ../../../src/Font.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/Font.o ../../../src/Font.cpp

${OBJECTDIR}/_ext/613309208/Themes.o: ../../../src/windowing/Themes.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/613309208
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/613309208/Themes.o ../../../src/windowing/Themes.cpp

${OBJECTDIR}/_ext/1487432554/Frustum.o: ../../../src/core/bv/Frustum.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1487432554
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1487432554/Frustum.o ../../../src/core/bv/Frustum.cpp

${OBJECTDIR}/_ext/1386528437/WindowEventHandler.o: ../../../src/WindowEventHandler.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/WindowEventHandler.o ../../../src/WindowEventHandler.cpp

${OBJECTDIR}/_ext/801113954/Angles.o: ../../../src/core/math/Angles.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/801113954
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/801113954/Angles.o ../../../src/core/math/Angles.cpp

${OBJECTDIR}/_ext/1550003496/StringUtils.o: ../../../src/core/containers/StringUtils.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1550003496
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1550003496/StringUtils.o ../../../src/core/containers/StringUtils.cpp

${OBJECTDIR}/_ext/1386528437/RenderWindow.o: ../../../src/RenderWindow.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/RenderWindow.o ../../../src/RenderWindow.cpp

${OBJECTDIR}/_ext/801113954/Vec4.o: ../../../src/core/math/Vec4.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/801113954
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/801113954/Vec4.o ../../../src/core/math/Vec4.cpp

${OBJECTDIR}/_ext/1386528437/Log.o: ../../../src/Log.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/Log.o ../../../src/Log.cpp

${OBJECTDIR}/_ext/2049961054/GLRenderer.o: ../../../src/renderers/gl/GLRenderer.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/2049961054
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/2049961054/GLRenderer.o ../../../src/renderers/gl/GLRenderer.cpp

${OBJECTDIR}/_ext/801113954/Mat4.o: ../../../src/core/math/Mat4.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/801113954
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/801113954/Mat4.o ../../../src/core/math/Mat4.cpp

${OBJECTDIR}/_ext/2049961054/GLTextRenderer.o: ../../../src/renderers/gl/GLTextRenderer.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/2049961054
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/2049961054/GLTextRenderer.o ../../../src/renderers/gl/GLTextRenderer.cpp

${OBJECTDIR}/_ext/613309208/DragArea.o: ../../../src/windowing/DragArea.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/613309208
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/613309208/DragArea.o ../../../src/windowing/DragArea.cpp

${OBJECTDIR}/_ext/869353830/XWMLReader.o: ../../../src/loaders/XWMLReader.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/869353830
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/869353830/XWMLReader.o ../../../src/loaders/XWMLReader.cpp

${OBJECTDIR}/_ext/1386528437/ResourceManager.o: ../../../src/ResourceManager.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/ResourceManager.o ../../../src/ResourceManager.cpp

${OBJECTDIR}/_ext/1386528437/Timer.o: ../../../src/Timer.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/Timer.o ../../../src/Timer.cpp

${OBJECTDIR}/_ext/801113954/Point2.o: ../../../src/core/math/Point2.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/801113954
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/801113954/Point2.o ../../../src/core/math/Point2.cpp

${OBJECTDIR}/_ext/801113954/Vec2.o: ../../../src/core/math/Vec2.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/801113954
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/801113954/Vec2.o ../../../src/core/math/Vec2.cpp

${OBJECTDIR}/_ext/1386528437/Camera.o: ../../../src/Camera.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/Camera.o ../../../src/Camera.cpp

${OBJECTDIR}/_ext/1386528437/FontManager.o: ../../../src/FontManager.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/FontManager.o ../../../src/FontManager.cpp

${OBJECTDIR}/_ext/1386528437/MeshManager.o: ../../../src/MeshManager.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/MeshManager.o ../../../src/MeshManager.cpp

${OBJECTDIR}/_ext/1575338083/FileStream.o: ../../../src/core/streams/FileStream.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1575338083
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1575338083/FileStream.o ../../../src/core/streams/FileStream.cpp

${OBJECTDIR}/_ext/1550003496/Generic.o: ../../../src/core/containers/Generic.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1550003496
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1550003496/Generic.o ../../../src/core/containers/Generic.cpp

${OBJECTDIR}/_ext/4811410/StackLayout.o: ../../../src/windowing/layouts/StackLayout.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/4811410
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/4811410/StackLayout.o ../../../src/windowing/layouts/StackLayout.cpp

${OBJECTDIR}/_ext/801113954/Color.o: ../../../src/core/math/Color.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/801113954
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/801113954/Color.o ../../../src/core/math/Color.cpp

${OBJECTDIR}/_ext/1386528437/Frame.o: ../../../src/Frame.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/Frame.o ../../../src/Frame.cpp

${OBJECTDIR}/_ext/1386528437/XCursorControl.o: ../../../src/XCursorControl.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/XCursorControl.o ../../../src/XCursorControl.cpp

${OBJECTDIR}/_ext/1386528437/App.o: ../../../src/App.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/App.o ../../../src/App.cpp

${OBJECTDIR}/_ext/801113954/Vec3.o: ../../../src/core/math/Vec3.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/801113954
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/801113954/Vec3.o ../../../src/core/math/Vec3.cpp

${OBJECTDIR}/_ext/869353830/TGAImageLoader.o: ../../../src/loaders/TGAImageLoader.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/869353830
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/869353830/TGAImageLoader.o ../../../src/loaders/TGAImageLoader.cpp

${OBJECTDIR}/_ext/2049961054/GLVertexBuffer.o: ../../../src/renderers/gl/GLVertexBuffer.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/2049961054
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/2049961054/GLVertexBuffer.o ../../../src/renderers/gl/GLVertexBuffer.cpp

${OBJECTDIR}/_ext/1386528437/Image.o: ../../../src/Image.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/Image.o ../../../src/Image.cpp

${OBJECTDIR}/_ext/801113954/Plane.o: ../../../src/core/math/Plane.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/801113954
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/801113954/Plane.o ../../../src/core/math/Plane.cpp

${OBJECTDIR}/_ext/1386528437/MovableCamera.o: ../../../src/MovableCamera.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/MovableCamera.o ../../../src/MovableCamera.cpp

${OBJECTDIR}/_ext/1134220929/GarbageBin.o: ../../../src/core/mem/GarbageBin.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1134220929
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1134220929/GarbageBin.o ../../../src/core/mem/GarbageBin.cpp

${OBJECTDIR}/_ext/1386528437/Pixel.o: ../../../src/Pixel.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/Pixel.o ../../../src/Pixel.cpp

${OBJECTDIR}/_ext/1386528437/Light.o: ../../../src/Light.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/Light.o ../../../src/Light.cpp

${OBJECTDIR}/_ext/845302165/WGLRenderWindow.o: ../../../src/renderers/gl/wgl/WGLRenderWindow.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/845302165
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/845302165/WGLRenderWindow.o ../../../src/renderers/gl/wgl/WGLRenderWindow.cpp

${OBJECTDIR}/_ext/1602971067/Path.o: ../../../src/filesystem/Path.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1602971067
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1602971067/Path.o ../../../src/filesystem/Path.cpp

${OBJECTDIR}/_ext/613309208/Window.o: ../../../src/windowing/Window.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/613309208
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/613309208/Window.o ../../../src/windowing/Window.cpp

${OBJECTDIR}/_ext/608447161/Observable.o: ../../../src/core/Observable.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/608447161
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/608447161/Observable.o ../../../src/core/Observable.cpp

${OBJECTDIR}/_ext/613309208/ListView.o: ../../../src/windowing/ListView.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/613309208
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/613309208/ListView.o ../../../src/windowing/ListView.cpp

${OBJECTDIR}/_ext/1386528437/RenderTarget.o: ../../../src/RenderTarget.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/RenderTarget.o ../../../src/RenderTarget.cpp

${OBJECTDIR}/_ext/845302165/WGLGLSupport.o: ../../../src/renderers/gl/wgl/WGLGLSupport.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/845302165
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/845302165/WGLGLSupport.o ../../../src/renderers/gl/wgl/WGLGLSupport.cpp

${OBJECTDIR}/_ext/801113954/Mat3.o: ../../../src/core/math/Mat3.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/801113954
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/801113954/Mat3.o ../../../src/core/math/Mat3.cpp

${OBJECTDIR}/_ext/2049961054/GLTextureManager.o: ../../../src/renderers/gl/GLTextureManager.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/2049961054
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/2049961054/GLTextureManager.o ../../../src/renderers/gl/GLTextureManager.cpp

${OBJECTDIR}/_ext/1386528437/Material.o: ../../../src/Material.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/Material.o ../../../src/Material.cpp

${OBJECTDIR}/_ext/4811410/CanvasLayout.o: ../../../src/windowing/layouts/CanvasLayout.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/4811410
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/4811410/CanvasLayout.o ../../../src/windowing/layouts/CanvasLayout.cpp

${OBJECTDIR}/_ext/2049961054/GLIndexBuffer.o: ../../../src/renderers/gl/GLIndexBuffer.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/2049961054
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/2049961054/GLIndexBuffer.o ../../../src/renderers/gl/GLIndexBuffer.cpp

${OBJECTDIR}/_ext/613309208/WindowsManager.o: ../../../src/windowing/WindowsManager.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/613309208
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/613309208/WindowsManager.o ../../../src/windowing/WindowsManager.cpp

${OBJECTDIR}/_ext/613309208/TextBox.o: ../../../src/windowing/TextBox.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/613309208
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/613309208/TextBox.o ../../../src/windowing/TextBox.cpp

${OBJECTDIR}/_ext/1386528437/Entity.o: ../../../src/Entity.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/Entity.o ../../../src/Entity.cpp

${OBJECTDIR}/_ext/608447161/Object.o: ../../../src/core/Object.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/608447161
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/608447161/Object.o ../../../src/core/Object.cpp

${OBJECTDIR}/_ext/1575338083/Stream.o: ../../../src/core/streams/Stream.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1575338083
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1575338083/Stream.o ../../../src/core/streams/Stream.cpp

${OBJECTDIR}/_ext/613309208/Label.o: ../../../src/windowing/Label.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/613309208
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/613309208/Label.o ../../../src/windowing/Label.cpp

${OBJECTDIR}/_ext/2049961054/GLTexture.o: ../../../src/renderers/gl/GLTexture.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/2049961054
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/2049961054/GLTexture.o ../../../src/renderers/gl/GLTexture.cpp

${OBJECTDIR}/_ext/801113954/Quat.o: ../../../src/core/math/Quat.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/801113954
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/801113954/Quat.o ../../../src/core/math/Quat.cpp

${OBJECTDIR}/_ext/1093628612/TextInputWindow.o: ../../../src/windowing/toolbox/TextInputWindow.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1093628612
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1093628612/TextInputWindow.o ../../../src/windowing/toolbox/TextInputWindow.cpp

${OBJECTDIR}/_ext/2049961054/GLOcclusionQuery.o: ../../../src/renderers/gl/GLOcclusionQuery.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/2049961054
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/2049961054/GLOcclusionQuery.o ../../../src/renderers/gl/GLOcclusionQuery.cpp

${OBJECTDIR}/_ext/801113954/MathUtils.o: ../../../src/core/math/MathUtils.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/801113954
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/801113954/MathUtils.o ../../../src/core/math/MathUtils.cpp

${OBJECTDIR}/_ext/1575338083/FileSubStream.o: ../../../src/core/streams/FileSubStream.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1575338083
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1575338083/FileSubStream.o ../../../src/core/streams/FileSubStream.cpp

${OBJECTDIR}/_ext/1386528437/Sprite.o: ../../../src/Sprite.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/Sprite.o ../../../src/Sprite.cpp

${OBJECTDIR}/_ext/845317374/GLXGLSupport.o: ../../../src/renderers/gl/glx/GLXGLSupport.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/845317374
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/845317374/GLXGLSupport.o ../../../src/renderers/gl/glx/GLXGLSupport.cpp

${OBJECTDIR}/_ext/869353830/CRWDecoder.o: ../../../src/loaders/CRWDecoder.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/869353830
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/869353830/CRWDecoder.o ../../../src/loaders/CRWDecoder.cpp

${OBJECTDIR}/_ext/1386528437/Mesh.o: ../../../src/Mesh.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/Mesh.o ../../../src/Mesh.cpp

${OBJECTDIR}/_ext/1386528437/Skybox.o: ../../../src/Skybox.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/Skybox.o ../../../src/Skybox.cpp

${OBJECTDIR}/_ext/2049961054/GLSupport.o: ../../../src/renderers/gl/GLSupport.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/2049961054
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/2049961054/GLSupport.o ../../../src/renderers/gl/GLSupport.cpp

${OBJECTDIR}/_ext/1093628612/MessageWindow.o: ../../../src/windowing/toolbox/MessageWindow.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1093628612
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1093628612/MessageWindow.o ../../../src/windowing/toolbox/MessageWindow.cpp

${OBJECTDIR}/_ext/613309208/ScrollArea.o: ../../../src/windowing/ScrollArea.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/613309208
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/613309208/ScrollArea.o ../../../src/windowing/ScrollArea.cpp

${OBJECTDIR}/_ext/613309208/Button.o: ../../../src/windowing/Button.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/613309208
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/613309208/Button.o ../../../src/windowing/Button.cpp

${OBJECTDIR}/_ext/1386528437/WinCursorControl.o: ../../../src/WinCursorControl.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/WinCursorControl.o ../../../src/WinCursorControl.cpp

${OBJECTDIR}/_ext/1386528437/SceneManager.o: ../../../src/SceneManager.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/SceneManager.o ../../../src/SceneManager.cpp

${OBJECTDIR}/_ext/845317374/GLXRenderContext.o: ../../../src/renderers/gl/glx/GLXRenderContext.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/845317374
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/845317374/GLXRenderContext.o ../../../src/renderers/gl/glx/GLXRenderContext.cpp

${OBJECTDIR}/_ext/845302165/WGLRenderContext.o: ../../../src/renderers/gl/wgl/WGLRenderContext.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/845302165
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/845302165/WGLRenderContext.o ../../../src/renderers/gl/wgl/WGLRenderContext.cpp

${OBJECTDIR}/_ext/1386528437/Viewport.o: ../../../src/Viewport.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/Viewport.o ../../../src/Viewport.cpp

${OBJECTDIR}/_ext/1575338083/MemoryStream.o: ../../../src/core/streams/MemoryStream.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1575338083
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1575338083/MemoryStream.o ../../../src/core/streams/MemoryStream.cpp

${OBJECTDIR}/_ext/1386528437/LogManager.o: ../../../src/LogManager.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/windowing/toolbox -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1386528437/LogManager.o ../../../src/LogManager.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libcrown.a

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
