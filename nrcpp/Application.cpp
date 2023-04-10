// ������� ������ ��������� - Application.cpp

#pragma warning(disable: 4786)

#include <windows.h>

#include <string>
#include <cstring>
//using namespace std;
#include "Limits.h"
#include "Application.h"
#include "LexicalAnalyzer.h"
#include "Object.h"
#include "Scope.h"
#include "Class.h"
#include "Parser.h"
#include "Manager.h"

#include <nrc.h>
using namespace nrc;



// ������ ���������� �������� ��� ����
Application theApp;



// � ��������� ������������ �������� ��� ������
TranslationUnit::TranslationUnit( PCSTR fnam ) 
{
	fileName = fnam;
	shortFileName = fnam;
	
	if( shortFileName.find("\\") != -1 || 
		shortFileName.find("/") != -1)
		shortFileName = shortFileName.DeleteRightWhileNot("\\/");

	inStream = fopen(fileName, "r");
	currentPos.line = 1;

	if( !inStream )
		theApp.Fatal("'%s' - ���������� ������� ����", fileName.c_str() );

	lexicalAnalyzer = new LexicalAnalyzer( inStream, currentPos );
	parser = new Parser(*lexicalAnalyzer);	

	// ������� ������� ���������� ��������� ���������, ��� ���� �������
	// ���������� ������� ��������
	scope = new Scope( new GeneralSymbolTable(DEFAULT_GLOBAL_HASHTAB_SIZE, NULL) );
	isCompile = false;

	// ��������� ��������� ����������. � ��������� ���������� ��������� � 
	// ������������ ������
	MakeImplicitDefinations();
}


// ���������� ����������� ������
TranslationUnit::~TranslationUnit()
{
	fclose(inStream);
	delete lexicalAnalyzer;
	delete parser;
}


// ��������� ���������� ����������. ��������� new, new[], delete, delete[]
void TranslationUnit::MakeImplicitDefinations()
{
	SymbolTable *global = const_cast<SymbolTable *>(scope->GetFirstSymbolTable());
	DerivedTypeList empty, ptr;
	FunctionParametrList fpl;
	DerivedTypeList paramDtl;
		
	// ������� ����������� ��� ��� delete
	paramDtl.AddDerivedType( new Pointer(false,false));
	fpl.AddFunctionParametr( new Parametr( 
		(BaseType*)&ImplicitTypeManager(KWVOID).GetImplicitType(), false, false, paramDtl,
		"", global, NULL, false) );
	empty.AddDerivedType( new FunctionPrototype(false, false, fpl,
		FunctionThrowTypeList(), false, false) );	

	// ������� ����������� ��� ��� new
	fpl.ClearFunctionParametrList();
	fpl.AddFunctionParametr( new Parametr( 
		(BaseType*)&ImplicitTypeManager(KWINT, KWUNSIGNED).GetImplicitType(), false, false, 
		DerivedTypeList(), "", global, NULL, false) );	
	ptr.AddDerivedType( new FunctionPrototype(false, false, fpl, 
		FunctionThrowTypeList(), true, false) );
	ptr.AddDerivedType(new Pointer(false,false));

	struct
	{
		// ��� ��������������
		PCSTR name;

		// ��� ���������
		int opCode;

		// ��� ���������
		PCSTR opName;

		// ������ �� ������ ����������� �����
		const DerivedTypeList *pdtl;
	} opmas[4] = {
		 "operator new", KWNEW, "new", &ptr ,
		 "operator new[]", OC_NEW_ARRAY, "new[]", &ptr ,
		 "operator delete", KWDELETE, "delete", &empty,
		 "operator delete[]", OC_DELETE_ARRAY, "delete[]", &empty
	};

	BaseType *btvoid = &const_cast<BaseType&>(ImplicitTypeManager(KWVOID).GetImplicitType());
	for( int i = 0; i<4; i++ )
		global->InsertSymbol(
			new OverloadOperator( opmas[i].name, global, 
				btvoid, false, false, *opmas[i].pdtl, false, Function::SS_NONE, 
				Function::CC_NON, opmas[i].opCode, opmas[i].opName) );
}


// ��������� ������� ����������
void TranslationUnit::Compile() 
{
	isCompile = true;
	parser->Run();
}


// ������ ���� ��� ����������. ���� �� ������ ������������
void ApplicationGenerator::OpenFile( PCSTR fnam )
{
	INTERNAL_IF( fout != NULL );

#if !_DEBUG
	// ���������, ���� ���� ����������, ������� ������
	if( fopen(fnam, "r") != NULL )
		theApp.Fatal("'%s' - ���� ��� ����������; �������� ���������� ����� ����������", fnam);
#endif

	fout = fopen(fnam, "w");
	if( !fout )
		theApp.Fatal("'%s' - ���������� ������� ��������� ���� ��� ������", fnam);
}


// �������� ������� ����� � ���� � �������� ���
void ApplicationGenerator::FlushCurrentBuffer( ) 
{
	if( fputs(currentBuffer.c_str(), fout) == EOF )
		theApp.Fatal( "���������� ���������� ������ � �������� ����" );
	currentBuffer = "";
}


// �������� ����� ������ � ���� � �������� ���	
void ApplicationGenerator::FlushUndoBuffer( ) 
{
	if( fputs(undoBuffer.c_str(), fout) == EOF )
		theApp.Fatal( "���������� ���������� ������ � �������� ����" );
	undoBuffer = "";
}


