cmake_minimum_required(VERSION 3.10)
project(CanvasServer VERSION 0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1) # to build compile_commands.json

# if DEBUG_MODE is on, compile only the main.cc with the rest linked
option(USE_DEBUG "Build the debug binary" OFF)
option(USE_TEST "Build the test binary" OFF)

# openssl stuff
set(OPENSSL_USE_STATIC_LIBS TRUE)
if (APPLE)
  if (NOT DEFINED OPENSSL_ROOT_DIR)
    execute_process(
      COMMAND brew --prefix openssl@3
      RESULT_VARIABLE OPENSSL_ROOT_DIR_R
      OUTPUT_VARIABLE OPENSSL_ROOT_DIR
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (NOT OPENSSL_ROOT_DIR_R EQUAL 0 OR NOT EXISTS "${OPENSSL_ROOT_DIR}")
      message(SEND_ERROR "csrv on MacOS: Unable to locate OpenSSL installation.")
    endif()
  endif()
endif()
find_package(OpenSSL REQUIRED)

if (USE_DEBUG)
  add_executable(CanvasDebug main.cc)
  target_include_directories(CanvasDebug PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/debug-include
  )
  target_link_libraries(CanvasDebug OpenSSL::SSL)
  target_link_libraries(CanvasDebug ${CMAKE_CURRENT_SOURCE_DIR}/build/libCanvasServer.a)
elseif (NOT USE_DEBUG)
  # `CanvasServer` as main output library
  set(SOURCES 
    types.cc
    errors.cc
    canvas_api.cc
    filetree.cc
    server.cc
  )
  add_library(CanvasServer STATIC ${SOURCES})
  target_link_libraries(CanvasServer OpenSSL::SSL)
  target_include_directories(CanvasServer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  )
endif()


if (USE_TEST)
  # get GoogleTest suite
  include(FetchContent)
  FetchContent_Declare(googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  # `cs_test` as testing binary
  enable_testing()
  add_executable(cs_test types.cc
    filetree.cc filetree_test.cc
    canvas_api.cc canvas_api_test.cc
  )
  target_include_directories(cs_test PUBLIC include)
  target_link_libraries(cs_test GTest::gtest_main GTest::gmock_main)
  include(GoogleTest)
  gtest_discover_tests(cs_test)
endif()
