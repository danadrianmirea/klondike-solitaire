cmake_minimum_required(VERSION 3.10)
project(klondike-solitaire)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set raylib path - adjust this to your raylib installation path
set(RAYLIB_PATH "C:/raylib/raylib" CACHE PATH "Path to raylib source directory")

# Configure static linking
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)

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

# Add raylib as a subdirectory
add_subdirectory(${RAYLIB_PATH} ${CMAKE_BINARY_DIR}/raylib)



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
    raylib
)

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Copy assets folder to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
) 

# Create zip file of bin directory contents
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND PowerShell -Command "Compress-Archive -Path '${CMAKE_BINARY_DIR}/${PROJECT_NAME}.exe','${CMAKE_BINARY_DIR}/assets' -DestinationPath '${CMAKE_BINARY_DIR}/${PROJECT_NAME}.zip' -Force"
        COMMENT "Creating ${PROJECT_NAME}.zip with correct folder structure"
    )
else()
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E tar "cfv" "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.zip" 
            --format=zip 
            "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.exe"
            "${CMAKE_BINARY_DIR}/assets"
        COMMENT "Creating ${PROJECT_NAME} with correct folder structure"
    )
endif()