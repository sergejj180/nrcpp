// ���������� ������� �������-������� - Checker.h

#pragma warning(disable: 4786)
#include <nrc.h>

using namespace nrc;

#include "Limits.h"
#include "Application.h"
#include "LexicalAnalyzer.h"
#include "Object.h"
#include "Scope.h"
#include "Class.h"
#include "Manager.h"
#include "Maker.h"
#include "Parser.h"
#include "Checker.h"
#include "Overload.h"


// ������������ ������� ��������
using namespace CheckerUtils;


// ���������, ����� �� ����� ���� ������� ��� ������� ������
bool CheckerUtils::BaseClassChecker(const ClassType &cls, const SymbolTableList &stl, 
									const Position &errPos, PCSTR cname )
{
	// 1. ����� ������ ���� ��������� �����������
	// 2. ����� �� ������ ���� ������������
	// 3. ����� ������ ���� ��������� � ������ ������� ���������
	if( cls.IsUncomplete() )
	{
		theApp.Error(errPos, 
			"'%s' - �� ��������� ����������� ����� ������������ � �������� ��������",
			cname);
		return false;
	}

	if( cls.GetBaseTypeCode() == BaseType::BT_UNION )
	{
		theApp.Error(errPos, 
			"'%s' - ����������� �� ����� ���� ������� �������",
			cname);
		return false;
	}

	return true;
}


// ��������� ����������� ����������� ������
bool CheckerUtils::ClassDefineChecker( const ClassType &cls, const SymbolTableList &stl, 
									   const Position &errPos )
{		
	// 1. ����� ������ ����������, ���� �� ��� ���������
	// 2. ����� ������ ����������, ���� ������� ������� ��������� �� ����������,
	//	  � ����� �������������� ������� ��������� ���������
	if( !cls.IsUncomplete() )
	{
		theApp.Error(errPos, "'%s' - ����� ��� ���������", cls.GetName().c_str());	
		return false;
	}

	if( !stl.IsEmpty() && 
		!(GetCurrentSymbolTable().IsGlobalSymbolTable() ||
		  GetCurrentSymbolTable().IsNamespaceSymbolTable()) )		 
	{ 
		theApp.Error(errPos, 
			"'%s' - ����� ������ ������������ � ���������� ������� ���������", 
			cls.GetName().c_str());	
		return false;		
	}

	return true;
}


// ���������, ���� ��� typedef, �������� �������, ������� ����� ����� 0
const ClassType *CheckerUtils::TypedefIsClass( const ::Object &obj )
{
	INTERNAL_IF( obj.GetStorageSpecifier() != ::Object::SS_TYPEDEF );

	// ���� ������ ����������� ����� �� ������, ������� ����� ���
	// �� ������, ������������ cv-�������������, 
	BaseType::BT bt = obj.GetBaseType().GetBaseTypeCode();
	if( !obj.GetDerivedTypeList().IsEmpty() ||
		(bt != BaseType::BT_CLASS && bt != BaseType::BT_STRUCT && bt != BaseType::BT_UNION) ||
		obj.IsConst() || obj.IsVolatile() )
		return NULL;

	return static_cast<const ClassType *>(&obj.GetBaseType());
}


// ��������� ���������� �����. ���� ��� �� �������� ������ ������, ��� �� �����������
// �� �����������
void CheckerUtils::CheckAccess( const QualifiedNameManager &qnm, const Identifier &id, 
				const Position &errPos, const SymbolTable &ct )
{
	if( !id.GetSymbolTableEntry().IsClassSymbolTable() )
		return;

	// ���� ������������� ���������� � ������ ���������, ������ ��� ����������
	// ��������� ������ ��� ��������
	const ClassMember *cm = NULL;
	if( const Identifier *ui = qnm.GetSynonymList().find_using_identifier(&id) )
		cm = dynamic_cast<const ClassMember *>(ui);
	else
		cm = dynamic_cast<const ClassMember *>(&id);

	INTERNAL_IF( cm == NULL );


	// ��������������� ��������� ��� ��������� �������������� ��������
	// � ������ ������
	struct ENoAccess
	{
		// ���������� ����������� ��� ������ ������
		CharString stName, memName, asName;

		// �� ��������� ���������� ������ �����
		ENoAccess( const SymbolTable &ct, const ClassType &mcls, const ClassMember &cm ) {
			stName = ManagerUtils::GetSymbolTableName(ct);			
			memName = dynamic_cast<const Identifier &>(cm).GetQualifiedName();
			asName = ManagerUtils::GetAccessSpecifierName(cm.GetAccessSpecifier());								
		}
	};


	// �������� ������� ������� ���������. ���� ��� �������� ���������,
	// ������ ��������� ��������� �� ��������������
	const SymbolTable *curST = &ct;
	if( curST->IsLocalSymbolTable() )	
		curST = &GetScopeSystem().GetFunctionalSymbolTable();	

	// � ����� ����� �������������� �������������� �������� ���� ENoAccess
	try {

	// ��� ��������� � ������� ���������� �������� �� ���������
	// ������� ������� ���������
	if( qnm.GetQualifierList().IsEmpty() )
	{
		// ���� ������� ������� ��������� ��������������, ��� �����������
		// ������ ���� �������� ������
		if( curST->IsFunctionSymbolTable() )
		{
			const Function &fn = static_cast<const FunctionSymbolTable *>(curST)->GetFunction();
			INTERNAL_IF( !fn.IsClassMember() );

			// �������� ����� � �������� ����������� �������-����,
			// ���������� ��������� � ����� ����� 'this'
			const ClassType &fnCls = 
				static_cast<const ClassType &>(fn.GetSymbolTableEntry());

			AccessControlChecker achk( *curST, fnCls, *cm );

			// ���� ���� ����������, ���������� �������� ������ ������
			if( !achk.IsAccessible() )
				throw ENoAccess( *curST, fnCls, *cm );
		}

		// ����� ������� ������� ��������� ������ ���� �������
		else if( curST->IsClassSymbolTable() )
		{
			// ��� ���������, �����. ������ � ���� ����������� ����� 'this',
			// ����������. �.�. ����� ������� �����
			const ClassType &memCls = static_cast<const ClassType &>(*curST);
			AccessControlChecker achk( *curST, memCls , *cm );

			// ���� ���� ����������, ���������� �������� ������ ������
			if( !achk.IsAccessible() )
				throw ENoAccess( *curST, memCls , *cm );
		}

		// ����� �������� ���������� � ����������� �������, � ���
		// �� ����� �������� ���������� � ����� ������ ��� ������������,
		// ������� ���������� ������
		else
			INTERNAL( "'CheckerUtils::CheckAccess' ������� ������� ��������� "
					  "����������� ��� �������� ����� ������" );
	}

	// ����� ���������, ���� ��� �����������������, ������ ��������� ���������
	// ��� ������������ � � �������� ���������� �������� ��������� �� ��������� �����
	else if( const ClassType *cls = CheckQualifiedAccess(qnm, errPos, *curST) )
	{
		// ��������� ������ � ����� ����� �����
		AccessControlChecker achk( *curST, *cls, *cm );

		// ���� ���� ����������, ���������� �������� ������ ������
		if( !achk.IsAccessible() )
			throw ENoAccess( *curST, *cls, *cm );
	}

	
	// ���� ������ �������, ������������� ���������� ��� ������ ������
	} catch( const ENoAccess &einfo ) {
		theApp.Error( errPos, "'%s' - %s ���� ���������� � '%s'",
			einfo.memName.c_str(), einfo.asName.c_str(), 
			einfo.stName.c_str() );
	}
}


// �������� ������� ��� ������������������ �����, ��������� �����������
// ������� ����� � ������������ � ���� ��������� ���� �������� �������,
// ������� ��� ��� �������� ������ � ������ � CheckAccess, ����� ������� 0.
// ������ ��������� 'ct' ������ ���� ��������� ������������� �� ���������
// � ��������������, ���� ���������. ������ �������������� � 'qnm' �� ������
// ���� ������.	
const ClassType *CheckerUtils::CheckQualifiedAccess( const QualifiedNameManager &qnm,
		const Position &errPos, const SymbolTable &ct )
{
	INTERNAL_IF( ct.IsLocalSymbolTable() );

	// ������ �������������� ����� ������ ���� ��������
	INTERNAL_IF( qnm.GetQualifierList().IsEmpty() );
	const SymbolTableList &qualList = qnm.GetQualifierList();

	for( int i = 0; i<qualList.GetSymbolTableCount(); i++ )
	{
		const SymbolTable &qst = qualList.GetSymbolTable(i);

		// ���� ������������ �������� ���������, ��������, ����
		// �� �������� �������, ������� ��� ����� 0
		if( i == qualList.GetSymbolTableCount()-1 )
		{		
			if( qst.IsClassSymbolTable() )
				return &static_cast<const ClassType &>(qst);
			return NULL;
		}


		// ���� ������� ��������� �������� �������, �������� ���������
		// � ��������� �� �� �����������
		if( qst.IsClassSymbolTable() )
		{
			const ClassMember *mem = 
				dynamic_cast<const ClassMember *>(&qualList.GetSymbolTable(i));

			INTERNAL_IF( mem == NULL );
			AccessControlChecker achk( ct, static_cast<const ClassType &>(qst), *mem);

			// ���� ���� �� �������� ���������, ������� ������
			if( !achk.IsAccessible() )
			{
				theApp.Error( errPos,
					"'%s' - %s ���� ���������� � '%s'",
					dynamic_cast<const Identifier *>(mem)->GetQualifiedName().c_str(),
					ManagerUtils::GetAccessSpecifierName(mem->GetAccessSpecifier()),
					ManagerUtils::GetSymbolTableName(ct).c_str() );

				return NULL;
			}
		}
	}

	return NULL;
}


