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
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

set(TEST_SOURCES
    src/testMain.cpp
)

add_executable(test_small_vector ${TEST_SOURCES})

# Set compile options, definitions and include directories
target_compile_options(test_small_vector PRIVATE -Wall -Wextra -Werror -pedantic)
target_include_directories(test_small_vector PRIVATE src)

# Format target
find_program(CLANG_FORMAT_EXECUTABLE clang-format)
if(CLANG_FORMAT_EXECUTABLE)
    file(GLOB_RECURSE ALL_SOURCE_FILES src/*.cpp src/*.hpp)
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXECUTABLE} -i ${ALL_SOURCE_FILES}
        COMMENT "Formatting source files"
    )
endif()
