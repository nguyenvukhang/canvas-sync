cmake_minimum_required(VERSION 3.22)
project(CanvasSync VERSION 0.1 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

add_subdirectory(${PROJECT_SOURCE_DIR}/app)
