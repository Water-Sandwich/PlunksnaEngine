add_library(assethandler)

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS
        *.cpp
)

target_sources(assethandler PRIVATE ${SRC})

target_include_directories(assethandler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(assethandler PUBLIC ${PROJECT_SOURCE_DIR})

target_link_libraries(assethandler PUBLIC Vulkan::Vulkan Tracy::TracyClient)

