// заголовочный файл ошибок - error.h


#define ERROR_EXIT_CODE	-1


// фатальна ошибка, выводит ошибку и выходит
void Fatal( const char *fmt, ... );


// ошибка компиляции
void Error( const char *fmt, ... );


// предупреждение
void Warning( const char *fmt, ... );


// счетчик ошибок
extern int errcount;


// кодировка, в которой выводиться сообщения (1251, 866)
extern int code_page;


// счетчик предупреждений
extern int warncount;


// ошибки препроцессора, которые возбуждаются при
// обработке выражений в директивах #if/#elif
enum KPP_EXCEPTION { 
	EXP_EMPTY, SYNTAX_ERROR, REST_SYMBOLS
};
