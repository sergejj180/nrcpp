// ���������� ����������� � ����� - comment.cpp


#include <cstdio>
#include <cstdlib>

#include "kpp.h"


// ���������� ������������ �����������
static void inline IgnoreSimpleComment(FILE *in, FILE *out)
{	
	register int c;

	while( (c = fgetc(in)) != EOF )
		if(c == '\n')
		{
			fputc(c, out);
			return;
		}

	ungetc(c, in);
}


// ���������� ������������� �����������
static void inline IgnoreMultiComment(FILE *in, FILE *out)
{
	register int c;

	while( (c = fgetc(in)) != EOF )
	{
		if(c == '\n')
			fputc(c, out);
			
		else if( c == '*' )
		{
			c = fgetc(in);
			if( c == '/' )
			{ fputc(' ', out);	break; }	// ����������� ���������� ��������
			else
				ungetc(c, in);
		}
	}

	if( c == EOF )
		Fatal( "����������� ����� �����: �� �������� �����������" );
}


// ���������� ������������ � ������������� ����������� � �����
static void inline IgnoreAllComments( FILE *in, FILE *out )
{
	register int c;

	while( (c = fgetc(in)) != EOF )
	{
		if( c == '/' )
		{
			int pc = fgetc(in);

			if( pc == '/' )
			{	IgnoreSimpleComment(in, out); continue; }

			else if( pc == '*' )
			{	IgnoreMultiComment(in, out); continue; }

			else
				ungetc(pc, in);
		}

		// ���������� ������, ����� �� ���������������� ������� � ���
		else if( c == '\"' )
		{
			fputc(c, out);
			IgnoreStringLiteral( in,  out );
			continue;
		}

		fputc(c, out);
	}
}


// ������������ ����� ������ �������� �� ����� � ����
void IgnoreStringLiteral( FILE *in, FILE *out )
{
	register int c;

	while( (c = fgetc(in)) != EOF )
	{
		if( c == '\"' )
		{
			fputc(c, out);
			break;
		}

		else if( c == '\\' )
		{
			int pc = fgetc(in);
			if(pc == '\"')
			{
				fputc('\\', out); fputc('\"', out); 
				continue;
			}

			else
				ungetc(pc, in);
		}

		else if( c == '\n' )
		{
			fputc(c, out);
			break;
		}

		fputc(c, out);
	}

	if( c == EOF )
		Fatal( "����������� ����� �����: ��������� ������� �� ������" );
}


// ���������� ����������� � ���� �����, ���� ���������� ���� all
// ����� ���������� ������ � �������� �������������
// ���� ���������� ���� line - ������������ � �������� �����������
void IgnoreComment( const char *fnamein, const char *fnameout )
{
	FILE *in, *out;

	in = xfopen(fnamein, "r");
	out = xfopen(fnameout, "w");

	IgnoreAllComments(in, out);

	fclose(in);
	fclose(out);
}

