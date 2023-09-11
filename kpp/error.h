// ������������ ���� ������ - error.h


#define ERROR_EXIT_CODE	-1


// ��������� ������, ������� ������ � �������
void Fatal( const char *fmt, ... );


// ������ ����������
void Error( const char *fmt, ... );


// ��������������
void Warning( const char *fmt, ... );


// ������� ������
extern int errcount;


// ���������, � ������� ���������� ��������� (1251, 866)
extern int code_page;


// ������� ��������������
extern int warncount;


// ������ �������������, ������� ������������ ���
// ��������� ��������� � ���������� #if/#elif
enum KPP_EXCEPTION { 
	EXP_EMPTY, SYNTAX_ERROR, REST_SYMBOLS
};
