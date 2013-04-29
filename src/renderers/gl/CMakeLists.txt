cmake_minimum_required(VERSION 2.8)

project(crown-gl)

set (GL_SRC
	GLIndexBuffer.cpp
	GLOcclusionQuery.cpp
	GLRenderer.cpp
	GLUtils.cpp
	GLVertexBuffer.cpp
)

set (GL_HEADERS
	GLIndexBuffer.h
	GLOcclusionQuery.h
	GLRenderer.h
	GLUtils.h
	GLVertexBuffer.h
)

link_libraries(GL GLEW)

add_library(crown-gl SHARED ${GL_SRC} ${GL_HEADERS})

#target_link_libraries(crown-gl crown)

install (TARGETS crown-gl DESTINATION lib/${CMAKE_PROJECT_NAME})
