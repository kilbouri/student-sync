# CMakeList.txt : CMake project for comp-4990-project, include source and define
# project specific logic here.
cmake_minimum_required (VERSION 3.12)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project("student-sync" VERSION 1.0.0)

# Elements in this set have both .hpp and .cpp files
set(CLASSES
	"client/client/client" # lol
	"client/preferenceseditor/preferenceseditor"
	"client/preferencesmanager/preferencesmanager"
	"client/window/window"

	"common/gdiplusutil/gdiplusutil"
	"common/dynamicbitmap/dynamicbitmap"
	"common/ffmpeg/encoders/h264Encoder"
	"common/modeselectdialog/modeselectdialog"
	"common/screenresolution/screenresolution"
	"common/timer/timer"

	"networking/socket/socket"
	"networking/socket/tcpsocket"
	"networking/tlvmessage/tlvmessage"

	"server/preferenceseditor/preferenceseditor"
	"server/preferencesmanager/preferencesmanager"
	"server/server/server" # lol
	"server/session/session"
	"server/window/window"
	"server/window/eventdispatcher/eventdispatcher"
)

# Elements in this set are header-only
set(EXTRA_HEADERS
	"common/appinfo/appinfo.hpp"
	"common/generator/generator.hpp"
	"common/preferenceseditor/preferenceseditor.hpp"
	"common/task/task.hpp"

	"networking/message/message.hpp"

	"win32includes.h"
)

# Elements in this set can be whatever, as long as they aren't a class nor a header
set(EXTRA_SOURCES
	"main.cpp"
	"appManifest.manifest"
)

add_executable(
	${PROJECT_NAME} WIN32
	${EXTRA_HEADERS}
	${EXTRA_SOURCES}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

foreach(class ${CLASSES})
	target_sources(${PROJECT_NAME} PRIVATE "${class}.hpp" "${class}.cpp")
endforeach()

add_compile_definitions(_CRT_SECURE_NO_WARNINGS=1)
add_compile_definitions(GDIPVER=0x0110) # all Win32 OSes newer than Vista have GDI+ >= 1.1

# Use address sanitizer in debug builds
#if(CMAKE_BUILD_TYPE MATCHES "Debug")
#  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
#  target_link_options(${PROJECT_NAME} BEFORE PUBLIC -fsanitize=address)
#endif()

# Add cpp::result library
# Repository: https://github.com/bitwizeshift/result
add_subdirectory(libs/result)
target_include_directories(${PROJECT_NAME} PRIVATE libs/result/include)

# Add wxWidgets library
# Config info: https://docs.wxwidgets.org/latest/overview_cmake.html#cmake_options
set(wxBUILD_SHARED OFF)
set(wxUSE_STL ON)
set(wxBUILD_COMPATIBILITY "3.1")
add_subdirectory(libs/wxWidgets)
target_link_libraries(${PROJECT_NAME} PUBLIC wx::net wx::core wx::base)

# Add Ffmpeg, apparently it doesnt support find_package !!!! cause ofc
# Info here: https://stackoverflow.com/questions/50760024/cmake-configuration-for-ffmpeg-in-c-project
# and here: https://ffmpeg-api.com/learn

add_compile_definitions("__STDC_CONSTANT_MACROS") # idk what this does but it is apparently needed

# Set the path to the FFmpeg library directory within the project
set(FFMPEG_DIR "${CMAKE_SOURCE_DIR}/libs/FFmpeg")
set(FFMPEG_LIB_DIR "${FFMPEG_DIR}/lib")
set(FFMPEG_HEADER_DIR "${FFMPEG_DIR}/include")

# Find FFmpeg components
find_path(AVCODEC_INCLUDE libavcodec/avcodec.h PATHS ${FFMPEG_HEADER_DIR})
find_path(AVFORMAT_INCLUDE libavformat/avformat.h PATHS ${FFMPEG_HEADER_DIR})
find_path(AVUTIL_INCLUDE libavutil/avutil.h PATHS ${FFMPEG_HEADER_DIR})
find_path(AVUTIL_PIXDESC_INCLUDE libavutil/pixdesc.h PATHS ${FFMPEG_HEADER_DIR})
find_path(AVDEVICE_INCLUDE libavdevice/avdevice.h PATHS ${FFMPEG_HEADER_DIR})
find_path(SWSCALE_INCLUDE libswscale/swscale.h PATHS ${FFMPEG_HEADER_DIR})

# Check if all FFmpeg headers are found
if (NOT AVCODEC_INCLUDE OR NOT AVFORMAT_INCLUDE OR NOT AVUTIL_INCLUDE OR NOT AVUTIL_PIXDESC_INCLUDE OR NOT AVDEVICE_INCLUDE OR NOT SWSCALE_INCLUDE)
	message(FATAL_ERROR "FFmpeg includes not found. Please check the path to your FFmpeg includes.")
endif()

target_include_directories(
	${PROJECT_NAME} PUBLIC
	${AVCODEC_INCLUDE}
	${AVFORMAT_INCLUDE}
	${AVUTIL_INCLUDE}
	${AVUTIL_PIXDESC_INCLUDE}
	${AVDEVICE_INCLUDE}
	${SWSCALE_INCLUDE}
)


find_library(AVCODEC_LIBRARY avcodec.lib PATHS ${FFMPEG_LIB_DIR})
find_library(AVFORMAT_LIBRARY avformat.lib PATHS ${FFMPEG_LIB_DIR})
find_library(AVUTIL_LIBRARY avutil.lib PATHS ${FFMPEG_LIB_DIR})
find_library(AVDEVICE_LIBRARY avdevice.lib PATHS ${FFMPEG_LIB_DIR})
find_library(SWSCALE_LIBRARY swscale.lib PATHS ${FFMPEG_LIB_DIR})

target_link_libraries(
	${PROJECT_NAME} PUBLIC
	${AVCODEC_LIBRARY}
	${AVFORMAT_LIBRARY}
	${AVUTIL_LIBRARY}
	${AVDEVICE_LIBRARY}
	${SWSCALE_LIBRARY}
)

# Check if all FFmpeg components are found
if(NOT AVCODEC_LIBRARY OR NOT AVFORMAT_LIBRARY OR NOT AVUTIL_LIBRARY OR NOT AVDEVICE_LIBRARY OR NOT SWSCALE_LIBRARY)
    message(FATAL_ERROR "FFmpeg not found. Please check the path to your FFmpeg libraries.")
endif()

# Runtime FFmpeg DLL dependencies
# idk what this looks like for non-Windows machines but hey we're only targeting Win32 anyway!
set(FFMPEG_DLLS
	"${FFMPEG_DIR}/bin/avcodec-60.dll"
	"${FFMPEG_DIR}/bin/avformat-60.dll"
	"${FFMPEG_DIR}/bin/avutil-58.dll"
	"${FFMPEG_DIR}/bin/swresample-4.dll"
	"${FFMPEG_DIR}/bin/swscale-7.dll"
)

foreach(DLL_FILE ${FFMPEG_DLLS})
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${DLL_FILE} $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMENT "Copying ${DLL_FILE} to output directory"
    )
endforeach()