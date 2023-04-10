// ����������� ���������� ��� ����������� C++ - LexicalAnalyzer.cpp

#pragma warning(disable: 4786)
#include <nrc.h>
using namespace nrc;

#include "Limits.h"
#include "Application.h"
#include "LexicalAnalyzer.h"



// ��������� ������� ������������ ����� ���� �������
// �� �� ����� 
struct keywords
{
	// ��� ��������� �����
	const char *name;


	// ��� ��������� �����
	int code;

}	kpp_words[] = {
	{ "define", KPP_DEFINE },
	{ "error",  KPP_ERROR  },
	{ "undef",  KPP_UNDEF  },
	{ "elif",   KPP_ELIF   },
	{ "if",		KPP_IF     },
	{ "include", KPP_INCLUDE },
	{ "else",   KPP_ELSE   },
	{ "ifdef",  KPP_IFDEF  },
	{ "line",   KPP_LINE   },
	{ "endif",  KPP_ENDIF  },
	{ "ifndef", KPP_IFNDEF },
	{ "pragma", KPP_PRAGMA }
	},

	c_words[] = {
	{ "auto", KWAUTO }, { "break", KWBREAK },
	{ "case", KWCASE }, { "char",  KWCHAR },
	{ "const", KWCONST }, { "continue", KWCONTINUE }, 
	{ "default", KWDEFAULT }, { "do", KWDO },
	{ "double", KWDOUBLE }, { "else", KWELSE }, 
	{ "enum", KWENUM },	{ "extern", KWEXTERN },  
	{ "float", KWFLOAT }, { "for", KWFOR },
	{ "goto", KWGOTO }, { "if", KWIF }, 
	{ "int", KWINT }, { "long", KWLONG },
	{ "register", KWREGISTER }, { "return", KWRETURN }, 
	{ "short", KWSHORT }, { "signed", KWSIGNED }, 
	{ "sizeof", KWSIZEOF },  { "static", KWSTATIC }, 
	{ "struct", KWSTRUCT }, { "switch", KWSWITCH }, 
	{ "typedef", KWTYPEDEF }, { "union", KWUNION },  
	{ "unsigned", KWUNSIGNED }, { "void", KWVOID },  
	{ "volatile", KWVOLATILE }, { "while", KWWHILE } 
	},

	cpp_words[] = {
	{ "asm", KWASM }, { "auto", KWAUTO }, 
	{ "bool", KWBOOL }, { "break", KWBREAK },
	{ "case", KWCASE }, { "catch", KWCATCH },
	{ "char",  KWCHAR }, { "class", KWCLASS },
	{ "const", KWCONST }, { "const_cast", KWCONST_CAST },
	{ "continue", KWCONTINUE }, { "default", KWDEFAULT },
	{ "delete", KWDELETE }, { "do", KWDO },
	{ "double", KWDOUBLE }, { "dynamic_cast", KWDYNAMIC_CAST },
	{ "else", KWELSE }, { "enum", KWENUM },	
	{ "explicit", KWEXPLICIT }, { "export", KWEXPORT },
	{ "extern", KWEXTERN },  { "false", KWFALSE }, 
	{ "float", KWFLOAT }, { "for", KWFOR },
	{ "friend", KWFRIEND }, { "goto", KWGOTO },
	{ "if", KWIF }, { "inline", KWINLINE }, 
	{ "int", KWINT }, { "long", KWLONG },
	{ "mutable", KWMUTABLE }, { "namespace", KWNAMESPACE },
	{ "new", KWNEW }, { "operator", KWOPERATOR }, 
	{ "private", KWPRIVATE }, { "protected", KWPROTECTED },
	{ "public", KWPUBLIC }, { "register", KWREGISTER },
	{ "reinterpret_cast", KWREINTERPRET_CAST },
	{ "return", KWRETURN }, { "short", KWSHORT },
	{ "signed", KWSIGNED }, { "sizeof", KWSIZEOF },  
	{ "static", KWSTATIC }, { "static_cast", KWSTATIC_CAST }, 
	{ "struct", KWSTRUCT }, { "switch", KWSWITCH }, 
	{ "template", KWTEMPLATE }, { "this", KWTHIS },
	{ "throw", KWTHROW }, { "true", KWTRUE }, 
	{ "try", KWTRY }, { "typedef", KWTYPEDEF },
	{ "typeid", KWTYPEID }, { "typename", KWTYPENAME },
	{ "union", KWUNION },  { "unsigned", KWUNSIGNED },
	{ "using", KWUSING },  { "virtual", KWVIRTUAL },
	{ "void", KWVOID },  { "volatile", KWVOLATILE },
	{ "wchar_t", KWWCHAR_T }, { "while", KWWHILE } 
};


