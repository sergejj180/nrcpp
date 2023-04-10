// ���������� ������� ������� �������� - SymbolTable.cpp

#pragma warning(disable: 4786)
#include <nrc.h>
using namespace nrc;

#include "Limits.h"
#include "Application.h"
#include "Object.h"
#include "Scope.h"



// �������
bool IdentifierListFunctor::operator() ( const IdentifierList &il ) const
{
	return il.front()->GetName() == name;
}


// �������� �������
HashTab::HashTab( unsigned htsz )
	: size(htsz)
{
	table = new ListOfIdentifierList[size];
}


// ���������� ���������� �������
HashTab::~HashTab()
{
	delete [] table;
}


// ������� �����������, ���������� ������ �� �����
unsigned HashTab::Hash( const CharString &key ) const
{
	register const char *p;
	register unsigned int h = 0, g;
		
	for(p = key.c_str(); *p != '\0'; p++)
		if(g = (h = (h << 4) + *p) & 0xF0000000)
			h ^= g >> 24 ^ g;
		
	return h % size;
}


// ����� �������
IdentifierList *HashTab::Find( const CharString &key ) const
{
	ListOfIdentifierList &lst = table[Hash(key)];
	ListOfIdentifierList ::iterator p = 
		find_if( lst.begin(), lst.end(), IdentifierListFunctor(key) );

	return p == lst.end() ? NULL : &*p;
}


// �������� ������� � �������
unsigned HashTab::Insert( const Identifier *id )
{
	ListOfIdentifierList &lst = table[ Hash(id->GetName()) ];
	ListOfIdentifierList::iterator p = find_if(lst.begin(), lst.end(), 
		IdentifierListFunctor(id->GetName()) );

	// ���� ����� ������� �� ������, ������� ����� ������ �������������, 
	// ��������� ��� � ������ �������
	if( p == lst.end() )
	{
		IdentifierList il;
		il.push_back(id);
		lst.push_back( il );		
		return 1;
	}
	
	// ����� ��������� ������������� � ��������� ������
	else	
	{
		(*p).push_back(id);	
		return (*p).size();
	}	
}


// �������� using-������� ���������, ������� ����� ��������������
// ������������� ��� ������
void GeneralSymbolTable::AddUsingNamespace( NameSpace *ns ) 
{
	if( usingList.HasSymbolTable(ns) < 0 )
		usingList.AddSymbolTable(ns);
}


// �������� using-������� ���������, ������� ����� ��������������
// ������������� ��� ������
void FunctionSymbolTable::AddUsingNamespace( NameSpace *ns ) 
{
	if( usingList.HasSymbolTable(ns) < 0 )
		usingList.AddSymbolTable(ns);
}


// ������� ������, ���������� ���������� ��� ������ � ������ ������������
// �������� ���������. ������ ����������������� � NameSpace
bool GeneralSymbolTable::FindSymbolWithUsing( const CharString &name,
					SymbolTableList &tested, IdentifierList &out ) const
{	
	if( IdentifierList *il = hashTab->Find(name) )
		out.insert( out.end(), il->begin(), il->end() );
	
	// ����� ��� ������ ������������ ��, ��������� �������� ������
	for( int i = 0; i<usingList.GetSymbolTableCount(); i++ )
	{
		// ���� ��� ������� ��������� ��� ����������� � ������,
		// ������������� ������������, ����� ��� �� ���������� ���� �����
		if( tested.HasSymbolTable( usingList[i] ) >= 0 )
			continue;

		tested.AddSymbolTable( usingList[i] );
		const GeneralSymbolTable *ns = dynamic_cast<const NameSpace *>(usingList[i]);
		INTERNAL_IF( ns == NULL );

		// ��� ��� �������, ������������ � out
		ns->FindSymbolWithUsing(name, tested, out);			
	}

	// ���� ������ �� ������
	return !out.empty();
}


// ����� ������� � ��������� ��� ���������� ������� ���������,
// ���� � ����� ������� ��������� � ����� � ���������� ����������
// ���������� ����� � ��������� �������� ��������� (using). ��� ����
// ������� ��������������, ����� ������� �� ������������ �.�. 2 �������
// ����� ���� ���������� �� ��������� ���� � �����. ���� �� ���� ��
// ���� �� ������� - ������������ false
bool GeneralSymbolTable::FindSymbol( const nrc::CharString &name, 
			IdentifierList &out ) const 
{
	SymbolTableList tested;	
	return FindSymbolWithUsing(name, tested, out);
}


// ���������� ����� ��� ����� using-��������, ������ ���������� (��� ���������) ��
bool GeneralSymbolTable::FindInScope( const nrc::CharString &name, IdentifierList &out ) const 
{		
	if( IdentifierList *il = hashTab->Find(name) )
		out.insert( out.end(), il->begin(), il->end() );

	return !out.empty();	
}


// ������� ������� �������
bool GeneralSymbolTable::InsertSymbol( Identifier *id ) 
{
	unsigned icnt = hashTab->Insert(id);

	// ���������� � �-����� �����, ��� ���������� ��������� ����,
	// ���� ������ � �������������� ���� �-���
	if( icnt > 1 && !id->GetC_Name().empty() )
		const_cast<string &>(id->GetC_Name()) += CharString((int)icnt).c_str();
	return true;
}


