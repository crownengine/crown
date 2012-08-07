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
	${OBJECTDIR}/_ext/1694839831/XWMLReader.o


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
LDLIBSOPTIONS=../crown/dist/Debug/GNU-Linux-x86/libcrown.a -lGL -lX11 -lGLEW -lXrandr -lfreetype

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/tests

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/tests: ../crown/dist/Debug/GNU-Linux-x86/libcrown.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/tests: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/tests ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/_ext/1694839831/XWMLReader.o: ../../../src/tests/xwmlreader/XWMLReader.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1694839831
	${RM} $@.d
	$(COMPILE.cc) -g -I../../../src/tests -I../../../src/core/math -I../../../src/core/mem -I../../../src/core -I../../../src/renderers/gl -I../../../src/renderers/gl/glx -I../../../src/renderers/gl/wgl -I../../../src/windowing -I../../../src/core/bv -I../../../src/windowing/layouts -I../../../src -I/usr/include/freetype2 -I../../../src/windowing/themes -I../../../src/core/containers -I../../../src/filesystem -I../../../src/core/streams -I../../../src/loaders -I../../../src/windowing/templates -I../../../src/windowing/toolbox -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1694839831/XWMLReader.o ../../../src/tests/xwmlreader/XWMLReader.cpp

# Subprojects
.build-subprojects:
	cd ../crown && ${MAKE}  -f Makefile CONF=Debug
	cd ../crown && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/tests

# Subprojects
.clean-subprojects:
	cd ../crown && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../crown && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
