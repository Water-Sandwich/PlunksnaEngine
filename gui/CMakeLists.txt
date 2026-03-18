add_library(GUI)

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS
        *.cpp
)

target_link_libraries(GUI PUBLIC SDL3::SDL3 Tracy::TracyClient Vulkan::Vulkan imgui)

target_sources(GUI PRIVATE ${SRC})

target_include_directories(GUI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(GUI PUBLIC ${PROJECT_SOURCE_DIR})

