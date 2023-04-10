// �������� ���������� ������������ �� ������ ������� - Application.h


#ifndef _APPLICATION_H_INCLUDE
#define _APPLICATION_H_INCLUDE

#include <nrc.h>
using namespace nrc;

#include <string>
#include <cstring>
//using namespace std;

// ���������� ������ ������� ��� ������ �������� �������
#include <ctime>


// ����� ���������� ������ �����������
#define INTERNAL( msg )	(theApp.Internal( msg, __FILE__, __LINE__ ) )

// ����� ���������� ������ �����������, ���� ������� �����
#define INTERNAL_IF( exp )  ( (exp) ? INTERNAL( "\"" #exp "\"" ) : (void)0 )


// ����� � ��������
#define ERROR_EXIT_CODE		-1


// ���������� ���������� ���������
#define SUCCESS_EXIT_CODE	0


// ������� ���� - ��������� �� ����������� ������
typedef const char *PCSTR;


// ��������� ��������� ������� � �����, ������� ������������ � 
// ����������� �����������
struct Position
{
	// ������, �������
	unsigned line, col;

	// �����������
	Position( unsigned l = 0, unsigned c = 0 ) : line(l), col(c) {
	}
};

// ����� �������� � LexicalAnalyzer.h
class LexicalAnalyzer;

// ����� �������� � Parser.h
class Parser;

// ����� �������� � Scope.h
class Scope;

// ����� �������� � Class.h
class ClassType;

// �������� � Object.h
class Identifier;


// ������ ����������
class TranslationUnit
{
	// ������ ��� ������
	CharString fileName;

	// �������� ���, ��� ������� ����
	CharString shortFileName;

	// ������� ������� � �����
	Position currentPos;


	// ��������� �� ����� �����
	FILE *inStream;

	// �������������� ���������� ������
	LexicalAnalyzer *lexicalAnalyzer;

	// �������������� ���������� ������
	Parser *parser;

	// ������� ���������� ��������� ���������
	Scope *scope;

	// true, ���� ���������� ��������
	bool isCompile;

	// ��������� ���������� ����������. ��������� new, new[], delete, delete[]
	void MakeImplicitDefinations();

public:

	// � ��������� ������������ �������� ��� ������
	TranslationUnit( PCSTR fnam ) ;

	// ���������� ����������� ������
	~TranslationUnit();

	// ��������� ������� ����������
	void Compile();

	// �������� ������� �������
	Position GetPosition() const { return currentPos; }


	// �������� ��� �����
	const CharString &GetFileName() const { return fileName; }

	// �������� �������� ��� �����
	const CharString &GetShortFileName() const { return shortFileName; }

	// �������� ������� ���������� ��������� ���������
	const Scope &GetScopeSystem() const {
		return *scope;
	}

	// �������� ������
	const Parser &GetParser() const {
		return *parser;
	}
};


// ��������� ����������, ������ �������� � ������
class ApplicationGenerator
{
	// ������� �����
	string currentBuffer;

	// ����� ������
	string undoBuffer;

	// ��������� �� �������� �����
	FILE *fout;

public:
	// ������ ����. ���� �� ������ ������������
	ApplicationGenerator( ) 
		: fout(NULL) {
	}

	// ������� ����
	~ApplicationGenerator() {
		if( fout )
			fclose(fout);
	}

	// ������� ����
	void OpenFile( PCSTR fnam );

	// ������������ � ������� �����
	void GenerateToCurrentBuffer( const string &buf ) {
		currentBuffer += buf;
	}

	// ������������ � ����� ������
	void GenerateToUndoBuffer( const string &buf ) {
		currentBuffer += buf;
	}

	// �������� ������� ����� � ���� � �������� ���
	void FlushCurrentBuffer( );

	// �������� ����� ������ � ���� � �������� ���	
	void FlushUndoBuffer( );
};


// ����������
class Application
{
	// ������ cpp-������ ��� ����������
	TranslationUnit *translationUnit;

	// ��������� ����������. ������������ ������������ ��� ������ ���������������
	// ���������� � �������� ����
	ApplicationGenerator generator;
	
	// ������� ������ � ��������������
	int errcount, warncount;
	
	// ����� ������ ������ ���������
	clock_t startTime;	

	// ������� ��������� �� ����������� ����� ������
	void PutMessage( PCSTR head, PCSTR fname, const Position &pos, PCSTR fmt, va_list lst );

public:
	// �����������
	Application()
		: translationUnit(NULL), errcount(0), warncount(0), startTime( clock() ){		
	} 

	// ���������� ������� ����� ������ ���������
	~Application() {				
//		printf( "����� ����������: %lf ������\n", 
//				(double)(clock() - startTime) / CLOCKS_PER_SEC );		
	}


	// �������� ������ �������� �������������� �����
	const TranslationUnit &GetTranslationUnit() const {
		return *translationUnit;
	}

	// ������� ���������
	ApplicationGenerator &GetGenerator() {
		return generator;
	}

	// ���� �������������� ��������� � ������ ����������� ������� true
	bool IsDiagnostic() const {
		return errcount > 0;
	}

	// ��������� ����� �� ��������� ������
	void LoadOptions( int argc, char *argv[] );

	// ������������� �����
	int Make();

	// ������� ������, ������� ���������� � ���. �������
	void Error( const Position &pos, PCSTR fmt, ... );

	// ������� ��������������, ������� ���������� � ���. �������
	void Warning( const Position &pos, PCSTR fmt, ... );

	// ������� ��������� ������, ������� ���������� � ���. �������
	void Fatal( const Position &pos, PCSTR fmt, ... );

	// ������� ���������� ������ �����������, ������� ���������� � ���. �������
	void Internal( const Position &pos, PCSTR msg, PCSTR fname, int line  );

	// ������� ������
	void Error( PCSTR fmt, ... );

	// ������� ��������������
	void Warning( PCSTR fmt, ... );

	// ������� ��������� ������
	void Fatal( PCSTR fmt, ... );

	// ������� ���������� ������ �����������
	void Internal( PCSTR msg, PCSTR fname, int line );

};


// ������ ���������� �������� ��� ����
extern Application theApp;


#endif		// end _APPLICATION_H_INCLUDE
