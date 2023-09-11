// ����������� ���� ��� ������������ ����������� C++ - cpplex.h



#define IS_NAME_START(c)  (isalpha( (c) ) || (c) == '_')
#define IS_NAME( c )	  ( IS_NAME_START( c ) || isdigit(c) )
#define IS_LITERAL( c )   ((c) == INTEGER10 || (c) == UINTEGER10 ||	\
						   (c) == INTEGER16 || (c) == UINTEGER16 ||	\
						   (c) == INTEGER8  || (c) == UINTEGER8  ||	\
						   (c) == CHARACTER || (c) == WCHARACTER )

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
	OR_ASSIGN, COLON_COLON, ELLIPSES,

	// �������� �����
	KWASM,	KWAUTO,	KWBOOL,	KWBREAK, 
	KWCASE,	KWCATCH, KWCHAR, KWCLASS,
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


// ����� ���������� �� �����
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


// ������� �����
extern int linecount;


// ����� � ���������� �������
extern string lexbuf;


// ��������� ������� ������� �����
void SplitSpaces( string &s );


// ������� ����������� ������ s � ��������� �������
string &MakeStringLiteral( string &s );


// ������� ��������� ������ �� �����,
// ���������� fasle, ���� ��������� ����� �����
bool ReadString( BaseRead &ob, string &fstr );


// ��������� ����� �� �������� ������, ����
// ������� isfunc ���������� true 
void ReadDigit( BaseRead &ob, int (*isfunc)(int) );


// ���������� ������� � ����� ������
int IgnoreNewlinesAndSpaces( BaseRead &ob );


// ������������ ������ �������
int IgnoreSpaces( BaseRead &ob, bool putspaces = true );


// ������� ���������� ��� ��������� ����� ��� -1
// � ������ ���� ������ ��������� ����� ���
int LookupKeywordCode( const char *keyname, struct keywords *kmas, int szmas );


// ���� �������� ����� kpp
int LookupKppKeywords( const char *keyname );


// �������� ������� '�������������'
inline int LexemName( BaseRead &ob );


// ������� �������� ��������� ������� ��
// ������ in
int Lex( BaseRead &ob );
