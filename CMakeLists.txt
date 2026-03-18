cmake_minimum_required(VERSION 3.29)
project(Plunksna)
set(CMAKE_CXX_STANDARD 26)

add_compile_options(-Werror -Wreturn-type)

include(cmake/Dependencies.cmake)

add_subdirectory(ecs)
add_subdirectory(engine)
add_subdirectory(vkRenderer)
add_subdirectory(assetHandler)
add_subdirectory(gui)

add_executable(Plunksna main.cpp)

target_link_libraries(Plunksna
        PRIVATE
        engine
        vkrenderer
        assethandler
        GUI
)

target_compile_definitions(Plunksna PRIVATE TRACY_ENABLE)

enable_testing()
add_subdirectory(tests)