// ����� � ���������� �������
static string lexbuf;


// ������� ���������� ��� ��������� ����� ��� -1
// � ������ ���� ������ ��������� ����� ���
inline int LookupKeywordCode( const char *keyname, keywords *kmas, int szmas )
{
	int r;
	for( int i = 0; i< szmas / sizeof(keywords); i++ )
	{
		r = strcmp( kmas[i].name, keyname );
		if( !r )
			return kmas[i].code;
		else if( r > 0 )
			break;
	}

	return -1;
}


// ���� �������� ����� kpp
static int LookupKppKeywords( const char *keyname )
{
	return LookupKeywordCode( keyname, kpp_words, sizeof( kpp_words ) );
}


// ���� �������� ����� ����� �
static int LookupCKeywords( const char *keyname )
{
	return LookupKeywordCode( keyname, c_words, sizeof( c_words ) );
}


// ���� �������� ����� ����� �++
static int LookupCPPKeywords( const char *keyname )
{
	return LookupKeywordCode( keyname, cpp_words, sizeof( cpp_words ) );
}


// ���������� ��� ��������� ����� �� ����
const char *GetKeywordName( int code )
{
	return cpp_words[code - KWASM].name;
}


// ���������� ������� � ����� ������
static int IgnoreNewlinesAndSpaces( BaseRead &ob )
{
	register int c;

	while( (ob >> c) != EOF )

		// ��������: ��������� ��������� ��� 1 ������ (� �� 4 �������)
		if( c == ' ' || c == '\t' )		
			continue;

		else if( c == '\n' )		
			((CppFileRead &)(ob)).NewLine();		

		else
			break;

	ob << c; // ���������� ���� ������ � �����
	return c;
}


// ���������, ���� 'nam' - �������������� ��� �����
// ��� and, or, ... �� ������� ��������� �������� - ���
// ��������� �������, ����� 0
inline static int IsAlternativeName( const char *n ) 
{
	struct TempAgr
	{
		const char *name;
		int tok;
	} alt[] = {
		"and",	  LOGIC_AND,
		"and_eq", AND_ASSIGN, 
		"bitand", '&',
		"bitor",  '|',
		"compl",  '~',
		"not",	  '!',
		"not_eq", NOT_EQUAL,
		"or",	  LOGIC_OR,
		"or_eq",  OR_ASSIGN,
		"xor",	  '^',
		"xor_eq", XOR_ASSIGN
	};

	for( int i = 0; i<11; i++ )
		if( !strcmp( alt[i].name, n ) )
			return alt[i].tok;
	return 0;
}


// �������� ������� '�������������'
inline static int LexemName( BaseRead &ob )
{
	register int c;

	while( (ob >> c) != EOF )
		if( !IS_NAME(c) )
			break;
		else
			lexbuf += (char)c;

	ob << c;
	return NAME;
}


