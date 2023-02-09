cmake_minimum_required(VERSION 3.5)

project(cs2 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# additional flags set for canvas-sync development
set(CMAKE_EXPORT_COMPILE_COMMANDS 1) # to build compile_commands.json
# note that CMAKE_PREFIX_PATH must contain a Qt directory. Meaning that
# if you `ls` on this directory then stuff like bin/ and include/ shows

if (EXISTS ${QT_STATIC_DIR})
  set(CMAKE_PREFIX_PATH ${QT_STATIC_DIR})
endif()

add_subdirectory(lib)
add_subdirectory(ui)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

set(PROJECT_SOURCES
  main.cc
  app.cc
  app.h
  app.ui
  assets.qrc
)

if(WIN32)
  set(APP_ICON_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/appicon.rc")
  set(PROJECT_SOURCES ${PROJECT_SOURCES} ${APP_ICON_WINDOWS})
elseif(APPLE)
  set(APP_ICON_MACOS "${CMAKE_CURRENT_SOURCE_DIR}/assets/app-icon/canvas-sync.icns")
  set_source_files_properties(${APP_ICON_MACOS} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
  set(PROJECT_SOURCES ${PROJECT_SOURCES} ${APP_ICON_MACOS})
endif()

qt_add_executable(cs2 MANUAL_FINALIZATION ${PROJECT_SOURCES})

target_link_libraries(cs2 PRIVATE
  Qt6::Core
  Qt6::Widgets
  Qt6::Network
  cs2lib
  csui
)

set_target_properties(cs2 PROPERTIES
  OUTPUT_NAME "Canvas Sync"
  MACOSX_BUNDLE_GUI_IDENTIFIER canvassync.nguyenvukhang.com
  MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
  MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
  MACOSX_BUNDLE_ICON_FILE canvas-sync.icns
  MACOSX_BUNDLE TRUE
  WIN32_EXECUTABLE TRUE
)

install(TARGETS cs2
  BUNDLE DESTINATION .
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_finalize_executable(cs2)
