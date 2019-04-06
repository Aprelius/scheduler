cmake_minimum_required(VERSION 3.2)

# Project directories
set(PROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

message("Common include: ${PROJECT_INCLUDE_DIR}")

# Define the project name
project(Scheduler-Common
    LANGUAGES CXX
    VERSION 0.1.0)

# Declare the Library
add_library(Scheduler-Common)
add_library(Scheduler::Common ALIAS Scheduler-Common)

# Allow the users to decide if we are building a shared
# or static library
option(BUILD_SHARED_LIBS "Build shared library" OFF)

# Set the C++14 minimum
target_compile_features(Scheduler-Common PUBLIC cxx_std_14)

# Set the target include path
target_include_directories(Scheduler-Common
    PUBLIC ${PROJECT_INCLUDE_DIR})

# Create the directory for the Header Files
FILE(GLOB_RECURSE HEADERS ${PROJECT_INCLUDE_DIR} "include/*.h")
# Create the source group folders for the different source files
FILE(GLOB SOURCES "src/*.cpp")

target_sources(Scheduler-Common
    PRIVATE ${HEADERS} ${SOURCES})

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    target_compile_options(Scheduler-Common
        PRIVATE
            -Wno-uninitialized
            -Wno-unused-variable
            -Wno-unused-parameter
            -Wno-comment
            -Wall)
endif()