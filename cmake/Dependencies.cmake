include(FetchContent)

function(findOrFetch package_name repo_url tag)
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
    else()
        message(STATUS "Using system ${package_name}...")
    endif()
endfunction()

findOrFetch(
        SDL3
        https://github.com/libsdl-org/SDL.git
        release-3.2.14
)

findOrFetch(
        glm
        https://github.com/g-truc/glm.git
        1.0.1
)

findOrFetch(
        VulkanMemoryAllocator
        https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
        3.3.0
)

findOrFetch(
        Tracy
        https://github.com/wolfpld/tracy.git
        v0.13.1
)


find_package(Vulkan REQUIRED)

#find_package(Tracy CONFIG QUIET)
#
#if(NOT Tracy_FOUND)
#    FetchContent_Declare(
#            Tracy
#            GIT_REPOSITORY https://github.com/wolfpld/tracy.git
#            GIT_TAG v0.13.1
#    )
#
#    FetchContent_MakeAvailable(Tracy)
#endif()

