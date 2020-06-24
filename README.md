# opengl-surface
Basic matrix transformations + Phong shading through OpenGL

# Building
Create a build directory

    mkdir build

then generate building files, you can also specify a generator by `-G "Generator Name"`

    cmake -S . -B build

and build.

    cmake --build build

# Running
Your executable is in the `build` directory. 
    
    ./build/surface

# Controls
Using **arrow** keys press **UP** to zoom in and **DOWN** to zoom out.
