cmake_minimum_required (VERSION 3.8)

project("nrc")

set(CMAKE_BINARY_DIR "${CMAKE_SOURCE_DIR}/bin/${PROJECT_NAME}")

#add_library (${PROJECT_NAME} "nrc.cpp" "nrc.h")

add_library (${PROJECT_NAME} "nrc.cpp" "nrc.h")