// ��������� ������������ ������ ����������� �����
bool CheckerUtils::CheckDerivedTypeList( const TempObjectContainer &object )
{
	// ���������� ����������� �������� ����������� �����. 
	// �� ����� ���� ��������� �� ������, ������ �� ������, �������� ������, 
	// ������� �������, ������� ������������ ������, ��������� �� ����-������.
	// � ������� �� ����� ���� cv-�������������� � ���������� 
	// ������� ���������, ������ ���� �� ����� ������������ �������� typedef.
	for( int i = 0; i<object.dtl.GetDerivedTypeCount()-1; i++ )
	{
		const DerivedType &dt1 = *object.dtl.GetDerivedType(i),
						  &dt2 = *object.dtl.GetDerivedType(i+1);

		if( dt1.GetDerivedTypeCode() == DerivedType::DT_POINTER &&
			dt2.GetDerivedTypeCode() == DerivedType::DT_REFERENCE )
		{
			theApp.Error(object.errPos, "'%s' - ������������ ��� '��������� �� ������'",
				object.name.c_str());
			return false;
		}

		if( dt1.GetDerivedTypeCode() == DerivedType::DT_REFERENCE &&
			dt2.GetDerivedTypeCode() == DerivedType::DT_REFERENCE )
		{
			theApp.Error(object.errPos, "'%s' - ������������ ��� '������ �� ������'",
				object.name.c_str());
			return false;
		}

		if( dt1.GetDerivedTypeCode() == DerivedType::DT_ARRAY && 
			( dt2.GetDerivedTypeCode() == DerivedType::DT_REFERENCE ||
			  dt2.GetDerivedTypeCode() == DerivedType::DT_FUNCTION_PROTOTYPE) )
		{
			
			theApp.Error(object.errPos, "'%s' - ������������ ��� '������ %s'",
				object.name.c_str(), 
				dt2.GetDerivedTypeCode() == DerivedType::DT_REFERENCE ? "������" : "�������");

			return false;
		}

		if( dt1.GetDerivedTypeCode() == DerivedType::DT_POINTER_TO_MEMBER &&
			dt2.GetDerivedTypeCode() == DerivedType::DT_REFERENCE )
		{
			theApp.Error(object.errPos, "'%s' - ������������ ��� '��������� �� ����-������'",
				object.name.c_str());
			return false;
		}

	
		if( dt1.GetDerivedTypeCode() == DerivedType::DT_FUNCTION_PROTOTYPE &&
			(dt2.GetDerivedTypeCode() == DerivedType::DT_ARRAY || 
			 dt2.GetDerivedTypeCode() == DerivedType::DT_FUNCTION_PROTOTYPE) )
		{
			theApp.Error(object.errPos, "'%s' - ������������ ��� '������� ������������ %s'",
				object.name.c_str(), 
				dt2.GetDerivedTypeCode() == DerivedType::DT_ARRAY ? "������" : "�������");
			return false;
		}

		// ���� ������, ������� �� �������� ������� ������ - ��� �������,
		// ��� ������
		if( dt2.GetDerivedTypeCode() == DerivedType::DT_ARRAY &&
			dt2.GetDerivedTypeSize() <= 0 )
		{
			theApp.Error(object.errPos, "'%s' - ����������� ��� ������� ������ �������",
				object.name.c_str());
			return false;
		}

		// ��������� ��������, �������� ������� �� ����� ����� cv-��������������,
		// ���� ��� �� ��������� �� ���� �������. 
		// ������� ��������, ��� ���� �������� ������� �������� ������� ������, 
		// ����� ��� ��������� ���������� ���������� �������
		if( dt2.GetDerivedTypeCode() == DerivedType::DT_FUNCTION_PROTOTYPE )
		{
			if( dt1.GetDerivedTypeCode() != DerivedType::DT_POINTER_TO_MEMBER &&
				((FunctionPrototype &)dt2).CV_Qualified() != 0 )
			{
				theApp.Error(object.errPos, 
					"'%s' - ������������ ������������� cv-�������������� �������",
					object.name.c_str());
				return false;
			}
		}
	}

	return true;
}


// ���������� �������� ������������� �������� ���� � �����������. 
// �� ����� ���� �������, �������, ��������� �� ����, ������ ���� void. 
// �� ����� ���� ������� ��� ������� �������� ������������ ������, 
// ������ ���� ��� �� typedef ����������. 
// �� ����� ���� ������� ��� ������� �� �������������� ������, ������ ����
// ��� �� ����������
bool CheckerUtils::CheckRelationOfBaseTypeToDerived( TempObjectContainer &object,
							 bool declaration, bool expr ) 
{
	// ���� ������� ��� void
	if( object.finalType->GetBaseTypeCode() == BaseType::BT_VOID )
	{
		// ���� ��� ����������� ����� � ���������� �� typedef, ������
		if( object.dtl.IsEmpty() )
		{
			if( object.ssCode == KWTYPEDEF || expr )
				return true;
			else
			{
				theApp.Error(object.errPos,
					"'%s' - ������ �� ����� ����� ��� 'void'", object.name.c_str());
				return false;
			}
		}

		// ����� ���� ����������� ����, ������ ����������, ����� ��� ���
		// ��������� ��� �������
		const PDerivedType &ldt = object.dtl.GetTailDerivedType();
		if( ldt->GetDerivedTypeCode() == DerivedType::DT_POINTER   ||
			ldt->GetDerivedTypeCode() == DerivedType::DT_FUNCTION_PROTOTYPE )
			return true;

		// ����� ������
		else
		{
			PCSTR msg;
			if( ldt->GetDerivedTypeCode() == DerivedType::DT_ARRAY )
				msg = "������ ���� void";
			else if( ldt->GetDerivedTypeCode() == DerivedType::DT_REFERENCE )
				msg = "������ �� void";
			else if( ldt->GetDerivedTypeCode() == DerivedType::DT_POINTER_TO_MEMBER )
				msg = "��������� �� ���� ���� void";
			else 
				INTERNAL("'CheckRelationOfBaseTypeToDerived' "
						 "��������� ������������ ��� ������������ ����");

			theApp.Error(object.errPos,
					"'%s' - ������ �� ����� ����� ��� '%s'", object.name.c_str(), msg);
				return false;
		}
	}

	// ����� ���� ������� ��� ��� �����
	else if( object.finalType->IsClassType() )
	{
		ClassType *cls = static_cast<ClassType *>(object.finalType);
		

		// ���� ����� �� ��������� �������� ��� �����������, �� ����� ������� �����
		// ������ ��� ������, ����������, ���������� �� �����
		if( cls->IsUncomplete() || cls->IsAbstract() ) 
		{
			bool incomplete = cls->IsUncomplete();

			// ������ ����������� ����� ������, ������� ������
			// ���� ������ ���������� (typedef, extern, ���� static, typedef � ������)
			if( object.dtl.IsEmpty() )
				if( declaration )				
					return true;
			
				else
				{
					theApp.Error(object.errPos,
						"'%s' - ����� '%s' �������� %s",						 
						object.name.c_str(), cls->GetQualifiedName().c_str(), 
							incomplete ? "�������������" : "�����������");

					return false;
				}
	
			DerivedType::DT dtc = object.dtl.GetTailDerivedType()->GetDerivedTypeCode();
			if( (dtc == DerivedType::DT_ARRAY || dtc == DerivedType::DT_FUNCTION_PROTOTYPE) &&
				!declaration )
			{
				theApp.Error(object.errPos,
						"'%s' - ����� '%s' �������� %s",						 
						object.name.c_str(), cls->GetQualifiedName().c_str(), 
							incomplete ? "�������������" : "�����������");
				return false;
			}
			
			return true;
		}		
	}

	return true;
}


// ��������� ������������ ���������� ���������� �� ��������� � �������.
// ���� ������ �������� 0, ������ ��������� ������������ ������ � ������
void CheckerUtils::DefaultArgumentCheck( const FunctionPrototype &declFn, 
				const FunctionPrototype *fnInTable, const Position &errPos )
{
	// ���������, ������ ����� ��������� �� ��������� ��� �� �������
	if( fnInTable == NULL )
	{
		const FunctionParametrList &fpl = declFn.GetParametrList();
		bool haveDP = false;
		for( int i = 0; i<fpl.GetFunctionParametrCount(); i++ )
			if( fpl[i]->IsHaveDefaultValue() )
				haveDP = true;
			else if( haveDP )
			{
				theApp.Error(errPos,
					"'%s' - ��������� �������� �� ���������",
					fpl[i]->GetName().c_str());
				break;
			}
	}

	// ����� ���������� ��� ������� � �������� ���������� �� ���������
	// ������� �� �������
	else
	{
		const FunctionParametrList &dFpl = declFn.GetParametrList(),
			&inFpl = fnInTable->GetParametrList();
		INTERNAL_IF( dFpl.GetFunctionParametrCount() != inFpl.GetFunctionParametrCount() );

		bool haveDP = false;
		for( int i = 0; i<dFpl.GetFunctionParametrCount(); i++ )
		{
			// �������� �� ��������� ��� ��������� �� ����� ����������������
			if( dFpl[i]->IsHaveDefaultValue() )
			{
				haveDP = true;
				if( inFpl[i]->IsHaveDefaultValue() )
				{
					theApp.Error(errPos,
						"'%s' - �������� �� ��������� ����������������",
						dFpl[i]->GetName().c_str());
					break;
				}

				// ����� ������ �������� �� ��������� ��� ������� � �������
				else
					const_cast<Parametr&>(*inFpl[i]).
							SetDefaultValue( dFpl[i]->GetDefaultValue() );
			}

			// ����� ���� �������� �� ��������� ��� ����, � � �������,
			// ������� � ������� ��� ���������, ������� ������
			else if( haveDP && !inFpl[i]->IsHaveDefaultValue() )
			{
				theApp.Error(errPos,
					"'%s' - ��������� �������� �� ���������",
					dFpl[i]->GetName().c_str());							
				break;
			}
		}
	}
}


