set(FFMPEG_SOURCE_URL "https://github.com/BtbN/FFmpeg-Builds/releases/download/latest/ffmpeg-master-latest-win64-gpl-shared.zip")

<<<<<<< Updated upstream
set(SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(ARCHIVE_NAME my_archive.zip)
set(URL "https://example.com/my_archive.zip")
=======
# Specify the minimum CMake version 
cmake_minimum_required(VERSION 3.12)
>>>>>>> Stashed changes

# Download the file
file(DOWNLOAD ${URL} ${SOURCES_DIR}/${ARCHIVE_NAME})

# Unzip the file
file(ARCHIVE_EXTRACT INPUT ${SOURCES_DIR}/${ARCHIVE_NAME} DESTINATION ${SOURCES_DIR})
