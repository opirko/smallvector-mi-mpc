cmake_minimum_required(VERSION 3.16)
project(small-vector CXX)
set(PROJECT_VERSION 0.1.0)

######################################
# determine git commit of the sources

if(IS_DIRECTORY "${CMAKE_SOURCE_DIR}/.git")
    # a git repo
    find_package(Git REQUIRED)

    # output style is:
    # 1.3.2-g846ffe7, an exact commit
    # 1.3.2-g846ffe7+dirty, an exact commit, but dirty
    execute_process(COMMAND "${GIT_EXECUTABLE}" describe --dirty=+dirty --always
                    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                    OUTPUT_VARIABLE PROJECT_GIT_VERSION
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
elseif(EXISTS "${CMAKE_SOURCE_DIR}/.git_revision")
    # Buildroot stores packages in archives, it does not clone repositories into output/build
    # directory. That makes "git describe" ineffective on the buildserver. Luckily, there is a
    # ".git_revision" file inside the buildroot archive.

    file(STRINGS ".git_revision" PROJECT_GIT_VERSION LIMIT_COUNT 1 LIMIT_INPUT 8)
endif()

if(PROJECT_GIT_VERSION)
    set(PROJECT_VERSION "${PROJECT_VERSION}-g${PROJECT_GIT_VERSION}")
    unset(PROJECT_GIT_VERSION)
else()
    # no git repo
    set(PROJECT_VERSION "${PROJECT_VERSION}-no-git")
endif()

message(STATUS "TC version: ${PROJECT_VERSION}.")

######################
# build type

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
      "Choose the type of build, options are: Debug and Release."
      FORCE)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

######################
# compilation options

# Allow user to specify C++ standard (default to 11 for compatibility)
set(CXX_STANDARD "11" CACHE STRING "C++ standard to use (11, 14, 17, 20, etc.)")
set(CMAKE_CXX_STANDARD ${CXX_STANDARD})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

######################
# tests
enable_testing()
configure_file(test/CMakeLists.txt.gtest.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
                RESULT_VARIABLE result
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
                RESULT_VARIABLE result
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)

set(TEST_SOURCES
    test/test_small_vector.cpp
)

add_executable(test_small_vector ${TEST_SOURCES})

# Set compile options, definitions and include directories
target_compile_options(test_small_vector PRIVATE -Wall -Wextra -Werror -pedantic)
target_include_directories(test_small_vector PRIVATE src)
target_link_libraries(test_small_vector gtest_main)
add_test(NAME SmallVectorTests COMMAND test_small_vector)

# Format target
find_program(CLANG_FORMAT_EXECUTABLE clang-format)
if(CLANG_FORMAT_EXECUTABLE)
    file(GLOB_RECURSE ALL_SOURCE_FILES src/*.cpp src/*.hpp)
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXECUTABLE} -i ${ALL_SOURCE_FILES}
        COMMENT "Formatting source files"
    )
endif()