// ������� ������, ���������� ����
void GlobalDeclarationChecker::Error( PCSTR msg, PCSTR arg ) 
{
	theApp.Error(object.errPos, msg, arg);
	incorrect = true;
}


// ������� ������� ��������
void GlobalDeclarationChecker::Check()
{
	// ��������� ������������� �������� ������� ����� ��������������
	// � ���������� ������� ���������
	if( object.ssCode != -1		  &&
		object.ssCode != KWSTATIC &&
		object.ssCode != KWEXTERN &&
		object.ssCode != KWTYPEDEF )
	{
		if( localDeclaration			&&
			object.ssCode != KWAUTO		&&
			object.ssCode != KWREGISTER )
		{
			Error("'������������ �������� %s' ����������� � ������ ���������",
				GetKeywordName(object.ssCode));
			object.ssCode = -1;
		}
	}

	// ��������� ������������ friend
	if( object.friendSpec )
		Error("'%s' - '������������ ������ friend' ����������� � ������ ���������",
			object.name.c_str());

	// ��������� ���� ����� ������������ �������, � ������ �� �������� �-����
	if( object.fnSpecCode != -1 )
	{
		if( !object.dtl.IsFunction() )
		{
			theApp.Error( object.errPos,
				"'%s' - ������������� ������������� ������� '%s', � ���������� ��-�������",
				object.name.c_str(), GetKeywordName(object.fnSpecCode) );
			incorrect = true;
		}

		// � ���������� ���������� ����� �������������� ������ ������������ inline
		if( object.fnSpecCode != KWINLINE )
		{
			theApp.Error( object.errPos,
				"'%s' - ������������� ������������� ������� '%s' ����������� � ������ ���������",
				object.name.c_str(), GetKeywordName(object.fnSpecCode) );
			incorrect = true;
		}
	}

	// ����������� �������� ������ ����������� �����	
	if( !CheckDerivedTypeList(object) )
		incorrect = true;	

	// �������� ������������� �������� ���� � �����������
	if( !CheckRelationOfBaseTypeToDerived(object, object.ssCode == KWEXTERN ||
									object.ssCode == KWTYPEDEF ) )
		incorrect = true;	

	// ��������, ���� ���������� �������� ��������
	if( object.dtl.IsFunction() )
	{
		const FunctionPrototype &fnp =	
			static_cast<const FunctionPrototype &>(*object.dtl.GetHeadDerivedType());

		// cv-������������� ������� ����� �������������� ������ ���
		// ������� typedef
		if( fnp.CV_Qualified() != 0 && object.ssCode != KWTYPEDEF )
			Error( "'%s' - ������������ ������������� cv-�������������� �������",
					object.name.c_str());
	}

	// �������� ���� ���������� �������� ������, ��� ������ ������ ���� ��������
/*	else if( object.dtl.IsArray() )
	{
		if( object.dtl[0]->GetDerivedTypeSize() < 0 &&
			object.ssCode != KWTYPEDEF && object.ssCode != KWEXTERN )			
			Error( "'%s' - ����������� ������ �������", object.name.c_str());

		if( object.dtl[0]->GetDerivedTypeSize() == 0 )
			Error( "'%s' - ������� ������ �������", object.name.c_str());
	} */

	// ���������, ���� ������� ��� ��������� � �� ��������� �
	// ������������ ������� extern ��� static, ������
	if( object.finalType->IsClassType() &&
		static_cast<const ClassType*>(object.finalType)->IsLocal() &&
		(object.ssCode == KWSTATIC || object.ssCode == KWEXTERN) )	
		Error( "'%s' - ������� ��� �� ����� ����������", object.name.c_str());	
}


// ������� ������� �������� ���������, ��������� �������� ������
// ������� 
void ParametrChecker::Check()
{	
	// �������� ����� ����� ������ ����. �������� ������� ��� ������ �� �����
	if( parametr.ssCode != -1		  &&
		parametr.ssCode != KWREGISTER )
		theApp.Error( parametr.errPos,
		 "'%s' - '������������ �������� %s' ����������� ��� ���������",
			parametr.name.c_str(), GetKeywordName(parametr.ssCode)),
			incorrect = true;

	// ��������� ������������ friend
	if( parametr.friendSpec )
		theApp.Error( parametr.errPos,
			"'%s' - '������������ ������ friend' ����������� ��� ���������",
			parametr.name.c_str()), incorrect = true;

	// ��������� ���� ����� ������������ ������� � ��������� ��� ��������� �������
	if( parametr.fnSpecCode != -1 )
	{
		theApp.Error( parametr.errPos,
			"'%s' - ������������� ������������� ������� '%s', � ���������� ���������",
			parametr.name.c_str(), GetKeywordName(parametr.fnSpecCode) );
		incorrect = true;
	}

	// ����������� �������� ������ ����������� �����	
	if( !CheckDerivedTypeList(parametr) )
		incorrect = true;

	// �������� ������������� �������� ���� � �����������
	if( !CheckRelationOfBaseTypeToDerived(parametr, false) )
		incorrect = true;	

	// ����� ���� �������� �������� ������� - ����������� ��� � ���������
	// �� ������� � ��������� cv-�������������
	if( parametr.dtl.IsFunction() )
	{
		const FunctionPrototype &fp = 
			static_cast<const FunctionPrototype &>(*parametr.dtl[0]);		

		if( fp.CV_Qualified() != 0 )
			theApp.Error( parametr.errPos,
			"'%s' - ������������ ������������� cv-�������������� �������",
				parametr.name.c_str()), 
				incorrect = true;

		parametr.dtl.PushHeadDerivedType( new Pointer(false, false) );
	}

	// ���� ������, ����������� ��� � ���������
	else if( parametr.dtl.IsArray() )
	{
		parametr.dtl.PopHeadDerivedType();
		parametr.dtl.PushHeadDerivedType( new Pointer(false, false) );
	}


	// �������� ��������������� ���������
	if( fnParamList.HasParametr(parametr.name) >= 0 )
	{
		theApp.Error( parametr.errPos, "'%s' - �������� �������������", parametr.name.c_str());
		parametr.name = (string("<��� ����� ") + 
			CharString(fnParamList.GetFunctionParametrCount()).c_str() + ">").c_str();
	}

}


// �������� ���� throw-������������
void ThrowTypeChecker::Check()
{
	register TempObjectContainer &toc = throwType;

	// �� ����� ����� ������������ ��������
	if( toc.ssCode != -1 )		
		theApp.Error( toc.errPos,
		 "'������������ �������� %s' ����������� ��� %s",
			GetKeywordName(toc.ssCode), toc.name.c_str());			

	// ��������� ������������ friend
	if( toc.friendSpec )
		theApp.Error( toc.errPos,
		 "'������������ ������ friend' ����������� ��� %s", toc.name.c_str());
			

	// ��������� ���� ����� ������������ ������� � ��������� ��� ��������� �������
	if( toc.fnSpecCode != -1 )	
		theApp.Error( toc.errPos,
			"'%s' - ������������� ������������� ������� '%s' �����������",
			toc.name.c_str(), GetKeywordName(toc.fnSpecCode) );			

	// ����������� �������� ������ ����������� �����	
	CheckDerivedTypeList(toc);		

	// �������� ������������� �������� ���� � �����������
	CheckRelationOfBaseTypeToDerived(toc, false);	
}


// ������� ������� �������� ����, ��������� �������� ������� ������ 
void CatchDeclarationChecker::Check()
{
	// �� ����� ����� ������������ ��������
	if( toc.ssCode != -1 )		
		theApp.Error( toc.errPos,
		 "'������������ �������� %s' ����������� ��� catch-����������",
			GetKeywordName(toc.ssCode));			

	// ��������� ������������ friend
	if( toc.friendSpec )
		theApp.Error( toc.errPos,
		 "'������������ ������ friend' ����������� ��� ��� catch-����������");
			

	// ��������� ���� ����� ������������ ������� � ��������� ��� ��������� �������
	if( toc.fnSpecCode != -1 )	
		theApp.Error( toc.errPos,
			"'%s' - ������������� ������������� ������� '%s' �����������",
			toc.name.c_str(), GetKeywordName(toc.fnSpecCode) );			

	// ����������� �������� ������ ����������� �����	
	CheckDerivedTypeList(toc);		

	// �������� ������������� �������� ���� � �����������
	CheckRelationOfBaseTypeToDerived(toc, false);

	// ����������� ������� ���� T - � ��������� �� ������� ���� T
	if( toc.dtl.IsFunction() )
	{
		if( static_cast<const FunctionPrototype &>(*toc.dtl[0]).CV_Qualified() != 0 )
			theApp.Error( toc.errPos,
			"'%s' - ������������ ������������� cv-�������������� �������",
				toc.name.c_str());				

		toc.dtl.PushHeadDerivedType( new Pointer(false, false) );
	}

	// ������ ���� T - � ��������� �� T
	else if( toc.dtl.IsArray() )
	{
		toc.dtl.PopHeadDerivedType();
		toc.dtl.PushHeadDerivedType( new Pointer(false, false) );
	}

	// ����� ���������, � catch-���������� �� ����� ���� �������������� ���� 
	// ��� ��������� �� ����
	if( (toc.finalType->IsClassType() && 
		 static_cast<const ClassType *>(toc.finalType)->IsUncomplete()) ||
		(toc.finalType->GetBaseTypeCode() == BaseType::BT_VOID &&
		 toc.dtl.IsEmpty()) )
		theApp.Error( toc.errPos, 
			"�� ������ ��� '%s' �������� ������������� ��� catch-����������",
			toc.finalType->IsClassType() ? 
			static_cast<const ClassType *>(toc.finalType)->GetQualifiedName().c_str() :
			"void" );
}


