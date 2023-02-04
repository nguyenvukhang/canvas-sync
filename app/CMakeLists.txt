cmake_minimum_required(VERSION 3.22)
project(CanvasSyncLib VERSION 0.1 LANGUAGES CXX)

set(PROJECT_SOURCES
    main.cc
    canvas_sync.h
    mainwindow.cc mainwindow.h mainwindow.ui
    updates.cc updates.h updates.ui
    tree_model.h tree_model.cc
    canvas.h canvas.cc
    convert.h convert.cc
    clickable_tree_view.h clickable_tree_view.cc
    filetree.h filetree.cc
    types.h types.cc
)

# include the autogen stuff so ui files can be imported
# for example, `nice_component.ui` can be imported as `ui_nice_component.h`
# in C++ files.
include_directories(
  ${PROJECT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/build/CanvasSyncLib_autogen/include
)

option(BUILD_STATIC_LIBS ON)
option(BUILD_SHARED_LIBS OFF)

qt_add_library(CanvasSyncLib MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(CanvasSyncLib PRIVATE Qt6::Widgets Qt6::Network)
install(TARGETS CanvasSyncLib
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
