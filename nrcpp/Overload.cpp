// ���������� ��������� ����������� � ����������� � ����������� ���� - Overload.cpp

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



// ���������� destID � �������������� ��������� � ���� �� ���� �� ������������,
// ������� false. ����� ��������� ��� �������� ���������. �������� � 
// ��������� ������� �� ����������� ������������� � throw-������������.
// ������� ������, ��� ����� �� ��������� ������������� ��������
bool RedeclaredChecker::DeclEqual( const TypyziedEntity &ob1, const TypyziedEntity &ob2 )
{
	// 1. ��������� ������� ����
	const BaseType &bt1 = ob1.GetBaseType(),
				   &bt2 = ob2.GetBaseType();

	// ��������� ������� ����, ��� ���� ������ ���������
	if( bt1.GetBaseTypeCode() != bt2.GetBaseTypeCode() ||
		bt1.GetSignModifier() != bt2.GetSignModifier() ||
		bt1.GetSizeModifier() != bt2.GetSizeModifier() )
		return false;

	// ���� ���� ��������� � ��������� ����� ����� ��� ����� ��� ������������
	// �� ��������� ������ ���������
	BaseType::BT bc = bt1.GetBaseTypeCode();
	if( bc == BaseType::BT_CLASS || bc == BaseType::BT_STRUCT || 
		bc == BaseType::BT_UNION || bc == BaseType::BT_ENUM )
		if( &bt1 != &bt2 )
			return false;

	// 2. ��������� cv-�������������	
	if( ob1.IsConst() != ob2.IsConst() ||
		ob1.IsVolatile() != ob2.IsVolatile() )
	{
		// � ����������, ����� ���������������, ���� 
		// ������������ ��������� � �������� ����, �� � ������������
		if( ob1.IsParametr() && ob1.GetDerivedTypeList().IsEmpty() &&
			ob2.IsParametr() && ob2.GetDerivedTypeList().IsEmpty() )
			;
		else
			return false;
	}


	// 3. ��������� ������ ����������� �����
	const DerivedTypeList &dtl1 = ob1.GetDerivedTypeList(),
						  &dtl2 = ob2.GetDerivedTypeList();

	// 3�. ������� ������� ������ ���������
	if( dtl1.GetDerivedTypeCount() != dtl2.GetDerivedTypeCount() )
		return false;

	// �������� �� ����� ������ �������� ������ ����������� ��� �� �����������
	for( int i = 0; i<dtl1.GetDerivedTypeCount(); i++ )
	{
		const DerivedType &dt1 = *dtl1.GetDerivedType(i),
						  &dt2 = *dtl2.GetDerivedType(i);

		// ���� ���� ����������� ����� �� ����� �������
		if( dt1.GetDerivedTypeCode() != dt2.GetDerivedTypeCode() )
			return false;

		// ����� ��������� ������������� �������� ������������ ����
		DerivedType::DT dc1 = dt1.GetDerivedTypeCode(), 
						dc2 = dt2.GetDerivedTypeCode();

		// ���� ������, �� ��������� ������
		if( dc1 == DerivedType::DT_REFERENCE )
			;

		// ���� ���������, �������� �������������
		else if( dc1 == DerivedType::DT_POINTER )
		{
			if( i == 0 && ob1.IsParametr() && ob2.IsParametr() )
				continue;

			if( ((const Pointer &)dt1).IsConst() != ((const Pointer &)dt2).IsConst() ||
				((const Pointer &)dt1).IsVolatile() != ((const Pointer &)dt2).IsVolatile() )
				return false;
		}

		// ���� ��������� �� ���� �������� ������������� � ����� � ��������
		// ����������� ���������
		else if( dc1 == DerivedType::DT_POINTER_TO_MEMBER )
		{
			if( i == 0 && ob1.IsParametr() && ob2.IsParametr() )
				continue;

			const PointerToMember &ptm1 = static_cast<const PointerToMember &>(dt1),
								  &ptm2 = static_cast<const PointerToMember &>(dt2);

			if( ptm1.IsConst() != ptm2.IsConst()			||
				ptm1.IsVolatile() != ptm2.IsVolatile()		||
				&ptm1.GetMemberClassType() != &ptm2.GetMemberClassType() )
				return false;
		}

		// ���� ������, �������� ������� ���� ��������
		else if( dc1 == DerivedType::DT_ARRAY )
		{
			if( dt1.GetDerivedTypeSize() != dt2.GetDerivedTypeSize() )
				return false;
		}

		// ���� �������� �������, �� �������� ������ ������� ��� ������� ���������,
		// � ����� �������� cv-������������� � throw-������������
		else if( dc1 == DerivedType::DT_FUNCTION_PROTOTYPE )
		{
			const FunctionPrototype &fp1 = static_cast<const FunctionPrototype &>(dt1),
								    &fp2 = static_cast<const FunctionPrototype &>(dt2);	
			
			// ���������� ���������� ������ ���������
			const FunctionParametrList &fpl1 = fp1.GetParametrList(),
									   &fpl2 = fp2.GetParametrList();

			if( fpl1.GetFunctionParametrCount() != fpl2.GetFunctionParametrCount() )
				return false;

			// ��������� ������ �������� � ������ �� ������������
			for( int i = 0; i<fpl1.GetFunctionParametrCount(); i++ )
				if( !DeclEqual(*fpl1[i], *fpl2[i]) )
					return false;

			// ��������� cv-�������������
			if( fp1.IsConst() != fp2.IsConst() ||
				fp1.IsVolatile() != fp2.IsVolatile() )
				return false;

			// ��������� throw-������������
			if( !ob1.IsParametr() )
			{
			if( fp1.CanThrowExceptions() != fp2.CanThrowExceptions() )
				return false;

			// ��������� ������ throw-�����
			const FunctionThrowTypeList &ftt1 = fp1.GetThrowTypeList(),
									    &ftt2 = fp2.GetThrowTypeList();
				
			// ���������� ������ ���� �����
			if( ftt1.GetThrowTypeCount() != ftt2.GetThrowTypeCount() )
				return false;

			// ��������� ������ ��� � ������������
			for( i = 0; i<ftt1.GetThrowTypeCount(); i++ )
				if( !DeclEqual(*ftt1[i], *ftt2[i]) )
					return false;
			}
		}

		// ����� ����������� ���
		else
			INTERNAL("'RedeclaredChecker::DeclEqual' ����������� ��� ������������ ����");
	}

	return true;
}


