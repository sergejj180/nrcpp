// ����������� ���� ��� ������������ ����������� C++ - LexicalAnalyzer.h


#ifndef _LEXICAL_ANALYZER_H_INCLUDE
#define _LEXICAL_ANALYZER_H_INCLUDE

#include <string>
#include <cstring>
//using namespace std;
#include <nrc.h>
using namespace nrc;

// ������� �������� ��������, ������������ ��� �������������� � ����������� �������
#define IS_NAME_START(c)  (isalpha( (c) ) || (c) == '_')
#define IS_NAME( c )	  ( IS_NAME_START( c ) || isdigit(c) )
#define IS_INT_LITERAL( c )   ((c) == INTEGER10 || (c) == UINTEGER10 ||	\
			  			       (c) == INTEGER16 || (c) == UINTEGER16 ||	\
						       (c) == INTEGER8  || (c) == UINTEGER8  ||	\
						       (c) == CHARACTER || (c) == WCHARACTER )

#define IS_LITERAL( c )		  (IS_INT_LITERAL(c) ||						\
							   (c) == STRING || (c) == WSTRING ||		\
							   (c) == LFLOAT || (c) == LDOUBLE ||		\
							   (c) == KWTRUE || (c) == KWFALSE )	


// ���� ������� �������� ������� �������������� ����, ������� ����� ������������
// � �������� ���� ��� ������ ������ ������������ ��� �����������
#define IS_SIMPLE_TYPE_SPEC( c )	( (c) == KWINT     || (c) == KWCHAR || (c) == KWBOOL || \
									  (c) == KWWCHAR_T || (c) == KWSHORT|| (c) == KWLONG || \
									  (c) == KWSIGNED  || (c) == KWUNSIGNED ||				\
									  (c) == KWFLOAT   || (c) == KWDOUBLE || (c) == KWVOID )


// ���� ������ �++ 
enum CPP_TOKENS {

	// ����� ���, ����� �������� ����
	NAME = 257,

	// ���������
	STRING, WSTRING, CHARACTER, WCHARACTER, INTEGER10, INTEGER16, INTEGER8,
	UINTEGER10, UINTEGER16, UINTEGER8, LFLOAT, LDOUBLE,

	// ���������
	ARROW, INCREMENT, DECREMENT, DOT_POINT, ARROW_POINT,
	LEFT_SHIFT, RIGHT_SHIFT, LESS_EQU, GREATER_EQU, EQUAL, 
	NOT_EQUAL, LOGIC_AND, LOGIC_OR, MUL_ASSIGN, DIV_ASSIGN,
	PERCENT_ASSIGN, PLUS_ASSIGN, MINUS_ASSIGN, 
	LEFT_SHIFT_ASSIGN, RIGHT_SHIFT_ASSIGN, AND_ASSIGN, XOR_ASSIGN,
	OR_ASSIGN, COLON_COLON, DOUBLE_SHARP, ELLIPSES,

	// �������� �����
	KWASM,	KWAUTO,	KWBOOL,	KWBREAK, 
	KWCASE,	KWCATCH , KWCHAR, KWCLASS,				// KWCATCH = 300
	KWCONST, KWCONST_CAST, KWCONTINUE, KWDEFAULT,
	KWDELETE, KWDO,	KWDOUBLE, KWDYNAMIC_CAST,
	KWELSE,	KWENUM,	KWEXPLICIT, KWEXPORT,
	KWEXTERN, KWFALSE, KWFLOAT, KWFOR,
	KWFRIEND, KWGOTO, KWIF,	KWINLINE, 
	KWINT,	KWLONG,	KWMUTABLE, KWNAMESPACE,
	KWNEW, KWOPERATOR, KWPRIVATE, KWPROTECTED,
	KWPUBLIC, KWREGISTER, KWREINTERPRET_CAST, KWRETURN,
	KWSHORT, KWSIGNED, KWSIZEOF, KWSTATIC,
	KWSTATIC_CAST, KWSTRUCT, KWSWITCH, KWTEMPLATE,
	KWTHIS, KWTHROW, KWTRUE, KWTRY,
	KWTYPEDEF, KWTYPEID, KWTYPENAME, KWUNION,
	KWUNSIGNED, KWUSING, KWVIRTUAL, KWVOID,
	KWVOLATILE, KWWCHAR_T, KWWHILE
};


// ���� ������ �������������
enum KPP_TOKENS {
	KPP_DEFINE = 257, KPP_ERROR,  KPP_UNDEF,  
	KPP_ELIF,   KPP_IF,     KPP_INCLUDE,
	KPP_ELSE,   KPP_IFDEF,  KPP_LINE,   
	KPP_ENDIF,  KPP_IFNDEF, KPP_PRAGMA
};


// ������� ����� ���������� 
class BaseRead
{
public:
	BaseRead() { }
	virtual ~BaseRead() { }
 
	// ���������� �� ������ � ������
	virtual int operator>>( register int &c ) = 0;

	// ������� ������� � �����
	virtual void operator<<( register int &c ) = 0;
};


// ����� ���������� �� ������
class BufferRead : public BaseRead
{
	string buf;

	// ������� ��������� �� ����� � ������
	int i;	
public:
	BufferRead( string b ) : buf(b) { i = 0; }

