// ���������� ��� ������� ������������� - kpp.h


#include "error.h"


// ������� ��������������� ���� � ���������� (in) � ������� ���� (out)
void TrigraphPhase( const char *fnamein, const char *fnameout );


// ��������� ������ �� �������
void ConcatSlashStrings( const char *fnamein, const char *fnameout );


// �������� ���� ������������������ ����� ��������� ��������� �
// ���������� �����
void Preprocess( const char *fnamein, const char *fnameout );


// ���������� ����������� � ���� �����, ���� ���������� ���� all
// ����� ���������� ������ � �������� �������������
// ���� ���������� ���� line - ������������ � �������� �����������
void IgnoreComment( const char *fnamein, const char *fnameout );


// ������������ ����� ������ �������� �� ����� � ����
void IgnoreStringLiteral( FILE *in, FILE *out );


// ������ ������������������ �����:
// 1. ����������� ��������
// 2. ��������� ������ �� �������
// 3. ���������� �����������
// 4. �������� ������� � ����������� �������
void FullPreprocessing( const char *fnamein, const char *fnameout );


// ��������� ������� ����
FILE *xfopen(const char *name, const char *fmt);


// ���������� �������� ���������: hex, oct, char, wchar_t, int
int CnstValue( char *s, int code );


// ���� ��������� �� ������������� ������ ���������� �� �����
// ��������� ��� ���������� �������� ��������� ���������� (#if, #ifdef, ...)
extern bool PutOut;