// ��������� �� ������ ����� � �������� ����� �������
// ����������� �� ��������� � ���������. ���� ������� �������, ������������
// ��������� �� ���, ����� NULL
const Function *RedeclaredChecker::FnMatch( ) const
{
	INTERNAL_IF( !(destIDRole >= R_FUNCTION && destIDRole <= R_CONSTRUCTOR) );

	const Function *rval = NULL;
	const FunctionPrototype &fp1 = dynamic_cast<const FunctionPrototype &>(
		*destID.GetDerivedTypeList().GetHeadDerivedType());

	for( RoleList::const_iterator p = roleList.begin(); p != roleList.end(); p++ )
	{
		if( (*p).second != destIDRole )
			continue;

		// ��������� ����� ��������� ������� ���������
		const Function *cand = dynamic_cast<const Function *>((*p).first);
		const FunctionPrototype &fp2 = cand->GetFunctionPrototype();
		INTERNAL_IF( cand == NULL );

		// ���������� ������������� ������� � ��������� ������� ���������,
		// ������� ��������� ��-�� �������� ��������� ����
		if( (destIDRole == R_FUNCTION || destIDRole == R_OVERLOAD_OPERATOR) &&
			(GetCurrentSymbolTable().IsFunctionSymbolTable() || 
			 GetCurrentSymbolTable().IsLocalSymbolTable()) )
			theApp.Error(errPos, 
				"'%s' - ���������� ��������� ������������� ������� ����������",
				cand->GetName().c_str());

		// ���������� ���������� ������ ���������
		const FunctionParametrList &fpl1 = fp1.GetParametrList(),
								   &fpl2 = fp2.GetParametrList();

		if( fpl1.GetFunctionParametrCount() != fpl2.GetFunctionParametrCount() )
			continue;

		// cv-������������� ����� ������ ���������
		if( fp1.IsConst() != fp2.IsConst() || fp1.IsVolatile() != fp2.IsVolatile() )
			continue;

		// ��������� ������ �������� � ������ �� ������������
		bool eq = true;
		for( int i = 0; i<fpl1.GetFunctionParametrCount(); i++ )
			if( !DeclEqual(*fpl1[i], *fpl2[i]) )
			{
				eq = false;
				break;
			}

		// ���� ������ ���������� ������� ������� ��������� ���������
		// ���� ������� � ������������� ��������
		if( eq )
		{
			// ����� ���� ������ ���� �����. ������������� �������
			if( rval != NULL )
				INTERNAL( "��������� ������������ ������������� �������" );

			rval = cand;
			if( !DeclEqual( destID, *cand ) || 
				(destID.IsFunction() && 
				  (cand->GetStorageSpecifier() != 
				    static_cast<const Function&>(destID).GetStorageSpecifier() ||
				   cand->GetCallingConvention() != 
				    static_cast<const Function&>(destID).GetCallingConvention()) ) )
				theApp.Error( errPos,
					"'%s' - �������������� ���� � ���������� �����������",
					cand->GetName().c_str());
		}			
	}

	return rval;
}