// ���������� true, ���� ������ �������� ����������
bool DataMemberChecker::ConstantMember()
{
	const DerivedType *dt = NULL;
	for( int i = 0; i<dm.dtl.GetDerivedTypeCount(); i++ )
		if( dm.dtl.GetDerivedType(i)->GetDerivedTypeCode() == DerivedType::DT_ARRAY )
			continue;
		else
		{
			dt = &*dm.dtl.GetDerivedType(i);
			break;
		}

	if( !dt )
		return dm.constQual;

	if( dt->GetDerivedTypeCode() == DerivedType::DT_POINTER &&
		((Pointer *)dt)->IsConst() )
		return true;

	if( dt->GetDerivedTypeCode() == DerivedType::DT_POINTER_TO_MEMBER && 
		((PointerToMember *)dt)->IsConst() )
		return true;

	return false;
}


// ���������� true, ���� ����� �������� ����������� �-���,
// �-��� �����������, ����������, �������� �����������
PCSTR DataMemberChecker::HasNonTrivialSMF( const ClassType &cls )
{
	SMFManager smfm(cls);
	if( smfm.GetDefaultConstructor().first &&
		!smfm.GetDefaultConstructor().first->IsTrivial() )
		return "����������� �� ���������";

	if( smfm.GetCopyConstructor().first &&
		!smfm.GetCopyConstructor().first->IsTrivial() )
		return "����������� �����������" ;
	
	if( smfm.GetCopyOperator().first &&
		!smfm.GetCopyOperator().first->IsTrivial() )
		return "�������� �����������";

	if( smfm.GetDestructor().first && 
		!smfm.GetDestructor().first->IsTrivial() )
		return "����������";

	return NULL;
}


// ����� �������� ����� ����� �������� ��� � �������
void DataMemberChecker::Check()
{
	// ��������� ������������� �������� �������-�����
	if( dm.ssCode != -1		  &&
		dm.ssCode != KWSTATIC &&
		dm.ssCode != KWMUTABLE &&
		dm.ssCode != KWTYPEDEF )
		theApp.Error(dm.errPos,
			"'������������ �������� %s' ����������� ��� �������-�����",
			GetKeywordName(dm.ssCode)), incorrect = true;

	// ��������� ������������ friend
	if( dm.friendSpec )
		theApp.Error(dm.errPos,
			"'%s' - '������������ ������ friend' ����������� ��� �������-�����",
			dm.name.c_str()), incorrect = true;

	// ��������� ���� ����� ������������ �������, � ������ �� �������� �-����
	if( dm.fnSpecCode != -1 )	
		theApp.Error( dm.errPos,
				"'%s' - ������������� ������������� ������� '%s', � ���������� ��-�������",
				dm.name.c_str(), GetKeywordName(dm.fnSpecCode) ), incorrect = true;
			

	// ����������� �������� ������ ����������� �����	
	if( !CheckDerivedTypeList(dm) )
		incorrect = true;	

	// �������� ������������� �������� ���� � �����������
	if( !CheckRelationOfBaseTypeToDerived(dm, 
					dm.ssCode == KWTYPEDEF || dm.ssCode == KWSTATIC ) )
		incorrect = true;

	// ��� ����� ������ ���������� �� ����� ������ � ������� ��� ������������
	ClassType *cls = dynamic_cast<ClassType *>(&GetCurrentSymbolTable());
	INTERNAL_IF( cls == NULL );
	INTERNAL_IF( dm.name.empty() );
	
	if( cls->GetName() == dm.name )
		theApp.Error( dm.errPos,
			"'%s' - ������-���� �� ����� ����� ��� ������ � ������� �����������",
			dm.name.c_str() ), redeclared = true;

	// ��������� ����� �� ����� ����� ����������� ������-������
	if( cls->IsLocal() &&  dm.ssCode == KWSTATIC )
		theApp.Error( dm.errPos,
			"'%s' - ��������� ����� '%s', �� ����� ����� ����������� ������-������",
			dm.name.c_str(), cls->GetName().c_str() ), incorrect = true;


	// � ������ ���� ���� ����������� ������ �����������, �� �� �� ������
	// ���� ����������� ��� �������
	if( cls->GetBaseTypeCode() == BaseType::BT_UNION )
	{
		// ������� �� ����� ���� ������, ��� ������ �������� ���� ��������
		// ��������������, ���� �������� ����������� � ������� � ����������
		// �� ���� ������
		if( dm.dtl.IsReference() )
			theApp.Error( dm.errPos,
				"'%s' - ������ �� ����� ���� ������ �����������",
				dm.name.c_str() ), incorrect = true;

		if( dm.ssCode == KWSTATIC )
			theApp.Error( dm.errPos,
				"'%s' - ����������� ������-���� �� ����� ���� ������ �����������",
				dm.name.c_str() ), incorrect = true;

		// ����� ������� ��������� ����� � ������� ��� ����������� �����������,
		// ����������, �������� ����������� � �-��� �����������
		if( dm.finalType->IsClassType() )
		{
			bool chk = true;
			for( int i = 0; i<dm.dtl.GetDerivedTypeCount(); i++ )
				if( dm.dtl.GetDerivedType(i)->GetDerivedTypeCode() != DerivedType::DT_ARRAY )
				{
					chk = false;
					break;
				}

			if( chk )
			{
				const ClassType &dmCls = static_cast<const ClassType &>(*dm.finalType);
				if( PCSTR smfName = HasNonTrivialSMF(dmCls) )
					theApp.Error(dm.errPos,
						"'%s' - ����� '%s' �������� ������������� %s",
						dm.name.c_str(), dmCls.GetQualifiedName().c_str(), smfName);				
			}
		}
	}
	

	// � ������ ���� ���� �������� ��� mutable, �� ������ ���� �� �����������
	// � �� �������
	if( dm.ssCode == KWMUTABLE )
	{
		if( dm.dtl.IsReference() )
		{
			theApp.Error( dm.errPos,
				"'%s' - ������ �� ����� ����� ������������ �������� 'mutable'",
				dm.name.c_str() ), incorrect = true;
		}

		else if( ConstantMember() )
		{
			theApp.Error( dm.errPos,
				"'%s' - ����������� ������-���� �� ����� ����� ������������ �������� 'mutable'",
				dm.name.c_str() ), incorrect = true;	
		}
	}

	// �������� ���� ���������� �������� ������, ��� ������ ������ ���� ��������
	if( dm.dtl.IsArray() )
	{
		if( dm.dtl[0]->GetDerivedTypeSize() < 0 && 
			!(dm.ssCode == KWSTATIC || dm.ssCode == KWTYPEDEF) )			
			theApp.Error( dm.errPos, "'%s' - ����������� ������ �������", dm.name.c_str());

		if( dm.dtl[0]->GetDerivedTypeSize() == 0 )
			theApp.Error( dm.errPos, "'%s' - ������� ������ �������", dm.name.c_str());
	}
}


// ����� �������� ������ ����� �������� ��� � �������
void MethodChecker::Check()
{
	// ��������� ������������� �������� �������-�����
	if( method.ssCode != -1		  &&
		method.ssCode != KWSTATIC )
		theApp.Error(method.errPos,
			"'������������ �������� %s' ����������� ��� ������ ������",
			GetKeywordName(method.ssCode)), incorrect = true;

	// ��������� ������������ friend
	INTERNAL_IF( method.friendSpec );
		
	// ������������ ������� �� ����� ���� explicit
	if( method.fnSpecCode == KWEXPLICIT )	
		theApp.Error( method.errPos,
				"'%s' - 'explicit' ����� �������������� ������ � ��������������",
				method.name.c_str() ), incorrect = true;
			
	// ����������� �������� ������ ����������� �����	
	if( !CheckDerivedTypeList(method) )
		incorrect = true;	

	// �������� ������������� �������� ���� � �����������
	if( !CheckRelationOfBaseTypeToDerived(method, false) )					
		incorrect = true;

	// ����������� ������� �� ����� ���� ����������� � �����������
	// � const,volatile
	if( method.ssCode == KWSTATIC )
	{
		if( method.fnSpecCode == KWVIRTUAL )
			theApp.Error( method.errPos,
					"'%s' - ����������� ����� �� ����� ���� �����������",
					method.name.c_str() ), incorrect = true;		

		INTERNAL_IF( !method.dtl.IsFunction() );
		FunctionPrototype &fp = ((FunctionPrototype &)*method.dtl.GetHeadDerivedType());
		if( fp.IsConst() || fp.IsVolatile() )
			theApp.Error( method.errPos,
					"'%s' - ����������� ����� �� ����� ����������� � cv-���������������",
					method.name.c_str() ), incorrect = true;		
	}

	
	// ���� ����� ����� ��� ������ ������, ������ ��� �����������
	// � �� ������ ����������� � ConstructorChecker'e
	INTERNAL_IF( method.name.empty() );
	const ClassType &cls = static_cast<const ClassType&>(GetCurrentSymbolTable());
	if( method.name == cls.GetName() )
	{
		theApp.Error(method.errPos,
			"'%s' - ����� ����� ��� ������ � ������� ����������� "
			"(�������� ��� ������ ���� �����������)",
			method.name.c_str() ), incorrect = true;
		redeclared = true;
	}

	// ����������� �� ����� ����� ����������� �������
	if( cls.GetBaseTypeCode() == BaseType::BT_UNION && method.fnSpecCode == KWVIRTUAL )
		theApp.Error(method.errPos,
			"'%s' - ����������� �� ����� ����� ����������� ������",
			method.name.c_str() ), incorrect = true;
}

	
// ����� ������������ true, ���� �������� �������� �����
bool ClassOperatorChecker::IsInteger( const Parametr &prm ) const
{
	return prm.GetBaseType().GetBaseTypeCode() == BaseType::BT_INT &&
		   prm.GetDerivedTypeList().IsEmpty();
}


