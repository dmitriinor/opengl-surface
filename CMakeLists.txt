cmake_minimum_required(VERSION 3.1..3.16)
set(CMAKE_CXX_STANDARD 14)

# create a project
project(OpenGL_surface VERSION 1.0 DESCRIPTION "Learning OpenGL" LANGUAGES CXX)

# add an executable target opengl from main.cpp
add_executable(surface src/main.cpp include/lodepng/lodepng.cpp)

# set a path to an include directory (for GL/GLFW header files)
target_include_directories(surface PUBLIC include)

# add a library target for our mat4x4 library
add_library(mat4x4 include/Mat4x4/mat4x4.cpp)
target_link_libraries(surface mat4x4)

# add a library target for our vec3 library
add_library(vec3 include/vec3/vec3.cpp)
target_link_libraries(surface vec3)

# link against GLEW and GLFW libraries.

if(MINGW)
  target_link_libraries(surface ${CMAKE_SOURCE_DIR}/lib/mingw/libglfw3.a 
                               ${CMAKE_SOURCE_DIR}/lib/mingw/libglew32.a)
elseif(CMAKE_COMPILER_IS_GNUCXX)
  if (UNIX AND NOT APPLE)
    target_link_libraries(surface ${CMAKE_SOURCE_DIR}/lib/gcc/libglfw3.a
                                 ${CMAKE_SOURCE_DIR}/lib/gcc/libGLEW.a)
  else()
    # GLEW
	find_package(GLEW REQUIRED)
    target_link_libraries(surface GLEW::GLEW)
    message(STATUS "Found GLEW in ${GLEW_INCLUDE_DIR}")
    # GLFW3
	find_package(GLFW3 REQUIRED)
    target_link_libraries(surface GLFW3::GLFW3)
    message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")
  endif()
elseif(MSVC)
  target_link_libraries(surface ${CMAKE_SOURCE_DIR}/lib/msvc/glew32s.lib
                               ${CMAKE_SOURCE_DIR}/lib/msvc/glfw3.lib)
endif()

# link against a GL
find_package(OpenGL REQUIRED)
target_link_libraries(surface OpenGL::GL)

if(UNIX AND NOT APPLE)
  target_link_libraries(surface dl X11 -pthread)
endif()