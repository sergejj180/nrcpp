# CMakeList.txt: проект CMake для nrcpp; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required (VERSION 3.8)

project(nrcpp)

#set (CMAKE_CXX_STANDARD 14)
#
set(CMAKE_BINARY_DIR "${CMAKE_SOURCE_DIR}/bin/${ROJECT_NAME}")
#
#include_directories("${CMAKE_SOURCE_DIR}/nrc")
#set(ALLEGRO_INCLUDES PUBLIC 
#    ${CMAKE_SOURCE_DIR}/allegro/include
#    ${CMAKE_SOURCE_DIR}/allegro/include/allegro 
#    ${CMAKE_SOURCE_DIR}/allegro/include/allegro/inline
#    ${CMAKE_SOURCE_DIR}/allegro/include/allegro/internal
#    ${CMAKE_SOURCE_DIR}/allegro/include/allegro/platform)

set(NRC_INCLUDES PUBLIC "${CMAKE_SOURCE_DIR}/nrc" )

#set(ALLEGRO_LIBRARIES ${CMAKE_SOURCE_DIR}/allegro/lib)
set(NRC_LIBRARIES "${CMAKE_SOURCE_DIR}/nrc")
# Добавьте источник в исполняемый файл этого проекта.
add_executable (${CMAKE_PROJECT_NAME}
    "Application.cpp" "Application.h"
    "Body.cpp" "Body.h"
    "BodyMaker.cpp" "BodyMaker.h"
    "Checker.cpp" "Checker.h"
    "Class.cpp" "Class.h"
    "Coordinator.cpp" "Coordinator.h"
    "ExpressionChecker.cpp" 
    "ExpressionMaker.cpp" "ExpressionMaker.h" 
    "LexicalAnalyzer.cpp" "LexicalAnalyzer.h"
    "Limits.h"
    "Maker.cpp" "Maker.h"
    "Manager.cpp" "Manager.h"
    "MemberMaker.cpp" "MemberMaker.h"
    "Object.cpp" "Object.h"
    "Overload.cpp" "Overload.h"
    "PackCode.h"
    "Parser.cpp" "Parser.h"
    "Reader.cpp" "Reader.h"
    "Scope.cpp" "Scope.h"
    "Translator.cpp" "Translator.h" )

#
#target_include_directories(${PROJECT_NAME} ${ALLEGRO_INCLUDES})
#target_include_directories(${PROJECT_NAME} ${NRC_INCLUDES})
target_include_directories(nrcpp PUBLIC "${CMAKE_SOURCE_DIR}/nrc")
#find_library(ALLEGRO_LIBRARY
#   NAME alleg44 alleggl jpgalleg lodpng logg 
#   HINTS ${ALLEGRO_LIBRARIES})

find_library(NRC_LIBRARY
    NAME nrc
    PATH ${NRC_LIBRARIES})
#
#target_link_libraries(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/nrc")
#link_directories("${CMAKE_SOURCE_DIR/nrc")
target_link_libraries(${PROJECT_NAME} PUBLIC "${NRC_LIBRARY}/" )
#target_link_libraries(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/nrc/nrc")

