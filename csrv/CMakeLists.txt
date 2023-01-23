cmake_minimum_required(VERSION 3.10)

project(CanvasServer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1) # to build compile_commands.json

option(WITH_TESTS "Compile a test binary to run separately" OFF)
option(DEBUG_BIN "Compile a debug binary to run separately" OFF)

# `CanvasServer` as main output library
set(SOURCES 
  types.cc
  errors.cc
  canvas_api.cc
  filetree.cc
  httpjson.cc
  server.cc
)

# link openssl stuff so `https` requests are handled properly
set(OPENSSL_DIR /opt/homebrew/Cellar/openssl@3/3.0.7)
set(OPENSSL_LIB ${OPENSSL_DIR}/lib/libssl.a ${OPENSSL_DIR}/lib/libcrypto.a)
set(LINK_LIBS ${OPENSSL_LIB})
set(INCLUDE_DIRS
  ${OPENSSL_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(CanvasServer ${SOURCES})
target_link_libraries(CanvasServer ${LINK_LIBS})
target_include_directories(CanvasServer PUBLIC ${INCLUDE_DIRS})

# get GoogleTest suite
include(FetchContent)
FetchContent_Declare(googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# `cs_test` as testing binary
enable_testing()
add_executable(cs_test
  filetree.cc filetree_test.cc
  canvas_api.cc canvas_api_test.cc
  types.cc)
target_include_directories(cs_test PUBLIC include)
target_link_libraries(cs_test GTest::gtest_main GTest::gmock_main)
include(GoogleTest)
gtest_discover_tests(cs_test)