// �������� ������� '��������'
inline static int LexemOperator( BaseRead &ob )
{
	register int c;
	
	ob >> c;
	if( c == '-' )
	{
		ob >> c;
		
		if(c == '-') { lexbuf = "--"; return DECREMENT; }
		else if(c == '=') { lexbuf = "-="; return MINUS_ASSIGN; }
		else if(c == '>') 
		{
			ob >> c;
			if(c == '*') { lexbuf = "->*"; return ARROW_POINT; }
			else { ob << c; lexbuf = "->"; return ARROW; }
		}
		else { ob << c; lexbuf = '-'; return '-'; }
	}

	else if( c == '+' )
	{
		ob >> c;
		
		if(c == '+') { lexbuf = "++"; return INCREMENT; }
		else if(c == '=') { lexbuf = "+="; return PLUS_ASSIGN; }
		else { ob << c; lexbuf = '+'; return '+'; }
	}

	else if( c == '*' )
	{
		ob >> c;
		
		if(c == '=') { lexbuf = "*="; return MUL_ASSIGN; }
		else { ob << c; lexbuf = '*'; return '*'; }
	}

	else if( c == '/' )
	{
		ob >> c;
		
		if(c == '=') { lexbuf = "/="; return DIV_ASSIGN; }
		else { ob << c; lexbuf = '/'; return '/'; }
	}

	else if( c == '%' )
	{
		ob >> c;
		
		if(c == '=') { lexbuf = "%="; return PERCENT_ASSIGN; }			
		else if(c == '>') { lexbuf = "%>"; return '}'; }	
		
		// ������������� '%>'  - '{' , '%:' - #, '%:%:' - ##
		else if(c == ':') 
		{ 
			ob >> c;
			if( c == '%' )
			{
				ob >> c;
				if( c != ':' )
					theApp.Error("�������� ������ ':' � ������� '%%:%%:'"),
					ob << c;
				lexbuf = "%:%:";
				return DOUBLE_SHARP;
			}

			else
			{
				ob << c;
				lexbuf = "%:";
				return '#';
			}
		}

		else { ob << c; lexbuf = '%'; return '%'; }
	}

	else if( c == '<' )
	{
		ob >> c;
		
		if(c == '=') { lexbuf = "<="; return LESS_EQU; }
		else if(c == '<') 
		{
			ob >> c;
			if(c == '=') { lexbuf = "<<="; return LEFT_SHIFT_ASSIGN; }
			else { ob << c; lexbuf = "<<"; return LEFT_SHIFT; }
		}

		// ������������� '<%'  - '{', '<:' - '['
		else if(c == '%') { lexbuf = "<%"; return '{'; }
		else if(c == ':') { lexbuf = "<:"; return '['; }
		else { ob << c; lexbuf = '<'; return '<'; }
	}
	
	else if( c == '>' )
	{
		ob >> c;
		
		if(c == '=') { lexbuf = ">="; return GREATER_EQU; }
		else if(c == '>') 
		{
			ob >> c;
			if(c == '=') { lexbuf = ">>="; return RIGHT_SHIFT_ASSIGN; }
			else { ob << c; lexbuf = ">>"; return RIGHT_SHIFT; }
		}
		
		else { ob << c; lexbuf = '>'; return '>'; }
	}

	else if( c == '=' )
	{
		ob >> c;
		if( c == '=' ) { lexbuf = "=="; return EQUAL; }
		else { ob << c; lexbuf = '='; return '='; }
	}

	else if( c == '!' )
	{
		ob >> c;
		if( c == '=' ) { lexbuf = "!="; return NOT_EQUAL; }
		else { ob << c; lexbuf = '!'; return '!'; }
	}

	else if( c == '^' )
	{
		ob >> c;
		if( c == '=' ) { lexbuf = "^="; return XOR_ASSIGN; }
		else { ob << c; lexbuf = '^'; return '^'; }
	}

	else if( c == '&' )
	{
		ob >> c;
		if( c == '=' ) { lexbuf = "&="; return AND_ASSIGN; }
		else if( c == '&' ) { lexbuf = "&&"; return LOGIC_AND; }
		else { ob << c; lexbuf = '&'; return '&'; }
	}

	else if( c == '|' )
	{
		ob >> c;
		if( c == '=' ) { lexbuf = "|="; return OR_ASSIGN; }
		else if( c == '|' ) { lexbuf = "||"; return LOGIC_OR; }
		else { ob << c; lexbuf = '|'; return '|'; }
	}
    
	else if( c == ':' )
	{
		ob >> c;
		if( c == ':' ) { lexbuf = "::"; return COLON_COLON; }
		else if(c == '>') { lexbuf = ":>"; return ']'; }	// ':>' - ']'
		else { ob << c; lexbuf = ':'; return ':'; }
	}

	else if( c == '.' )
	{
		ob >> c;
		if( c == '*' ) { lexbuf = ".*"; return DOT_POINT; }
		else if( c == '.' ) 
		{
			ob >> c;
			if(c == '.') { lexbuf = "..."; return ELLIPSES; }
			else 
			{
				ob << c;
				theApp.Error( "��������� '.' � ��������� '...'");
				lexbuf = "...";
				return ELLIPSES;
			}
		}

		else { ob << c; lexbuf = '.'; return '.'; }
	}

	else if( c == '#' )
	{
		ob >> c;
		if( c == '#' )
		{
			lexbuf = "##";
			return DOUBLE_SHARP;
		}

		ob << c;
		lexbuf = "#";
		return '#';
	}

	else if( c == EOF )
	{
		lexbuf = "<����� �����>";
		return c;
	}

	else
	{
		lexbuf = c;
		return c;
	}
}


