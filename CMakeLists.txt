cmake_minimum_required(VERSION 3.10)
project(raylib_solitaire)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set raylib path - adjust this to your raylib installation path
set(RAYLIB_PATH "C:/raylib" CACHE PATH "Path to raylib installation")

# Debug messages to help locate raylib
message(STATUS "Looking for raylib in: ${RAYLIB_PATH}")

# Find raylib
find_path(RAYLIB_INCLUDE_DIR raylib.h
    PATHS 
        ${RAYLIB_PATH}/include
        ${RAYLIB_PATH}/src
        ${RAYLIB_PATH}/raylib/src
    DOC "Path to raylib headers"
)

find_library(RAYLIB_LIBRARY
    NAMES raylib
    PATHS 
        ${RAYLIB_PATH}/lib
        ${RAYLIB_PATH}/src
        ${RAYLIB_PATH}/raylib/src
    DOC "Path to raylib library"
)

# Debug messages
message(STATUS "RAYLIB_INCLUDE_DIR: ${RAYLIB_INCLUDE_DIR}")
message(STATUS "RAYLIB_LIBRARY: ${RAYLIB_LIBRARY}")

if(NOT RAYLIB_INCLUDE_DIR OR NOT RAYLIB_LIBRARY)
    message(FATAL_ERROR "raylib not found. Please set RAYLIB_PATH to your raylib installation directory")
endif()

# Add nlohmann/json library
include(FetchContent)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)

# Add executable
add_executable(${PROJECT_NAME} 
    src/main.cpp
    src/Card.cpp
    src/Solitaire.cpp
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${RAYLIB_INCLUDE_DIR}
    ${json_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE 
    ${RAYLIB_LIBRARY}
    winmm
    gdi32
    opengl32
)

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Copy assets folder to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
) 