cmake_minimum_required(VERSION 3.29)
set(CMAKE_CXX_STANDARD 26)
project(PlunksnaTEST)

include(../cmake/Dependencies.cmake)

include(CTest)

#Catch.cmake may not be found easily on some systems
include(Catch OPTIONAL RESULT_VARIABLE CATCH_MODULE_PATH)

if(NOT CATCH_MODULE_PATH)
    message(STATUS "Standard Catch.cmake not found, trying Catch2_DIR...")
    include("${Catch2_DIR}/Catch.cmake" OPTIONAL RESULT_VARIABLE CATCH_DIRECT_PATH)
endif()

if(NOT COMMAND catch_discover_tests)
    message(FATAL_ERROR "Could not find Catch.cmake to provide catch_discover_tests!")
endif()

add_executable(test_run
        test_Random.cpp
        TestIncludes.h
        test_ECS.cpp
)

target_link_libraries(test_run PRIVATE Catch2::Catch2WithMain)
target_include_directories(test_run PRIVATE ${CMAKE_SOURCE_DIR})

catch_discover_tests(test_run)