	// ���������� �� ������ � ������
	int operator>>( register int &c ) {
		if( i == buf.length() ) 
			return (c = EOF);

		c = (unsigned char)buf[i++];
		return c;
	}

	// ������� ������� � �����
	void operator<<( register int &c ) { if(c != EOF) i--; }
};


// ����� ���������� �� �����
class FileRead : public BaseRead
{
	FILE *in;

public:
	FileRead( FILE *i ) : in(i) { }
	~FileRead( ) { fclose(in); }

	// ���������� �� ������ � ������
	int operator>>( register int &c ) {
		c = fgetc(in);
		return c;
	}

	// ������� ������� � �����
	void operator<<( register int &c ) { ungetc(c, in); }
};


// ����� ���������� �� ����� ��� �++ �����������
class CppFileRead : public BaseRead 
{
	// ��������� �� ������� �����
	FILE *in;


	// ������ �� ������ ������������ ����������� - �������
	Position &pos;

public:
	CppFileRead( FILE *i, Position &p ) : in(i), pos(p) { }
	~CppFileRead( ) {   }

	// ���������� �� ������ � ������
	int operator>>( register int &c ) {
		c = fgetc(in);
		pos.col++;
		return c;
	}

	// ������� ������� � �����
	void operator<<( register int &c ) { pos.col--; ungetc(c, in); }


	// ����� ���������� ��� ��������� ����� ������ � �����
	void NewLine() { pos.line++, pos.col = 1; }

	// �������� �������
	Position GetPosition() const { return pos; }

};


// ��������� ��������� �������, ������ ��������� �� ����� �������
// ������������� � ������ ����
class Lexem
{
	// �����
	CharString	buf;

	// ��� �������
	int code;

	// ������� � �����
	Position pos;

public:

	// ����������� �� ���������
	Lexem() {
		code = 0;
	}


	// ����������� � �������� ����������
	Lexem( const CharString &b, int c, const Position &p ) : buf(b), code(c), pos(p) {
	}

	// �������� �����
	const CharString &GetBuf() const {
		return buf;
	}

	// �������� ���
	int GetCode() const {
		return code;
	}

	// �������� �������
	const Position &GetPos() const {
		return pos;
	}

	// �������� ���, � ������� ���������� � ���� int
	operator int() const {
		return code;
	}

	// ������ � �������� ����� ������ ����� ����� ������ ����� LexicalAnalyzer
	// ��� ������� ��� ����, ����� ������ ���� ����� ��� �������� �������� �������
	friend class LexicalAnalyzer;
};


// ��������� ������, ����� ��������� � ���� �������������� 
// ���������� ������. ��������� ��� ������������� ���������� ����� ���������.
// ������� ��������������� � ������� LexicalAnalyzer
typedef list<Lexem>	LexemContainer;


// �������� ������ ���������� �� ����������� �����, ������������ ��� �������
ostream &operator<<( ostream &out, const LexemContainer &lc );


// ������� ����� ������ - ����������� ����������
class LexicalAnalyzer
{
	// ��������� � ���������� ��������� �������
	Lexem lastLxm, prevLxm;

	// �������� �������, ����� ����������� ����� � �����
	Lexem backLxm;
		
	// ��������� �� ������� ����� 
	CppFileRead *inStream;

	// ������� � �����
	const Position &curPos;

	// ��������� �� ���������, ���� �� ����� NULL, ������ ����������
	// ���������� �� ����, ����� �� ������. ���������� ���� ��� �������
	// �������
	LexemContainer *lexemContainer;

public:

	// ������ ������������ ����������� ����� ������� ������ 
	// ��������� ����� ����� �� �������� ������� ����������� ������ 
	// ������
	LexicalAnalyzer( FILE *in, Position &pos ) : curPos(pos), lexemContainer(NULL) {
		inStream = new CppFileRead(in, pos);
	}


	// ���������� ���������� ������� �����
	~LexicalAnalyzer() {
		delete inStream;
	}


	// �������� ��������� �������
	const Lexem &NextLexem();

	// �������� ���������� �������
	const Lexem &PrevLexem() const {
		return prevLxm; 
	}

	// �������� ��������� ��������� �������
	const Lexem &LastLexem() const {
		return lastLxm;
	}

	// ���������� ��������� ��������� ������� � �����,
	// ��� ��������� ������ NextLexem, ����� �������� ������
	// ���. ���� ����� ���������� � LAM_FILE_TO_CONTAINER, 
	// ������������� ������� ������ ��� �� ������������	
	void BackLexem( ) {	
		// ���������� ����� ������ ���� ������� � �����, ��� ��������
		// ���������� ������, ��������� ���������
		INTERNAL_IF( backLxm.GetCode() != 0 );		
		backLxm = lastLxm;		
	}

	// ��������� ��������� ��� ����������. ��� ���� ��������� �� ���������
	// ������ ��������� 0
	void LoadContainer( LexemContainer *lc ) {
		INTERNAL_IF( lexemContainer != NULL );
		lexemContainer = lc;
	}

	// ������ ��������� ��������� �������
	void SetLastLexem( const Lexem &lxm ) {
		lastLxm = lxm;
	}
};


// ���� �������� ����� ����� �++
int LookupCPPKeywords( const char *keyname );


// ���������� ��� ��������� ����� �� ����
const char *GetKeywordName( int code );


#endif // end  _LEXICAL_ANALYZER_H_INCLUDE
