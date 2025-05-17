cmake_minimum_required(VERSION 3.29)
project(Plunksna)
set(CMAKE_CXX_STANDARD 26)

find_package(SDL3 REQUIRED)
find_package(glm REQUIRED)

add_executable(Plunksna main.cpp
        engine/Engine.cpp
        engine/Engine.h
        engine/Window.cpp
        engine/Window.h
        engine/Exception.cpp
        engine/Exception.h
        engine/Log.h
        engine/GameObject.cpp
        engine/GameObject.h
        ecs/Registry.cpp
        ecs/Registry.h
        ecs/ComponentStore.cpp
        ecs/ComponentStore.h
        ecs/PaginatedVector.cpp
        ecs/PaginatedVector.h
)

TARGET_LINK_LIBRARIES(Plunksna PRIVATE SDL3)