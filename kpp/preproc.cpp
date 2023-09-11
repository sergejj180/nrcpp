// ������ ��������� �������� ������ ������������� - preproc.cpp


// ����� ��������� ��������� � ������,
// ����������� �������� ����� ������ ������������� - 
// * ����������� ��������� �������������
// * ������������� �������� ��������
// * ������������ �����������

#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <list>
#include <algorithm>
#include <string>
#include <stack>

using namespace std;
#include "cpplex.h"
#include "kpp.h"
#include "macro.h"
#include "limits.h"


// ��� �������� �����
extern string inname;


// ���� ����������� ��� #if/#elif
extern stack<int> IfResults;


// ����� ���� - ��������� �� �����, ��� �����
// ������������ � include
extern string IncName;


// ��������� ��������� �������� �����
struct FileAttributes
{
	// ������ �� ������� ������������ � �����
	int line;


	// ��� �����
	string fname;


	// ��������� ����� #if/#elif, ��� ����� � ������,
	// ������ ���� ���������� ��������
	int state;


	// ��������� �� ����� �� �������� ��������� ������
	FileRead *buf;

	FileAttributes( FileRead *b ) { 
		line = linecount + 1; 
		fname = inname;
		state = IfResults.size();
		buf = b;
	}

	~FileAttributes() { }
};


// ������� ��� ������ ����� ����� � ������ ������������ ������
class FuncIncludeStack
{
	string &str;
public:
	FuncIncludeStack( string &s ) : str(s)  { }
	bool operator()( FileAttributes &attr ) { return attr.fname == str; }
}; 


// ������ ������������ ������
list<FileAttributes> IncFiles;


// ��������� 3 ���� ��������������� ��������� ��� ������
static string inline Do3Phases( const char *fnamein );


// ������� � ���� ��������� #line
void PutLine( FILE *out )
{
	string s = inname;
	fprintf(out, "#line %d %s\n", linecount, MakeStringLiteral(s).c_str() );
}


// ��������� �������� ����� � ������
static void inline PushFileAttr( FileRead * &file, FILE *out )
{
	if( IncFiles.size() == MAX_INCLUDE_DEEP )
		Fatal( "���� ����������: ������� �������� ����������� ������" );

	// ����� ���� ��� ���� � ����� ��� ��� ������� ����
	if( (find_if( IncFiles.begin(), IncFiles.end(), 
			FuncIncludeStack( IncName )) != IncFiles.end()) ||
		IncName == inname )
		Fatal( "'%s': ����������� ����������� �����", IncName.c_str() );
	

	// ��������� � ������ ����� ������, ��� �����, 
	// ��������� ����� if
	IncFiles.push_back( FileAttributes( file ) );
	
	// �������� �� ������ ����� ��� ����: ��������, �����, �����������
	string newin = Do3Phases( IncName.c_str() );	

	// ������ ����� �����
	file = new FileRead( xfopen(newin.c_str(), "r") );
	inname = IncName;
	linecount = 1;	


	// ������� #line
	PutLine( out );
}


// ������������ �������� �����
static inline bool PopFileAttr( FileRead * &file, FILE *out )
{
	if( IncFiles.empty() ) 	
		return false;

	FileAttributes &attr = IncFiles.back();

	if( attr.state != IfResults.size() )
		Fatal( "�������� '#endif'" ); 

	linecount = attr.line;
	inname = attr.fname;

	delete file;		// ��� �������� ������, ���� �����������
	file   = attr.buf;

	PutLine( out );
	IncFiles.pop_back();

	
	return true;
}


// ������� ��������� ������ �� �����,
// ���������� fasle, ���� ��������� ����� �����
bool ReadString( BaseRead &ob, string &fstr )
{
	register int c;

	if( (c = IgnoreSpaces(ob, false)) == EOF )
		return false;

	for(;;)
	{
		ob >> c;
		
		if( c == '\n' )
			break;

		else if( c == EOF )
		{
			ob << c;
			break;
		}

		else 
			fstr += (char)c;
	}
	
	SplitSpaces(fstr);	// ������� ������� �����
	return true;
}


// ��������� ��� �������� ������ �������������
static inline void KppWork(FILE *in, FILE *out)
{
	string s;
	FileRead *file = new FileRead(in);
	int Directive( string, FILE * );

	do
	{
		while( ReadString( *file, s ) )
		{
			if( s[0] == '#' )
			{
				int r;
				if( (r = 
					Directive(s, out)) == KPP_INCLUDE )		// ���� ��������� #include, ���������� ����
				{
					PushFileAttr( file, out );
					s = "";
					continue;
				}

				else if( r == KPP_LINE  || r == KPP_PRAGMA )
					fprintf(out, "%s", s.c_str());	// ������� ���������� ��� �����������				
			}

			else
				if( PutOut )
					fprintf(out, "%s", Substitution(s).c_str() );
		
			fputc('\n', out);
			s = "";
			linecount++;
		}

	} while(  PopFileAttr(file, out) );

	if( !IfResults.empty() )
		Fatal( "�������� '#endif'" );
}


// �������� ���� ������������������ ����� ��������� ��������� �
// ���������� �����
void Preprocess( const char *fnamein, const char *fnameout )
{
	FILE *in, *out;
	
	in = xfopen(fnamein, "r");
	out = xfopen(fnameout, "w");

	KppWork(in, out);

	fclose(in);
	fclose(out);
}


// ��������� 3 ���� ��������������� ��������� ��� ������
static string inline Do3Phases( const char *fnamein )
{
	string fcur = fnamein;
	string fout;
	int p;
	

	// ��������� ��������� ����� ������ � ������� ����������
	// ������� '\\' 
	if( (p = fcur.find_last_of("\\")) != -1 )
	{
		if( fcur.size() == p+1)
			fcur.erase( fcur.end() - 1 );
		else
			fcur.erase(0, p+1);
	}


	// ����������� �������
	fout = fcur + ".trig";
	TrigraphPhase( fnamein, fout.c_str() );

	// ��������� ������ �� �������	
	fout = fcur + ".slash";
	ConcatSlashStrings( (fcur + ".trig").c_str(), fout.c_str() );

	// ���������� �����������
	fout = fcur + ".comment";
	IgnoreComment( (fcur + ".slash").c_str(), fout.c_str() );

	return fout;
}


// ������ ������������������ �����:
// 1. ����������� ��������
// 2. ��������� ������ �� �������
// 3. ���������� �����������
// 4. �������� ������� � ����������� �������
void FullPreprocessing( const char *fnamein, const char *fnameout )
{
	string fout = Do3Phases( fnamein );

	// �������� ������������������
	Preprocess( fout.c_str(), fnameout );
} 