// �������� ������� '��������� �������'
inline static int LexemString( BaseRead &ob )
{
	register int c;
	bool wstr = lexbuf.at(0) == 'L';

	// ���� ����������� ���� ���� ����������� ��������� ������
	for( ;; )
	{	
		for(;;)
		{
			ob >> c;
			if( c == '\"' )			
				break;		// ������ �������

			else if( c == '\\' )
			{
				int pc;
						
				ob >> pc;
				if(pc == '\"' || pc == '\\')
				{
					lexbuf += '\\'; lexbuf += (char)pc;
					continue;
				}

				else
					ob << pc;
			}

			else if( c == '\n' || c == EOF )
			{
				ob << c;

				theApp.Error( "�� ������� `\"' � ����� ������" );
				lexbuf += '\"';
				return STRING;
			}		

			lexbuf += c;
		}

		// ��������� � ��������� �������, �������� ��� ����� ����� ������,
		// ����� �������� ����� ������������
		c = IgnoreNewlinesAndSpaces( ob );
		if( c == '\"' )		// ���������� �������� �����
		{
			if( wstr )
				theApp.Error("������������ ����� ������ �����");
			wstr = false;
			ob >> c;
		}

		// �������� ������ ���� wchar_t
		else if( c == 'L' )		
		{
			ob >> c, ob >> c;

			if( c == '\"' )
			{
				if( !wstr )
					theApp.Error("������������ ����� ������ �����");
				wstr = true;
			}

			else
			{
				ob << c, ob << (c = 'L');
				lexbuf += '\"';
				return STRING;
			}
		}

		else
		{
			lexbuf += '\"';
			return STRING;
		}
	}

	return STRING;	// kill warning
}


// ������� ���������� ��������� �������� ���� ������ ������������
int isdigit8( int c )
{
	return c >= '0' && c <= '7';
}


// ��������� ����� �� �������� ������, ����
// ������� isfunc ���������� true 
static void ReadDigit( BaseRead &ob, int (*isfunc)(int) )
{
	register int c;

	while( (ob >> c) != EOF )
		if( !isfunc(c) )
			break;
		else
			lexbuf += c;

	ob << c;
}


// ������� ������� � �����, ������� true ���� ������� suf
// ����� �����
static inline bool ReadDigitSuffix( BaseRead &ob, char suf )
{
	bool sl, ss;

	sl = ss = false;	// ��� �������� ����� ���� �����������

	// ������ ������� 'l', ������ suf
	for( register int c;; )
	{
		ob >> c;
		if( toupper(c) == 'L' )
		{
			if( sl ) 
				theApp.Warning("������� 'L' � ����� ��� �����");
			else
				sl = true, lexbuf += c;
		}

		// ��� 'U' ��� 'F'
		else if( toupper(c) == toupper(suf) )
		{
			if( ss )
				theApp.Warning("������� '%c' � ����� ��� �����", suf);
			else 			
				ss = true, lexbuf += c;				
		}

		else
		{
			ob << c;
			break;
		}
	}

	return ss;
}


