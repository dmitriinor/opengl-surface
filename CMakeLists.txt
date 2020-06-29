cmake_minimum_required(VERSION 3.10..3.16)

# create a project
project(OpenGL_surface VERSION 1.0 DESCRIPTION "Learning OpenGL" LANGUAGES CXX)

# add an executable target opengl from main.cpp
add_executable(surface src/main.cpp include/lodepng/lodepng.cpp)

# set compile features
target_compile_features(surface PRIVATE cxx_std_11)

# set a path to an include directory (for GL/GLFW header files)
target_include_directories(surface PUBLIC include)

# add a library target for our mat4x4 library
add_library(mat4x4 include/Mat4x4/mat4x4.cpp)
target_link_libraries(surface mat4x4)

# add a library target for our vec3 library
add_library(vec3 include/Vec3/Vec3.cpp)
target_link_libraries(surface vec3)

# check for OpenGL
find_package(OpenGL REQUIRED)
if(NOT OPENGL_FOUND)
     message("ERROR: OpenGL not found")
endif()

# link against GLEW and GLFW libraries.
if(WIN32 AND MINGW)
  target_link_libraries(surface ${CMAKE_SOURCE_DIR}/lib/mingw/libglfw3.a 
                               ${CMAKE_SOURCE_DIR}/lib/mingw/libglew32.a)
  set(LIBS opengl32)
elseif(WIN32 AND MSVC)
  target_link_libraries(surface ${CMAKE_SOURCE_DIR}/lib/msvc/glew32s.lib
                               ${CMAKE_SOURCE_DIR}/lib/msvc/glfw3.lib)
  set(LIBS opengl32)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND UNIX AND NOT APPLE)
    target_link_libraries(surface ${CMAKE_SOURCE_DIR}/lib/gnu/libglfw3.a
                                 ${CMAKE_SOURCE_DIR}/lib/gnu/libGLEW.a)
  set(LIBS GL dl x11 -pthread)
elseif(APPLE)
    target_link_libraries(surface ${CMAKE_SOURCE_DIR}/lib/macos/libglfw3.a
	                              ${CMAKE_SOURCE_DIR}/lib/macos/libGLEW.a)
  set(LIBS "-framework OpenGL")
endif()

# link against other libs
target_link_libraries(surface ${LIBS})