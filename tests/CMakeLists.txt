cmake_minimum_required(VERSION 2.8)

project(tests)

link_directories(${CROWN_BINARY_DIR})

link_libraries(GLU pthread)
add_executable(terrain terrain/terrain.cpp)
add_executable(texturing texturing/texturing.cpp)
add_executable(trash trash/trash.cpp)

set (CHAINSAWBUFFET_SRC
	chainsawbuffet/Globals.cpp
	chainsawbuffet/Globals.h
	chainsawbuffet/main.cpp
	chainsawbuffet/entities/Background.cpp
	chainsawbuffet/entities/Bullet.cpp
	chainsawbuffet/entities/Joystick.cpp
	chainsawbuffet/entities/Pg.cpp
	chainsawbuffet/entities/SolidSceneNode.cpp
	chainsawbuffet/entities/Wall.cpp
	chainsawbuffet/entities/Zombie.cpp
	chainsawbuffet/entities/Background.h
	chainsawbuffet/entities/Bullet.h
	chainsawbuffet/entities/Joystick.h
	chainsawbuffet/entities/Pg.h
	chainsawbuffet/entities/SolidSceneNode.h
	chainsawbuffet/entities/Wall.h
	chainsawbuffet/entities/Zombie.h
	chainsawbuffet/scenes/ArenaScene.cpp
	chainsawbuffet/scenes/ArenaScene.h
)

include_directories(chainsawbuffet chainsawbuffet/entities chainsawbuffet/scenes)
add_executable(chain ${CHAINSAWBUFFET_SRC})

add_executable(fs filesystem/filesystem.cpp)
add_executable(textrenderer textrenderer/textrenderer.cpp)
add_executable(collision collision/collision.cpp)

target_link_libraries(terrain crown)
target_link_libraries(texturing crown)
target_link_libraries(trash crown)
target_link_libraries(chain crown)
target_link_libraries(fs crown)
target_link_libraries(textrenderer crown)
target_link_libraries(collision crown)
