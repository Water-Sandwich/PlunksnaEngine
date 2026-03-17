include(${CMAKE_CURRENT_LIST_DIR}/find.cmake)

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

findOrFetch(
        tinyobjloader
        https://github.com/tinyobjloader/tinyobjloader.git
        v1.0.6
)

findOrFetch(
        Catch2
        https://github.com/catchorg/Catch2.git
        v3.13.0
)

find_package(Vulkan REQUIRED)

include(${CMAKE_CURRENT_LIST_DIR}/imguiDep.cmake)