// �������� ������� '�����'
inline static int LexemDigit( BaseRead &ob )
{
	register int c;
	int state = 0;

	ob >> c;

	for(;;)
	switch(state)
	{
	case 0:
		if(c == '0') state = 1;
		else if(c == '.') 
		{
			int p;
			
			ob >> p; ob << p;

			if( !isdigit(p) )	// ������ �������� �����
			{ ob << c;	return -1; }

			else
				state = 2;
		}

		// ���������� ����� 1-9
		else 
		{
			lexbuf += c;
			ReadDigit( ob, isdigit );
			
			ob >> c;
			if( c == '.' )
				state = 2;
			else if( c == 'e' || c == 'E' )
				state = 3;
			else
			{
				ob << c;
				return ReadDigitSuffix(ob, 'U') ? UINTEGER10 : INTEGER10;
			}
		}
		
		break;

	case 1:
		lexbuf += c;
		ob >> c;

		if( c == '.' ) state = 2;
		else if( c == 'e' || c == 'E' ) state = 3;
		else if( c == 'x' || c == 'X' ) 
		{
			lexbuf += c;
			ReadDigit( ob, isxdigit );

			if( toupper( *(lexbuf.end() - 1) ) == 'X' )
				theApp.Error("����������� 16-������ ������������������ ����� '%c'",c);
			return ReadDigitSuffix(ob, 'U') ? UINTEGER16 : INTEGER16;
		}

		else if( isdigit8(c) ) 
		{
			lexbuf += c;
			ReadDigit( ob, isdigit8 );
			return ReadDigitSuffix(ob, 'U') ? UINTEGER8 : INTEGER8;			
		}

		else 
		{
			ob << c;
			return ReadDigitSuffix(ob, 'U') ? UINTEGER10 : INTEGER10;
		}

		break;

	case 2:
		// ���� ������� ������ ����� �����
		lexbuf += c;
		ob >> c;
	
		if( c == 'e' || c == 'E' ) 
			state = 3;

		else if( isdigit(c) )
		{
			lexbuf += c;
			ReadDigit(ob, isdigit);

			ob >> c;
			if( c == 'e' || c == 'E' ) 
				state = 3;
			else 
			{
			read_suffix:
				ob << c;
				return ReadDigitSuffix(ob, 'F') ? LFLOAT : LDOUBLE;
			}
		}

		// ����� ���� ������� ����� � �������� ������ �����
		else
			goto read_suffix;

		break;

	case 3:
		// ���� ������� ����� E
		lexbuf += c;
		ob >> c;

		if( c == '+' || c == '-' )
			lexbuf += c, (ob >> c);

		if( !isdigit(c) )
		{
			ob << c;
			theApp.Error( "��������� �������� ����������" );
			return LDOUBLE;
		}

		else
		{	
			lexbuf += c;
			ReadDigit(ob, isdigit);	
			return ReadDigitSuffix(ob, 'F') ? LFLOAT : LDOUBLE;
		}
	}
}