// ���������, �������� �� ���������� ������� ��� ������� � �������
// ������� ���������. ���� ���������� ������������� redeclared � false
void RedeclaredChecker::Check() 
{
	INTERNAL_IF( !(destIDRole >= R_OBJECT && destIDRole <= R_CONSTRUCTOR) );
	bool isFn = destIDRole >= R_FUNCTION && destIDRole <= R_CONSTRUCTOR;

	// � ������ �������, ���� ������ ����� ������, ������ ������ ���������
	// �� �����
	if( roleList.empty() )
		return;

	// ����� ������� ��������� ������ �����, �� ���������� �� �����,
	// ������� �������� ������� ��� �������������, � ����� �� ������� 
	// ����� ��� ������ ��������������
	for( RoleList::const_iterator p = roleList.begin(); p != roleList.end(); p++ )
	{
		if( (*p).second == R_CLASS_TYPE || (*p).second == R_UNION_CLASS_TYPE ||
			(*p).second == R_ENUM_TYPE )
		{
			// ���������, ���� ����������� typedef, �� ��� ������
			if( destIDRole == R_OBJECT )
			{
				const ::Object *ob = dynamic_cast<const ::Object *>(&destID);
				INTERNAL_IF( ob == NULL );
				if( ob->GetStorageSpecifier() == ::Object::SS_TYPEDEF )
				{
					redeclared = true;
					theApp.Error(errPos,
						"'%s' - ��� ��� ��������", ob->GetQualifiedName().c_str());
					return;
				}
			}

			// ����� ���������� ���
			continue;
		}

		// ���� ���� �������, ��������� ����������� ���������������
		if( (*p).second == destIDRole )
		{
			// ������������ ������������� ������� �������� �����
			if( isFn )
				;

			// ����� ���� ������, ���������� ����� �� ��� 
			// typedef ��� extern � ���� ���������
			else
			{
				INTERNAL_IF( prevID != NULL );
				prevID = (*p).first;
				redeclared = true;
				
				// ���� �������� �������������� ������, �������� ������ �� ���������������,
				// ����� ������� ������ ����
				if( const ::Object *ob1 = dynamic_cast<const ::Object *>(&destID) )
				{
					const ::Object &ob2 = static_cast<const ::Object&>(*prevID);								
					::Object::SS ss1 = ob1 ? ob1->GetStorageSpecifier() : ::Object::SS_NONE,
								 ss2 = ob2.GetStorageSpecifier();

					if( ob1->IsCLinkSpecification() != ob2.IsCLinkSpecification() )
						goto err;
					if( ss1 != ss2 )
					{
						// ���������, ���� ����������� ���� ������������
						// � ���������� ������� ���������
						if( ss2 == ::Object::SS_STATIC && ob2.IsClassMember() &&
							ss1 == ::Object::SS_NONE && 
							!GetCurrentSymbolTable().IsClassSymbolTable() )
							;

						else if( ss2 != ::Object::SS_EXTERN && ss1 != ::Object::SS_NONE )
							goto err;

						if( !DeclEqual(destID, ob2) )
							goto err;
					}
					
					else if( (ss1 != ::Object::SS_TYPEDEF && ss1 != ::Object::SS_EXTERN ) ||
						!DeclEqual(destID, ob2) )
					err:
						theApp.Error(errPos, 
							"'%s' - ��� ��������", 
							ob2.GetQualifiedName().c_str());
				}

				// ����� ������� ������ ����
				else if( !DeclEqual(destID, static_cast<const ::Object&>(*prevID)) ) 	
					theApp.Error(errPos, 
						"'%s' - �� ���������� ����� � ���������� �����������", 
						prevID->GetQualifiedName().c_str());
			}
		}

		// � ��������� ������ ��� ��� ������������ � ��� ��������� �������
		else 
		{
			theApp.Error(errPos,
				"'%s' - ��� ��������������", (*p).first->GetName().c_str());
			prevID = (*p).first;
			redeclared = true;
			return ;
		}
	}

	// ���� �������, ������� ��������� ����������� �� ����������� ����� �������������
	// �������
	if( isFn )			
		if( const Function *fn = FnMatch() )
		{
			prevID = fn;
			redeclared = true;
		}	
}


