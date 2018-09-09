cmake_minimum_required(VERSION 3.2)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR AND NOT MSVC_IDE)
    message(FATAL_ERROR, "In-Source builds are not permitted")
endif()

# Define the root paths.
set(PROJECT_BUILD_DIR ${PROJECT_SOURCE_DIR}/build)
set(PROJECT_CONTRIB_PATH ${PROJECT_SOURCE_DIR}/contrib)
set(PROJECT_FILE_PATH ${PROJECT_BUILD_DIR}/project)

# Define the components needed for the build folder.
# Eventually this will be done dynamically.
set(SYSTEM_NAME "linux")
set(COMPILER_NAME "gcc8")
set(BUILD_ARCH "x86_64")
string(TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_TYPE)

# Compose the final output path
set(PROJECT_OUTPUT_FOLDER
    "${SYSTEM_NAME}-${BUILD_ARCH}-${COMPILER_NAME}-${BUILD_TYPE}")

# Set the final build path which will include the
# output folder.
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
    ${PROJECT_BUILD_DIR}/${PROJECT_OUTPUT_FOLDER}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
    ${PROJECT_BUILD_DIR}/${PROJECT_OUTPUT_FOLDER}/obj)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    ${PROJECT_BUILD_DIR}/${PROJECT_OUTPUT_FOLDER}/bin)

# Add the source sub directories
add_subdirectory(contrib)
add_subdirectory(tests)
add_subdirectory(tools)
add_subdirectory(scheduler)
