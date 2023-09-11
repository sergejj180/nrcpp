// ��������� ����������� ��������� - cnst-expr.cpp


#include <list>
#include <algorithm>
#include <string>
#include <cerrno> 
#include <cstdlib>

using namespace std;

#include "cpplex.h"
#include "error.h"
#include "limits.h"
#include "macro.h"

// ������� �����
static int token;


// ��������� �� ������� ������ 
static BufferRead *pBuf;


// ����� ������������ ������������ ����� � �������
class ParserStack
{
public:
	static int curdeep;

	ParserStack( ) { 
		curdeep++; 
		if( curdeep == MAX_PARSER_DEEP )
			Fatal( "���� ����������: ������� ������� ���������" );
	}

	~ParserStack( ) { curdeep--; }
};


int ParserStack::curdeep = 0;


static void EvalExpr2( int &result );
static void EvalExpr3( int &result );
static void EvalExpr4( int &result );
static void EvalExpr5( int &result );
static void EvalExpr6( int &result );
static void EvalExpr7( int &result );
static void EvalExpr8( int &result );
static void EvalExpr9( int &result );
static void EvalExpr10( int &result );
static void EvalExpr11( int &result );
static void EvalExpr12( int &result );
static void EvalExpr13( int &result );
static void EvalExpr14( int &result );



// ����������� ����������, ����������� token �������� �������
static inline int TokLex()
{
	return token = Lex(*pBuf);
}


// ������� ����������� ������ � �����
// -1 - ���� �������������� �� �������
static inline int ConvertCharToInt( char *s, bool wide )
{
	register char *p;
	extern int isdigit8( int );
	int r;
	char *end;	// ����� ���������, ������ ��������� �� '\''

	p = wide ? s+2 : s + 1;	// ����� '\''
	end = p + 1;
	r = *p;

	if(*p == '\\')
	{
		end = p+2;
		if( *(p+1) == 'x' && *(p+2) == '\'')
		{
			Error("����������� 16-������ ������������������ ����� '\\x'");
			return 'x';
		}

		if( *(p+1) == 'x' || isdigit8(*(p+1)) )
		{
			int base = *(p+1) == 'x' ? 16 : 8;
			char *stop;
			char *start = base == 16 ? p+2 : p+1;

			r = strtol( start, &stop, base );

			// ��������� ������������
			if( (errno == ERANGE) || 
				(wide ? r > MAX_WCHAR_T_VALUE : r > MAX_CHAR_VALUE) )
				return -1;

			if( *stop != '\'' )
			{
				Error( "'%x': ����������� ������ � %d-������ ������������������", 
					*stop, base );
				return *(p+1);
			}

			else
				return r;
		}

		else
		switch( *(p + 1) )
		{
		case 't' : r = '\t'; break;
		case 'v' : r = '\v'; break;
		case 'b' : r = '\b'; break;
		case 'r' : r = '\r'; break;
		case 'f' : r = '\f'; break;
		case 'a' : r = '\a'; break;
		case '\\': r = '\\'; break;
		case '?' : r = '\?'; break;
		case '\'': r = '\''; break;
		case '\"': r = '\"'; break;
		default:
			Error( "'\\%c': ������������ ��������� ������������������", *(p+1));
			return *(p+1);
		}
	}

	if( *end == '\'' )
		return *p;

	else
	{
		if( wide )
		{
			Warning( "������� ����� ������� ������������ � ��������� 'wchar_t'" ); 
			return r;
		}

		else
			return -1;
	}	
}


// ������ � 16-, 10-, 8-������ ������ ������������� � �����
// �����. -1 ���� �������������� �� ������� (������������)
static inline int ConvertInteger( char *s, int base )
{
	int r = strtol( s, NULL, base );
	return errno == ERANGE ? -1 : r;
}


// ���������� �������� ���������: hex, oct, char, wchar_t, int
int CnstValue( char *s, int code )
{
	int r;
	char *tname;

	if( code == CHARACTER ) 
		r = ConvertCharToInt(s, false), tname = "char";

	else if( code == WCHARACTER ) 
		r = ConvertCharToInt(s, true), tname = "wchar_t";

	else if( code == INTEGER10 || code == UINTEGER10 )
		r = ConvertInteger(s, 10), 
			tname = (code == UINTEGER10 ? "unsigned int" : "int");
	
	else if( code == INTEGER8 || code == UINTEGER8 )
		r = ConvertInteger(s, 8),
			tname = (code == UINTEGER8 ? "unsigned int" : "int");
	
	else if( code == INTEGER16 || code == UINTEGER16 )
		r = ConvertInteger(s, 16),
			tname = (code == UINTEGER16 ? "unsigned int" : "int");
	
	else 
		return -1;

	if( r == -1 )
	{
		Error("�������� ��������� ������� ������ ��� ���� '%s'", tname);
		errno = 0; 
	}

	return r;
}


