# opengl-surface
Basic matrix transformations + Phong shading through OpenGL
Using GLEW and GLFW libraries

# Requirements
GLFW and GLEW binaries are included for Windows, no need to install anything.

For other platforms please install glew 2.1 and glfw 3.3 with your package manager.

# Building
Create a build directory

    mkdir build

then generate building files, you can also specify a generator (e.g. `-G "Ninja"`)

    cmake -S . -B build

and build.

    cmake --build build

# Running
Your executable is in the `build` directory. 
    
    ./build/surface

# Controls
Using **arrow** keys press **UP** to zoom in and **DOWN** to zoom out.
