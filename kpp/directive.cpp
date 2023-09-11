// ����������� ��������� ������������� - directive.cpp

#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cstring>
#include <stack>
#include <list>
#include <algorithm>
#include <string>

using namespace std;
#include "kpp.h"
#include "cpplex.h"
#include "macro.h"
#include "limits.h"


// ���� ��������� �� ������������� ������ ���������� �� �����
// ��������� ��� ���������� �������� ��������� ���������� (#if, #ifdef, ...)
bool PutOut = true;


// ���� � ������� �������� �������� �����. ��������� �� 
// �������� �������� ���������� (1 - ������������� ���������,
// 0 - ������������� (���������� �����), -1 - ���� �������� ���������,
// �� ��������� �� ���������, ��� ��� ������� ����� PutOut=false
stack<int> IfResults;


// ���������� ��������� ������������ ��������� � ������ s
int CnstExpr( string &s );


// ����� �������� ���� � ����������� � ������������� �������
list<string> IncludeDirs;


// ����� ���� - ��������� �� �����, ��� �����
// ������������ � include
string IncName;


// ������� #line
void PutLine( FILE *out );


// ��������� ��������� � ���������� #if/#elif
static bool inline EvalExpression( BaseRead &buf )
{
	string s;
	int r;

	ReadString( buf, s );

	try
	{
		r = CnstExpr( Substitution(s, true) );
	}

	catch(KPP_EXCEPTION exc)
	{
		switch(exc)
		{
		case EXP_EMPTY:
			Fatal("#if/#elif: ����������� ����������� ���������");

		case REST_SYMBOLS:
			Fatal("#if/#elif: �������������� ������");
		}
	}

	catch( const char *msg )
	{
		Fatal("#if/#elif: %s", msg);					
	}
	
	return r != 0;
}


// ���������� ������� ����� ��� �����������
// sys - true, ���� ���������� ��������� �����������
static bool inline TryInclude( string &finc, bool sys ) 
{
	string temp;
	FILE *input;

	if( sys )
	{
		for( list<string>::iterator p = IncludeDirs.begin();
			 p != IncludeDirs.end(); p++ )
		{
			temp = (*p) + finc;
			input = fopen( temp.c_str(), "r" );
			if( input )
			{
				fclose(input);
				IncName = temp;
				finc = temp;
				return true;
			}
		}
	}

	if( (input = fopen( finc.c_str(), "r" )) != NULL )
	{
		fclose(input);
		IncName = finc;
		return true;
	}

	return false;
}


// ������������� ������ include, ������ ����������� ��������, ����
// ����������, ���������� ������� ������
static string inline ViewIncludeString( BaseRead &buf )
{
	string s;

	ReadString(buf, s);
	if( s[0] != '<' )
		s = Substitution(s, false);
	return s;
}


// ���������� true ���� ��� ��������� �����-������� � ����������� �������
static bool inline CheckParams( list<Param> &p, const char *fname )
{
	bool rval = true;

	for( list<Param>::iterator i = p.begin(); i != p.end(); i++)
	{
		list<Param>::iterator j = i;
		j++;
		while( j != p.end() )
		{
			if( (*i).name == (*j).name )
			{
				Error( "'%s': '%s' - ��������� ���������� � ����� ������", 
					fname, (*i).name.c_str() );
				rval = false;
			}

			j++;
		}
	}

	return rval;
}	


// ���������� true ���� � mac � ob ���������� ���������
static inline bool EqualParams( Macro &mac, Macro &ob )
{
	list<Param>	&pm = mac.params, &pob = ob.params;
	list<Param>::iterator im, iob; 

	im = pm.begin();
	iob = pob.begin();

	for( ;; )
	{
		if( (im == pm.end() && iob != pob.end()) ||
			(im != pm.end() && iob == pob.end()) )
			return false;

		if( (im == pm.end()) && (iob == pob.end()) )
			return true;

		if( (*im).name == (*iob).name )
			im++, iob++;

		else
			return false;
	}

	return true;
}


// �������� ������ � ������� � ���������
static void inline InsertWithCheck( Macro &ob )
{
	Macro *mac = mtab.Find( (char *)ob.name.c_str() );
	if( mac )
	{
		// ���� ������ �������������
		if( mac->pred == true )
			Error( "'%s': ������ �������������", mac->name.c_str() );
		else
		{
			// ���� ������� ������� ��������� �����. ����������
			if( (mac->type == Macro::FUNCTION) && (ob.type == Macro::FUNCTION) )
				if( !EqualParams( *mac, ob ) )
				{
					Warning( "'%s': ������ �������������", mac->name.c_str());
					mtab.Remove( (char *)mac->name.c_str() );
					mtab.Insert( ob );
					return;
				}


			// ����� ���������� �������� � ���
			if( (mac->type != ob.type) || (mac->val  != ob.val ) )
			{
				Warning( "'%s': ������ �������������", mac->name.c_str() );
				mtab.Remove( (char *)mac->name.c_str() );
				mtab.Insert( ob );
			}

			// ����� ������ �� ����������			
		}
	}

	else
	{
		// defined ����� ������ ��������������
		if( ob.name == "defined" )
			Error( "'defined': �������� ������ ������������ � '#define'");

		else
			mtab.Insert(ob);
	}
}


