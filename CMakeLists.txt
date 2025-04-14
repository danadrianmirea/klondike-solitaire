cmake_minimum_required(VERSION 3.10)
project(raylib_solitaire)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set default build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

# Set available build types
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")

set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,--subsystem,windows -static-libgcc -static-libstdc++ -static")

# Set raylib path - adjust this to your raylib installation path
set(RAYLIB_PATH "C:/raylib" CACHE PATH "Path to raylib installation")

# Debug messages to help locate raylib
message(STATUS "Looking for raylib in: ${RAYLIB_PATH}")

# Find raylib
find_path(RAYLIB_INCLUDE_DIR raylib.h
    PATHS 
        ${RAYLIB_PATH}/raylib/src
        ${RAYLIB_PATH}/include
    DOC "Path to raylib headers"
)

find_library(RAYLIB_LIBRARY
    NAMES raylib
    PATHS 
        ${RAYLIB_PATH}/w64devkit/x86_64-w64-mingw32/lib
        ${RAYLIB_PATH}/lib
    DOC "Path to raylib library"
)

# Debug messages
message(STATUS "RAYLIB_INCLUDE_DIR: ${RAYLIB_INCLUDE_DIR}")
message(STATUS "RAYLIB_LIBRARY: ${RAYLIB_LIBRARY}")

if(NOT RAYLIB_INCLUDE_DIR OR NOT RAYLIB_LIBRARY)
    message(FATAL_ERROR "raylib not found. Please set RAYLIB_PATH to your raylib installation directory")
endif()

# Add nlohmann/json as a header-only library
include(FetchContent)
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp
    DOWNLOAD_NO_EXTRACT TRUE
)
FetchContent_MakeAvailable(json)

# Create directory for json.hpp
file(MAKE_DIRECTORY ${json_SOURCE_DIR}/nlohmann)
file(COPY ${json_SOURCE_DIR}/json.hpp DESTINATION ${json_SOURCE_DIR}/nlohmann)

# Add executable
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_executable(${PROJECT_NAME} WIN32
        src/main.cpp
        src/Card.cpp
        src/Solitaire.cpp
    )
else()
    add_executable(${PROJECT_NAME}
        src/main.cpp
        src/Card.cpp
        src/Solitaire.cpp
    )
endif()

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${RAYLIB_INCLUDE_DIR}
    ${json_SOURCE_DIR}
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