// �������� ������� ���������� ���������
inline static int LexemCharacter( BaseRead &ob )
{
	register int c;
	
	// ������ ' ��� ������, ��������� �� ������� '
	// ���� �� ����� ������, ���������� �������� �� �����
	// ��������, ������������ �������� ������� ����������� �����
	
	ob >> c;
	if( c == '\'' )	// ������ ������
	{
		lexbuf += '\\',
		lexbuf += '0', lexbuf += '\'';	// ������������� ��������� \0
		theApp.Error( "������ ������" );
		return CHARACTER;
	}

	ob << c; 
	for( ;; )
	{
		ob >> c;
		if( c == '\'' )
		{
			lexbuf += c;
			return CHARACTER;
		}

		else if( c == '\\' )
		{
			int pc;
			
			ob >> pc;
			if(pc == '\'')
			{
				lexbuf += '\\'; lexbuf += '\'';
				continue;
			}

			else if(pc == '\\')
			{
				lexbuf += "\\\\";
				continue;
			}

			else
				ob << pc;
		}

		else if( c == '\n' || c == EOF )
		{
			ob << c;

			theApp.Error( "�� ������� `\'' � ����� ������" );
			lexbuf += '\'';
			return CHARACTER;
		}		

		lexbuf += c;
	}

	return CHARACTER;	// kill warning
}


// ������� �������� ��������� ������� ��
// ������ in
static int Lex( BaseRead &ob, Position &lxmPos )
{
	register int c;

	lexbuf = "";
	c = IgnoreNewlinesAndSpaces(ob);

	lxmPos = ((CppFileRead&)ob).GetPosition();		// ��������� ������� �������	

	if( IS_NAME_START(c) ) 
	{
		ob >> c;  // ��������� ���� ������ ��� ���
		lexbuf += c;
		
		// �������� ������� ����������� wide-string
		if( c == 'L' )	
		{
			int p; 

			ob >> p;
			if( p == '\'')
			{
				lexbuf += p;
				LexemCharacter(ob);
				return WCHARACTER;
			}

			else if( p == '\"' )
			{
				lexbuf += p;
				LexemString(ob);
				return WSTRING;
			}

			else
				ob << p;
		}

		LexemName(ob);

		// �������� �������������� ���, ����� ��� and, or...
		if( int a = IsAlternativeName( lexbuf.c_str() ) )
			return a;

		// ����� ������ ���,
		// �������� ����� ������������ �����
		return NAME;	
	}

	else if( isdigit(c) || c == '.' )
	{
		int r;
		if( (r = LexemDigit(ob)) == -1 )
			return LexemOperator(ob);	// ����� ��������� ����� (.*)
		else
			return r;
	}

	else if( c == '\"' )
	{
		lexbuf += c;
		ob >> c;
		return LexemString(ob);
	}

	else if( c == '\'' )
	{
		lexbuf += c;
		ob >> c;
		return LexemCharacter(ob);
	}

	else
		return LexemOperator(ob);
}

// �������� ������ ���������� �� ����������� �����, ������������ ��� �������
ostream &operator<<( ostream &out, const LexemContainer &lc )
{
	out << "CALL   \"operator<<( ostream &out, const LexemContainer &lc )\"\n ";
	for( list<Lexem>::const_iterator p = lc.begin();
		 p !=  lc.end(); p++ )
		out << (*p).GetBuf() << ' ';
	out << endl << endl;
	return out;
}


// �������� ��������� �������
const Lexem &LexicalAnalyzer::NextLexem()
{
	// ��������� ������� �������, ��� ���������� � ��������� ����.
	prevLxm = lastLxm;

	// ���� �������� ������� ������, ������ ��
	if( backLxm.GetCode() != 0 )
	{
		lastLxm = backLxm;
		backLxm = Lexem();		// ������� �������� �������
		return lastLxm ;
	}

	// ���� ����� ���������, ������ �� ����
	if( lexemContainer != NULL )
	{
		lastLxm = lexemContainer->front();
		lexemContainer->pop_front();
		if( lexemContainer->empty() )
			lexemContainer = NULL;

		return lastLxm;
	}

	// ����� ����� ���������� �� �����	
	lastLxm.code = Lex(*inStream, lastLxm.pos);
	lastLxm.buf = lexbuf.c_str();
	
	// ���� ��� ���, �� �������� ��� ������������� ��������
	if( lastLxm.code == NAME )
	{
		int nc = LookupCPPKeywords(lastLxm.buf.c_str());
		if( nc != -1 )
			lastLxm.code = nc;
	}

	return lastLxm;
}