// ������� ������, ���������� ���������� ��� ������ � ������ ������������
// �������� ���������. ������ ����������������� � NameSpace
bool FunctionSymbolTable::FindSymbolWithUsing( const CharString &name,
							SymbolTableList &tested, IdentifierList &out ) const
{
	FindInScope(name, out);
	
	// ����� ��� ������ ������������ ��, ��������� �������� ������
	for( int i = 0; i<usingList.GetSymbolTableCount(); i++ )
	{
		// ���� ��� ������� ��������� ��� ����������� � ������,
		// ������������� ������������, ����� ��� �� ���������� ���� �����
		if( tested.HasSymbolTable( usingList[i] ) >= 0 )
			continue;

		tested.AddSymbolTable( usingList[i] );
		const GeneralSymbolTable *ns = dynamic_cast<const NameSpace *>(usingList[i]);
		INTERNAL_IF( ns == NULL );
		ns->FindSymbolWithUsing(name, tested, out);		
	}

	// ���� ���-�� ������� - true
	return !out.empty();
}


// ����� ������� � �������������� ������� ���������, ����� � ������ ���������� �������
// ���� � ����� ������� ��������� � ����� � ���������� ����������
// ���������� ����� � ��������� �������� ��������� (using). 
bool FunctionSymbolTable::FindSymbol( const nrc::CharString &name, IdentifierList &out ) const
{
	SymbolTableList tested;	
	return FindSymbolWithUsing(name, tested, out);
}


// ���������� ����� ��� ����� using-��������, ������ � �������������� �������
// ��������� � � ����� ����������
bool FunctionSymbolTable::FindInScope( const nrc::CharString &name, IdentifierList &out ) const
{
	ListOfIdentifierList::const_iterator p = 
		find_if( localIdList.begin(), localIdList.end(), IdentifierListFunctor(name) );
	if( p != localIdList.end() )
		out.insert( out.end(), (*p).begin(), (*p).end() );
	
	// ���� ����� � � ����������
	const FunctionParametrList &fpl = pFunction.GetFunctionPrototype().GetParametrList();
	int pix = fpl.HasParametr(name);

	// ���� ������ ��������, ��������� ��� � �������������� ������
	if( pix >= 0 )	
		out.push_back( &*fpl[pix] );
	return !out.empty();
}


// ������� ������� �������
bool FunctionSymbolTable::InsertSymbol( Identifier *id )
{
	ListOfIdentifierList::iterator p = 
		find_if( localIdList.begin(), localIdList.end(), IdentifierListFunctor(id->GetName()) );

	// ���� ������ � ����� ������ ������, ��������� � ����
	if( p != localIdList.end() )
		(*p).push_back(id);

	// ����� ������� ����� ������
	else
	{
		IdentifierList il;
		il.push_back(id);
		localIdList.push_back(il);
	}
	
	return true;	
}


// ������� ��� �������
void FunctionSymbolTable::ClearTable()
{
}


// ����� �������	
bool LocalSymbolTable::FindSymbol( const nrc::CharString &name, 
					IdentifierList &out ) const 
{
	if( !table )
		return false;

	ListOfIdentifierList::const_iterator p = 
			find_if( table->begin(), table->end(), IdentifierListFunctor(name) );
	if( p != table->end() )
	{
		out.insert( out.end(), (*p).begin(), (*p).end() );
		return true;
	}

	return false;
}


// ������� ������� �������
bool LocalSymbolTable::InsertSymbol( Identifier *id ) 
{
	// ���� ������� �� �������, ������� ��
	if( table == NULL )
		table = new ListOfIdentifierList;
	ListOfIdentifierList::iterator p = 
		find_if( table->begin(), table->end(), IdentifierListFunctor(id->GetName()) );

	// ���� ������ � ����� ������ ������, ��������� � ����
	if( p != table->end() )
		(*p).push_back(id);

	// ����� ������� ����� ������
	else
	{
		IdentifierList il;
		il.push_back(id);
		table->push_back(il);
	}

	return true;
}


// �������� ����� �� ���� �������� ���������, 
// ����� ���������� � �����, �.�. � ������� �� � ������������
// ������ ������������ - ������ ��������������� �������
// �������� ���. ���� �����. ��� - ���������� ������ ������
bool Scope::DeepSearch( const CharString &name, IdentifierList &out ) const
{	
	// �������� �� ���� �������� ���������
	list<SymbolTable *>::const_iterator i = symbolTableStack.end();	
	for( i--; ; i-- )	
	{
		if( (*i)->FindSymbol(name, out) )
			return true;
	
		if( i == symbolTableStack.begin() )
			break;
	}

	return false;
}


// �������� ��������� ���������� ������� ���������
const SymbolTable &Scope::GetGlobalSymbolTable() const
{
	// �������� �� ���� �������� ���������
	list<SymbolTable *>::const_iterator i = symbolTableStack.end();	
	for( i--; ; i-- )	
	{
		if( (*i)->IsGlobalSymbolTable() || (*i)->IsNamespaceSymbolTable() )
			return **i;

		if( i == symbolTableStack.begin() )
			break;
	}

	INTERNAL( "'Scope::GetGlobalSymbolTable' �� ��������� ���������� ��" );
	return *(SymbolTable *)0;
}


// �������� ��������� �������������� ������� ���������. 
// ������� ������� ��������� ����������� ������ ���� ���������
const SymbolTable &Scope::GetFunctionalSymbolTable() const
{
	// ���� ������� ������� ��������� ��������������, ������� ��
	if( GetCurrentSymbolTable()->IsFunctionSymbolTable() )
		return *GetCurrentSymbolTable();

	INTERNAL_IF( !GetCurrentSymbolTable()->IsLocalSymbolTable() );
	list<SymbolTable *>::const_iterator i = symbolTableStack.end();	
	for( i--; ; i-- )	
		if( (*i)->IsFunctionSymbolTable() )
			return **i;

	INTERNAL( "'Scope::GetFunctionalSymbolTable' �� ��������� �������������� ��" );
	return *(SymbolTable *)0;
}

