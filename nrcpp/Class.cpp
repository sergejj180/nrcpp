// ���������� ������� ������� �������������� �������� �����, ������� ���, 
// ������, ��������� �������� - BaseType.cpp

#pragma warning(disable: 4786)
#include <nrc.h>

using namespace nrc;

#include "Limits.h"
#include "Application.h"
#include "LexicalAnalyzer.h"
#include "Object.h"
#include "Scope.h"
#include "Class.h"


// ������� �����������
namespace TranslatorUtils
{
	// ���������� ��� ������� ��������� ��� ���������� ��������. ���������
	// ����. ������� ���������: ����������, �����������, ���������, �������������� (���������).
	// ��������� � Translator.h
	const string &GenerateScopeName( const SymbolTable &scope );

	// ������������� ��� ��� ����������� ��������������
	string GenerateUnnamed( );
}


// ���������� ������ ����� �� ���������, ���� ����� ��� ������� -1
int ClassFriendList::FindClassFriend( const Identifier *p ) const
{
	for( int i = 0; i<friendList.size(); i++ )
		if( friendList[i].IsClass() ) 
		{
			if( &friendList[i].GetClass() == p )
				return i;
		}
		else
			if( &friendList[i].GetFunction() == p )
				return i;

	return -1;
}


// �������� ��� ����� � ������ name � ���� ������ �����,
// ���� ������ ����� ���, ������� ������ ������
IdentifierList *ClassMemberList::FindMember( const CharString &name ) const
{
	ListOfIdentifierList::iterator p = 
		find_if( memberList.begin(), memberList.end(), IdentifierListFunctor(name) );
	if( p != memberList.end() )
		return &*p;
	return NULL;
}


// �������� ���� � ������. ��� ���� ���� ������ �������������
// ������ �� ������, �� ���������, � ���� ������, ���� �����������
// � ����� ����� ������
void ClassMemberList::AddClassMember( PClassMember cm ) 
{	
	Identifier *id;
	// ���� ������ ����������������� � ��������������
	INTERNAL_IF( (id = dynamic_cast<Identifier *>(&*cm)) == NULL );

	if( IdentifierList *il = FindMember( id->GetName() ) )
	{
		il->push_back(id);

		// ���������� � �-����� �����, ��� ���������� ��������� ����,
		// ���� ������ � �������������� ���� �-���
		if( !id->GetC_Name().empty() )
			const_cast<string &>(id->GetC_Name()) += CharString((int)il->size()).c_str();
	}

	else
	{
		// ����� ������� ����� ������ ���������������
		IdentifierList newil;
		newil.push_back(id);
		memberList.push_back(newil);
	}
	
	// ��������� ��������� �� ���� � ������ ��� ��������
	// ������� ���������� ������ ��� ����������
	order.push_back(cm);
}


// �������� ������ ������ ������������� ������������� ������
void ClassMemberList::ClearMemberList()
{
	// ������� ����������� ������ ���������� ��� ������
	order.clear();
	memberList.clear();
}


// ����������� � �������� �������������� ����������, �.�. ������
// ������ ��������� ��� ��� ����������, � �� �����������
ClassType::ClassType( const nrc::CharString &name, SymbolTable *entry, BT bt, AS as ) :
	Identifier( (name[0] == '<' ? TranslatorUtils::GenerateUnnamed().c_str() : name), entry), 
	BaseType(bt), accessSpecifier(as), uncomplete(true), polymorphic(false), madeVmTable(false),
	abstractMethodCount(0), virtualMethodCount(0), castOperatorList(NULL),
	destructor(NULL), virtualFunctionList(NULL)
{
	c_name = ( "__" + TranslatorUtils::GenerateScopeName(*entry) + GetName().c_str() );
}


// �������� ������� �����
void ClassType::AddBaseClass( const PBaseClassCharacteristic &bcc ) 
{
	baseClassList.AddBaseClassCharacteristic(bcc);
	virtualMethodCount += bcc->GetPointerToClass().virtualMethodCount;

	// ������� �������� ��������� ����������
	if( bcc->GetPointerToClass().GetCastOperatorList() != NULL )
	{
		const ClassType &cls = bcc->GetPointerToClass();
		if( castOperatorList == NULL )
			castOperatorList = new CastOperatorList;
		castOperatorList->insert(castOperatorList->end(), 
			cls.GetCastOperatorList()->begin(), cls.GetCastOperatorList()->end());
	}
}


// ����� ����� � ������, � ����� � ������� �������, � ������ ��������� ������,
// ���������� ������ ������, � ��������� ������, ������ ����. 
// friend-���������� ������� �� �������� � ������� � ������ �� ����������
bool ClassType::FindSymbol( const nrc::CharString &name, IdentifierList &out ) const
{	
	// ���� ������� ��� � ���� ������, ������ ��� ����������� ���
	// ����� �� ������� ������� � ���������� ����� �� ����� ������
	if( IdentifierList *il = (memberList).FindMember(name) )
	{
		out.insert( out.end(), il->begin(), il->end() );
		return true;
	}
	
	// ����� ������ �� �������, ���������� ����� �� ������� �������
	// � ����������� ������ � �������������� ������
	StringList tempList;
	for( int i = 0; i<baseClassList.GetBaseClassCount(); i++ )	
		baseClassList[i]->GetPointerToClass().FindSymbol(name, out);
	
	return !out.empty();	
}


// ����� ������ ������ ������, ��� ����� ������� �������
bool ClassType::FindInScope( const nrc::CharString &name, IdentifierList &out ) const
{
	if( IdentifierList *il = (memberList).FindMember(name) )
	{
		out.insert( out.end(), il->begin(), il->end() );
		return true;
	}
	else
		return false;	
}


// ������� ����� � �������
bool ClassType::InsertSymbol( Identifier *id )
{
	ClassMember *cm = dynamic_cast<ClassMember *>(id);
	INTERNAL_IF( cm == NULL );
	memberList.AddClassMember( cm );
	
	// ���� ���� �������� �������, ��������, �������� ��� �����
	// ��������� � ��������� ���������
	if( Method *meth = dynamic_cast<Method *>(&*cm) )
	{		
		if( meth->IsConstructor() )
			constructorList.push_back(static_cast<ConstructorMethod *>(&*cm));
		else if( meth->IsOverloadOperator() &&
			static_cast<const ClassOverloadOperator*>(meth)->IsCastOperator() )
		{
			if( castOperatorList == NULL )
				castOperatorList = new CastOperatorList;
			castOperatorList->push_back(static_cast<ClassCastOverloadOperator *>(&*cm));
		}

		else if( meth->IsDestructor() )
		{
			INTERNAL_IF( destructor != NULL );
			destructor = meth;
		}
	}

	return true;
}


// ������� ��� �������
void ClassType::ClearTable()
{
	memberList.ClearMemberList();
}
