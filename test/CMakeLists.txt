cmake_minimum_required(VERSION 3.22)
project(CanvasSyncTest VERSION 0.1 LANGUAGES CXX)

set(TEST_SOURCES
  main.cc
)

include_directories(${PROJECT_SOURCE_DIR})

option(BUILD_STATIC_LIBS ON)
option(BUILD_SHARED_LIBS OFF)

qt_add_executable(CanvasSyncTest MANUAL_FINALIZATION ${TEST_SOURCES})
target_link_libraries(CanvasSyncTest PRIVATE
  Qt6::Widgets
  Qt6::Network
  Qt6::Gui
  Qt6::Test
)

# set_target_properties(CanvasSyncTest PROPERTIES MACOSX_BUNDLE TRUE WIN32_EXECUTABLE TRUE)

install(TARGETS CanvasSyncTest
  BUNDLE DESTINATION .
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

qt_finalize_executable(CanvasSyncTest)

add_test(NAME CanvasSyncTest COMMAND CanvasSyncTest)
