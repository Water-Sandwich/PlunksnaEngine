add_library(vkrenderer)

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS
        *.cpp
)

target_sources(vkrenderer PRIVATE ${SRC})

target_include_directories(vkrenderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(vkrenderer PUBLIC ${PROJECT_SOURCE_DIR})

target_link_libraries(vkrenderer PUBLIC SDL3::SDL3 Tracy::TracyClient Vulkan::Vulkan)

