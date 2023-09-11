// ���������� ������� ��������


#include "hash.h"
#define MACROTAB_SIZE	531


// ��������� ���������� ��������
struct Param { 
	string name, val; 
	bool operator==( const Param &ob ) { return ob.name == name; }
};


// ��������� ��������
struct Macro
{
	string name, val;
	enum { MACROS, FUNCTION } type;
	list<Param> params;
	bool pred;	// ���� ������ ������������� - true
	
	Macro(string n, string v, bool p = false) { 
		name = n, val = v, type = MACROS;
		pred = p;
	}

	Macro(string n, string v, list<Param> p) { 
		name = n, val = v, params = p, type = FUNCTION;
		pred = false;
	}

	bool operator==( const Macro &ob ) { return ob.name == name; }
};


// ������� ��������
class MacroTable : public HashTab<Macro, MACROTAB_SIZE>
{
	// ���������� �������� �� ������
	bool _Find( char *name, list<Macro>::iterator &i ) {
		list<Macro> &p = HashFunc( name );

		return (i = find(p.begin(), p.end(), 
			Macro(string(name), string(""))) ) == p.end() ? false : true;
	}

public:
	// ���������� ��������� �� ������ � ������, ��� NULL
	Macro *Find( char *name );


	// ��������� ������� � �������
	void Insert( Macro ob );


	// ������� ������� �� �������
	void Remove( char *name );
};


// ������� ��� ������ ���������
class FuncParam 
{
	string &str;
public:
	FuncParam( string &s ) : str(s) { }
	bool operator() ( Param &p ) { return p.name == str; }
};


extern MacroTable mtab;


// �������� �� ������ s � ��������� ����������������,
// dodef ��������� �� ������������� ��������� ��������� defined
// � ���������� #if/#elif
string Substitution( string buf, bool dodef = false );