// ����� �������� �������������� ��������� ������
void ClassOperatorChecker::Check()
{
	// ��������� ������������� �������� �������������� ���������
	if( op.ssCode == KWSTATIC )
	{
		if( tooc.opCode != KWNEW && tooc.opCode != KWDELETE &&
			tooc.opCode != OC_NEW_ARRAY && tooc.opCode != OC_DELETE_ARRAY )
			theApp.Error(op.errPos,
				"'%s' - ������������� �������� ������ �� ����� ���� �����������",
				op.name.c_str());
	}

	else if( op.ssCode != -1 )		
		theApp.Error(op.errPos,
			"'������������ �������� %s' ����������� ��� �������������� ��������� ������",
			GetKeywordName(op.ssCode));

	// ��������� ������������ friend
	INTERNAL_IF( op.friendSpec );
		
	// ������������ ������� �� ����� ���� explicit
	if( op.fnSpecCode == KWEXPLICIT )	
		theApp.Error( op.errPos,
				"'%s' - 'explicit' ����� �������������� ������ � ��������������",
				op.name.c_str() );
			
	// ����������� �������� ������ ����������� �����	
	CheckDerivedTypeList(op);		

	// �������� ������������� �������� ���� � �����������
	CheckRelationOfBaseTypeToDerived(op, false);


	// ������������� �������� ����������� ������ ���� ��������
	if( !op.dtl.IsFunction() )
		theApp.Error( op.errPos,
				"'%s' - ������������� �������� ������ ���� ��������",
				op.name.c_str() );


	else
	{
	// ������ ���������, �����. ���������� � ���������. ������� ���������
	// �� ������ ����� ����������, �������� ������ ����� 1 ��������,
	// "+, -, ++, --, *, &" ����� ����� ��� 0 ��� � 1 ��������, �������� ()
	// ����� ����� ��������� ���������� � '...'.
	int code = tooc.opCode; 
	const FunctionPrototype &fp = 
			static_cast<const FunctionPrototype&>(*op.dtl.GetHeadDerivedType());
	int pcount = fp.GetParametrList().GetFunctionParametrCount();

	if( code == '+' || code == '-' || code == '*' || code == '&' )
	{
		if( pcount > 1 ) 
			theApp.Error( op.errPos,
				"'%s' - �������� ������ ����������� � 0 ��� 1 ����������",
				op.name.c_str() );
	}

	else if( code == INCREMENT || code == DECREMENT )
	{
		// ����� ����������� � ����� ����������, ����� ��� ����������� �������,
		// �� ���������� ����� �������� ���� ��� int
		if( pcount == 1 )
		{
			if( !IsInteger( *fp.GetParametrList().GetFunctionParametr(0) ) )
				theApp.Error( op.errPos,
					"'%s' - �������� ������������ %s ������ ����� �������� ���� 'int'",
					op.name.c_str(), code == INCREMENT ? "����������" : "����������" );
		}

		else if( pcount != 0 )
			theApp.Error( op.errPos,
				"'%s' - �������� ������ ����������� � 0 ��� 1 ����������",
				op.name.c_str() );
	}

	// ���� �������� �������
	else if( code == '!' || code == '~' || code == ARROW )
	{
		if( pcount != 0 )
			theApp.Error( op.errPos,
				"'%s' - �������� ������ ����������� ��� ����������",
				op.name.c_str() );
	}

	// ���� �������� �������� ���������� ��������� ��� ������������ ������
	else if( code == KWNEW || code == OC_NEW_ARRAY )
	{
		op.ssCode = KWSTATIC;
		if( op.fnSpecCode == KWVIRTUAL )	
			theApp.Error( op.errPos,
				"'%s' - �������� �������� ����������� � �� ����� ���� �����������",
				op.name.c_str() );
				
		// � ��������� new � new[] ������ ��������
		// ������ ���� ����� � ������������ �������� ������ ���� void *. 
		if( !IsInteger(*fp.GetParametrList().GetFunctionParametr(0)) )
			theApp.Error( op.errPos,
					"'%s' - ������ �������� ������ ���� ���� 'size_t'",
					op.name.c_str());

		// ������������ �������� ������ ���� ���� void*
		if( op.finalType->GetBaseTypeCode() != BaseType::BT_VOID ||
			op.dtl.GetDerivedTypeCount() != 2					 ||
			op.dtl.GetDerivedType(1)->GetDerivedTypeCode() != DerivedType::DT_POINTER )
			theApp.Error( op.errPos,
					"'%s' - ������������ �������� ������ ���� ���� 'void *'",
					op.name.c_str());

	}
	
	// ���� �������� ������������-��������� ��� ��������
	else if( code == KWDELETE || code == OC_DELETE_ARRAY )
	{
		op.ssCode = KWSTATIC;
		if( op.fnSpecCode == KWVIRTUAL )	
			theApp.Error( op.errPos,
				"'%s' - �������� �������� ����������� � �� ����� ���� �����������",
				op.name.c_str() );

		// ���������� ��������� ���������� ����� ����� ��� �����:
		// 'void operator delete( void *, size_t)' ��� 'void operator delete(void*)'.	
		bool correct = op.finalType->GetBaseTypeCode() == BaseType::BT_VOID && 
			op.dtl.GetDerivedTypeCount() == 1;
		
		// ��������� ������ ��������, �� ������ ���� ���� void *
		if( pcount >= 1 )
		{
			const Parametr &prm = *fp.GetParametrList().GetFunctionParametr(0);
			if( prm.GetBaseType().GetBaseTypeCode() != BaseType::BT_VOID	||
				prm.GetDerivedTypeList().GetDerivedTypeCount() != 1			||
				prm.GetDerivedTypeList().GetDerivedType(0)->GetDerivedTypeCode() != 
					DerivedType::DT_POINTER )
				correct = false;
		}
		
		// ��������� ������ ��������
		if( pcount == 2 )
		{			
			if( !IsInteger(*fp.GetParametrList().GetFunctionParametr(1)) )
				correct = false;
		}

		else if( pcount != 1 )
			correct = false;

		// ������, ���� �������� �������� �� ���������, ������� ������
		if( !correct )
			theApp.Error( op.errPos,
				"���������� ��������� ���������� ����� ����� ��� �����: "
				"'void operator %s(void *)' ��� 'void operator %s(void *, size_t)'",
				tooc.opString.c_str(), tooc.opString.c_str());
	}

	// ����� ���� �������� �� �������, �� �������� �������� � ������ ����������� �
	// ����� ����������
	else if( code != OC_FUNCTION )
	{
		if( pcount != 1 )
			theApp.Error( op.errPos,
				"'%s' - �������� ������ ����������� � 1 ����������",
				op.name.c_str() );				
	}

	// ���������, ���� �������� �����������, �� �� ����� ����������� 
	// � ���������������
	if( op.ssCode == KWSTATIC && (fp.IsConst() || fp.IsVolatile()) )
		theApp.Error( op.errPos,
			"'%s' - ����������� ����� �� ����� ����������� � cv-���������������",
				op.name.c_str() ) ;

	// ��������� ������� ���������� �� ���������, 
	if( code != OC_FUNCTION )
	for( int i = 0; i<pcount; i++ )
		if( fp.GetParametrList().GetFunctionParametr(i)->IsHaveDefaultValue() )
		{
			theApp.Error( op.errPos,
				"'%s' - �������� �� ����� ����� ���������� �� ���������",
				op.name.c_str() );			
			break;
		}
	}	
}


