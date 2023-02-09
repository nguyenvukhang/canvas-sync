cmake_minimum_required(VERSION 3.14)

project(csui LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

add_library(csui STATIC
  modal.cc modal.ui
  pushbutton.cc
  oneshotbutton.cc
  # header files
  include/csui/modal.h
  include/csui/pushbutton.h
  include/csui/oneshotbutton.h
)

target_include_directories(csui PUBLIC include)
target_include_directories(csui PRIVATE include/csui)

target_link_libraries(csui PRIVATE
  Qt6::Core
  Qt6::Widgets
)

target_compile_definitions(csui PRIVATE CSUI_LIBRARY)