void do_define( BaseRead &buf )
{
	register int c = Lex( buf );

	if( c != NAME )
	{
		Error( "��������� ��� ������� ����� '#define'" );
		return;
	}

	string name = lexbuf;
	list<Param> params;
	int type = Macro::MACROS;

	buf >> c;
	// �����-�������, ��������� ���������
	if( c == '(' )
	{
		c = Lex(buf);
		
		if( c != ')' )
		for(;;)
		{
			Param prm;
			
			if( c != NAME )
			{
				Error( "'%s': ��������� �������� �����-�������", name.c_str() );
				return;
			}
		
			prm.name = lexbuf;
			params.push_back( prm );
			c = Lex(buf);

			if( c == ')' )
				break;

			else if( c == ',' )
				c = Lex(buf);

			else
			{
				Error( "'%s': ��������� `,' ��� `)'", name.c_str() );
				return;
			}
		}

		type = Macro::FUNCTION;

		// �������� ������������ ����� ������� ���������
		if( CheckParams( params, name.c_str() ) == false )
			return;
	}

	else
		buf << c;

	string val;
	ReadString( buf, val );		// ��������� �������� �������

	if( type == Macro::MACROS )
		InsertWithCheck( Macro( name, val ) );
		
	else
		InsertWithCheck( Macro( name, val, params ) );
}


void do_error( BaseRead &buf )
{
	string s;

	ReadString( buf, s );
	Error( s.c_str() );
}


void do_undef( BaseRead &buf )
{
	if( Lex( buf ) != NAME )
	{
		Error( "��������� ��� ������� ����� '#undef'" );
		return;
	}
	
	// remove ����� ��������� ���������������� �������
	mtab.Remove( (char *)lexbuf.c_str() );		
	if( Lex(buf) != EOF )
		Warning( "'#undef': ������ ������� � ������" );
}


void do_elif( BaseRead &buf )
{
	if( IfResults.empty() )
		Fatal( "'#elif' ��� '#if'" );

	int prev = IfResults.top();
	
	if( prev == -1 )
		return;	// ������ �� ������, ����� ������������

	if( prev == 0 )	// ���������� if/elif ��� false
	{		
		bool r = EvalExpression(buf);
	
		IfResults.pop();
		IfResults.push( (unsigned)r );
		PutOut = (r ? true : false);
	}


	// #elif ����� #else
	else if( prev == 2 )
	{
		Fatal( "������������ ����������� #if/#elif/#else: "
			   "'#elif' ���� ����� '#else'" );
	}

	else
	{
		IfResults.pop();
		IfResults.push( 0 ); 
		PutOut = false;
	}
}


void do_if( BaseRead &buf ) 
{
	if( IfResults.empty() )
	{
		bool r = EvalExpression(buf);
		IfResults.push( (unsigned)r );
		PutOut = r;
	}

	else
	{
		int prev = IfResults.top();

		if( prev == 0 || prev == -1 )
			IfResults.push( -1 );

		else
		{
			bool r = EvalExpression(buf);
			IfResults.push( (unsigned)r );
			PutOut = r;
		}
	}
}


void do_include( BaseRead &buf )
{
	int c;
	string finc, s;
	bool sys;
	BufferRead nbuf( ViewIncludeString(buf) );
	
	c = Lex(nbuf);
	
	// ���� �� ���� ��������� �����������
	if( c == '<' )
	{
		ReadString( nbuf, finc );
		if( finc == "" || (*(finc.end() - 1) != '>') )
			Fatal("'#include': �������� '>' � ����� ������");

		finc.erase( finc.end() - 1 );
		sys = true;		
	}

	else if( c == STRING )
	{
		finc = lexbuf;
		if( Lex(nbuf) != EOF )
			Warning( "'#include': ������ ������� � ������" );

		// ������� �������
		finc.erase( finc.begin() );	
		finc.erase( finc.end()-1 );

		sys = false;
	}

	else
		Fatal( "'#include': ��������� ��� �����" );

	if( finc == "" )
		Fatal("'#include': ������ ��� �����" );


	// ������� ���������� ���� �� ��������� ��������� ���������� 
	if( !TryInclude( finc, sys ) )
		Fatal("'#include': '%s' - ���� �� ������", finc.c_str() );
}


void do_else( BaseRead &buf )
{
	if( IfResults.empty() )
		Fatal( "#else ��� ���������������� #if" );

	if( Lex(buf) != EOF )
		Warning( "'#else': ������ ������� � ������" );


	int prev = IfResults.top();
	if( prev == -1 )
		;	// ������ �� ������, ����� ������������

	else if( prev == 0 )	// ���������� if/elif ��� false
	{
		IfResults.pop();

		// �������� � ���� 2, ��� ���� ��� ��� ��� else,
		// ����� �� ���� ������ else'��
		IfResults.push( 2 );	
		PutOut = true; 
	}

	// #else ��� ���
	else if( prev == 2 )
		Fatal( "������������ ����������� #if/#elif/#else: '#else' ���� ����� '#else'" );

	else
	{
		IfResults.pop();
		IfResults.push( 0 ); 
		PutOut = false;
	}
}