// �������, ������������ ��� ����������� ������������� ������ �����. 
// ��-������, ��� ���� � ������, ������ ���� ����� r, �� ������,
// ���� ������� ���� �����, ������ ������ ��������� � ������, ������
// ���� ����� ������� ��������� � ������. � ������ ���� ��� �������
// �����, ������������ �������������, ����� - NULL
const Identifier *AmbiguityChecker::IsDestRole( Role r ) const
{
	Identifier *rval = NULL;

	if( rlist.empty() || rlist.front().second != r )
		return NULL;

	rval = rlist.front().first;
	for( RoleList::const_iterator p = rlist.begin(); p != rlist.end(); p++ )
	{
		// ���� ���� �� ��������� ���� ���������
		if( (*p).second != r || (*p).first != rval )
		{
			// ������� �����. ������ ���� �������������� �� ������ ��
			if( diagnostic )
			{
				if( &(*p).first->GetSymbolTableEntry() != &rval->GetSymbolTableEntry() )				
				theApp.Error(errPos, "��������������� ����� '%s' � '%s'", 
					rval->GetQualifiedName().c_str(), (*p).first->GetQualifiedName().c_str());

				// ��� ����� ���������� rval, ����� ���������� ������� ����� ���
				//	����������������
				return rval;		
			}

			// ����� 0
			else
				return NULL;
		}
				
	}
	
	return rval;
}


// ���� ��� �������� ��������� ���� typedef, ������� ��������� �� ����
const ::Object *AmbiguityChecker::IsTypedef( ) const
{	
	if( rlist.empty() )
		return NULL;
	
	Role r = (*rlist.begin()).second;
	if( r != R_OBJECT  && r != R_DATAMEMBER )
		return NULL;
	
	const ::Object *rval = static_cast<const ::Object *>(IsDestRole(r));
	if( !rval || rval->GetStorageSpecifier() != ::Object::SS_TYPEDEF )
		return NULL;
	return rval;
}


// ��������� �������� �� ��� �������������, ���� �������� withOverload == true,
// ����� ����������, ����� ���� �������������� � ������, � ��������� ������,
// ���� ������ ���� ����
const EnumType *AmbiguityChecker::IsEnumType( bool withOverload ) const
{
	if( withOverload )
	{
		// ������� ��������� ���� ������, �.�. � ������ ����� ���� ���������
		// �����, � ���� ������ ����� ��������������� � ������� ������� NULL
		EnumType *rval = NULL;
		for( RoleList::const_iterator p = rlist.begin(); p != rlist.end(); p++ )
		{
			// ����� ��� ����� �������, �������, ��������� ������������,
			// � ����� ����������� �� ���, �� ����� �������������� � ������������ �����.
			// ������ ���� ���� ������ �������� ��� typedef, ��� ������ �� ������,
			// �.�. ���� ������������ �������� ���� (���� withOverload == true)
			Role r = (*p).second; 
			if( r == R_OBJECT	|| r == R_DATAMEMBER || r == R_PARAMETR  ||
				r == R_ENUM_CONSTANT ||  r == R_CLASS_ENUM_CONSTANT      ||
				r == R_FUNCTION		 ||  r == R_METHOD )
				continue;
	
			else if( (*p).second == R_ENUM_TYPE )
			{
				if( rval == NULL )	
					rval = static_cast<EnumType *>( (*p).first );

				// ������������ ��� ������. ���� �� ��������� �����,
				// ����� ����� �������� ��� ������������ ��� using,
				// ����� ��� �� ��������� ����������������
				else if( rval == (*p).first ) 
					continue;

				// ����� ���������������
				else
				{
					if( diagnostic )
					theApp.Error(errPos, "��������������� ����� '%s' � '%s'", 
						rval->GetQualifiedName().c_str(), 
						(*p).first->GetQualifiedName().c_str());

					// ���������� rval, ����� ���������� �������
					// ����� ��� ����������������
					return rval;	
				}
			}

			// � ��������� ������ ��� �� ����� �������������� � ������������
			// ����� � �� �������� ��
			else
				return NULL;
		}

		return rval;
	}

	else
	{
		return static_cast<const EnumType *>( IsDestRole( R_ENUM_TYPE) );
	}
}


