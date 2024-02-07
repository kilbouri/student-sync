# CMakeLists.txt in libs/FFmpeg directory

# Specify the minimum CMake version
cmake_minimum_required(VERSION 3.12)

# Set FFmpeg options
set(FFMPEG_BUILD_SHARED OFF)  # Adjust as needed
set(FFMPEG_USE_STL ON)        # Adjust as needed
set(FFMPEG_BUILD_COMPATIBILITY "3.1")  # Adjust as needed

# Add FFmpeg library
add_subdirectory(libs/FFmpeg)