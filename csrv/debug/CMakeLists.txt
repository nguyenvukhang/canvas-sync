cmake_minimum_required(VERSION 3.10)

project(canvas_debug VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1) # to build compile_commands.json

# link openssl stuff so `https` requests are handled properly
set(OPENSSL_DIR /opt/homebrew/Cellar/openssl@3/3.0.7)
set(OPENSSL_LIB)
set(LINK_LIBS
  ${OPENSSL_DIR}/lib/libssl.a
  ${OPENSSL_DIR}/lib/libcrypto.a
  ${CMAKE_CURRENT_SOURCE_DIR}/../build/libCanvasServer.a
)
set(INCLUDE_DIRS
  ${OPENSSL_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

add_executable(canvas_debug main.cc ${CMAKE_CURRENT_SOURCE_DIR}/../csrv.h)
target_link_libraries(canvas_debug ${LINK_LIBS})
target_include_directories(canvas_debug PUBLIC ${INCLUDE_DIRS})
