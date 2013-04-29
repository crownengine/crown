cmake_minimum_required(VERSION 2.8)

project(crown-gles)

set (GLES_SRC
	GLESIndexBuffer.cpp
	GLESVertexBuffer.cpp
	GLESRenderer.cpp
	GLESUtils.cpp
)

set (GLES_HEADERS
	GLESIndexBuffer.h
	GLESVertexBuffer.h
	GLESRenderer.h
	GLESUtils.h
)

link_libraries(GLESv1_CM)

add_library(crown-gles SHARED ${GLES_SRC} ${GLES_HEADERS})

install (TARGETS crown-gles DESTINATION lib/${CMAKE_PROJECT_NAME})