// ��������� �������� �� ��� �������, ���� �������� withOverload == true,
// ����� ����������, ����� ���� �������������� � ������, � ��������� ������,
// ���� ������ ���� ����
const ClassType *AmbiguityChecker::IsClassType( bool withOverload ) const
{
	if( withOverload )
	{
		// ������� ��������� ���� ������, �.�. � ������ ����� ���� ���������
		// �����, � ���� ������ ����� ��������������� � ������� ������� NULL
		ClassType *rval = NULL;
		for( RoleList::const_iterator p = rlist.begin(); p != rlist.end(); p++ )
		{
			Role r = (*p).second; 
			if( r == R_OBJECT		 || r == R_DATAMEMBER || r == R_PARAMETR  ||
				r == R_ENUM_CONSTANT || r == R_CLASS_ENUM_CONSTANT      ||
				r == R_FUNCTION		 || r == R_METHOD )
				continue;
	
			else if( (*p).second == R_CLASS_TYPE || 
					 (*p).second == R_UNION_CLASS_TYPE )
			{
				if( rval == NULL )	
					rval = static_cast<ClassType *>( (*p).first );

				else if( rval == (*p).first ) 
					continue;

				// ����� ���������������
				else
				{
					if( diagnostic )
						theApp.Error(errPos, "��������������� ����� '%s' � '%s'", 
						rval->GetQualifiedName().c_str(), 
						(*p).first->GetQualifiedName().c_str() );
					return rval;
				}
			}

			// � ��������� ������ ��� �� ����� �������������� � ������������
			// ����� � �� �������� ��
			else
				return NULL;
		}
		
		return rval;
	}

	else
	{
		if( rlist.empty() || 
			(rlist.front().second != R_CLASS_TYPE && 
			 rlist.front().second != R_UNION_CLASS_TYPE) )
			return NULL;

		return static_cast<const ClassType *>( IsDestRole(rlist.front().second) );
	}
}


// ����������������� ��� - �������� �� ��� ������ ����,
// �.�. �������, �������������, ����� typedef, ��������� ���������� ����.
// ���� � ����� > 1 ����, ������� false, �.�. ��� ���� ������ ���� ����������
// � ����� ������� ���������, � ��������� ������ ��� ������������� 
const Identifier *AmbiguityChecker::IsTypeName( bool withOverload  )  const
{
	const Identifier *id = NULL;

	// ���� ����� ���� � ��������� ���������� ����
	if( IsDestRole(R_TEMPLATE_TYPE_PARAMETR) != NULL )
		return (*rlist.begin()).first;
	
	else if( (id = IsTypedef()) != NULL )
		return id;

	else if( (id = IsClassType( withOverload)) != NULL )
		return id;

	else if( (id = IsEnumType( withOverload)) != NULL )
		return id;

	// ����� �� ��� ����
	return NULL;
}


// ���� ��� �������� ��������� �������
const TemplateClassType *AmbiguityChecker::IsTemplateClass( ) const
{
	return static_cast<const TemplateClassType *>( IsDestRole( R_TEMPLATE_CLASS) );		
}


// ���� ��� �������� ����������� �������� ���������
const NameSpace *AmbiguityChecker::IsNameSpace( ) const
{
	return static_cast<const NameSpace *>( IsDestRole(R_NAMESPACE) );
}

