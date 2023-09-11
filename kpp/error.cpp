// ������ ������ ������ - error.cpp

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

using namespace std;

#include <windows.h>

#include "cpplex.h"
#include "kpp.h"

#define ERRBUFSIZE	512


// ������� ������
int errcount;


// ������� ��������������
int warncount;


// ���������, � ������� ���������� ��������� (1251, 866)
int code_page;


// ��������� ����� ��������������
bool no_warnings = false;


// ��� �������� �����
extern string inname;


// ����� ������
static inline void ErrorMessage( const char *pred, const char *fmt, va_list lst )
{
	char errbuf[ERRBUFSIZE];	// ����� ��� ������������ ��������� �� ������

	_vsnprintf( errbuf, ERRBUFSIZE, fmt, lst );
	
	if(code_page == 866)
	{	
		char temp[ERRBUFSIZE], temp2[ERRBUFSIZE];		
		
		if( pred )
		{			
			CharToOem(pred, temp2);
			fprintf(stderr, "%s: ", temp2);
		}

		if(linecount == -1)	// ��������� ������ ����� ���� �� ������ �������� �����
			_snprintf(temp, ERRBUFSIZE, "%s\n", errbuf);
		else
			_snprintf(temp, ERRBUFSIZE, "%s: %d: %s\n", inname.c_str(), 
				linecount, errbuf);

		CharToOem(temp, errbuf);
		fprintf(stderr, "%s", errbuf);
	}

	else
	{
		if( pred )
			fprintf( stderr, "%s: ", pred );

		if( linecount == -1 )
			fprintf( stderr, "��������� ������: %s\n", errbuf );

		else
			fprintf( stderr, "��������� ������: %s: %d: %s\n", inname, 
				linecount, errbuf );
	}	
}



// ��������� ������, ������� ������ � �������
void Fatal( const char *fmt, ... )
{
	va_list vlst;	

	va_start( vlst, fmt );
	ErrorMessage( "��������� ������", fmt, vlst );
	va_end( vlst );
	exit(ERROR_EXIT_CODE);	
}


// ������ ����������
void Error( const char *fmt, ... )
{
	va_list vlst;	

	errcount++;
	va_start( vlst, fmt );
	ErrorMessage( NULL, fmt, vlst );
	va_end( vlst );	
}


// ��������������
void Warning( const char *fmt, ... )
{
	va_list vlst;	

	warncount++;
	if( no_warnings )
		return;

	va_start( vlst, fmt );
	ErrorMessage( "��������������", fmt, vlst );
	va_end( vlst );	
}


