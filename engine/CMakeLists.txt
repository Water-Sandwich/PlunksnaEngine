add_library(engine)

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS
    *.cpp
)

target_sources(engine PRIVATE ${SRC})

target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(engine PUBLIC ${PROJECT_SOURCE_DIR})

target_link_libraries(engine PUBLIC SDL3::SDL3 Tracy::TracyClient)

