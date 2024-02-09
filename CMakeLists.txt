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

	"common/displaycapturer/displaycapturer"
	"common/dynamicbitmap/dynamicbitmap"
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

# Use address sanitizer in debug builds
if(CMAKE_BUILD_TYPE MATCHES "Debug")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
  target_link_options(${PROJECT_NAME} BEFORE PUBLIC -fsanitize=address)
endif()

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

# Find FFmpeg package
set(FFMPEG_ROOT "libs/ffmpeg")
find_package(FFmpeg REQUIRED COMPONENTS AVCODEC AVFORMAT AVUTIL SWSCALE)

# Include FFmpeg headers
include_directories(${FFMPEG_INCLUDE_DIRS})

# Link FFmpeg libraries
link_directories(${FFMPEG_LIBRARIES})
target_link_libraries(YourTargetName ${FFMPEG_LIBRARIES})