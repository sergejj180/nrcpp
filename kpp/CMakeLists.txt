# CMakeList.txt: проект CMake для CMakeProject2; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
# CMakeList.txt: файл проекта CMake верхнего уровня; проведите глобальную конфигурацию
# и включите сюда подпроекты.
#
cmake_minimum_required (VERSION 3.8)

#project(kpp)

#set (CMAKE_CXX_STANDARD 14)

#set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin/kpp)

# Добавьте источник в исполняемый файл этого проекта.
add_executable (kpp "cpplex.cpp" "cpplex.h"
			"error.cpp"  "error.h" 
			"kpp.cpp"    "kpp.h"
			"macro.cpp"  "macro.h"
			"read.cpp"   "read.h"

			"cnst-expr.cpp"
			"comment.cpp"
			"directive.cpp"
			"preproc.cpp"
			"slash.cpp"
			"trigraph.cpp"

			"hash.h" 
			"limits.h"

)

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
