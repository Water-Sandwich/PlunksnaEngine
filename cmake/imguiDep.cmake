include(${CMAKE_CURRENT_LIST_DIR}/find.cmake)

fetchPackage(
        imgui
        https://github.com/ocornut/imgui.git
        v1.92.6-docking
)

FetchContent_GetProperties(imgui)

if(NOT TARGET imgui)
    set(CURRENT_DIR ${imgui_SOURCE_DIR})

    file(GLOB SRC CONFIGURE_DEPENDS
            ${CURRENT_DIR}/*.cpp
            ${CURRENT_DIR}/backends/imgui_imp_sdl3.cpp
            ${CURRENT_DIR}/backends/imgui_imp_vulkan.cpp
    )

    add_library(imgui STATIC)
    target_sources(imgui PRIVATE ${SRC})
    target_include_directories(imgui PUBLIC
            ${CURRENT_DIR}
            ${CURRENT_DIR}/backends
    )
endif()

get_target_property(INCLUDE_DIRS imgui INTERFACE_INCLUDE_DIRECTORIES)
message(STATUS "Imgui include dirs: ${INCLUDE_DIRS}")