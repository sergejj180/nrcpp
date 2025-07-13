# CMakeList.txt: файл проекта CMake верхнего уровня; проведите глобальную конфигурацию
# и включите сюда подпроекты.
#
cmake_minimum_required (VERSION 3.8)

#set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)

project ("nrcpp")

set(MYCOMPILE_FLAGS "-Wno-register")

set (CMAKE_CXX_STANDARD 17)

add_definitions(${MYCOMPILE_FLAGS})

# Включите подпроекты.

add_subdirectory ("nrc")

add_subdirectory ("nrcpp")

add_subdirectory ("kpp")