// ������� ��������� �� ����������� ����� ������
void Application::PutMessage( PCSTR head, PCSTR fname, const Position &pos, 
							 PCSTR fmt, va_list lst )
{
	char errbuf[512];	// ����� ��� ������������ ��������� �� ������

	_vsnprintf( errbuf, 512, fmt, lst );
	
	if( head )
		fprintf( stderr, "%s: ", head );

	if( fname )
      	fprintf( stderr, "%s", fname );

	if( pos.col > 0 || pos.line > 0 )
		fprintf( stderr, "(%d, %d): ", pos.line, pos.col );
	else
		fprintf( stderr, ": " );
	
	fprintf( stderr, "%s\n", errbuf );
}


// ��������� ����� �� ��������� ������
void Application::LoadOptions( int argc, char *argv[] )
{
}


// ������� ������
void Application::Error( const Position &pos, PCSTR fmt, ... )
{
	va_list vlst;	

	errcount++;
	va_start( vlst, fmt );
	PutMessage( "������", translationUnit == NULL ? "<���� �� ������>" : 
			translationUnit->GetFileName().c_str(), pos, fmt, vlst );
	va_end( vlst );	

	if( errcount == MAX_ERROR_COUNT )
		Fatal( "���������� ������ ��������� ���������� ������" );
}


// ������� ��������������
void Application::Warning( const Position &pos, PCSTR fmt, ... )
{
	va_list vlst;	

	warncount++;
	va_start( vlst, fmt );
	PutMessage( "��������������", translationUnit == NULL ? "<���� �� ������>" : 
		translationUnit->GetFileName().c_str(), pos, fmt, vlst );
	va_end( vlst );	

	if( warncount == MAX_WARNING_COUNT )
		Fatal( "���������� �������������� ��������� ���������� ������" );
}


// ������� ��������� ������
void Application::Fatal( const Position &pos, PCSTR fmt, ... )
{
	va_list vlst;	

	errcount++;
	va_start( vlst, fmt );
	PutMessage( "��������� ������", translationUnit == NULL ? "<���� �� ������>" : 
		translationUnit->GetFileName().c_str(), pos, fmt, vlst );
	va_end( vlst );	
	exit( ERROR_EXIT_CODE );
}


// ������� ���������� ������ �����������
void Application::Internal( const Position &pos, PCSTR msg, PCSTR fname, int line )
{
	errcount++;	

	string fullMsg = msg;
	fullMsg = fullMsg + " --> (" + CharString(fname).DeleteRightWhileNot("\\/").c_str() + 
		", " + CharString(line).c_str() + ")";
	PutMessage( "���������� ������ �����������", translationUnit == NULL ? "<���� �� ������>" : 
		translationUnit->GetFileName().c_str(), pos, fullMsg.c_str(), 0 );	
	exit( ERROR_EXIT_CODE );
}


// ������� ������, ������� ���������� � ���. �������
void Application::Error( PCSTR fmt, ... )
{
	va_list vlst;	

	errcount++;
	va_start( vlst, fmt );
	PutMessage( "������", translationUnit == NULL ? "<���� �� ������>" : 
		translationUnit->GetFileName().c_str(),	
		translationUnit == NULL ? Position() : translationUnit->GetPosition(), fmt, vlst );
	va_end( vlst );

	// ��������� ���������� ���������� ������ � ���� ��������� ������,
	// �������
	if( errcount == MAX_ERROR_COUNT )
		Fatal( "���������� ������ ��������� ���������� ������" );
}

// ������� ��������������, ������� ���������� � ���. �������
void Application::Warning( PCSTR fmt, ... )
{
	va_list vlst;	

	warncount++;
	va_start( vlst, fmt );
	PutMessage( "��������������", translationUnit == NULL ? "<���� �� ������>" : 
		translationUnit->GetFileName().c_str(), 
		translationUnit == NULL ? Position() : translationUnit->GetPosition(), fmt, vlst );
	va_end( vlst );	

	if( warncount == MAX_WARNING_COUNT )
		Fatal( "���������� �������������� ��������� ���������� ������" );
}

// ������� ��������� ������, ������� ���������� � ���. �������
void Application::Fatal( PCSTR fmt, ... )
{
	va_list vlst;	

	errcount++;
	va_start( vlst, fmt );
	PutMessage( "��������� ������", translationUnit == NULL ? "<���� �� ������>" : 
		translationUnit->GetFileName().c_str(), 
		translationUnit == NULL ? Position() : translationUnit->GetPosition(), fmt, vlst );
	va_end( vlst );	
	exit( ERROR_EXIT_CODE );
}

// ������� ���������� ������ �����������, ������� ���������� � ���. �������
void Application::Internal( PCSTR msg, PCSTR fname, int line  )
{
	errcount++;	
	string fullMsg = msg;
	fullMsg = fullMsg + " --> (" + CharString(fname).DeleteRightWhileNot("\\/").c_str() +
		", " + CharString(line).c_str() + ")";

	PutMessage( "���������� ������ �����������", translationUnit == NULL ? "<���� �� ������>" : 
		translationUnit->GetFileName().c_str(), 
		translationUnit == NULL ? Position() : translationUnit->GetPosition(), 
		fullMsg.c_str(), 0 );	
	exit( ERROR_EXIT_CODE );
}


// ������������� �����
int Application::Make()
{	
	// ������ �������� ���� ��� ����������
	generator.OpenFile("out.txt");
	translationUnit = new TranslationUnit ("in.txt");		

	translationUnit->Compile();
	delete translationUnit;

	return SUCCESS_EXIT_CODE;
}


// ��������� �����
int main( int argc, char *argv[] )
{
	SetConsoleCP(1251); 
	SetConsoleOutputCP(1251);

	try { 
		theApp.LoadOptions(argc, argv);
		return theApp.Make();

	} catch( PCSTR msg ) {	// �������� � ����������
		INTERNAL( msg );

	} catch( ... )	{		// �������� �� ����� ������� ������
		INTERNAL( "�������������� ��������� ����������" );
	}

	return 0;
}
