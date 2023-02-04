cmake_minimum_required(VERSION 3.22)
project(CanvasSync VERSION 0.1 LANGUAGES CXX)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "RECEIVED QT_STATIC type: ${QT_STATIC_DIR}")

# additional flags set for canvas-sync development
set(CMAKE_EXPORT_COMPILE_COMMANDS 1) # to build compile_commands.json
# note that CMAKE_PREFIX_PATH must contain a Qt directory. Meaning that
# if you `ls` on this directory then stuff like bin/ and include/ shows
if (EXISTS ${QT_STATIC_DIR})
  set(CMAKE_PREFIX_PATH ${QT_STATIC_DIR})
endif()

# auto-generated flags
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network Gui Test)

add_subdirectory(${PROJECT_SOURCE_DIR}/app)
add_subdirectory(${PROJECT_SOURCE_DIR}/test)

if(WIN32)
  set(APP_ICON_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/appicon.rc")
  set(PROJECT_SOURCES ${PROJECT_SOURCES} ${APP_ICON_WINDOWS})
elseif(APPLE)
  set(APP_ICON_MACOS "${CMAKE_CURRENT_SOURCE_DIR}/deploy/canvas-sync.icns")
  set_source_files_properties(${APP_ICON_MACOS} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
  set(PROJECT_SOURCES ${PROJECT_SOURCES} ${APP_ICON_MACOS})
endif()

qt_add_executable(CanvasSync MANUAL_FINALIZATION main.cc)

include_directories(
  ${PROJECT_SOURCE_DIR}/app
  ${PROJECT_SOURCE_DIR}/build/app/CanvasSyncLib_autogen/include
)

target_link_libraries(CanvasSync PRIVATE Qt6::Widgets Qt6::Network CanvasSyncLib)
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
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
qt_finalize_executable(CanvasSync)
