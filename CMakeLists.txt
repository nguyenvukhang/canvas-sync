cmake_minimum_required(VERSION 3.5)
project(CanvasSync VERSION 0.1 LANGUAGES CXX)

# additional flags set for canvas-sync development
set(CMAKE_EXPORT_COMPILE_COMMANDS 1) # to build compile_commands.json
set(CMAKE_PREFIX_PATH ~/Qt/6.4.2/macos)

# auto-generated flags
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# auto-generated package resolution
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    samplewidget.h
    samplewidget.cpp
    tree_model.h
    tree_model.cpp
    clickable_tree_view.h
    clickable_tree_view.cpp
)

# include the autogen stuff so ui files can be imported
# for example, `nice_component.ui` can be imported as `ui_nice_component.h`
# in C++ files.
include_directories(${PROJECT_SOURCE_DIR}/build/CanvasSync_autogen/include)

# ultimately run add_executable() and register CanvasSync as The build target
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(CanvasSync MANUAL_FINALIZATION ${PROJECT_SOURCES})
else()
    if(ANDROID)
      add_library(CanvasSync SHARED ${PROJECT_SOURCES})
    else()
      add_executable(CanvasSync ${PROJECT_SOURCES})
    endif()
endif()
target_link_libraries(CanvasSync PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
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

# some Qt-auto-generated line
if(QT_VERSION_MAJOR EQUAL 6)
  qt_finalize_executable(CanvasSync)
endif()
