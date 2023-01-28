cmake_minimum_required(VERSION 3.22)
project(CanvasSync VERSION 0.1 LANGUAGES CXX)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# additional flags set for canvas-sync development
set(CMAKE_EXPORT_COMPILE_COMMANDS 1) # to build compile_commands.json
# note that CMAKE_PREFIX_PATH must contain a Qt directory. Meaning that
# if you `ls` on this directory then stuff like bin/ and include/ shows
set(CMAKE_PREFIX_PATH ${QT_STATIC_DIR})

# auto-generated flags
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
    main.cc
    mainwindow.cc mainwindow.h mainwindow.ui
    updates.cc updates.h updates.ui
    tree_model.h
    tree_model.cc
    convert.h
    convert.cc
    clickable_tree_view.h
    clickable_tree_view.cc
    tree.h
    tree.cc
)

# include the autogen stuff so ui files can be imported
# for example, `nice_component.ui` can be imported as `ui_nice_component.h`
# in C++ files.
include_directories(
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/build/CanvasSync_autogen/include
)

option(BUILD_STATIC_LIBS ON)
option(BUILD_SHARED_LIBS OFF)

qt_standard_project_setup()
qt_add_executable(CanvasSync MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(CanvasSync PRIVATE Qt6::Widgets Qt6::Network)
set_target_properties(CanvasSync PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER canvassync.nguyenvukhang.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE WIN32_EXECUTABLE TRUE
)
install(TARGETS CanvasSync
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

# pull in the canvas server library
add_subdirectory(csrv)
target_link_libraries(CanvasSync PUBLIC CanvasServer)

qt_finalize_executable(CanvasSync)

include(CPack)
