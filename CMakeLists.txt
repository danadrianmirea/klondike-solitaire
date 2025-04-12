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

# Add executable
add_executable(${PROJECT_NAME} 
    src/main.cpp
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${RAYLIB_INCLUDE_DIR})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE 
    ${RAYLIB_LIBRARY}
    winmm
    gdi32
    opengl32
)

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Find raylib.dll in various possible locations
set(RAYLIB_DLL_PATHS
    "${RAYLIB_PATH}/lib/raylib.dll"
    "${RAYLIB_PATH}/raylib.dll"
    "${RAYLIB_PATH}/src/raylib.dll"
    "${RAYLIB_PATH}/raylib/src/raylib.dll"
)

# Debug message for DLL paths
foreach(DLL_PATH ${RAYLIB_DLL_PATHS})
    message(STATUS "Checking for DLL at: ${DLL_PATH}")
    if(EXISTS ${DLL_PATH})
        message(STATUS "Found raylib.dll at: ${DLL_PATH}")
        set(RAYLIB_DLL_FOUND ${DLL_PATH})
        break()
    endif()
endforeach()

# Copy DLLs to output directory (Windows specific)
if(WIN32)
    if(RAYLIB_DLL_FOUND)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${RAYLIB_DLL_FOUND}"
                $<TARGET_FILE_DIR:${PROJECT_NAME}>
        )
    else()
        message(WARNING "raylib.dll not found in any of the expected locations. The executable may not run correctly.")
    endif()
endif() 