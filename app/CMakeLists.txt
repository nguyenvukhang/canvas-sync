cmake_minimum_required(VERSION 3.22)
project(CanvasSync VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network Test Gui)
set(LIB_NAME csync)

set(PROJECT_SOURCES
  mainwindow.cc
  updates.cc

  canvas.cc
  clickable_tree_view.cc
  convert.cc
  filetree.cc
  settings.cc
  tree_model.cc
  types.cc

  # actual ui files
  include/csync/mainwindow.ui
  include/csync/updates.ui

  # header files
  include/csync/canvas.h
  include/csync/clickable_tree_view.h
  include/csync/convert.h
  include/csync/filetree.h
  include/csync/settings.h
  include/csync/tree_model.h
  include/csync/types.h
  include/csync/mainwindow.h
  include/csync/updates.h
)

option(BUILD_STATIC_LIBS ON)
option(BUILD_SHARED_LIBS OFF)
option(BUILD_TESTS "Build a separate binary to run tests" ON)

qt_add_library(${LIB_NAME} STATIC ${PROJECT_SOURCES})

target_include_directories(${LIB_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include/csync
)

target_include_directories(${LIB_NAME} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${LIB_NAME} PRIVATE Qt6::Widgets Qt6::Network)

qt_finalize_target(${LIB_NAME})

function(save_ui UI_FILE)
  add_custom_command(TARGET ${LIB_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_BINARY_DIR}/${LIB_NAME}_autogen/include/${UI_FILE}
    ${CMAKE_CURRENT_SOURCE_DIR}/include/csync/${UI_FILE}
  )
endfunction()

save_ui(ui_mainwindow.h)
save_ui(ui_updates.h)
