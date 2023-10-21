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

# Add source to this project's executable.
add_executable (${PROJECT_NAME} WIN32 "main.cpp" "server/server.cpp" "server/server.h" "client/client.h" "client/client.cpp" "common/message/message.h" "common/message/message.cpp" "win32includes.h" "common/displaycapturer/displaycapturer.cpp" "common/displaycapturer/displaycapturer.h" "common/managedhandle/managedhandle.h" "GUI/GUI.h" "GUI/GUI.cpp")
target_sources(${PROJECT_NAME} PRIVATE appManifest.manifest)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
endif()

# Add wxWidgets library
# Config info: https://docs.wxwidgets.org/latest/overview_cmake.html#cmake_options
set(wxBUILD_SHARED OFF)
set(wxBUILD_COMPATIBILITY "3.1")
add_subdirectory(libs/wxWidgets)
target_link_libraries(${PROJECT_NAME} wx::net wx::core wx::base)