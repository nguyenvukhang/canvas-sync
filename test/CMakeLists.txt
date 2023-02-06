cmake_minimum_required(VERSION 3.22)
project(CanvasSync VERSION 0.1 LANGUAGES CXX)

set(TEST_NAME CanvasSyncTest)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network Gui Test)

set(TEST_SOURCES
  main.cc
  fake_canvas.cc
)

qt_add_executable(${TEST_NAME} MANUAL_FINALIZATION ${TEST_SOURCES})

target_link_libraries(${TEST_NAME} PRIVATE
  Qt6::Widgets
  Qt6::Network
  Qt6::Gui
  Qt6::Test
  csync
)

qt_finalize_target(${TEST_NAME})

add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
