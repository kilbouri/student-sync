# CMakeList.txt : CMake project for comp-4990-project, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(PROJECT_NAME "comp-4990-project")
project(${PROJECT_NAME})

# Elements in this set have both a header and a cpp source file.
set(CLASSES
	"server/server"
	"serverwindow/serverwindow"
	"client/client"
	"clientwindow/clientwindow"
	"common/socket/socket"
	"common/displaycapturer/displaycapturer"
	"common/networkmessage/networkmessage"
	"videostreamwindow/videostreamwindow"
	"videostreamwindow/videoframebitmap"
	"modeselectdialog/modeselectdialog"
)

# Elements in this set are a standalone header
set(EXTRA_HEADERS
	"win32includes.h"
	"common/managedhandle/managedhandle.h"
	"common/socket/socket.h"
	"common/messages/hellomessage.h"
	"common/messages/stringmessage.h"
	"common/messages/number64message.h"
	"common/messages/streamframemessage.h"
)

# Elements in this set are standalone source files.
set(EXTRA_SOURCES "main.cpp" "appManifest.manifest")

add_compile_definitions(_CRT_SECURE_NO_WARNINGS=1) # stupid warning spam removal
add_executable(
	${PROJECT_NAME} WIN32
	${EXTRA_HEADERS}
	${EXTRA_SOURCES}
)

foreach(class ${CLASSES})
	target_sources(${PROJECT_NAME} PRIVATE "${class}.h" "${class}.cpp")
endforeach()

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
endif()

# Add wxWidgets library
# Config info: https://docs.wxwidgets.org/latest/overview_cmake.html#cmake_options
set(wxBUILD_SHARED OFF)
set(wxUSE_STL ON)
set(wxBUILD_COMPATIBILITY "3.1")
add_subdirectory(libs/wxWidgets)
target_link_libraries(${PROJECT_NAME} wx::net wx::core wx::base)