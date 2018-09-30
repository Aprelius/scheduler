cmake_minimum_required(VERSION 3.10)

# Define project name
project(SchedulerTests
    LANGUAGES CXX
    VERSION 0.1.0)

# Project directories
set(PROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Declare the primary executable for the application. We declare the
# dependencies and link in the library from the main project.
add_executable(SchedulerTests)

target_include_directories(SchedulerTests
    PUBLIC ${PROJECT_INCLUDE_DIR}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../scheduler/Lib/internal)

# Create the directory for the Header Files
FILE(GLOB_RECURSE HEADERS ${PROJECT_INCLUDE_DIR} "include/*.h")
FILE(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR} "src/*.cpp")

target_sources(SchedulerTests
    PRIVATE ${HEADERS} ${SOURCES})

target_link_libraries(SchedulerTests
    PRIVATE
        gtest gmock
    PUBLIC
        Scheduler::Lib Scheduler::Common)