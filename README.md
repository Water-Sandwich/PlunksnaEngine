# Plunksna Engine

## Description

Plunksna is a work in progress 3D Vulkan game engine with a custom Entity Component System (ECS) using template metaprogramming.

## Planned features

- Seperation of engine code into seperate static library
- Draw sorting and culling
- Physics
- Audio

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
# pacman -Sy vulkan-devel stb {optionally: glm, catch2, ...}
```

The other dependecies can be fetched by CMake using FetchContent, however the build system will prefer to use system installed packages over fetched ones.

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
