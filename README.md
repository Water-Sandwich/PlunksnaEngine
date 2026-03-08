# Plunksna Engine

## Purpose

Plunksna is a work in progress 3D Vulkan game engine with a custom Entity Component System (ECS) using template metaprogramming.

## Planned features

- Seperation of engine code into seperate static library
- Asset management
- Entity handles
- Unified rendering and audio

## Usage

Currently, if you would like to make a game with this, you will have to write code within the engine engine itself, however, I plan to properly seperate the engine code into a static library which your project could link to.

## Dependencies

- SDL3: Input and window management
- Vulkan: 3D renderering
- Vulkan Memory Allocator: Vulkan helper
- glm: Mathematics
- tinyobjloader: Loading 3D assets
- stb_image: Loading textures
- Tracy: CPU and GPU profiling 
- Catch2: Testing
- CMake and make: Building the project

### Linux

Install the dependencies above using your distro's package manager such as:

```
# paru -Sy sdl3 vulkan-devel vulkan-memory-allocator glm tinyobjloader stb catch2
```

SDL3, glm, VMA, tinyobjloader and Tracy can be omitted and you can let CMake fetch it for you.

### Windows

Make sure the dependencies are built and properly placed in your PATH such that CMake can see those files during building in case fetchcontent fails

## Build

In your terminal run the following:

```
git clone https://github.com/Water-Sandwich/PlunksnaEngine.git
cd PlunksnaEngine/
Cmake CmakeLists.txt
make
```

and on Linux to run the project:

```
./Plunksna
```