// �������� ������������ ���������� ��������� ����������
void CastOperatorChecker::Check()
{
	if( cop.ssCode != -1 )		
		theApp.Error(cop.errPos,
			"'������������ �������� %s' ����������� ��� ��������� ���������� ����",
			GetKeywordName(cop.ssCode));
	
	// ������������ ������� �� ����� ���� explicit
	if( cop.fnSpecCode == KWEXPLICIT )	
		theApp.Error( cop.errPos,
				"'%s' - 'explicit' ����� �������������� ������ � ��������������",
				cop.name.c_str() );

	// ��������� ������������ friend
	if( cop.friendSpec )
		theApp.Error( cop.errPos,
				"�������� ���������� �� ����� ���� ���������",
				cop.name.c_str() );

	// � ��������� �� ����� ���� cv-�������������� � ������� �����
	if( cop.constQual || cop.volatileQual || cop.finalType != NULL )
		theApp.Error( cop.errPos,
				"�������� ���������� �� ����� ��������� ������� ���",
				cop.name.c_str() );

	// �������� ���������� ������ ���� �������� � �� ��������� ������ �����������
	// �����
	if( !cop.dtl.IsFunction() || cop.dtl.GetDerivedTypeCount() > 1 )
	{
		theApp.Error( cop.errPos,
			"'%s' - �������� ���������� ������ ���� ��������",
			cop.name.c_str() );

		// �������, �.�. �� ������� ��������� �� �����
		incorrect = true;
		return ;
	}

	// ������ ������������ � ��������� ��������� ������ �� ���� ����������
	// � ��������� ����������� ��������
	{
		cop.finalType = const_cast<BaseType*>(&tcoc.castType->GetBaseType());
		cop.constQual = tcoc.castType->IsConst();
		cop.volatileQual = tcoc.castType->IsVolatile();
		PDerivedType fn = cop.dtl.GetHeadDerivedType();
		cop.dtl = tcoc.castType->GetDerivedTypeList();
		cop.dtl.PushHeadDerivedType(fn);
	}

	
	// ����������� �������� ������ ����������� �����	
	CheckDerivedTypeList(cop);		

	// �������� ������������� �������� ���� � �����������
	CheckRelationOfBaseTypeToDerived(cop, true);


	// ������ ���������, ����� �������� ���������� �� �������� ����������
	if( static_cast<const FunctionPrototype &>(*cop.dtl.GetHeadDerivedType()).
			GetParametrList().GetFunctionParametrCount() != 0 )
		theApp.Error( cop.errPos,
			"'%s' - �������� ���������� ������ ����������� ��� ����������",
			cop.name.c_str() );
}


// ������������� ��������� ���������
ConstructorChecker::ConstructorChecker( TempObjectContainer &c, const ClassType &cl )
		: ctor(c), cls(cl), incorrect(false) 
{
	INTERNAL_IF( ctor.name.empty() );
	Check();
}


// ������������� ��������� ���������
void ConstructorChecker::Check()
{
	// ���� �������� ���� ���, ����������� ��� �������� ������
	if( ctor.finalType == NULL )
		ctor.finalType = const_cast<ClassType *>(&cls);

	// 1. ���� ������� ��� �� ��������� � �������, ���������� �������� �� ����� ������. 
	//    ������� ������ �������
	if( static_cast<ClassType *>(ctor.finalType) != &cls )
	{
		theApp.Error( ctor.errPos, "� ���������� ����� ��������� ���");
		incorrect = true;
		return;
	}
	
	// 2. ����������� ������ ���� �������� � ���������� ������
	if( !ctor.dtl.IsFunction() || ctor.dtl.GetDerivedTypeCount() != 1 )
	{
		theApp.Error( ctor.errPos, 
			"'%s' - ����������� ������ ���� ��������", ctor.name.c_str());
		incorrect = true;
		return;
	}

	// 3. ����������� �� ����� ����� ������������� ��������, cv-�������������, friend, virtual.
	if( ctor.ssCode != -1 )
		theApp.Error(ctor.errPos,
			"'������������ �������� %s' ����������� ��� ������������",
			GetKeywordName(ctor.ssCode));

	if( ctor.constQual || ctor.volatileQual || ctor.friendSpec )
		theApp.Error(ctor.errPos,
			"'%s' - %s ����������� � ���������� ������������",
			ctor.name.c_str(), ctor.friendSpec ? "friend" : "cv-������������");

	if( ctor.fnSpecCode == KWVIRTUAL )
		theApp.Error(ctor.errPos,
			"'%s' - ����������� �� ����� ���� �����������",
			ctor.name.c_str());

	
	// 4. ����������� �� ����� ��������� cv-������������� �������
	if( static_cast<const FunctionPrototype&>(
			*ctor.dtl.GetHeadDerivedType()).CV_Qualified() != 0 )
		theApp.Error( ctor.errPos, 
			"'%s' - ����������� �� ����� ��������� cv-������������� �������", ctor.name.c_str());
}


// ����� ������������ true, ���� �������� �������� �����
bool GlobalOperatorChecker::IsInteger( const Parametr &prm ) const
{
	return prm.GetBaseType().GetBaseTypeCode() == BaseType::BT_INT &&
		   prm.GetDerivedTypeList().IsEmpty();
}


// ���������, ����� �������� ��� �������, ������� �� �����, �������������,
// ������� �� ������������
bool GlobalOperatorChecker::IsCompoundType( const Parametr &prm ) const
{
	if( prm.GetBaseType().IsClassType() || prm.GetBaseType().IsEnumType() )
	{
		int cnt = prm.GetDerivedTypeList().GetDerivedTypeCount() ;
		if( cnt == 0 || 
			(cnt == 1 && prm.GetDerivedTypeList().IsReference()) )
			return true;
		else 
			return false;
	}

	else
		return false;
}


// ������� �������� ����������� �������������� ���������
void GlobalOperatorChecker::Check()
{
	// ��������� ���
	op.name = tooc.opFullName;

	if( op.ssCode != -1 && op.ssCode != KWSTATIC && op.ssCode != KWEXTERN )		
		theApp.Error(op.errPos,
			"'������������ �������� %s' ����������� ��� �������������� ���������",
			GetKeywordName(op.ssCode));

	// ��������� ������������ friend
	if( op.friendSpec )
		theApp.Error(op.errPos,
			"'������������ ������ friend' ����������� ��� �������������� ���������");			
		
	// ������������ ������� �� ����� ���� explicit
	if( op.fnSpecCode != -1 && op.fnSpecCode != KWINLINE )	
		theApp.Error( op.errPos,
				"'������������ ������� %s' ����������� ��� �������������� ���������",
				GetKeywordName(op.fnSpecCode) );
					
	// ����������� �������� ������ ����������� �����	
	CheckDerivedTypeList(op);		

	// �������� ������������� �������� ���� � �����������
	CheckRelationOfBaseTypeToDerived(op, false);


	// ������������� �������� ����������� ������ ���� ��������
	if( !op.dtl.IsFunction() )
	{
		theApp.Error( op.errPos,
				"'%s' - ������������� �������� ������ ���� ��������",
				op.name.c_str() );
		return;
	}

	const FunctionPrototype &fp = 
		static_cast<const FunctionPrototype&>(*op.dtl.GetHeadDerivedType());
	int code = tooc.opCode,
		pcount = fp.GetParametrList().GetFunctionParametrCount();

	// �������� ������������, ������ �������, ����������, �������� �����
	// �� ����� ����������� ���������. 
	if( code == '=' || code == OC_FUNCTION || code == ARROW || 
		code == OC_ARRAY )
	{
		theApp.Error( op.errPos,
				"'%s' - ������������� �������� ����� ���� ������ ������ ������",
				op.name.c_str() );
		return;
	}


	// ������������� �������� �� ����� ����� cv-�������������� 
	if( fp.CV_Qualified() != 0 )
		theApp.Error( op.errPos,
				"'%s' - ������������� �������� �� ����� ����� cv-�������������� �������",
				op.name.c_str() );
	

	// 1. ������������� �������� ������ ��������� ��� ������� 
	// ���� ��������, ������� �������� �������, ������� �� �����, �������������, 
	// ������� �� ������������.

	// ���� �������� ������� ��� ��������	
	if( code == '+' || code == '-' || code == '*' || code == '&' ||
		code == INCREMENT || code == DECREMENT )
	{
		if( pcount == 1 )
		{
			if( !IsCompoundType( *fp.GetParametrList().GetFunctionParametr(0) ) )
				theApp.Error( op.errPos,
					"'%s' - �������� ������ ����� ��� ������ ��� ������������"
					"(��� ������ �� ���)",
					op.name.c_str() );
		}

		else if( pcount == 2 )
		{
			if( code == INCREMENT || code == DECREMENT )
			{
				if( !(IsCompoundType(*fp.GetParametrList().GetFunctionParametr(0)) &&
					  IsInteger(*fp.GetParametrList().GetFunctionParametr(1)) ) )
  				theApp.Error( op.errPos,
					"'%s' - ������ �������� ������ ����� ��� ������ ��� ������������"
					"(��� ������ �� ���). ������ �������� ������ ����� ��� int",
					op.name.c_str() );

			}

			else
			if( !(IsCompoundType( *fp.GetParametrList().GetFunctionParametr(0) ) ||
				  IsCompoundType( *fp.GetParametrList().GetFunctionParametr(1) )) )
				theApp.Error( op.errPos,
					"'%s' - ���� �� ���������� ������ ����� ��� ������ ��� ������������"
					"(��� ������ �� ���)",
					op.name.c_str() );
		}

		else
			theApp.Error( op.errPos,
				"'%s' - �������� ������ ����������� � 1 ��� 2 �����������",
				op.name.c_str() );

	}

	// ���� �������� �������
	else if( code == '!' || code == '~' )
	{
		if( pcount == 1 )
		{
			if( !IsCompoundType( *fp.GetParametrList().GetFunctionParametr(0) ) )
				theApp.Error( op.errPos,
					"'%s' - �������� ������ ����� ��� ������ ��� ������������"
					"(��� ������ �� ���)",
					op.name.c_str() );
		}

		else
			theApp.Error( op.errPos,
				"'%s' - �������� ������ ����������� � ����� ����������",
				op.name.c_str() );
	}

	// ���� �������� �������� ���������� ��������� ��� ������������ ������
	else if( code == KWNEW || code == OC_NEW_ARRAY )
	{							 	
		// � ��������� new � new[] ������ ��������
		// ������ ���� ����� � ������������ �������� ������ ���� void *. 
		if( !IsInteger(*fp.GetParametrList().GetFunctionParametr(0)) )
			theApp.Error( op.errPos,
					"'%s' - ������ �������� ������ ���� ���� 'size_t'",
					op.name.c_str());

		// ������������ �������� ������ ���� ���� void*
		if( op.finalType->GetBaseTypeCode() != BaseType::BT_VOID ||
			op.dtl.GetDerivedTypeCount() != 2					 ||
			op.dtl.GetDerivedType(1)->GetDerivedTypeCode() != DerivedType::DT_POINTER )
			theApp.Error( op.errPos,
					"'%s' - ������������ �������� ������ ���� ���� 'void *'",
					op.name.c_str());

	}
	
	// ���� �������� ������������-��������� ��� ��������
	else if( code == KWDELETE || code == OC_DELETE_ARRAY )
	{
		// ���������� ��������� ���������� ����� ����� ��� �����:
		// 'void operator delete( void *, size_t)' ��� 'void operator delete(void*)'.	
		bool correct = op.finalType->GetBaseTypeCode() == BaseType::BT_VOID && 
			op.dtl.GetDerivedTypeCount() == 1;
		
		// ��������� ������ ��������, �� ������ ���� ���� void *
		if( pcount >= 1 )
		{
			const Parametr &prm = *fp.GetParametrList().GetFunctionParametr(0);
			if( prm.GetBaseType().GetBaseTypeCode() != BaseType::BT_VOID	||
				prm.GetDerivedTypeList().GetDerivedTypeCount() != 1			||
				prm.GetDerivedTypeList().GetDerivedType(0)->GetDerivedTypeCode() != 
					DerivedType::DT_POINTER )
				correct = false;
		}
		
		// ��������� ������ ��������
		if( pcount == 2 )
		{			
			if( !IsInteger(*fp.GetParametrList().GetFunctionParametr(1)) )
				correct = false;
		}

		else if( pcount != 1 )
			correct = false;

		// ������, ���� �������� �������� �� ���������, ������� ������
		if( !correct )
			theApp.Error( op.errPos,
				"���������� ��������� ���������� ����� ����� ��� �����: "
				"'void operator %s(void *)' ��� 'void operator %s(void *, size_t)'",
				tooc.opString.c_str(), tooc.opString.c_str());
	}
	
	// ����� �������� �������� ��������
	else
	{
		if( pcount == 2 )
		{
			if( !(IsCompoundType( *fp.GetParametrList().GetFunctionParametr(0) ) ||
				  IsCompoundType( *fp.GetParametrList().GetFunctionParametr(1) )) )
				theApp.Error( op.errPos,
					"'%s' - ���� �� ���������� ������ ����� ��� ������ ��� ������������"
					"(��� ������ �� ���)",
					op.name.c_str() );
		}

		else
			theApp.Error( op.errPos,
				"'%s' - �������� ������ ����������� � ����� �����������",
				op.name.c_str() );
	}


	// ��������� ������� ���������� �� ���������, 
	for( int i = 0; i<pcount; i++ )
		if( fp.GetParametrList().GetFunctionParametr(i)->IsHaveDefaultValue() )
		{
			theApp.Error( op.errPos,
				"'%s' - �������� �� ����� ����� ���������� �� ���������",
				op.name.c_str() );			
			break;
		}		
}


