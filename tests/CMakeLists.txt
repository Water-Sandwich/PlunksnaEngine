find_package(Catch2 3 REQUIRED)


add_executable(test_run test_Random.cpp
        test_Random.cpp
        TestIncludes.h
        test_ECS.cpp)

#target_link_libraries(test_ECS_run PRIVATE Plunksna)
target_link_libraries(test_run PRIVATE Catch2::Catch2WithMain)
target_include_directories(test_run PRIVATE ${CMAKE_SOURCE_DIR})

include(Catch)
catch_discover_tests(test_run)

