cmake_minimum_required(VERSION 3.14)
project(BreakOut)

set(CMAKE_CXX_STANDARD 14)

set(GLFW_H /usr/local/Cellar/glfw/3.3/include/GLFW)                     # brew install glfw
set(GLAD_H /usr/local/include/glad)                                     # http://glad.dav1d.de/
set(GLM_H /usr/local/Cellar/glm/0.9.9.5/include/glm)                    # brew install glm
set(STB_H /usr/local/include/stb)                                       # https://github.com/nothings/stb
set(ASSIMP_H /usr/local/Cellar/assimp/4.1.0/include/assimp)             # brew install assimp
set(FREETYPE_H /usr/local/Cellar/freetype/2.10.1/include/freetype2)     # brew install freetype
set(IRRKlANG_H /usr/local/Cellar/irrKlang/include)                      # https://www.ambiera.com/irrklang/downloads.html mp3 to wav : https://convertio.co/zh/mp3-wav/
set(SRC_H src)
include_directories(${GLFW_H} ${GLAD_H} ${GLM_H} ${STB_H} ${ASSIMP_H} ${FREETYPE_H} ${IRRKlANG_H} ${SRC_H})

set(GLFW_LINK /usr/local/Cellar/glfw/3.3/lib/libglfw.3.dylib)
set(ASSIMP_LINK /usr/local/Cellar/assimp/4.1.0/lib/libassimp.4.1.0.dylib)
set(FREETYPE_LINK /usr/local/Cellar/freetype/2.10.1/lib/libfreetype.dylib)
set(IRRKlANG_LINK /usr/local/Cellar/irrKlang/bin/macosx-gcc/libirrklang.dylib)
link_libraries(${OPENGL} ${GLFW_LINK} ${ASSIMP_LINK} ${FREETYPE_LINK} ${IRRKlANG_LINK})

set(SOURCE_FILES src/glad.c main.cpp src/game.cpp src/game.h src/shader.cpp src/shader.h src/texture2d.cpp src/texture2d.h src/resource_manager.cpp src/resource_manager.h src/sprite_renderer.cpp src/sprite_renderer.h src/game_object.cpp src/game_object.h src/game_level.cpp src/game_level.h src/ball_object.cpp src/ball_object.h src/particle_generator.cpp src/particle_generator.h src/post_processor.cpp src/post_processor.h src/power_up.h src/text_renderer.cpp src/text_renderer.h)
add_executable(BreakOut ${SOURCE_FILES})
if (APPLE)
    target_link_libraries(BreakOut "-framework OpenGL")
endif ()