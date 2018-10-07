cmake_minimum_required(VERSION 3.2)

# Project directories
set(PROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(PROJECT_INTERNAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/internal)
set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Define the project name
project(Scheduler-Lib
    LANGUAGES CXX
    VERSION 0.1.0)

# Declare the Library
add_library(Scheduler-Lib)
add_library(Scheduler::Lib ALIAS Scheduler-Lib)

# Allow the users to decide if we are building a shared
# or static library
option(BUILD_SHARED_LIBS "Build shared library" OFF)

# Set the C++14 minimum
target_compile_features(Scheduler-Lib PUBLIC cxx_std_14)

# Set the target include path
target_include_directories(Scheduler-Lib
    PUBLIC ${PROJECT_INCLUDE_DIR}
    PRIVATE ${PROJECT_INTERNAL_DIR})

# Create the directory for the Header Files
FILE(GLOB_RECURSE HEADERS ${PROJECT_INCLUDE_DIR} "include/*.h")
FILE(GLOB_RECURSE INTERNAL_H ${PROJECT_INTERNAL_DIR} "internal/*.h")
# Create the source group folders for the different source files
FILE(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR} "src/*.cpp")

target_link_libraries(Scheduler-Lib
    PUBLIC Scheduler::Common TemplateUtils)

target_sources(Scheduler-Lib
    PRIVATE ${HEADERS} ${INTERNAL_H} ${SOURCES})

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    target_compile_options(Scheduler-Lib
        PRIVATE
            -Wno-uninitialized
            -Wno-unused-variable
            -Wno-unused-parameter
            -Wno-comment
            -Wall)
endif()