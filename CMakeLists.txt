cmake_minimum_required(VERSION 3.22)
project(CanvasSync VERSION 0.1 LANGUAGES CXX)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "RECEIVED QT_STATIC type: ${QT_STATIC_DIR}")

# additional flags set for canvas-sync development
set(CMAKE_EXPORT_COMPILE_COMMANDS 1) # to build compile_commands.json
# note that CMAKE_PREFIX_PATH must contain a Qt directory. Meaning that
# if you `ls` on this directory then stuff like bin/ and include/ shows
if (EXISTS ${QT_STATIC_DIR})
  set(CMAKE_PREFIX_PATH ${QT_STATIC_DIR})
endif()

# auto-generated flags
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network Gui Test)

add_subdirectory(${PROJECT_SOURCE_DIR}/app)
add_subdirectory(${PROJECT_SOURCE_DIR}/test)