// ���������� ��������� ������������ ��������� � ������ s
int CnstExpr( string &s )
{
	pBuf = new BufferRead(s);

	if( TokLex() == EOF )
		throw EXP_EMPTY;

	int result = 0;
	
	EvalExpr2(result);

	if( token != EOF )
		throw REST_SYMBOLS;

	delete pBuf;
	return result;
}


// �������� '?:'
static void EvalExpr2( int &result )
{
	int temp1, temp2;

	// ��� �������� ������� �������������� ������������ �����
	ParserStack control;	

	EvalExpr3( result );
	if( token == '?' )
	{
		TokLex();
		EvalExpr2(temp1);

		if( token != ':' )
		{	
			throw "�������������� ������: ��������� ':'"; 
		}

		TokLex();
		EvalExpr3(temp2);
		result = (result ? temp1 : temp2);
	}
}


// �������� ||
static void EvalExpr3( int &result )
{
	int temp;
	ParserStack control;	

	EvalExpr4( result );
	while( token == LOGIC_OR )
	{
		TokLex();
		EvalExpr4( temp );

		result = result || temp;
	}
}


// �������� &&
static void EvalExpr4( int &result )
{
	int temp;
	ParserStack control;

	EvalExpr5( result );
	while( token == LOGIC_AND )
	{
		TokLex();
		EvalExpr5( temp );

		result = result && temp;
	}
}


// �������� |
static void EvalExpr5( int &result )
{
	int temp;
	ParserStack control;

	EvalExpr6( result );
	while( token == '|' )
	{
		TokLex();
		EvalExpr6( temp );

		result = result | temp;
	}
}


// �������� '^'
static void EvalExpr6( int &result )
{
	int temp;
	ParserStack control;

	EvalExpr7( result );
	while( token == '^' )
	{
		TokLex();
		EvalExpr7( temp );

		result = result ^ temp;
	}
}


// �������� '&'
static void EvalExpr7( int &result )
{
	int temp;
	ParserStack control;

	EvalExpr8( result );
	while( token == '&' )
	{
		TokLex();
		EvalExpr8( temp );

		result = result & temp;
	}
}


// ��������� ==, !=
static void EvalExpr8( int &result )
{
	int temp, op;
	ParserStack control;

	EvalExpr9( result );
	while( (op = token) == EQUAL || op == NOT_EQUAL)
	{
		TokLex();
		EvalExpr9( temp );

		if( op == EQUAL )
			result = result == temp;
		else
			result = result != temp;
	}
}


// ��������� <=, <, >, >=
static void EvalExpr9( int &result )
{
	int temp, op;
	ParserStack control;

	EvalExpr10( result );
	while( (op = token) == LESS_EQU || op == GREATER_EQU || 
			op == '<' || op == '>' )
	{
		TokLex();
		EvalExpr10( temp );

		if( op == LESS_EQU )
			result = result <= temp;

		else if( op == GREATER_EQU )
			result = result >= temp;

		else if( op == '>' )
			result = result > temp;

		else
			result = result < temp;
	}
}


// ��������� <<, >>
static void EvalExpr10( int &result )
{
	int temp, op;
	ParserStack control;

	EvalExpr11( result );
	while( (op = token) == RIGHT_SHIFT || op == LEFT_SHIFT )
	{
		TokLex();
		EvalExpr11( temp );

		if( op == LEFT_SHIFT )
			result = result << temp;

		else
			result = result >> temp;
	}
}


// ��������� +, -
static void EvalExpr11( int &result )
{
	int temp, op;
	ParserStack control;

	EvalExpr12( result );
	while( (op = token) == '+' || op == '-' )
	{
		TokLex();
		EvalExpr12( temp );

		if( op == '+' )
			result = result + temp;

		else
			result = result - temp;
	}	
}


// ��������� *, /, %
static void EvalExpr12( int &result )
{
	int temp, op;
	ParserStack control;

	EvalExpr13( result );
	while( (op = token) == '*' || op == '/' || op == '%' )
	{
		TokLex();
		EvalExpr13( temp );

		if( op == '*' )
			result = result + temp;

		else 
		{
			if( temp == 0 )			
				throw "������� �� 0";			

			if( op == '/' )
				result = result / temp;

			else 
				result = result % temp;
		}
	}	
}


// ������� ��������� !, ~, +, -
static void EvalExpr13( int &result )
{
	int op;
	ParserStack control;

	if( (op = token) == '!' || op == '~' ||
		 op == '+' || op == '-' )
	{
		TokLex();
		EvalExpr13( result );

		if( op == '!' )
			result = !result;

		else if( op == '~' )
			result = ~result;

		else if( op == '-' )
			result = -result;
	}

	else
		EvalExpr14( result );
}


// ������� ����������� ����� � ����� �����
// ��� ��������� ��������� � �������
static void EvalExpr14( int &result )
{
	if( token == '(' )
	{
		TokLex();
		EvalExpr2(result);
		if( token != ')' )		
			throw ("�������������� ������: ��������� ')'");				

		TokLex();
	}

	else if(IS_LITERAL(token))
	{
		result = CnstValue( (char *)lexbuf.c_str(), token );
		TokLex();
	}

	else
		throw ("�������������� ������");		
}