void do_ifdef( BaseRead &buf )
{
	register int c = Lex( buf );

	if( c != NAME )
	{
		Fatal( "'#ifdef': ��������� ��� �������"  );
		return;
	}

	bool r = mtab.Find( (char *)lexbuf.c_str()) != NULL;

	
	if( Lex(buf) != EOF )
		Warning( "'#ifdef': ������ ������� � ������" );

	if( IfResults.empty() )
	{
		IfResults.push( r );
		PutOut = r;
	}

	else
	{
		int prev = IfResults.top();

		if( prev == 0 || prev == -1 )
			IfResults.push( -1 );

		else
		{
			IfResults.push( r );
			PutOut = (r ? true : false);
		}
	}
}


void do_line( BaseRead &buf )
{
	extern string inname; // ��� �������� �����
	register int c;
	int temp;

	c = Lex( buf );
	if( IS_LITERAL(c) )
	{
		int r;
		if((r = CnstValue( (char *)lexbuf.c_str(), c )) != -1)
			temp = r;
	}
		
	else
	{
		Error("��������� ����� ����� ����� '#line'");
		return;
	}

	c = Lex( buf );
	if( c == STRING )
	{
		inname = lexbuf;

		// ������� ������ � ��������� ������ (�������)
		inname.erase( inname.begin() );	
		inname.erase( inname.end()-1 );
	}

	else if( c == EOF )
	{
		linecount = temp;
		return;
	}

	else
	{
		Error("��������� ��������� �������");
		return;
	}

	if( Lex(buf) != EOF )
		Warning( "'#line': ������ ������� � ������" );
	
	linecount = temp;
}


void do_endif( BaseRead &buf )
{
	if( IfResults.empty() )
		Fatal( "'#endif' ��� '#if'" );

	IfResults.pop();
	if( IfResults.empty() )
		PutOut = true;

	else
	{
		int r = IfResults.top();
		PutOut = (r > 0 ? true : false);
	}
}

void do_ifndef( BaseRead &buf )
{
	register int c = Lex( buf );

	if( c != NAME )
	{
		Fatal( "'#ifndef': ��������� ��� �������"  );
		return;
	}

	int r = mtab.Find( (char *)lexbuf.c_str() ) == NULL;

	
	if( Lex(buf) != EOF )
		Warning( "'#ifndef': ������ ������� � ������" );

	if( IfResults.empty() )
	{
		IfResults.push( r );
		PutOut = (r ? true : false);
	}

	else
	{
		int prev = IfResults.top();

		if( prev == 0 || prev == -1 )
			IfResults.push( -1 );

		else
		{
			IfResults.push( r );
			PutOut = (r ? true : false);
		}
	}
}


void do_pragma( BaseRead &buf )
{
	Warning( "'#pragma' ������������" );
}


// ������� ������� ������� �����
void SplitSpaces( string &s )
{
	if( s.empty() )
		return;

	register char *q, *p = (char *)s.c_str();
	
	q = p;
	p += s.length() - 1;

	while( *p == ' ' || *p == '\t' )
		p--;
	*(p+1) = 0;
	
	string t = q;
	s = t;
}


// ������� ����������� ������ s � ��������� �������
string &MakeStringLiteral( string &s )
{
	s = '\"' + s;

	int i = 1;
	while( i < s.length() )
	{
		if( s[i] == '\"' || s[i] == '\\' )
			s.insert( i, "\\" ), i++;
		i++;
	}

	s += '\"';
	return s;
}


// ���������� ����� ��������� ���������
// ���������� ��� ���������
int Directive( string s, FILE *out )
{
	// ������� �������, ������� ���������
	// ��������� �������������
	static void (*sfuncs[])( BaseRead & ) = {
		do_define, do_error, do_undef,
		do_elif, do_if, do_include,
		do_else, do_ifdef, do_line,
		do_endif, do_ifndef, do_pragma
	};

	BufferRead buf( s.erase(0, 1) );
	int c = Lex ( buf );
	

	// ������ ���������
	if( c == EOF )
		;

	// ��������� ��� ���������
	else if( c == NAME )
	{
		int r = LookupKppKeywords( lexbuf.c_str() );
		if( r == -1 )
			Error( "'%s' - ����������� ��������� �������������", lexbuf.c_str() );
		else
		{	 
			// ���� ����� ������������, �� ����� ��������� 
			// ������ �������� ���������
			if( !PutOut &&
				(r != KPP_ELIF)   && (r != KPP_IF) &&
				(r != KPP_ELSE)   && (r != KPP_IFDEF) &&
				(r != KPP_ENDIF)  && (r != KPP_ENDIF) &&
			    (r != KPP_IFNDEF) )
				return -1;

			(sfuncs[r - KPP_DEFINE])( buf );
			return r;				
		}
	}

	// ����� ������
	else
		Error("����������� ��� ��������� ����� '#'");
		
	return -1;

}
