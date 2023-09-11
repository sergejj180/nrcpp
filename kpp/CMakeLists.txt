# CMakeList.txt: проект CMake для CMakeProject2; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#

# Добавьте источник в исполняемый файл этого проекта.
add_executable (kpp	"cpplex.cpp" "cpplex.h"
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
