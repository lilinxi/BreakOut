cmake_minimum_required(VERSION 3.14)
project(BreakOut)

set(CMAKE_CXX_STANDARD 14)

set(GLFW_H /usr/local/Cellar/glfw/3.3/include/GLFW)                     # brew install glfw
set(GLAD_H /usr/local/include/glad)                                     # http://glad.dav1d.de/
set(GLM_H /usr/local/Cellar/glm/0.9.9.5/include/glm)                    # brew install glm
set(STB_H /usr/local/include/stb)                                       # https://github.com/nothings/stb
set(ASSIMP_H /usr/local/Cellar/assimp/4.1.0/include/assimp)             # brew install assimp
set(FREETYPE_H /usr/local/Cellar/freetype/2.10.1/include/freetype2)     # brew install freetype
include_directories(${GLFW_H} ${GLAD_H} ${GLM_H} ${STB_H} ${ASSIMP_H} ${FREETYPE_H})

set(GLFW_LINK /usr/local/Cellar/glfw/3.3/lib/libglfw.3.dylib)
set(ASSIMP_LINK /usr/local/Cellar/assimp/4.1.0/lib/libassimp.4.1.0.dylib)
set(FREETYPE_LINK /usr/local/Cellar/freetype/2.10.1/lib/libfreetype.dylib)
link_libraries(${OPENGL} ${GLFW_LINK} ${ASSIMP_LINK} ${FREETYPE_LINK})

set(SOURCE_FILES src/glad.c main.cpp)
add_executable(BreakOut main.cpp)
if (APPLE)
    target_link_libraries(BreakOut "-framework OpenGL")
endif ()