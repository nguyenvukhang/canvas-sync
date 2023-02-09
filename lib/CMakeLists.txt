cmake_minimum_required(VERSION 3.14)

project(cs2lib LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

add_library(cs2lib STATIC
  csync.cc
  canvas.cc
  convert.cc
  filetree.cc
  settings.cc
  tree_model.cc
  tree_view.cc
  types.cc

  # header files
  include/csync/csync.h
  include/csync/canvas.h
  include/csync/convert.h
  include/csync/filetree.h
  include/csync/settings.h
  include/csync/tree_model.h
  include/csync/tree_view.h
  include/csync/types.h
)

target_include_directories(cs2lib PUBLIC include)
target_include_directories(cs2lib PRIVATE include/csync)


target_link_libraries(cs2lib PRIVATE
  Qt6::Core
  Qt6::Widgets
  Qt6::Network
)

target_compile_definitions(cs2lib PRIVATE CS2LIB_LIBRARY)
