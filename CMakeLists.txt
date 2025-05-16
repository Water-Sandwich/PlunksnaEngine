cmake_minimum_required(VERSION 3.29)
project(Plunksna)
set(CMAKE_CXX_STANDARD 26)

find_package(SDL3 REQUIRED)

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
)

TARGET_LINK_LIBRARIES(Plunksna PRIVATE SDL3)