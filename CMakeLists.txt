# CMakeList.txt : CMake project for comp-4990-project, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("comp-4990-project")

# Add source to this project's executable.
add_executable (comp-4990-project "main.cpp"  "server/server.cpp"  "shared.h" "server/server.h" "client/client.h" "client/client.cpp" "common/message/message.h"  "common/message/message.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET comp-4990-project PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
