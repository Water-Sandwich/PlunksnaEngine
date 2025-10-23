## Purpose

Plunksna is a work in progress game engine with a custom Entity Component System (ECS) heavily leveraging template metaprogramming. 

## Usage

Currently, if you would like to make a game with this, you will have to write code within the engine engine itself, however, I plan to properly seperate the engine code into a static library which your project could link to.

## Dependencies

SDL3 is used for input, window management and 2D rendering
GLM is used for mathematics
Catch2 is used for unit testing
CMake and make for building

### Linux

install sld3, glm, catch2 using your distro's package manager such as:

```
# pacman -Sy sdl3 glm catch2
```

### Windows

Make sure the dependencies are built and properly placed in your PATH such that CMake can see those files during building in case fetchcontent fails

## Build

In your terminal run the following:

```
Cmake CmakeLists.txt
make
```

and on Linux to run:

```
./Plunksna
```
