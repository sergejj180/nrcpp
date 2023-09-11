// ����� ��������� ���������, ������������ ����� - slash.cpp


#include <cstdio>
#include <cstdlib>

#include "kpp.h"


// ������� �������� ���� �� �������, 
// ���������� �� ����� ���� ��� ������
static inline void SlashWork( FILE *in, FILE *out )
{
	register int c, pc;
	int line = 0;

	while((c = fgetc(in)) != EOF)
	{
		if(c == '\\')
		{
			if((pc = fgetc(in)) == '\n')
			{
				line++;
				continue;
			}

			else if(pc == EOF)
				Fatal("����������� ����� �����: ����� '\\'");
			ungetc(pc, in);
		}
			

		else if(c == '\n')
		{
			if(line)
				while(line)
					fputc('\n', out), line--;
		}
	
		fputc(c, out);
	}
}


// �������� �������, ��������� ������ �� �������
void ConcatSlashStrings( const char *fnamein, const char *fnameout )
{
	FILE *in, *out;
	
	in = xfopen(fnamein, "r");
	out = xfopen(fnameout, "w");

	SlashWork(in, out);

	fclose(in);
	fclose(out);

}
