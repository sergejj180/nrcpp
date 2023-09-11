// ��������� ��������� (������ ���� ��������������� ���������) - trigraph.cpp


// ���������� ������������������ � �� ���������� �����������:
//  ??=  #        ??(  [
//  ??/  \        ??)  [
//  ??'  ^        ??!  |
//  ??<  {		  ??>  } 
//  ??-  ~ 

#include <cstdio>
#include <cstdlib>

#include "kpp.h"


// ������� ����������� ���� � ���������� 
// ������������������� � ������� CPP-����
static inline void TrigraphToCPP( FILE *in, FILE *out )
{
	register int c;

	while( (c = fgetc(in)) != EOF )
	{
		if(c != '?')
		{
			fputc(c, out);
			continue;
		}

		if( (c = fgetc(in)) != '?' )
		{
			fputc('?', out); 
			ungetc(c, in);
			continue;
		}

		c = fgetc(in);

		// ����� ���������� ����� ����������
		switch((char)c)
		{
		case '=':  fputc('#', out);  break;
		case '/':  fputc('\\', out); break;
		case '\'': fputc('^', out);  break;
		case '<':  fputc('{', out); break;
		case '>':  fputc('}', out); break;
		case '-': fputc('~', out); break;
		case '(': fputc('[', out); break;
		case ')': fputc(']', out); break;
		case '!': fputc('|', out); break;
		default:  ungetc(c, in); fputc('?', out); fputc('?', out);
		}
	}
}


// ������� �������� TrigraphToCPP
static inline void CPPToTrigraph( FILE *in, FILE *out )
{
	register int c;

	while( (c = fgetc(in)) != EOF )
	{
		// ����� ���������� ����� ����������
		switch((char)c)
		{
		case '#':  fprintf(out, "??"); fputc('=', out); break;
		case '\\':  fprintf(out, "??"); fputc('/', out);break;
		case '^': fprintf(out, "??");  fputc('\'', out); break;
		case '{':  fprintf(out, "??"); fputc('<', out); break;
		case '}':  fprintf(out, "??"); fputc('>', out); break;
		case '~': fprintf(out, "??");  fputc('-', out); break;
		case '[': fprintf(out, "??");  fputc('(', out); break;
		case ']': fprintf(out, "??");  fputc(')', out); break;
		case '|': fprintf(out, "??");  fputc('!', out); break;
		default:  fputc(c, out); break;
		}
	}

}


// ��������� ������� ����
FILE *xfopen(const char *name, const char *fmt)
{
	FILE *r;
	if((r = fopen(name, fmt)) == NULL)
		Fatal("�� ����������� ������� ���� '%s'", name);
	
	return r;
}


// ������� ��������������� ���� � ���������� (in) � ������� ���� (out)
void TrigraphPhase( const char *fnamein, const char *fnameout )
{
	FILE *in, *out;

	in = xfopen(fnamein, "r");
	out = xfopen(fnameout, "w");

	TrigraphToCPP(in, out);

	fclose(in);
	fclose(out);
}


