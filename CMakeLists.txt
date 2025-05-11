cmake_minimum_required(VERSION 3.29)
project(Plunksna)
set(CMAKE_CXX_STANDARD 26)

find_package(SDL3 REQUIRED)

add_executable(Plunksna main.cpp
        engine/PlunksnaEngine.cpp
        engine/PlunksnaEngine.h
        engine/PlunksnaWindow.cpp
        engine/PlunksnaWindow.h
        engine/PlunksnaException.cpp
        engine/PlunksnaException.h
        engine/PlunksnaLog.h)

TARGET_LINK_LIBRARIES(Plunksna PRIVATE SDL3)