// ����������� �������, �������� �� ����� ������ ������� ������� � 
// �������� ������������ ������� ����� � ����������� �� �������������
// ������������ �� �������:  public-(public, protected, no_access), 
// protected-(protected, protected, no_access), private-(no_access, no_access, no_access).
// ������� ����������� ��� �������� ��� ��� �������� �������� ����� ����
// �������� �� ���������� �����, � ���� ������ ���������� �������� ��������� ����
void AccessControlChecker::AnalyzeClassHierarhy( RealAccessSpecifier &ras, 
							ClassMember::AS curAS, const ClassType &curCls, int level )
{
	INTERNAL_IF( level > 1000 );

	// ���� ���� ���������� �������� ������, ������ ����� � ����. �������
	if( ras.pClass == &curCls )
	{
		// ���� ����� ��� �����, �� ������ ��� ������ � ������ ����
		// ������� ������������ ��������� �����������
		if( ras.isClassFound )
		{
			// ������������� ���� � ����� �������: NOT_CLASS_MEMBER, AS_PRIVATE,
			// AS_PROTECTED, AS_PUBLIC
			if( curAS > ras.realAs )
				ras.realAs = curAS;			
		}

		// ����� ������ ��� ���� � �������
		else 			
		{
			ras.realAs = curAS;
			ras.isClassFound = true;
		}
		
		return;
	}

	// ��� ������� �������� ������
	register const BaseClassList &bcl = curCls.GetBaseClassList();
	for( int i = 0; i<bcl.GetBaseClassCount(); i++ )
	{
		const BaseClassCharacteristic &clh = *bcl.GetBaseClassCharacteristic(i);

		// ��������� ��������� �������� ������������� ������� �� ���������
		// ������������� ������� � ������������
		ClassMember::AS nxtAS, bcAS = clh.GetAccessSpecifier();
		
		// ������������ �� �������:  public-(public, protected, no_access), 
		// protected-(protected, protected, no_access), 
		// private-(private, private, no_access).
		if( bcAS == ClassMember::AS_PUBLIC )
			nxtAS = curAS == ClassMember::AS_PUBLIC || curAS == ClassMember::AS_PROTECTED ?
				curAS : ClassMember::NOT_CLASS_MEMBER;

		else if( bcAS == ClassMember::AS_PROTECTED )
			nxtAS = curAS == ClassMember::AS_PUBLIC || curAS == ClassMember::AS_PROTECTED  ?
				ClassMember::AS_PROTECTED :	ClassMember::NOT_CLASS_MEMBER;

		else if( bcAS == ClassMember::AS_PRIVATE )
			nxtAS = curAS == ClassMember::AS_PUBLIC || curAS == ClassMember::AS_PROTECTED ?
					ClassMember::AS_PRIVATE : ClassMember::NOT_CLASS_MEMBER;

		else
			INTERNAL( "'AccessControlChecker::AnalyzeClassHierarhy' ������������ "
					  "������������ ������� �������� ������" );

		// �������� ��������
		AnalyzeClassHierarhy( ras, nxtAS, clh.GetPointerToClass(), level+1);
	}
}


// ������� ���������� true, ���� d �������� ����������� ������� b
bool AccessControlChecker::DerivedFrom( const ClassType &d, const ClassType &b )
{
	if( &d == &b )
		return true;

	register const BaseClassList &dbcl = d.GetBaseClassList();
	for( int i = 0; i<dbcl.GetBaseClassCount(); i++ )
		if( DerivedFrom( dbcl.GetBaseClassCharacteristic(i)->GetPointerToClass(), b ) )
			return true;	

	return false;
}


