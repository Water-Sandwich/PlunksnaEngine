cmake_minimum_required(VERSION 3.29)
project(Plunksna)
set(CMAKE_CXX_STANDARD 26)

add_compile_options(-Werror -Wreturn-type)

include(cmake/Dependencies.cmake)

add_subdirectory(ECS)
add_subdirectory(Engine)
add_subdirectory(VKRenderer)
add_subdirectory(AssetHandler)

add_executable(Plunksna main.cpp)

target_link_libraries(Plunksna
        PRIVATE
        engine
        vkrenderer
        assethandler
)

add_subdirectory(tests)