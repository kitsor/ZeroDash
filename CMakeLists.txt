cmake_minimum_required(VERSION 3.10)
project(ZeroDash CXX)

# Set C++ standard to 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set compile flags for Release mode with maximum optimization
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -Wswitch -Wno-deprecated-declarations -Wempty-body -Wconversion -Wreturn-type -Wparentheses -Wno-format -Wuninitialized -Wunreachable-code -Wunused-function -Wunused-value -Wunused-variable -fno-strict-aliasing -fno-omit-frame-pointer -fthreadsafe-statics -fexceptions -frtti -Wno-psabi")

# Set build type to Release by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Find all .cpp files in the ZeroDash directory
file(GLOB SOURCES ZeroDash/*.cpp)

# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

# Create the executable
add_executable(ZeroDash ${SOURCES})

# Link necessary libraries
target_link_libraries(ZeroDash wiringPi gd curl)

# Copy default.conf to the output directory after the build
add_custom_command(
    TARGET ZeroDash POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/ZeroDash/default.conf
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/default.conf
)