// �������� �������, ������� ��������� �������� ������ ������
void AccessControlChecker::Check()
{
	INTERNAL_IF( member.GetAccessSpecifier() == ClassMember::NOT_CLASS_MEMBER );

	// ������� ������� �������� �� ������ ���� ���������
	INTERNAL_IF( curST.IsLocalSymbolTable() );

	// �������� ��������� ������������ ������� ����� ���������� ����� �� ��������,
	// ���� ����� ������� ��� ���� member ����������� memberCls
	RealAccessSpecifier ras(member.GetAccessSpecifier(), &member) ;
		
	// �������� ������������ ������� ������� �� ��������,
	// �� ������, ����� ������� ���������� � ����� �� ������ � ������� ��������� ����, 
	// ������� ��� ���� ������������ �������. ���� ���� �� ����������� �������� ������,
	// isClassFound ����� ����� 0
	AnalyzeClassHierarhy( ras, member.GetAccessSpecifier(), memberCls, 0);	

	// ����� ������� �������� ������������ ������� ������� ���������.
	// ���� ������� ������� ��������� ���������� ��� �����������, ������
	// ������������ ������� ������ ���� public
	if( curST.IsGlobalSymbolTable() || curST.IsNamespaceSymbolTable() )
		accessible = ras.realAs == ClassMember::AS_PUBLIC;


	// ���� ������� ������� ��������� �������� ��������
	else if( curST.IsFunctionSymbolTable() )
	{
		const Function &fn = static_cast<const FunctionSymbolTable &>(curST).GetFunction();

		// ���� ������� ����
		if( fn.IsClassMember() )
		{			
			const ClassType &fnCls = static_cast<const ClassType&>(fn.GetSymbolTableEntry());

			// ���� ������������ �� �������, ���� �� �������� ������ �������� ������,
			// ������ ���� �� private ��� protected, �������� �� �� no_access
			if( !ras.isClassFound )	
				ras.realAs = member.GetAccessSpecifier() == ClassMember::AS_PUBLIC ?
					ClassMember::AS_PUBLIC : ClassMember::NOT_CLASS_MEMBER;

			// ���� ��� ������� ���� ������ ras.pClass, ������ ������ 
			// �������� � ����� � ����� �������������� �������, ����� 
			// �������� ������ ������� �������
			if( &fnCls == ras.pClass )
				accessible = ras.realAs != ClassMember::NOT_CLASS_MEMBER;

			// ����� ����� � �������� ����������� �������
			// ����� ���� ������������� ��� ������ ras.pClass
			else if( ras.pClass->GetFriendList().FindClassFriend( &fnCls ) >= 0 )
				accessible = true;
						
			// ����� ���� ��� ������� ���� ������ ������������ �� ras.pClass,
			// ������ ������ �������� ��� �������� � ���������� ������
			else if( DerivedFrom( fnCls, *ras.pClass ) )				
			{
				// ���� �������� ���� �������� ������, �� ����������
				if( ras.realAs == ClassMember::NOT_CLASS_MEMBER )				
				{
					accessible = false;
					return;
				}

				// ����� ���� ���� ����������. ���� ������ � ����� ����������
				// �� ������� ����� this, ����� ���� �� ����� ���� ���������,
				// ������ ��� ���������� �� ������� ������, � ������. �� ������
				// ���� ���� ���� �� �������� �����������				
				if( &memberCls != &fnCls )
				{
					if( const ::Object *ob = dynamic_cast<const ::Object *>(&member) )
						accessible = ob->GetStorageSpecifier() == ::Object::SS_STATIC ?
							true : ras.realAs == ClassMember::AS_PUBLIC;
					
					else if( const Function *f = dynamic_cast<const Function *>(&member) )
						accessible = f->GetStorageSpecifier() == Function::SS_STATIC ?
							true : ras.realAs == ClassMember::AS_PUBLIC;
					else
						accessible = false;
				}

				// ����� �������� ��� ����� ������ �������-����� ������������ ������,
				// �.�. �������� ����� ��������, ��������� �� ������ ������� ��������
				else
					accessible = true;
			}

			// ����� ������ �������� ������ � �������� ������
			else
				accessible = ras.realAs == ClassMember::AS_PUBLIC;
		}

		// ����� ����� ������� �� ����
		else
		{
			// ���� ������� ������������� ������ �������� ��� �����,
			// ����� ������ ��������
			if( ras.pClass->GetFriendList().FindClassFriend( &fn ) >= 0 )
				accessible = true;
		
			else
				accessible = ras.realAs == ClassMember::AS_PUBLIC;
		}
	}

	// ����� ���� ������ ������
	else if( curST.IsClassSymbolTable() )
	{
		const ClassType &curCls = static_cast<const ClassType &>(curST);

		// ���� ������������ �� �������, ���� �� �������� ������ �������� ������,
		// ������ ���� �� private ��� protected, �������� �� �� no_access
		if( !ras.isClassFound )	
			ras.realAs = member.GetAccessSpecifier() == ClassMember::AS_PUBLIC ?
					ClassMember::AS_PUBLIC : ClassMember::NOT_CLASS_MEMBER;

		// ���� ����� �������� ������, �� ����������� �������� �����,
		// ����� ������ public
		if( ras.pClass->GetFriendList().FindClassFriend( &curCls ) >= 0 )
			accessible = true;

		// ��� ������������ ������������ ������ ���� protected ��� public
		else if( DerivedFrom(curCls, *ras.pClass ) )
			accessible = ras.realAs == ClassMember::AS_PUBLIC ||
				ras.realAs == ClassMember::AS_PROTECTED ;

		else
			accessible = ras.realAs == ClassMember::AS_PUBLIC;
	}

	// ����� ������
	else
		INTERNAL( "'AccessControlChecker::Check' �������� ����������� ������� ���������" );
}


// ���� ��������� � ������ 'vm', ����� �� ��� � 'method',
// ������� true. ��� ���� 'vm' ������ ���� �����������
bool VirtualMethodChecker::EqualSignature( const Method *vm )
{
	// vm - ������ ���� ����������� ��������
	if( !vm->IsVirtual() )
		return false;

	// ��������� ����� ��������� ������� ���������
	const FunctionPrototype &fp1 = method.GetFunctionPrototype(), 
							&fp2 = vm->GetFunctionPrototype();	

	// ���������� ���������� ������ ���������
	const FunctionParametrList &fpl1 = fp1.GetParametrList(),
							   &fpl2 = fp2.GetParametrList();

	if( fpl1.GetFunctionParametrCount() != fpl2.GetFunctionParametrCount() )
		return false;

	// cv-������������� ����� ������ ���������
	if( fp1.IsConst() != fp2.IsConst() || fp1.IsVolatile() != fp2.IsVolatile() )
		return false;

	// ��������� ������ �������� � ������ �� ������������	
	for( int i = 0; i<fpl1.GetFunctionParametrCount(); i++ )
		if( !RedeclaredChecker::DeclEqual(*fpl1[i], *fpl2[i]) )
			return false;
		
	// ���� ������ ���������� ������� ������� ��������� ���������
	// ���� �������
	if( !RedeclaredChecker::DeclEqual( method, *vm ) )
	{
		theApp.Error( errPos,
			"'%s' - ����������� ������� ������� ������ ����� ������������� �������� �� '%s'",
			method.GetQualifiedName().c_str(), vm->GetQualifiedName().c_str());		
		return false;
	}

	return true;
}


// �������� �����, ��������� ���������� ������, ��������,
// ������� ��������� �� ��������� � ��������� �������
void VirtualMethodChecker::FillVML( const ClassType &curCls )
{
	// �������� �� ������ ������� �������, � ������� ������,
	// ���� ��� �������, ����� ������������ � ���������� ���������
	// ��������, ���� ��� - �����
	register const BaseClassList &bcl = curCls.GetBaseClassList();
	for( int i = 0; i<bcl.GetBaseClassCount(); i++ )
	{
		// �������� ����� ��������
		const ClassType &baseCls = bcl.GetBaseClassCharacteristic(i)->GetPointerToClass();
		const VirtualFunctionList &vfl = baseCls.GetVirtualFunctionList();
		VirtualFunctionList::const_iterator pvf = 
			find_if(vfl.begin(), vfl.end(), VMFunctor(method.GetName().c_str()) );

		// ���� ������ �� �������, �������� ��������
		if( pvf == vfl.end() )
		{
			FillVML( baseCls );
			continue;
		}

		// ����� �� ���������� ������ �������� ����������� �������,
		// ������� ����� ����� �� ���������, ��� � ������� �����.
		// ���� ��������� ���������, �������� ����� � ������. �����
		// ������ ���� ������������ � ������� ������.
		bool haveVm = false;
		for( VirtualFunctionList::const_iterator p = pvf; p != vfl.end(); p++ )
		{			
			if( (*p)->GetName() == method.GetName() && EqualSignature(*p) )
			{
				INTERNAL_IF( haveVm );
				vml.push_back( *p );
				haveVm = true;
			}
		}
	}
}


// ��������� �������� ������������ ���������
// ������� �������
void VirtualMethodChecker::CheckDestructor( const ClassType &curCls )
{
	INTERNAL_IF( !curCls.IsDerived() );

	// ��� ������� ��������� ������ ��������� ������� ������,
	// �.�. ����������� ������������ ������������� ������������,
	// ���� �� �� ����� ����.
	register const BaseClassList &bcl = curCls.GetBaseClassList();
	for( int i = 0; i<bcl.GetBaseClassCount(); i++ )
	{
		// �������� ����� ��������
		const ClassType &baseCls = bcl.GetBaseClassCharacteristic(i)->GetPointerToClass();
		const VirtualFunctionList &vfl = baseCls.GetVirtualFunctionList();
		VirtualFunctionList::const_iterator pvf = 
			find_if(vfl.begin(), vfl.end(),
				VMFunctor(string("~") + baseCls.GetName().c_str()) );

		if( pvf != vfl.end() )
		{			
			const Method *dtor = *pvf;
			INTERNAL_IF( dtor == NULL );
			if( dtor->IsVirtual() )
			{
				method.SetVirtual(dtor);

				// ���������� ���������� � ������ ������������
				vml.push_back(dtor);

				// ���� ���������� �������� �����������, � ��� ����������
				// �� ��������, ��������� ���-��
				if( method.IsAbstract() )
					break;

				// ��������� ���-�� ����������� ������� �������� ������
				if( dtor->IsAbstract() )
					const_cast<ClassType &>(curCls).DecreaseAbstractMethods();
			}
		}
	}
}


// �������� �-��� ��������� ��������
void VirtualMethodChecker::Check()
{
	// ���� ����� ����������� ��� �����������, �� ��������� �������� 
	if( method.GetStorageSpecifier() == Function::SS_STATIC ||
		method.IsConstructor() )
		return;

	const ClassType *cls = dynamic_cast<const ClassType *>(&method.GetSymbolTableEntry());
	INTERNAL_IF( cls == NULL );

	// ����� �� �������� �����������, �������� ����������� ������� �� �����
	// ������
	if( !cls->IsDerived() )
		return;

	// ���� ����� �������� ������������, ��������� ��� ���� ����� ������
	// ��������� ������� ������� � ������� ����������� ������������,
	// ����  ����-�� ���� ����� ����� ����. ����������, ������ �����������
	// ������������� ������
	if( method.IsDestructor() )
	{
		CheckDestructor( *cls );
		return ;
	}

	// �������� ���� ������
	destRole = NameManager::GetIdentifierRole(&method);

	// ����� ��������� ������ ����������
	FillVML( *cls );

	// ���� ������ �� ������, ����������� ������������� ������� �������,
	// � ���������
	if( !vml.empty() )
	{
		// ������ �������� ����� - ������ ������� � ������ �����.
		// ��� ��� �� ����� �������� ����� ����� �� ����� ������ �����
		// �������� ��� ��������������, �.�. ��������� �� ���� � V-�������
		// ��� ����� �������� �� ��� �����
		method.SetVirtual( vml.front() );

		// ���� ����� �� �������� �����������, ��������� ���-�� �����������
		// �������, �� ���-�� ����������� ������� � ������
		if( !method.IsAbstract() )
			for( VML::iterator p = vml.begin(); p != vml.end(); p++ )
				if( (*p)->IsAbstract() )
					const_cast<ClassType *>(cls)->DecreaseAbstractMethods();
	}
}
