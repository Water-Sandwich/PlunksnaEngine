cmake_minimum_required(VERSION 3.29)
project(Plunksna)
set(CMAKE_CXX_STANDARD 26)

function(find_or_fetch package_name repo_url tag target_name)
    find_package(${package_name} QUIET)

    if (NOT ${package_name}_FOUND)
        message(STATUS "${package_name} not found, fetching from ${repo_url}...")

        include(FetchContent)
        FetchContent_Declare(
                ${package_name}
                GIT_REPOSITORY ${repo_url}
                GIT_TAG ${tag}
        )
        FetchContent_MakeAvailable(${package_name})

        # Optionally set ${package_name}_FOUND to TRUE
        set(${package_name}_FOUND TRUE PARENT_SCOPE)
    endif()
endfunction()

find_or_fetch(
        SDL3
        https://github.com/libsdl-org/SDL.git
        release-3.2.14
        SDL3::SDL3
)

find_or_fetch(
        glm
        https://github.com/g-truc/glm.git
        1.0.1
        glm
)

file(GLOB_RECURSE ENGINE_SRC CONFIGURE_DEPENDS engine/*.cpp engine/*.h)
file(GLOB_RECURSE ECS_SRC CONFIGURE_DEPENDS ecs/*.cpp ecs/*.h ecs/*.tpp)

set(SOURCE_FILES
        main.cpp
        ${ENGINE_SRC}
        ${ECS_SRC}
        Tests.cpp
)

add_executable(Plunksna ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(Plunksna PRIVATE SDL3 glm)