cmake_minimum_required(VERSION 3.1..3.16)
set(CMAKE_CXX_STANDARD 14)

#creating a project
project(OpenGL_surface VERSION 1.0 DESCRIPTION "Learning OpenGL" LANGUAGES CXX)

#executable target opengl from main.cpp.
add_executable(surface src/main.cpp)

#here we set a path to include directory (for GL/GLFW header files)
target_include_directories(surface PUBLIC include)

#library target for our mat4x4 library
add_library(mat4x4 include/Mat4x4/mat4x4.cpp)
target_link_libraries(surface mat4x4)

#library target for our vec3 library
add_library(vec3 include/vec3/vec3.cpp)
target_link_libraries(surface vec3)

#linking against libraries.
#specifying which libraries to link against statically depending on used compiler
if(MINGW)
  target_link_libraries(surface ${CMAKE_SOURCE_DIR}/lib/mingw/libglfw3.a 
                               ${CMAKE_SOURCE_DIR}/lib/mingw/libglew32.a)
elseif(CMAKE_COMPILER_IS_GNUCXX)
  if (UNIX AND NOT APPLE)
    target_link_libraries(surface ${CMAKE_SOURCE_DIR}/lib/gcc/libglfw3.a
                                 ${CMAKE_SOURCE_DIR}/lib/gcc/libGLEW.a)
  else()
    target_link_libraries(surface ${CMAKE_SOURCE_DIR}/lib/macos/libglfw3.a
	                              ${CMAKE_SOURCE_DIR}/lib/macos/libGLEW.a)
  endif()
elseif(MSVC)
  target_link_libraries(surface ${CMAKE_SOURCE_DIR}/lib/msvc/glew32s.lib
                               ${CMAKE_SOURCE_DIR}/lib/msvc/glfw3.lib)
endif()

find_package(OpenGL REQUIRED)
target_link_libraries(surface OpenGL::GL)

if(UNIX AND NOT APPLE)
  target_link_libraries(surface dl X11 -pthread)
endif()