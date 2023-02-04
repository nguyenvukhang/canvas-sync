cmake_minimum_required(VERSION 3.22)
project(CanvasSyncApp VERSION 0.1 LANGUAGES CXX)

set(PROJECT_SOURCES
    main.cc
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
  ${CMAKE_SOURCE_DIR}/build/CanvasSync_autogen/include
)

option(BUILD_STATIC_LIBS ON)
option(BUILD_SHARED_LIBS OFF)

if(WIN32)
  set(APP_ICON_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/appicon.rc")
  set(PROJECT_SOURCES ${PROJECT_SOURCES} ${APP_ICON_WINDOWS})
elseif(APPLE)
  set(APP_ICON_MACOS "${CMAKE_CURRENT_SOURCE_DIR}/../deploy/canvas-sync.icns")
  set_source_files_properties(${APP_ICON_MACOS} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
  set(PROJECT_SOURCES ${PROJECT_SOURCES} ${APP_ICON_MACOS})
endif()

qt_add_executable(CanvasSync MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_link_libraries(CanvasSync PRIVATE Qt6::Widgets Qt6::Network)
set_target_properties(CanvasSync PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME "Canvas Sync"
    MACOSX_BUNDLE_GUI_IDENTIFIER canvassync.nguyenvukhang.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE_ICON_FILE canvas-sync.icns
    MACOSX_BUNDLE TRUE WIN32_EXECUTABLE TRUE
)
install(TARGETS CanvasSync
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

qt_finalize_executable(CanvasSync)
