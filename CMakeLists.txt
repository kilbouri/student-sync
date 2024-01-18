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

# Elements in this set have both .hpp and .cpp files
set(CLASSES
	"client/client/client" # lol
	"client/preferenceseditor/preferenceseditor"
	"client/preferencesmanager/preferencesmanager"
	"client/window/window"

	"common/displaycapturer/displaycapturer"
	"common/modeselectdialog/modeselectdialog"
	"common/screenresolution/screenresolution"
	"common/dynamicbitmap/dynamicbitmap"

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

add_compile_definitions(_CRT_SECURE_NO_WARNINGS=1) # stupid warning spam removal
add_executable(
	${PROJECT_NAME} WIN32
	${EXTRA_HEADERS}
	${EXTRA_SOURCES}
)

foreach(class ${CLASSES})
	target_sources(${PROJECT_NAME} PRIVATE "${class}.hpp" "${class}.cpp")
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