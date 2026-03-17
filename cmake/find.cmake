include(FetchContent)
set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/libs)

function(fetchPackage package_name repo_url tag)
    FetchContent_Declare(
            ${package_name}
            GIT_REPOSITORY ${repo_url}
            GIT_TAG ${tag}
    )
    FetchContent_MakeAvailable(${package_name})

    # Optionally set ${package_name}_FOUND to TRUE
    set(${package_name}_FOUND TRUE PARENT_SCOPE)
endfunction()

function(findOrFetch package_name repo_url tag)
    find_package(${package_name} QUIET)

    if (NOT ${package_name}_FOUND)
        message(STATUS "${package_name} not found, fetching from ${repo_url}...")

        fetchPackage(${package_name} ${repo_url} ${tag})
    else()
        message(STATUS "Using system ${package_name}...")
    endif()
endfunction()