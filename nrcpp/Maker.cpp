// ���������� ���������� �������-���������� - Maker.cpp

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
#include "Parser.h"
#include "Maker.h"
#include "Checker.h"
#include "Overload.h"
#include "Body.h"
#include "ExpressionMaker.h"

using namespace MakerUtils;


// ������� ��������� ��������� �� ���� �� ������,
// �������� ������������� ��� ������ NodePackage 
inline static PointerToMember *MakePointerToMember( NodePackage &ptm )
{
	INTERNAL_IF( ptm.GetPackageID() != PC_POINTER_TO_MEMBER );
	INTERNAL_IF( ptm.GetChildPackageCount() < 3 );

	int lpid = ptm.GetLastChildPackage()->GetPackageID();
	bool cq = false, vq = false;

	// ���� �������� �� ���� �������� cv-�������������
	if( lpid == KWCONST || lpid == KWVOLATILE )
	{
		(lpid == KWCONST ? cq : vq) = true;
		delete ptm.childPackageList.back();
		ptm.childPackageList.pop_back();

		// ����� ���� ��� ���� ������������
		lpid = ptm.GetLastChildPackage()->GetPackageID();
		if( lpid == KWCONST || lpid == KWVOLATILE )
			(lpid == KWCONST ? cq : vq) = true,
			delete ptm.childPackageList.back(),
			ptm.childPackageList.pop_back();
	}

	INTERNAL_IF( ptm.GetLastChildPackage()->GetPackageID() != '*' );

	// ����������� ��������� ��� ������� ��� ��������� ������
	delete ptm.childPackageList.back();
	ptm.childPackageList.pop_back();
	delete ptm.childPackageList.back();
	ptm.childPackageList.pop_back();

	// ������� ������� �����
	Position epos = ParserUtils::GetPackagePosition(&ptm);
		
	// ������ ��� ������������������ �����
	ptm.SetPackageID(PC_QUALIFIED_NAME);
	QualifiedNameManager qnm(&ptm);
	AmbiguityChecker achk(qnm.GetRoleList(), epos, true);
	
	const ClassType *cls = NULL;
		
	cls = achk.IsClassType(true);			
	if( cls == NULL )		// ���� ��� �� �������� �������, �������� ��� ��� typedef	
		if( const ::Object *id = achk.IsTypedef() )
			cls = CheckerUtils::TypedefIsClass(*id);

	// ������� ��������� �� ����, ���� ����� ������
	if( cls != NULL )
	{
		// ��������� ����� �� ����������� � ���������� ����������� ��������� �� ����
		CheckerUtils::CheckAccess(qnm, *cls, epos);		
		return new PointerToMember(cls, cq, vq);
	}
			
	// ����� ����� �� ������
	else
	{
		theApp.Error( ParserUtils::GetPackagePosition(&ptm),
			"'%s' - �� �������� �������", ParserUtils::PrintPackageTree(&ptm).c_str());
		return NULL;
	}
}


// ������� ����� ���������� �� ������ �� ��������� ���������,
// ������������ ��� ����� ���������� ��� ���������� �������� � �������
bool MakerUtils::AnalyzeTypeSpecifierPkg( const NodePackage *typeSpecList, 
			TempObjectContainer *tempObjectContainer, bool canDefineImplicity )
{		
	bool errorFlag = false;
	// c������ ������������ ������������� ����
	for( int i = 0; i < typeSpecList->GetChildPackageCount(); i++ )
	{
		// ������� ����, ��� ������������, ��� �����, ��� ��������� �����
		if( typeSpecList->GetChildPackage(i)->IsNodePackage() )
		{
			INTERNAL_IF( 
				typeSpecList->GetChildPackage(i)->GetPackageID() != PC_QUALIFIED_TYPENAME);

			QualifiedNameManager qnm( (NodePackage *)typeSpecList->GetChildPackage(i) );
			AmbiguityChecker achk(qnm.GetRoleList(),tempObjectContainer->errPos, true);
			const Identifier *chkId = NULL;

			if( const ::Object *td = achk.IsTypedef() )
				tempObjectContainer->baseType = 
					new TempObjectContainer::SynonymType(*td), chkId = td ; 

			else if( const ClassType *ct = achk.IsClassType(false) )
				tempObjectContainer->baseType = 
					new TempObjectContainer::CompoundType(*ct), chkId = ct ;

			else if( const EnumType *et = achk.IsEnumType(false) )
				tempObjectContainer->baseType = 
					new TempObjectContainer::CompoundType(*et), chkId = et;

			else
				INTERNAL("'AnanlyzeTypeSpecifierPkg' ��������� ������� "
						 "����� � ����������� �����");		
			
			// �������� ��� �� �����������
			CheckerUtils::CheckAccess(qnm, *chkId, tempObjectContainer->errPos);
			continue;
		}

		// ����� ����� �������
		const Lexem &lxm = ((LexemPackage *)typeSpecList->GetChildPackage(i))->GetLexem();		
		TypeSpecifierManager ts(lxm);
		bool error = false;

		// ���������� ������� ���
		if( ts.IsBaseType() )
		{
			// ���� ������� ��� ��� �����
			if( tempObjectContainer->baseType != NULL )
			{
				BaseType::BT &bt = ((TempObjectContainer::ImplicitType *)tempObjectContainer->
						baseType)->baseTypeCode;

				if( tempObjectContainer->baseType->IsImplicit() &&
					bt == BaseType::BT_NONE )
					bt = ts.CodeToBaseType();
				else
					error = true;
			}
				
			else
				tempObjectContainer->baseType = 
					new TempObjectContainer::ImplicitType( ts.CodeToBaseType() );
		}

		// ���� ������������ ������
		else if( ts.IsClassSpec() )
		{
			if( tempObjectContainer->baseType != NULL )
				error = true;

			// ���� ����� ����������, ��� ��������� ������� 
			else if( i+1 == typeSpecList->GetChildPackageCount() ||
					 typeSpecList->GetChildPackage(i+1)->GetPackageID() != PC_QUALIFIED_NAME )
				theApp.Error(lxm.GetPos(), "��������� ��� ����� '%s'", lxm.GetBuf().c_str());

			// ���������� �����, ������ ���� ��� �� ���������
			else
			{
				i++;	// ���������� ���
				NodePackage *tsl = const_cast<NodePackage *>(typeSpecList);
				BaseType *bt = NULL;

				// ������� ����������� (��� ������� ������������) �����, ���� ���
				// �� ���������
				if( canDefineImplicity )
					bt = lxm == KWENUM 
					? (BaseType *)EnumTypeMaker(tsl, tempObjectContainer->curAccessSpec).Make() 
					: (BaseType *)ClassTypeMaker(tsl, tempObjectContainer->curAccessSpec).Make();

				// ����� ������ �������
				else
				{
					// ����
					QualifiedNameManager qnm((NodePackage *)tsl->GetChildPackage(i));
					AmbiguityChecker achk(qnm.GetRoleList());

					// ���������, ���� ��� �� ������ ������ ��� void � ������� ������
					if( (bt = (lxm == KWENUM ? (BaseType*)achk.IsEnumType(true) :
							(BaseType*)achk.IsClassType(true)) ) == NULL )
					{
						theApp.Error(tempObjectContainer->errPos,
							"'%s %s' - ��� �� ��������; ������������ ���������� ����� ����������",
							lxm.GetBuf().c_str(), 
							ParserUtils::PrintPackageTree((NodePackage *)
								typeSpecList->GetChildPackage(i)).c_str());

						// ������ ��� ��� void
						tempObjectContainer->baseType = 
							new TempObjectContainer::ImplicitType( BaseType::BT_VOID );
						continue;
					}
				}

				if( bt != NULL )
					tempObjectContainer->baseType = 
							new TempObjectContainer::CompoundType(*bt);					
				
			}
		}
		
		else if( ts.IsCVQualifier() )
		{
			(lxm == KWCONST ? 
				tempObjectContainer->constQual : tempObjectContainer->volatileQual) = true;
		}

		else if( ts.IsSignModifier() || ts.IsSizeModifier() )
		{
			if( tempObjectContainer->baseType == NULL )
			{
				TempObjectContainer::ImplicitType *bt = new TempObjectContainer::ImplicitType;
				if( ts.IsSignModifier() )
					bt->signMod = ts.CodeToSignModifier();
				else
					bt->sizeMod = ts.CodeToSizeModifier();
				tempObjectContainer->baseType = bt;
			}

			else if( tempObjectContainer->baseType->IsImplicit() )
			{
				TempObjectContainer::ImplicitType *bt = 
					 static_cast<TempObjectContainer::ImplicitType *>
						(tempObjectContainer->baseType);

				if( ts.IsSignModifier() )
				{
					if( bt->signMod != BaseType::MN_NONE )
						error = true;
					else
						bt->signMod = ts.CodeToSignModifier();
				}

				else
				{
					if( bt->sizeMod != BaseType::MZ_NONE )
						error = true;
					else
						bt->sizeMod = ts.CodeToSizeModifier();
				}
			}

			else
				error = true;			
		}
		

		else if( ts.IsStorageSpecifier() )
		{
			// -1, ���� ��� �� �����
			if( tempObjectContainer->ssCode != -1  )
				error = true;
			else
			{
				tempObjectContainer->ssCode = lxm;
				// ��������, ���� ��������� ������� ������ �
				// ������� extern, ������ ������ ������������
				if( lxm == KWEXTERN && 
					i != typeSpecList->GetChildPackageCount()-1 &&
					typeSpecList->GetChildPackage(i+1)->GetPackageID() == STRING )
				{
					string linkSpec = ((LexemPackage *)typeSpecList->GetChildPackage(i+1))->
							GetLexem().GetBuf().c_str();
					if( linkSpec == "\"C\"" )
						tempObjectContainer->clinkSpec = true;
					else if( linkSpec == "\"C++\"" )
						tempObjectContainer->clinkSpec = false;
					else
						theApp.Error(tempObjectContainer->errPos,
							"%s - ����������� ������������ ����������",
							linkSpec.c_str());
					i++;
				} 
			}
		}

		// ������������ �������
		else if( ts.IsFunctionSpecifier() )
		{
			if( tempObjectContainer->fnSpecCode != -1 )
				error = true;
			else
				tempObjectContainer->fnSpecCode = lxm;
		}

		// ������������ ������
		else if( ts.IsFriend() )
		{
			if( tempObjectContainer->friendSpec )
				error = true;
			else
				tempObjectContainer->friendSpec = true;

		}

		else
		{
			if( ts.IsUncknown() )
				theApp.Error(lxm.GetPos(), "������������� '%s' ����������� ��� ���������� �������",
					lxm.GetBuf().c_str());
			else
				theApp.Error(lxm.GetPos(), 
					"������������� '%s %s' ����������� ��� ���������� �������",
						ts.GetGroupNameRU().c_str(), lxm.GetBuf().c_str());
		}

		// �������� ������
		if( error )
		{
			theApp.Error(lxm.GetPos(), "������������� '%s' �����������, '%s' ��� �����",
					lxm.GetBuf().c_str(), ts.GetGroupNameRU().c_str());
			errorFlag = true;		// ����������, ��� ���������� ���������� �����������
		}
	}

	return errorFlag;
}


// ����� ����� ���������� �� ������ � ������������
// �� ��������� ��������� tempObjectContainer
void MakerUtils::AnalyzeDeclaratorPkg( const NodePackage *declarator, 
					TempObjectContainer *tempObjectContainer )
{	
	for( int i = 0; i<declarator->GetChildPackageCount(); i++ )
	{		
		// ��� ����������, �.�. � ������������ TempObjectContainer
		// ��� ��� ������������
		if( declarator->GetChildPackage(i)->GetPackageID() == PC_QUALIFIED_NAME )
			continue;

		else if( declarator->GetChildPackage(i)->GetPackageID() == '*' )
		{
			bool c = false, v = false;

			// ���� �� ���������, �������� cv-�������������
		check_cv:
			if( i != declarator->GetChildPackageCount()-1 )			
				if( declarator->GetChildPackage(i+1)->GetPackageID() == KWCONST )
				{
					c = true, i++;
					goto check_cv;
				}

				else if( declarator->GetChildPackage(i+1)->GetPackageID() == KWVOLATILE )
				{
					v = true, i++;
					goto check_cv;
				}				
			
			// ������� � ��������� ���������
			tempObjectContainer->dtl.AddDerivedType( new Pointer(c,v) );
		}

		else if( declarator->GetChildPackage(i)->GetPackageID() == '&' )
			tempObjectContainer->dtl.AddDerivedType( new Reference );

		else if( declarator->GetChildPackage(i)->GetPackageID() == PC_FUNCTION_PROTOTYPE )
		{
			FunctionPrototypeMaker fpm( *(NodePackage *)declarator->GetChildPackage(i));
			FunctionPrototype *fp = fpm.Make();
			INTERNAL_IF( fp == NULL );
			tempObjectContainer->dtl.AddDerivedType( fp );
		}

		else if( declarator->GetChildPackage(i)->GetPackageID() == PC_POINTER_TO_MEMBER )
		{
			if( PointerToMember *ptm = MakePointerToMember( 
					*(NodePackage *)declarator->GetChildPackage(i)) )
				tempObjectContainer->dtl.AddDerivedType( ptm );	
			
		}

		else if( declarator->GetChildPackage(i)->GetPackageID() == PC_ARRAY )
		{
			const NodePackage &ar = *static_cast<const NodePackage *>(
												declarator->GetChildPackage(i));

			INTERNAL_IF( ar.GetChildPackageCount() < 2 || ar.GetChildPackageCount() > 3 );

			// ��� �������� ������ - '[' � ']'
			if( ar.GetChildPackageCount() == 2 )			
				tempObjectContainer->dtl.AddDerivedType( new Array );
			
			// ����� ����������� ���������
			else
			{
				INTERNAL_IF( ar.GetChildPackage(0)->GetPackageID() != '[' ||
					ar.GetChildPackage(2)->GetPackageID() != ']' );
				INTERNAL_IF( !ar.GetChildPackage(1)->IsExpressionPackage() );
				const POperand &exp = static_cast<const ExpressionPackage *>(
					ar.GetChildPackage(1))->GetExpression();

				// ��������� ������ ���� ����� � �����������
				double arSize;
				if( ExpressionMakerUtils::IsInterpretable(exp, arSize) &&
					exp->GetType().GetBaseType().GetBaseTypeCode() != BaseType::BT_FLOAT &&
					exp->GetType().GetBaseType().GetBaseTypeCode() != BaseType::BT_DOUBLE )
					tempObjectContainer->dtl.AddDerivedType( new Array(arSize) );

				// ����� ������� ��� �������
				else
					tempObjectContainer->dtl.AddDerivedType( new Array );
			}
		}		

		else
			INTERNAL( "'AnalyzeDeclaratorPkg' ������� ������������ ������� �����");
	}
}


// �������� ������� ��� ���������� �������: 1. ���������� ���� �� ������� ���,
// � ���� ��� �� �������� ��� �� ���������, 2. ���� ������� ��� ����� ���
// ������� ���� typedef, ������������� 
void MakerUtils::SpecifyBaseType( TempObjectContainer *tempObjectContainer )
{
	// ������� ��� �� �����, ������� ���������� ������� ��� �� ���������
	if( tempObjectContainer->baseType == NULL )
	{
		theApp.Warning(tempObjectContainer->errPos, 
			"'%s' - �������� ������� ���",
			tempObjectContainer->name.c_str() );

		tempObjectContainer->finalType = (BaseType *)
			&ImplicitTypeManager(BaseType::BT_INT).GetImplicitType();
	}

	// ���� ������� ��� ������, �� �� �����, ������ ��� ��� int
	else if( tempObjectContainer->baseType->IsImplicit() )
	{ 
		TempObjectContainer::ImplicitType *it = (TempObjectContainer::ImplicitType *)
				tempObjectContainer->baseType;

		if( it->baseTypeCode == BaseType::BT_NONE )
			it->baseTypeCode = BaseType::BT_INT;
		
		// ������� �������� �������������
		// ��������� ����������� ����� 
		if( it->signMod != BaseType::MN_NONE && it->baseTypeCode != BaseType::BT_CHAR &&
			it->baseTypeCode != BaseType::BT_INT )
		{
			theApp.Error(tempObjectContainer->errPos, 
				"'����������� ����� %s' ����� �������������� ������ � ����� 'char', ���� 'int'",
				it->signMod == BaseType::MN_SIGNED ? "signed" : "unsigned");
			it->signMod = BaseType::MN_NONE ;
		}

		// ��������� ����������� �������
		if( it->sizeMod == BaseType::MZ_SHORT && it->baseTypeCode != BaseType::BT_INT )
		{
			theApp.Error( tempObjectContainer->errPos,
				"'����������� ������� short' ����� �������������� ������ � ����� 'int'"),
			it->sizeMod =  BaseType::MZ_NONE;
		}

		if( it->sizeMod == BaseType::MZ_LONG && 
			it->baseTypeCode !=  BaseType::BT_INT &&
			it->baseTypeCode !=  BaseType::BT_DOUBLE )
		{
			theApp.Error(tempObjectContainer->errPos, 
				"'����������� ������� long' ����� �������������� ������ "
				"� ����� 'int', ���� 'double'");
			it->sizeMod =  BaseType::MZ_NONE;
		}

		BaseType *tmp = (BaseType *)
			&ImplicitTypeManager(it->baseTypeCode, it->sizeMod, it->signMod).GetImplicitType();
		delete tempObjectContainer->baseType;
		tempObjectContainer->finalType = tmp;
	}

	// ���� ������� ��� ����� ��� typedef, ������ ��������� ��� ���������
	else if( tempObjectContainer->baseType->IsSynonym() )
	{
		const ::Object &tname = ((TempObjectContainer::SynonymType *)
			tempObjectContainer->baseType)->GetTypedefName();

		INTERNAL_IF( tname.GetStorageSpecifier() != ::Object::SS_TYPEDEF );
		
		// ������������ ������� ���� � ��������� ��� � ��� ���������� �� ����. ���������:
		// 1. ������������ ������ ����������� ����� �� obj, � ����� ����������������� ����
		// 2. � ������ ����������� ��� obj, ����� ��� '*', 'X::*', '()' ���������
		// cv-�������������, ���� ��� � ��� ����. ��������:  T* = U, ����� const U=T *const
		// �����. ��� ���������� ������������ ��������� �� ����������		
		// 3. cv-������������� � ������� ��� ��������� � �������. ����
		
		// ���� ������� ����������� ���� � ��������
		if( tname.GetDerivedTypeList().GetDerivedTypeCount() > 0 )
		{
			bool was_qual = false;
			was_qual = tempObjectContainer->dtl.AddDerivedTypeListCV(tname.GetDerivedTypeList(),
				tempObjectContainer->constQual, tempObjectContainer->volatileQual);
		
			if( was_qual )
				tempObjectContainer->constQual = tempObjectContainer->volatileQual = false;
		}

		// ���������� ������� ��� � ���������, ������ ������ �������
		delete tempObjectContainer->baseType;
		bool c = tempObjectContainer->constQual || tname.IsConst(),
			 v = tempObjectContainer->volatileQual || tname.IsVolatile();
		tempObjectContainer->constQual = c;
		tempObjectContainer->volatileQual = v;
		tempObjectContainer->finalType = (BaseType*)&tname.GetBaseType();
	}

	else if( tempObjectContainer->baseType->IsCompound() )
	{
		tempObjectContainer->finalType = (BaseType *)
			((TempObjectContainer::CompoundType *)tempObjectContainer->baseType)->GetBaseType();
		INTERNAL_IF( tempObjectContainer->finalType == NULL );
	}

	else
		INTERNAL( "����������� ������������� �������� ����");

	// � ��������� ������� ���������, ���� ����� ������������ ����������, ��
	// extern �� �����, ������, �������������� ��������
	if( tempObjectContainer->clinkSpec )
	{
		if( tempObjectContainer->ssCode != -1 &&
			tempObjectContainer->ssCode != KWEXTERN )
			theApp.Error(tempObjectContainer->errPos,
				"������������� '%s' �����������, ������������ �������� ��� �����",
				GetKeywordName(tempObjectContainer->ssCode) );
	}
}


// ��������� � ������� �������������� �������� ������, ����
// ������� ����� �� ������, ���� �� �� ��������, ������������ 0
PBaseClassCharacteristic MakerUtils::MakeBaseClass( const NodePackage *bc, bool defaultIsPrivate)
{
	// ����� ������ ����� ��������� PC_BASE_CLASS � ��������� �� 1
	// �� 3 �������� �������. ������ ��� - ������������ ������� � virtual.
	// ��������� ������ ��� ������.
	INTERNAL_IF( bc->GetPackageID() != PC_BASE_CLASS || bc->GetChildPackageCount() < 1 ||
		bc->GetChildPackageCount() > 3 );

	NodePackage *cnam = (NodePackage *)bc->GetLastChildPackage();
	INTERNAL_IF( cnam->GetPackageID() != PC_QUALIFIED_NAME );

	ClassMember::AS as = defaultIsPrivate ? ClassMember::AS_PRIVATE : ClassMember::AS_PUBLIC;
	bool vd = false;

	// ���� ������� ������������ ������� ��� virtual
	if( bc->GetChildPackageCount() > 1 )
	{
	for( int i = 0; i < bc->GetChildPackageCount()-1; i++ )
	{
		const Lexem &lxm = ((LexemPackage *)bc->GetChildPackage(i))->GetLexem();
		if( lxm == KWVIRTUAL )
			vd = true;

		else if( lxm == KWPUBLIC )
			as = ClassMember::AS_PUBLIC;

		else if( lxm == KWPROTECTED )
			as = ClassMember::AS_PROTECTED;

		else if( lxm == KWPRIVATE )
			as = ClassMember::AS_PRIVATE;
		
		else
			INTERNAL( "'MakerUtils::MakeBaseClass' ������� ������������ �����" );
	}
	}

	QualifiedNameManager qnm(cnam);
	AmbiguityChecker achk(qnm.GetRoleList(), ParserUtils::GetPackagePosition(cnam), true);
	const ClassType *cls = NULL;
		
	cls = achk.IsClassType(true);			
	if( cls == NULL )		// ���� ��� �� �������� �������, �������� ��� ��� typedef	
		if( const ::Object *id = achk.IsTypedef() )
			cls = CheckerUtils::TypedefIsClass(*id);

	if( cls != NULL )
	{
		// ���������, ����� �� ����� ������������ ��� ������� �
		// ���� ����� - ����������
		Position epos = ParserUtils::GetPackagePosition(cnam);
		if( CheckerUtils::BaseClassChecker(*cls, qnm.GetQualifierList(), epos,
				ParserUtils::PrintPackageTree(cnam).c_str()) )
		{
			// ��������� �� ����������� � ���������� �������������� �������� ������
			CheckerUtils::CheckAccess(qnm, *cls, epos);
			return new BaseClassCharacteristic(vd, as, *cls);
		}

		// ����� ����� �� ����� ���� ������� ������� � �� ���������� 0
		else
			return NULL;
	}

	// ����� ����� �� ������
	else
	{
		theApp.Error( ParserUtils::GetPackagePosition(cnam),
			"'%s' - ������� ����� �� ������", ParserUtils::PrintPackageTree(cnam).c_str());
		return NULL;
	}
}


// ������������� ����� � ������������� ���������� � ���������
//  ��� �� ��������� ���������
void MakerUtils::AnalyzeOverloadOperatorPkg( const NodePackage &op, 
											TempOverloadOperatorContainer &tooc )
{
	INTERNAL_IF( op.GetPackageID() != PC_OVERLOAD_OPERATOR );

	// ������ ������: KWOPERATOR opLxm1 [opLxm2]
	INTERNAL_IF( op.GetChildPackageCount() < 2 );
	INTERNAL_IF( op.GetChildPackage(0)->GetPackageID() != KWOPERATOR );
	INTERNAL_IF( !op.GetChildPackage(1)->IsLexemPackage() );

	// ���� �������� ������� �� ����� �������
	if( op.GetChildPackageCount() == 2 )
	{			
		// ���������� ��������� � �����		
		Lexem lxm = static_cast<const LexemPackage *>(op.GetChildPackage(1))->GetLexem();
		tooc.opCode = lxm.GetCode();
		tooc.opString = lxm.GetBuf();
		tooc.opFullName = (string("operator ") + lxm.GetBuf().c_str()).c_str();
	}

	// ����� ���� �������� ������� �� ���� ������ (������ '[]' � '()')
	else if( op.GetChildPackageCount() == 3 )
	{		
		Lexem lxm = static_cast<const LexemPackage *>(op.GetChildPackage(1))->GetLexem();
		INTERNAL_IF( lxm != '(' && lxm != '[' );

		tooc.opCode = lxm == '(' ? OC_FUNCTION : OC_ARRAY;
		tooc.opString = lxm == '(' ? "()" : "[]";
		tooc.opFullName = (string("operator ") + tooc.opString.c_str()).c_str();
	}

	// ����� ���� �������� �� 3 ������ (������ 'new[]' � 'delete[]')
	else if( op.GetChildPackageCount() == 4 )
	{
		Lexem kwlxm = static_cast<const LexemPackage *>(op.GetChildPackage(1))->GetLexem();
		

		INTERNAL_IF( kwlxm  != KWNEW && kwlxm != KWDELETE );
		INTERNAL_IF( op.GetChildPackage(2)->GetPackageID() != '[' ||
					 op.GetChildPackage(3)->GetPackageID() != ']' );

		tooc.opCode = kwlxm == KWNEW ? OC_NEW_ARRAY : OC_DELETE_ARRAY;
		tooc.opString = kwlxm == KWNEW ? "new[]" : "delete[]";
		tooc.opFullName = (string("operator ") + tooc.opString.c_str()).c_str();

	}	

	// ����� ���������� ������
	else
		INTERNAL( "'MakerUtils::AnalyzeOverloadOperatorPkg' ��������� ������������ �����");
}


// ������������� ����� � ���������� ���������� � ���������
// ��� �� ��������� ���������
void MakerUtils::AnalyzeCastOperatorPkg( const NodePackage &op, 
							TempCastOperatorContainer &tcoc  )
{
	INTERNAL_IF( op.GetPackageID() != PC_CAST_OPERATOR );

	// ������ ������: KWOPERATOR PC_CAST_TYPE
	INTERNAL_IF( op.GetChildPackageCount() != 2 );
	INTERNAL_IF( op.GetChildPackage(0)->GetPackageID() != KWOPERATOR || 
				 op.GetChildPackage(1)->GetPackageID() != PC_CAST_TYPE );

	const NodePackage &ct = *static_cast<const NodePackage *>(op.GetChildPackage(1));
	INTERNAL_IF( ct.IsNoChildPackages() );

	// ������� ��������� ��������� ��� ���� ����������
	TempObjectContainer toc( ParserUtils::GetPackagePosition(&ct), 
		CharString("<�������� ����������>") );

	// �������� ������ �������������� ����
	AnalyzeTypeSpecifierPkg( ((NodePackage *)ct.GetChildPackage(0)), &toc );

	// �����, ����������� ����������
	AnalyzeDeclaratorPkg( ((NodePackage *)ct.GetChildPackage(1)), &toc );
	
	// �������� ������� ���
	SpecifyBaseType( &toc );

	// ���������, ����� �� �������������� �� ������ �������������
	if( toc.ssCode != -1 || toc.fnSpecCode != -1 || toc.friendSpec )
		theApp.Error(toc.errPos, "'%s' ����������� � ��������� ���������� ��������� ����������",
			toc.friendSpec ? "friend" :	
				GetKeywordName(toc.ssCode < 0 ? toc.fnSpecCode : toc.ssCode));

	// ����� ���� ��������, ��� �����. ��������� ��������� ���������
	tcoc.opCode = OC_CAST;
	tcoc.castType = new TypyziedEntity(toc.finalType, toc.constQual, toc.volatileQual, toc.dtl); 
	tcoc.opString = tcoc.castType->GetTypyziedEntityName();
	tcoc.opFullName = (string("operator ") + tcoc.opString.c_str()).c_str();
}


// ������� ����� ������� ���������, ���� ��� �� �������, ���������
// �� � ���� �������� ���������, � ������ ����������� ��������.
// � ��������� ���������� ����� � ������, ������� ����� ���� NULL,
// ���� ��������� ���������� ��
bool MakerUtils::MakeNamepsaceDeclRegion( const NodePackage *nn )
{	
	// ���� ��� �����, �������� � ���������� �������� ���������
	if( nn == NULL )
	{			
		CharString tun = theApp.GetTranslationUnit().GetShortFileName();
		if( unsigned p = tun.find(".") )
			tun.erase(p, tun.size());
		CharString tmpName = ( ("__" + CharString(tun) + "_namespace").c_str() );		

		// ������� �������� ����� ��� ������� ���������, �������� ��� ���
		// ����������
		NameManager nm(tmpName, &GetCurrentSymbolTable(), false);
		INTERNAL_IF( nm.GetRoleCount() > 1 );

		// ������� ��������� �������
		if( nm.GetRoleCount() == 1 )
		{
			INTERNAL_IF( nm.GetRoleList().front().second != R_NAMESPACE );
				// ������ �� �������
			GetScopeSystem().MakeNewSymbolTable((NameSpace*)nm.GetRoleList().front().first);
			return true;
		}

		// ����� �������
		NameSpace *ns = new NameSpace(tmpName, &GetCurrentSymbolTable(), true);

		// ���������
		INTERNAL_IF( !GetCurrentSymbolTable().InsertSymbol(ns) );

		// ������ ��������� �� ��������� � �������, ����� ��� ����� 
		// ������ � ������ �� ����������
		GeneralSymbolTable *gst = dynamic_cast<GeneralSymbolTable *>(&GetCurrentSymbolTable());
		INTERNAL_IF( gst == NULL );
		gst->AddUsingNamespace(ns);

		// ��������� � �����, ����� �� �������
		GetScopeSystem().MakeNewSymbolTable(ns);
		return true;		// �������
	}

	INTERNAL_IF( nn->GetPackageID() != PC_QUALIFIED_NAME );

	// � ��������� ������, ������� ����������� ������� ���������
	Position ep = ParserUtils::GetPackagePosition(nn);
	CharString nam = ParserUtils::PrintPackageTree(nn);
	QualifiedNameManager qnm(nn, &GetCurrentSymbolTable());

	// ���� ������� �������� ����
	if( qnm.GetRoleCount() > 1 )
	{
		theApp.Error(ep, "'%s' - ��� ������� ��������� ������ ���� ����������",
			nam.c_str());
		return false;
	}

	// ���� ������� ���� ����, ��� ������ ���� ����������� ������ ������� ���������
	else if( qnm.GetRoleCount() == 1 )
	{
		RolePair r = qnm.GetRoleList().front();
		if( r.second != R_NAMESPACE )
		{
			theApp.Error(ep, "'%s' - ������������� �� �������� ����������� �������� ���������",
				nam.c_str());
			return false;
		}

		// �����, ��� �������� �������� ��������� � ��� ������� ��������� ��� � �����
		else
		{
			NameSpace *ns = dynamic_cast<NameSpace *>(r.first);
			GetScopeSystem().MakeNewSymbolTable(ns);
			return true;
		}	
	}

	// � ��������� ������, � ����� ��� �����. ��� ������� ���������, 
	// ���� ��� �������� ���������, �� ��� ������, ����� ������� ������� ���������
	else
	{
		if( nn->GetChildPackageCount() > 1 )
		{
			theApp.Error(ep, "'%s' - ������������� �� �������� ����������� �������� ���������",
				nam.c_str());
			return false;
		}

		// � ��������� ������, ������� ������� ���������
		INTERNAL_IF( nn->GetChildPackage(0)->GetPackageID() != NAME );
		NameSpace *ns = new NameSpace(nam, &GetCurrentSymbolTable(), false);

		// ���������
		INTERNAL_IF( !GetCurrentSymbolTable().InsertSymbol(ns) );

		// ��������� � �����, ����� �� �������
		GetScopeSystem().MakeNewSymbolTable(ns);
		return true;
	}
}


// ���������, ������� � �������� � ������� ������� ������� ���������
// � ���������� ����������� ��� �������� � ��� ������� ��������� ���
// ������� ������� ���������
void MakerUtils::MakeNamespaceAlias( const NodePackage *al, const NodePackage *ns )
{
	// 1. al - ������ ���� ������������������� ������
	// 2. ns � al ������ ����� ��� PC_QUALIFIED_NAME
	// 3. ns - ������ ���� ����������� �������� ���������
	// 4. al - ������ ���� ���������� ������ � ����� ������� ���������.
	//	  ���� ��� ��� �������, ��������� ����� ��� ���� �������� ��������� 
	//	  � ��������� ��������� ������� ��������� ns
	INTERNAL_IF( al == NULL || ns == NULL || al->GetPackageID() != PC_QUALIFIED_NAME ||
		ns->GetPackageID() != PC_QUALIFIED_NAME );

	Position alPos = ParserUtils::GetPackagePosition(al), 
			 nsPos = ParserUtils::GetPackagePosition(ns);

	INTERNAL_IF( al->GetChildPackageCount() != 1 || 
				 al->GetChildPackage(0)->GetPackageID() != NAME );
	
	QualifiedNameManager qnm(ns);
	AmbiguityChecker achk(qnm.GetRoleList(), alPos, true);

	// ���� ��� ���������� ������� ���������, �� ���������� ��������
	if( const NameSpace *tns = achk.IsNameSpace() )
	{
		CharString name = ParserUtils::PrintPackageTree(al);
		NameManager nm(name, &GetCurrentSymbolTable(), false); 
		if( nm.GetRoleCount() > 1 || 
			(nm.GetRoleCount() == 1 && nm.GetRoleList().front().second != R_NAMESPACE) )
		{
			theApp.Error(alPos, 
				"'%s' - ������������� �� �������� ����������� �������� ���������",
				name.c_str());
			return;
		}

		// ����� ���� ��� ��� �� �������, ������� � ��������� � �������
		if( nm.GetRoleCount() == 0 )
		{
			NameSpaceAlias *nsa = new NameSpaceAlias(name, &GetCurrentSymbolTable(), *tns);
			INTERNAL_IF( !GetCurrentSymbolTable().InsertSymbol(nsa) );
		}

		// ����� ��� ��� �������� �������� ���������, ��������, ����� ���
		// �����. ��������. ����������
		else
		{
			if( static_cast<NameSpace *>(nm.GetRoleList().front().first) != tns )
				theApp.Error(alPos, 
				"'%s' - ���������� �������� ������� ��������� �� ��������� � ����������", 
					name.c_str());
		}
	}

	// ����� ������� ������ � �������
	else
	{
		theApp.Error(alPos, "'%s' - ������� ��������� �� �������", 
			ParserUtils::PrintPackageTree(ns).c_str());
		return;
	}
}


// ������� � ��������� ���������� ��������� ������� ���������
void MakerUtils::MakeUsingNamespace( const NodePackage *ns )
{
	INTERNAL_IF( ns == NULL || ns->GetPackageID() != PC_QUALIFIED_NAME ||
		ns->IsNoChildPackages() );

	INTERNAL_IF( GetCurrentSymbolTable().IsClassSymbolTable() );

	QualifiedNameManager qnm(ns); 
	CharString name = ParserUtils::PrintPackageTree(ns);
	Position pos = ParserUtils::GetPackagePosition(ns);

	if( qnm.GetRoleCount() > 1 ||
		(qnm.GetRoleCount() == 1 && qnm.GetRoleList().front().second != R_NAMESPACE) )
		{
			theApp.Error(pos, 
				"'%s' - ������������� �� �������� ����������� �������� ���������",
				name.c_str());
			return;
		}

	// ����� ���� ����� ������ ���
	if( qnm.GetRoleCount() == 0 )
	{
		theApp.Error(pos, 
				"'%s' - ������� ��������� �� �������",
				name.c_str());
		return;		
	}

	//� ���� ����� ����� ������������ ������� ���������
	NameSpace *tns = dynamic_cast< NameSpace *>(qnm.GetRoleList().front().first);
	INTERNAL_IF( tns == NULL );

	// ���� ������� ������� ��������� ��������������, ��������� � ���,	
	if( GetCurrentSymbolTable().IsFunctionSymbolTable() )
	{
		FunctionSymbolTable &fst = static_cast<FunctionSymbolTable &>(GetCurrentSymbolTable());
		fst.AddUsingNamespace(tns);
	}

	// ����� � �����������
	else
	{
		GeneralSymbolTable *gst = dynamic_cast<GeneralSymbolTable *>(&GetCurrentSymbolTable());
		INTERNAL_IF( gst == NULL );
		gst->AddUsingNamespace(tns);
	}
}


// ������� ������������� �����
void MakerUtils::MakeFriendClass( const NodePackage *tsl )
{
	INTERNAL_IF( tsl == NULL || tsl->GetChildPackageCount() != 3 );
	INTERNAL_IF( !GetCurrentSymbolTable().IsClassSymbolTable() );

	ClassTypeMaker ctm((NodePackage *)tsl, ClassMember::NOT_CLASS_MEMBER, 
		(SymbolTable&)GetScopeSystem().GetGlobalSymbolTable());
	
	ClassType *frnd = ctm.Make(),
			  &curCls = static_cast<ClassType &>(GetCurrentSymbolTable());
	if( !frnd )
		return;

	// ���� ������ ����� ��� ���, ��������� ��� � ������ ������ �������� ������
	if( curCls.GetFriendList().FindClassFriend(frnd) < 0 )
		const_cast<ClassFriendList &>(
			curCls.GetFriendList()).AddClassFriend(ClassFriend(frnd));
}


// ������� using-���������� ����
void MakerUtils::MakeUsingMember( const NodePackage *npkg, ClassMember::AS as )
{
	INTERNAL_IF( npkg == NULL || npkg->GetPackageID() != PC_QUALIFIED_NAME ||
		npkg->IsNoChildPackages() );
	INTERNAL_IF( !GetCurrentSymbolTable().IsClassSymbolTable() ||
		as == ClassMember::NOT_CLASS_MEMBER );

	Position ep = ParserUtils::GetPackagePosition(npkg);
	CharString name = ParserUtils::PrintPackageTree(npkg);
	ClassType &cls = static_cast<ClassType &>(GetCurrentSymbolTable());

	// �� ����������������� ���
	if( npkg->GetChildPackageCount() == 1 )
	{
		theApp.Error(ep, 
			"� using-���������� ������ �������������� ������ ����������������� �����");
		return;
	}

	// ���� � ������ ��� ������� �������, �� ��������� ������ �� �����
	if( !cls.IsDerived() )
	{
		theApp.Error(ep, 
			"using-���������� ����� ���� ������ � ����������� ������");
		return;
	}

	// ���� ��� �������� ����������������� ������ 
	QualifiedNameManager qnm(npkg);
	
	// ���� ���� ���, �����
	if( qnm.GetRoleCount() == 0 )
		return;


	// ������� ������ ����� ������� ����� � ������� ������� ���������,
	// ��� �������� ���������������
	NameManager cnm(qnm.GetRoleList().front().first->GetName(), &GetCurrentSymbolTable(), false);

	// ����� ������� ���������, ����� ������ ��� ���� ������ ��������
	// ������ ������� ������ � ���� ������ ������ ������	
	for( RoleList::const_iterator p = qnm.GetRoleList().begin(); 
		p != qnm.GetRoleList().end(); p++)
	{
		Identifier *id = (*p).first;
		ClassMember *cm = dynamic_cast<ClassMember *>(id);
		if( !cm || !id->GetSymbolTableEntry().IsClassSymbolTable() ||
			cls.GetBaseClassList().HasBaseClass(
				static_cast<const ClassType *>(&id->GetSymbolTableEntry())) < 0 )
		{
			theApp.Error(ep, "'%s' - �� �������� ������ �������� ������", name.c_str());
			continue;
		}

		// ��������� ���� �� �����������
		CheckerUtils::CheckAccess(qnm, *id, ep);

		// ���� ������������� ��� �������� using-���������������, �����������
		// ��� � ������� ���
		if( UsingIdentifier *ui = dynamic_cast<UsingIdentifier *>(id) )
			id = const_cast<Identifier *>(&ui->GetUsingIdentifier());

		Role idr = NameManager::GetIdentifierRole(id);
		if( idr == R_CONSTRUCTOR )
		{
			theApp.Error(ep, 
				"'%s' - ����������� �� ����� ����������� � using-����������", name.c_str());
			break;
		}
	
		// ���������, �������������� �� ������������� �����-���� ����
		// ��� ����������� � ������
		if( TypyziedEntity *te = dynamic_cast<TypyziedEntity *>(id) )
		{
			RedeclaredChecker rchk(*te, cnm.GetRoleList(), ep, idr);				

			if( !rchk.IsRedeclared() )			
				GetCurrentSymbolTable().InsertSymbol( new UsingIdentifier(id->GetName(),
					&GetCurrentSymbolTable(), id, as) );
		}

		// ����� ��� ����� ���� ����� ��� ������������ 
		else
		{
			if( cnm.GetRoleCount() != 0 )
				theApp.Error(ep, "'%s' - �������������", id->GetName().c_str());
		}
	}
}


// ������� using-���������� �� ����
void MakerUtils::MakeUsingNotMember( const NodePackage *npkg )
{
	INTERNAL_IF( npkg == NULL || npkg->GetPackageID() != PC_QUALIFIED_NAME ||
		npkg->IsNoChildPackages() );
	INTERNAL_IF( GetCurrentSymbolTable().IsClassSymbolTable() );

	Position ep = ParserUtils::GetPackagePosition(npkg);
	CharString name = ParserUtils::PrintPackageTree(npkg);

	// �� ����������������� ���
	if( npkg->GetChildPackageCount() == 1 )
	{
		theApp.Error(ep, 
			"� using-���������� ������ �������������� ������ ����������������� �����");
		return;
	}

		// ���� ��� �������� ����������������� ������ 
	QualifiedNameManager qnm(npkg);
	
	// ���� ���� ���, �����
	if( qnm.GetRoleCount() == 0 )
		return;

	// ����� ������� ���������, ����� ������ ��� ���� ������ ��������
	// ������ ������� ������ � ���� ������ ������ ������	
	NameManager cnm( qnm.GetRoleList().front().first->GetName(), &GetCurrentSymbolTable(),false);
	for( RoleList::const_iterator p = qnm.GetRoleList().begin(); 
		p != qnm.GetRoleList().end(); p++)
	{
		Identifier *id = (*p).first;
		ClassMember *cm = dynamic_cast<ClassMember *>(id);
		if( !cm || cm->GetAccessSpecifier() != ClassMember::NOT_CLASS_MEMBER )
		{
			theApp.Error(ep, "'%s' - �� �������� ������ ���������� ������� ���������", 
				name.c_str());
			continue;
		}

		// ���� ������������� ��� �������� using-���������������, �����������
		// ��� � ������� ���
		if( UsingIdentifier *ui = dynamic_cast<UsingIdentifier *>(id) )
			id = const_cast<Identifier *>(&ui->GetUsingIdentifier());

		Role idr = NameManager::GetIdentifierRole(id);
		if( idr == R_CONSTRUCTOR )
		{
			theApp.Error(ep, 
				"'%s' - ����������� �� ����� ����������� � using-����������", name.c_str());
			break;
		}
	
		// ���������, �������������� �� ������������� �����-���� ����
		// ��� ����������� � ������
		if( TypyziedEntity *te = dynamic_cast<TypyziedEntity *>(id) )
		{
			RedeclaredChecker rchk(*te, cnm.GetRoleList(), ep, idr);				

			if( !rchk.IsRedeclared() )
				GetCurrentSymbolTable().InsertSymbol( new UsingIdentifier(id->GetName(),
					&GetCurrentSymbolTable(), id, ClassMember::NOT_CLASS_MEMBER) );
		}

		// ����� ��� ����� ���� ����� ��� ������������ 
		else
		{
			if( cnm.GetRoleCount() != 0 )
				theApp.Error(ep, "'%s' - �������������", id->GetName().c_str());
		}
	}	
}

// ������� ��������� ������������ � �������� �� � ������� ������� ���������
EnumConstant *MakerUtils::MakeEnumConstant(
		const CharString &name, ClassMember::AS curAccessSpec,
		int lastVal, const Position &errPos, EnumType *enumType )
{

	// ��������� ���������������
	NameManager nm(name, &GetCurrentSymbolTable(), false);
	if( nm.GetRoleCount() > 0 )
	{
		bool type = false;
		if( nm.GetRoleCount() == 1 )
		{
			Role r = nm.GetRoleList().front().second;
			type = r == R_CLASS_TYPE || r == R_ENUM_TYPE			||
				   r == R_UNION_CLASS_TYPE || r == R_TEMPLATE_CLASS	||
				   r == R_TEMPLATE_CLASS_SPECIALIZATION;
		}

		if( !type )
		{
			theApp.Error(errPos, "'%s' - �������������", name.c_str());
			return NULL;
		}
	}

	// ���������, ����� ����� �� ���� ��� ������
	if( GetCurrentSymbolTable().IsClassSymbolTable() &&
		static_cast<const ClassType &>(GetCurrentSymbolTable()).GetName() == name )
	{
		theApp.Error(errPos,
			"'%s' - �� ����� ����� ��� ������ � ������� �����������", name.c_str());
		return NULL;
	}

	// ������ �������. ���� ������������ ������� �����, ������� ���������
	// ���������, ����� �������
	EnumConstant *ec = curAccessSpec == ClassMember::NOT_CLASS_MEMBER ?
		new EnumConstant( name, &GetCurrentSymbolTable(), lastVal, enumType) :
	    new ClassEnumConstant( name, &GetCurrentSymbolTable(), lastVal, enumType, curAccessSpec);

	// ��������� ��������� � �������
	GetCurrentSymbolTable().InsertSymbol( ec );
	return ec;
}


// ����������� ��������� �����, ��� ���������� � ��� ����� � 
// ������������� nameManager
TempObjectContainer::TempObjectContainer( const NodePackage *qualName, ClassMember::AS cas ) 
{
	INTERNAL_IF( qualName == NULL || qualName->GetPackageID() != PC_QUALIFIED_NAME );
	nameManager = new QualifiedNameManager(qualName);
	const Lexem &namLxm = 
		((LexemPackage *)qualName->GetChildPackage( qualName->GetChildPackageCount()-1 ))
			->GetLexem();

	name = namLxm.GetBuf();
	errPos = ((LexemPackage *)qualName->GetChildPackage(0))->GetLexem().GetPos();
	baseType = NULL;
	constQual = volatileQual = false;
	ssCode = -1;
	fnSpecCode = -1;
	friendSpec = false;
	curAccessSpec = cas;
	clinkSpec = theApp.GetTranslationUnit().GetParser().GetLinkSpecification() == Parser::LS_C;		
}


// ����������� ��� ��������, ������� �� ����� ��������� ���������
// �����, ���� ����� �� ����� ����� �����
TempObjectContainer::TempObjectContainer( const Position &ep, 
			const CharString &n, ClassMember::AS cas )
{
	nameManager = NULL;
	name = n;
	errPos = ep;
	baseType = NULL;
	constQual = volatileQual = false;
	ssCode = -1;
	fnSpecCode = -1;
	friendSpec = false;
	curAccessSpec = cas;
	clinkSpec = theApp.GetTranslationUnit().GetParser().GetLinkSpecification() == Parser::LS_C;		
}


// ������� ������ ����������� �����, ������� ������� ������
TempObjectContainer::~TempObjectContainer() 
{
	dtl.ClearDerivedTypeList();
	delete nameManager;
}


// � ������������ ������ ��������� ���������,������� �����
// �������������� ��� ���������� �������
GlobalObjectMaker::GlobalObjectMaker( const PTempObjectContainer &toc, bool ld ) 
	: tempObjectContainer(toc), localDeclaration(ld), ictor(NULL)
{	
	targetObject = NULL;
	redeclared = false;
}
					

// ��������� ������������ �������� ������, ������� ���������� �� ��������� ���������,
// ������� ������-���������, ���������� ������� �������� �������-���������,
// �������� ��������� � ������� � ���������� ��� ��� ��������� ������
bool GlobalObjectMaker::Make( )
{
	// ��������� ������������ ��������������� �������
	GlobalDeclarationChecker goc(*tempObjectContainer, localDeclaration);

	
	// �������� �������� �� ��� ���������� � ����� ������� ���������
	NameManager nm(tempObjectContainer->name, &GetCurrentSymbolTable(), false);

	// ������� ������ ��� ��������
	register TempObjectContainer *toc = &*tempObjectContainer;
	targetObject = new ::Object(toc->name, &GetCurrentSymbolTable(), toc->finalType,
		toc->constQual, toc->volatileQual, toc->dtl, toc->ssCode < 0 ? ::Object::SS_NONE : 
			TypeSpecifierManager(toc->ssCode).CodeToStorageSpecifierObj(), toc->clinkSpec );	

	// ��������� �� ���������������
	RedeclaredChecker rchk(*targetObject, nm.GetRoleList(), toc->errPos, R_OBJECT);

	// ������ �������������
	if( rchk.IsRedeclared() )
	{		
		delete targetObject;
		redeclared = true;
		targetObject = const_cast<::Object *>(
			dynamic_cast<const ::Object *>(rchk.GetPrevDeclaration()) );
	}

	// ���� ������ �������� � ����� ������� ���������, ������� �
	// ��������� ������ � �������
	else
		INTERNAL_IF( !GetCurrentSymbolTable().InsertSymbol(targetObject) );
	return redeclared;
}


// � ������������ ������ ��������� ���������,������� �����
// �������������� ��� ���������� �������
GlobalFunctionMaker::GlobalFunctionMaker( const PTempObjectContainer &toc )
	: tempObjectContainer(toc)
{	
	targetFn = NULL;
	errorFlag = false;
}


// ������� ������� �� ��������� ���������
bool GlobalFunctionMaker::Make()
{
	// ��������� ������������ �������������� �������
	GlobalDeclarationChecker goc(*tempObjectContainer);

		
	// �������� �������� �� ��� ���������� � ����� ������� ���������
	NameManager nm(tempObjectContainer->name, &GetCurrentSymbolTable(), false);

	// ������� ������ ��� ��������
	register TempObjectContainer *toc = &*tempObjectContainer;
	targetFn = new Function(toc->name, &GetCurrentSymbolTable(), toc->finalType,
			toc->constQual, toc->volatileQual, toc->dtl, toc->fnSpecCode == KWINLINE,
			toc->ssCode < 0 ? Function::SS_NONE : 
			TypeSpecifierManager(toc->ssCode).CodeToStorageSpecifierFn(), 
			toc->clinkSpec ? Function::CC_CDECL : Function::CC_NON);

		// ��������� �� ���������������
	RedeclaredChecker rchk(*targetFn, nm.GetRoleList(), toc->errPos, R_FUNCTION);

	// ������ �������������
	if( rchk.IsRedeclared() )
	{		
		Function *prevFn = const_cast<Function *>(
			dynamic_cast<const Function *>(rchk.GetPrevDeclaration()) );
		if( prevFn )
			CheckerUtils::DefaultArgumentCheck( targetFn->GetFunctionPrototype(), 
				&prevFn->GetFunctionPrototype(), toc->errPos);

		delete targetFn;
		targetFn = prevFn;
	}

	// ���� ������ �������� � ����� ������� ���������, ������� �
	// ��������� ������ � �������
	else
	{
		CheckerUtils::DefaultArgumentCheck( targetFn->GetFunctionPrototype(), NULL, toc->errPos);
		INTERNAL_IF( !GetCurrentSymbolTable().InsertSymbol(targetFn) );
	}

	return errorFlag;
}


// � ������������ ������ ��������� ���������,������� �����
// �������������� ��� ���������� ���������
GlobalOperatorMaker::GlobalOperatorMaker( const PTempObjectContainer &toc,
										 const TempOverloadOperatorContainer &tc )
	: tempObjectContainer(toc), tooc(tc), targetOP(NULL)
{

}


// ������� ������������� ��������
bool GlobalOperatorMaker::Make()
{
	GlobalOperatorChecker goc(*tempObjectContainer, tooc);

	// ��������� ���������������
	register TempObjectContainer *toc = &*tempObjectContainer;
	NameManager nm(toc->name, &GetCurrentSymbolTable(), true);

	// ������� ������ ��� �������� �� ������������������
	targetOP = new OverloadOperator(toc->name, &GetCurrentSymbolTable(),
		toc->finalType,	toc->constQual, toc->volatileQual, 
		toc->dtl, toc->fnSpecCode == KWINLINE,
		toc->ssCode < 0 ? Function::SS_NONE : 
		TypeSpecifierManager(toc->ssCode).CodeToStorageSpecifierFn(),
			Function::CC_NON, tooc.opCode, tooc.opString );
						
	// ��������� �� ���������������
	RedeclaredChecker rchk(*targetOP, nm.GetRoleList(), toc->errPos, R_OVERLOAD_OPERATOR );

	// ���� �������������
	if( rchk.IsRedeclared() )	
		delete targetOP, targetOP = 0 ;

	// ����� ��������� ��� � �������
	else
		INTERNAL_IF( !GetCurrentSymbolTable().InsertSymbol(targetOP) );

	return true;
}


// ����������� ��������� ����� � ���� ����
FunctionPrototypeMaker::FunctionPrototypeMaker( const NodePackage &pp, bool nn ) 
	: protoPkg(pp), noNames(nn) 
{
	INTERNAL_IF( protoPkg.GetPackageID() != PC_FUNCTION_PROTOTYPE );
	constQual = volatileQual = false;
	ellipse = false;
	canThrow = true;
}


// ������� �������� � �������� ��� � ������ �� ������
void FunctionPrototypeMaker::MakeParametr( const NodePackage &pp, int pnum )
{
	INTERNAL_IF( pp.GetPackageID() != PC_PARAMETR					   || 
		pp.GetChildPackageCount() < 1 || pp.GetChildPackageCount() > 3 || 
		pp.IsErrorChildPackages()	  || 
		pp.GetChildPackage(0)->GetPackageID() != PC_TYPE_SPECIFIER_LIST );

	// ��������, ��� � ��� ���������� ������� �� ���� ������:
	// ������ �������������� ����, ���������� � �������� �� ���������,
	// ������ 2 ��������� ����� �������������. ��� ������� ��������� 
	// ��� ������������ ���������

	Position ep;			// ������� ��� ������ ������
	CharString pname;

	const Package *pk = ((NodePackage *)pp.GetChildPackage(0))->GetChildPackage(0);
	if( pk->IsNodePackage() )	
		pk = ((NodePackage *)pk)->GetChildPackage(0);

	INTERNAL_IF( !pk->IsLexemPackage() );			
	ep = ((LexemPackage *)pk)->GetLexem().GetPos();

	if( pp.GetChildPackageCount() == 1 ||
		pp.GetChildPackage(1)->GetPackageID() != PC_DECLARATOR )
		pname = ("<�������� " + CharString(pnum) + ">").c_str();
	else
	{
		int ix = ((NodePackage *)pp.GetChildPackage(1))->FindPackage(PC_QUALIFIED_NAME);
		if( ix < 0 )
			pname = ("<�������� " + CharString(pnum) + ">").c_str();
		else
		{
			NodePackage *pn = (NodePackage *)
				((NodePackage *)pp.GetChildPackage(1))->GetChildPackage(ix);
			INTERNAL_IF( pn->GetChildPackageCount() != 1 || 
				pn->GetChildPackage(0)->GetPackageID() != NAME );

			const Lexem &lx = ((LexemPackage *)pn->GetChildPackage(0))->GetLexem();
			pname = lx.GetBuf();
			ep = lx.GetPos();
		}
	}

	// ������� ������� ��������� ���������
	TempObjectContainer toc( ep, pname );

	// �������� ������ �������������� ����
	AnalyzeTypeSpecifierPkg( ((NodePackage *)pp.GetChildPackage(0)), &toc );

	// �����, ���� ���� ����������, ����������� � ���
	if( pp.GetChildPackageCount() > 1 && 
		pp.GetChildPackage(1)->GetPackageID() == PC_DECLARATOR )
		AnalyzeDeclaratorPkg( ((NodePackage *)pp.GetChildPackage(1)), &toc );
	
	// �������� ������� ���
	SpecifyBaseType( &toc );

	// ���������, ���� ������ 3, �� ������ ������ ���� ���������� 
	// (�������� �� ���������)
	POperand defaultArg = ( pp.GetChildPackageCount() == 3 && 
		pp.GetChildPackage(2)->IsExpressionPackage()) ? const_cast<POperand&>(
		static_cast<const ExpressionPackage*>(pp.GetChildPackage(2))->GetExpression()) : NULL;

	// ��������� �������� �� ������������
	ParametrChecker pchk( toc, parametrList );

	// �������� ��� ��������� ���������� ������� ���������
	static LocalSymbolTable *uniqueSt = new LocalSymbolTable(GetCurrentSymbolTable());
	
	// ������� �������� � ��������� ��� � ������
	Parametr *param = new Parametr(
		toc.finalType, toc.constQual, toc.volatileQual, toc.dtl, toc.name,
		uniqueSt, NULL, toc.ssCode == KWREGISTER );

	// ������ �������� �� ���������, ������ ����� �������� ����� ��������
	if( !defaultArg.IsNull() )
	{
		const POperand &rda = DefaultArgumentChecker(*param, defaultArg, ep).Check();

		// ������ �������� �� ���������, � ������ ���� ����� error operand,
		// ������� ���� ����� ����������, ����� �� ������� ���
		param->SetDefaultValue( rda->IsErrorOperand() ? &const_cast<Operand&>(*rda) : 
			const_cast<POperand&>(rda).Release() );
	}

	parametrList.AddFunctionParametr( param );
}


// ������� ��� throw-������������
void FunctionPrototypeMaker::MakeThrowType( const NodePackage &tt )
{
	INTERNAL_IF( tt.GetPackageID() != PC_THROW_TYPE || 
		tt.GetChildPackageCount() != 2				||
		tt.GetChildPackage(0)->GetPackageID() != PC_TYPE_SPECIFIER_LIST ||
		tt.GetChildPackage(1)->GetPackageID() != PC_DECLARATOR );

	Position ep = ParserUtils::GetPackagePosition(&tt);
	CharString tname = "<��� throw-������������>";


	// ������� ������� ��������� ���������
	TempObjectContainer toc( ep, tname );

	// �������� ������ �������������� ����
	AnalyzeTypeSpecifierPkg( ((NodePackage *)tt.GetChildPackage(0)), &toc );

	// �����, ����������� ����������
	AnalyzeDeclaratorPkg( ((NodePackage *)tt.GetChildPackage(1)), &toc );
	
	// �������� ������� ���
	SpecifyBaseType( &toc );


	// ������ ��������� ���
	ThrowTypeChecker ttc(toc);

	// ��������� ��� � ������
	throwTypeList.AddThrowType( PTypyziedEntity(
		new TypyziedEntity(toc.finalType, toc.constQual,toc.volatileQual, toc.dtl)) );
}


// ������� ������ throw-������������
void FunctionPrototypeMaker::MakeThrowSpecification( const NodePackage &ts )
{
	// ����� ���� ������ ������. � ����� ������ ������ ������ ��������� 
	// ( )
	INTERNAL_IF( ts.GetPackageID() != PC_THROW_TYPE_LIST ||
		ts.GetChildPackageCount() < 2 || ts.GetChildPackage(0)->GetPackageID() != '(' );

	// ���������, ���� ������ ����� - ��� ')', ������ � ������� ������ ������
	// ������������
	if( ts.GetChildPackageCount() == 2 && ts.GetChildPackage(1)->GetPackageID() == ')' )
	{
		canThrow = false;
		return;
	}

	// ����� ������������ ������ throw-�����
	for( int i = 1; ; i++ )
	{
		// ���� ����� ��������� �����
		if( i+1 == ts.GetChildPackageCount() )
		{
			INTERNAL_IF( ts.GetChildPackage(i)->GetPackageID() != ')' );
			break;
		}

		// ����� ������ ���� ������ throw-���
		INTERNAL_IF( ts.GetChildPackage(i)->GetPackageID() != PC_THROW_TYPE );
		MakeThrowType( *static_cast<const NodePackage *>(ts.GetChildPackage(i)) );
	}
}


// ����� ��������� �������� ������� �� ������ � ������������ ���
// � �������� ���������� ������ �������
FunctionPrototype *FunctionPrototypeMaker::Make()
{
	// �������� ������ ������ ���� ��� ������� ��� - '(' ')'
	INTERNAL_IF( protoPkg.GetChildPackageCount() < 2 || 
		protoPkg.GetChildPackage(0)->GetPackageID() != '(' ); 

	// ����� ���������, ���������� �� ��� ��������� ������ ����������,
	// ��������� �� ����� ����: ��� ���������� ����� ��������� '(' � ')',
	// ����� '(' � ')', ������ ������� ELLIPSES, ����� '(' � ')' ����
	// ��������, ������� �������� ������ ������ �������������� ����,
	// ������� �������� ������ void 

	int i = 1;
	if( protoPkg.GetChildPackage(1)->GetPackageID() == ')'	)
		goto skip_make_parametrs;

	else if( protoPkg.GetChildPackage(1)->GetPackageID() == ELLIPSES )
	{
		ellipse = true;
		i = 2;
		goto skip_make_parametrs;
	}

	// ����� 1 ��������, ���� ��� ������ void, ������ ��� �����������
	// ������ ������ ����������
	else if( protoPkg.GetChildPackage(1)->GetPackageID() == PC_PARAMETR &&
		 	 protoPkg.GetChildPackage(2)->GetPackageID() == ')' )		
	{
		NodePackage *np = ((NodePackage *)protoPkg.GetChildPackage(1));	// �������� ��������
	
		INTERNAL_IF( np->GetChildPackage(0)->GetPackageID() != PC_TYPE_SPECIFIER_LIST );
		NodePackage *tsl = (NodePackage *)np->GetChildPackage(0),
				*decl = np->IsNoChildPackages() ? 0 : (NodePackage *)np->GetChildPackage(1);			

		// � ������ �������������� ���� ������ ���� ������ void,
		// � ���������� ������ ���� ������
		if( tsl->GetChildPackageCount() == 1 && 
			tsl->GetChildPackage(0)->GetPackageID() == KWVOID &&
			(decl == NULL || decl->IsNoChildPackages()) )
			{
				i = 2;
				goto skip_make_parametrs;
			}
	}

	// � ��������� ������ ������������ ��� ��������� � ��������� ��
	// � ������	
	for( i = 1; ; i++ )
	{
		const Package *p = protoPkg.GetChildPackage(i);
		if( p->GetPackageID() == ELLIPSES )
		{
			INTERNAL_IF( protoPkg.GetChildPackage(++i)->GetPackageID() != ')' );
			ellipse = true;
			break;
		}

		else if( p->GetPackageID() == ')' )
			break;

		else
		{
			INTERNAL_IF( p->GetPackageID() != PC_PARAMETR );
			MakeParametr( *(NodePackage *)p , i);
		}
	}		
	
skip_make_parametrs:

	// i, ������ �������� ������ � ')'
	INTERNAL_IF( protoPkg.GetChildPackage(i)->GetPackageID() != ')' );
		
	// �������� ���������� cv-������������� � throw-������������
	i++;
	for( ; i<protoPkg.GetChildPackageCount(); i++ )
	{
		if( protoPkg.GetChildPackage(i)->GetPackageID() == KWCONST )
			constQual = true;

		else if( protoPkg.GetChildPackage(i)->GetPackageID() == KWVOLATILE )
			volatileQual = true;

		// ������������ throw-������������, ��� ������ ���� ��������� � ������
		else if( protoPkg.GetChildPackage(i)->GetPackageID() == PC_THROW_TYPE_LIST )
		{
			INTERNAL_IF( protoPkg.GetChildPackageCount() != i + 1 );
			MakeThrowSpecification(
				*static_cast<const NodePackage *>(protoPkg.GetChildPackage(i)) );
		}
	}

	return new FunctionPrototype( constQual, volatileQual, parametrList,
		throwTypeList, canThrow, ellipse);
}


// ��������� catch-����������, ������� ������
::Object *CatchDeclarationMaker::Make()
{
	INTERNAL_IF( typeSpec.GetPackageID() != PC_TYPE_SPECIFIER_LIST ||
		declPkg.GetPackageID() != PC_DECLARATOR );

	static int noNameCnt = 1;
	int ix = declPkg.FindPackage(PC_QUALIFIED_NAME);
	CharString name;
	if( ix < 0 )	
		name = (string("<catch-����������") + CharString(noNameCnt++).c_str() + ">").c_str();
	else
		name = ParserUtils::PrintPackageTree((NodePackage*)declPkg.GetChildPackage(ix) );
		
	TempObjectContainer toc( errPos, name );
			
	// �������� ������ �������������� ����
	MakerUtils::AnalyzeTypeSpecifierPkg( &typeSpec, &toc );

	// ����� ����������� ����������
	MakerUtils::AnalyzeDeclaratorPkg( &declPkg, &toc );
	
	// �������� ������� ���
	MakerUtils::SpecifyBaseType( &toc );

	// ������ ��������� ���
	CatchDeclarationChecker cdc(toc);

	// ���������� ������
	::Object *targetObject = new ::Object(toc.name, &GetCurrentSymbolTable(), toc.finalType,
		toc.constQual, toc.volatileQual, toc.dtl, toc.ssCode < 0 ? ::Object::SS_NONE : 
		TypeSpecifierManager(toc.ssCode).CodeToStorageSpecifierObj(), toc.clinkSpec );

	// ��������� ������ � ������� ������ ���� � ���� ���� ���
	if( ix >= 0 )
		GetCurrentSymbolTable().InsertSymbol(targetObject);
	return targetObject; 
}


//  �-��� ��������� ����� � ��������� ��� ������������
ClassTypeMaker::ClassTypeMaker( 
	NodePackage *np, ClassMember::AS a, SymbolTable &d, bool def )
	: typePkg(np), as(a), resultCls(NULL), destST(d), defination(def)
{ 		
	INTERNAL_IF( np == NULL || np->GetPackageID() != PC_TYPE_SPECIFIER_LIST ||
				 np->IsNoChildPackages() );		
}


// ������� �����, ���� �� ��� �� ������, � ����� ��������� 
// ����������� ��� ��������
ClassType *ClassTypeMaker::Make()
{
	const Package *pkg = typePkg->GetChildPackage(typePkg->GetChildPackageCount()-1);

	// ����� ���������� �����, ������� ���, ��������� � ������� � �������
	if( pkg->GetPackageID() == KWCLASS || pkg->GetPackageID() == KWSTRUCT ||
		pkg->GetPackageID() == KWUNION )
	{
		MakeUnnamedClass();
		return resultCls;	
	}

	// � ��������� ������, ����� ������� � ��������� ��������
	INTERNAL_IF( pkg->GetPackageID() != PC_QUALIFIED_NAME );
	NodePackage *namePkg = (NodePackage *)pkg;

	// �������� ����
	INTERNAL_IF( typePkg->GetChildPackageCount() < 2 );
	pkg = typePkg->GetChildPackage(typePkg->GetChildPackageCount()-2);
	INTERNAL_IF( pkg->GetPackageID() != KWCLASS && pkg->GetPackageID() != KWSTRUCT &&
				 pkg->GetPackageID() != KWUNION ) ;
	const Lexem &key = ((LexemPackage *)pkg)->GetLexem();
	int code = key.GetCode();

	// ���� ��� �� �������� �����������������, ������ �������� ��� ����������
	// �������
	if( namePkg->GetChildPackageCount() == 1 )
	{	
		// ���� ������������� � ������� ������� ���������
		const Lexem &name = ((LexemPackage *)namePkg->GetChildPackage(0))->GetLexem();

		// �� ������, ����������� ����� �� ������ ����� ��� �������� ������,
		// ���� ������ ����� ����������� ������ ������� ������
		if( GetCurrentSymbolTable().IsClassSymbolTable() &&
			dynamic_cast<Identifier &>(GetCurrentSymbolTable()).GetName() == name.GetBuf() )
		{
			theApp.Error(name.GetPos(), 
					"'%s' - ����� �� ����� ����� ��� ������ � ������� �����������",
					name.GetBuf().c_str() );
			return NULL;
		}

		NameManager nm( name.GetBuf(), &destST, false );
		AmbiguityChecker achk(nm.GetRoleList(), name.GetPos(), true);

		// ���� ��� ������, ����� ������ ���������
		if( const Identifier *tnam = achk.IsTypeName(true) )
		{
			resultCls = dynamic_cast<ClassType *>(const_cast<Identifier *>(tnam));

			// ���� �� �����
			if( !resultCls )		
				theApp.Error(name.GetPos(), "'%s' - ��� ��� ��������", name.GetBuf().c_str());

			// ���� ���� �� ���������
			else if( resultCls->GetBaseTypeCode() != 
							TypeSpecifierManager(code).CodeToClassSpec() )
			{
				theApp.Error(key.GetPos(), "'%s %s' - ����� ��� �������� � ������ ������",
					GetKeywordName(code), name.GetBuf().c_str() );
				return NULL;
			}

			// ���� ������������� ������� �� ���������
			else if( GetCurrentSymbolTable().IsClassSymbolTable() &&
					 resultCls->GetAccessSpecifier() != as )
			{
				theApp.Error(key.GetPos(), 
					"'%s' - ����� �� ����� �������� ������������ �������",
					name.GetBuf().c_str() );			
			}

			// �������� ������� �� ������������, �.�. ��� ���� �����, �������
			// �������� ������ ������� ������, ���� �����, ������� �����������

			// � ��������� ������ ����� ����������������� � �� ���������� ��� ������������
			return resultCls;
		}

		// ����� ��� �� ������, �� ����� ���� �� ��� ������ ������, �����
		// ������� ������ �����������
		else
		{
			if( achk.IsTemplateClass() != NULL )
			{
				theApp.Error(name.GetPos(), 
					"'%s' - �������� ��� '��������� �����'",
					name.GetBuf().c_str());
				return NULL;
			}

			if( achk.IsNameSpace() != NULL )
			{
				theApp.Error(name.GetPos(), 
					"'%s' - �������� ��� '����������� ������� ���������'",
					name.GetBuf().c_str());
				return NULL;
			}

			// ����� ������� ����� 			
			MakeUncompleteClass(); 			
			return resultCls;			
		}
	}

	// ����� ��� �������� ����������������� � ����������� ������
	// �������� ������������� ������ � �������� �����
	else
	{
		QualifiedNameManager qnm(namePkg);
		CharString cnam = ParserUtils::PrintPackageTree(namePkg);
		Position epos = ParserUtils::GetPackagePosition(namePkg);
		AmbiguityChecker achk(qnm.GetRoleList(), epos, true);

		if( (resultCls = const_cast<ClassType *>(achk.IsClassType(true)) ) != NULL )
		{
			// ���� ���� �� ���������
			if( resultCls->GetBaseTypeCode() != TypeSpecifierManager(code).CodeToClassSpec() )
			{
				theApp.Error(key.GetPos(), "'%s %s' - ����� ��� �������� � ������ ������",
					GetKeywordName(code), cnam.c_str() );
				return NULL;
			}

			// �����, ��������� ������ ��������������, ��������� � ���������� �����
			stList = qnm.GetQualifierList();

			if( !defination )
				CheckerUtils::CheckAccess(qnm, *resultCls, epos);
			return resultCls;
		}

		// ����� ����� �� �������� � ������� ���������
		else
			theApp.Error(key.GetPos(), "'%s %s' - ����� �� ��������",
					GetKeywordName(code), cnam.c_str() );				
	}

	return NULL;	
}


// ������� ����� �� ����� � �� �����
void ClassTypeMaker::MakeUncompleteClass()
{	
	// �������� ������� �����, ��������������� ��� ��� ��������� ����� � ������
	// � ����� PC_QUALIFIED_NAME � ����� �������� ������� NAME. ������������
	// ����� ����������� ����������� � ������ Make
	const CharString &name = ((LexemPackage *)((NodePackage *)typePkg->GetChildPackage(
		typePkg->GetChildPackageCount()-1))->GetChildPackage(0))->GetLexem().GetBuf();

	int code = ((LexemPackage *)typePkg->GetChildPackage(
		typePkg->GetChildPackageCount()-2))->GetLexem().GetCode();

	SymbolTable *cst = &destST;

	// ��������� �����
	resultCls = code == KWUNION
			? new UnionClassType(name, cst, as, false, ::Object::SS_NONE)
			: new ClassType(name, cst, 
				(code == KWCLASS ? BaseType::BT_CLASS : BaseType::BT_STRUCT), as);
	INTERNAL_IF( !cst->InsertSymbol(resultCls) );	
}


// ������� ���������� �����
void ClassTypeMaker::MakeUnnamedClass()
{	
	static int clsCounter = 0;
	SymbolTable *cst = &destST;
	const Lexem &lxm = ((LexemPackage *)typePkg->GetChildPackage(
		typePkg->GetChildPackageCount()-1))->GetLexem();
	int key = lxm.GetCode();

	CharString knam = (string("<����� ") + 
		CharString(clsCounter).c_str() + ">").c_str();

	clsCounter++;
	
	resultCls = key == KWUNION
			? new UnionClassType(knam, cst, as, false, ::Object::SS_NONE)
			: new ClassType(knam, cst, 
				(key == KWCLASS ? BaseType::BT_CLASS : BaseType::BT_STRUCT), as);
	INTERNAL_IF( !cst->InsertSymbol(resultCls) );	
	

	// ��������� � ������ ������� ��������������� ���
	NodePackage *np = new NodePackage(PC_QUALIFIED_NAME);	
	np->AddChildPackage( new LexemPackage( Lexem( knam, key, lxm.GetPos()) ) );
	typePkg->AddChildPackage(np);
}


//  �-��� ��������� ����� � ��������� ��� ������������
EnumTypeMaker::EnumTypeMaker( NodePackage *np, ClassMember::AS a, bool def )
	: typePkg(np), as(a), resultEnum(NULL), defination(def)
{ 		
	INTERNAL_IF( np == NULL || np->GetPackageID() != PC_TYPE_SPECIFIER_LIST ||
				 np->IsNoChildPackages() );		
}


// ������� �����, ���� �� ��� �� ������, � ����� ��������� 
// ����������� ��� ��������
EnumType *EnumTypeMaker::Make()
{
	const Package *pkg = typePkg->GetChildPackage(typePkg->GetChildPackageCount()-1);

	// ����� ���������� ������������, ������� ���, ��������� � ������� � �������
	if( pkg->GetPackageID() == KWENUM )
	{
		static int ecnt = 0;
		CharString nam = string(
				string("<������������ ") + CharString(ecnt).c_str() + ">").c_str();

		ecnt++;

		resultEnum = new EnumType(nam, &GetCurrentSymbolTable(), as);
		INTERNAL_IF( !GetCurrentSymbolTable().InsertSymbol(resultEnum) );		

		// ��������� � ������ ������� ��������������� ���
		NodePackage *np = new NodePackage(PC_QUALIFIED_NAME);	
		const Position &pos = static_cast<const LexemPackage*>(pkg)->GetLexem().GetPos();
		np->AddChildPackage( new LexemPackage( Lexem( nam, NAME,  pos) ) );
		typePkg->AddChildPackage(np);

		return resultEnum;
	}

	// � ��������� ������, ����� ������� � ��������� ��������
	INTERNAL_IF( pkg->GetPackageID() != PC_QUALIFIED_NAME );
	NodePackage *namePkg = (NodePackage *)pkg;

	// ��������� ������������ �����
	INTERNAL_IF( typePkg->GetChildPackageCount() < 2 );	
	INTERNAL_IF( typePkg->GetChildPackage(typePkg->GetChildPackageCount()-2)
						->GetPackageID() != KWENUM );

	pkg = typePkg->GetChildPackage(typePkg->GetChildPackageCount()-2);
	const Lexem &key = ((LexemPackage *)pkg)->GetLexem();	

	// ���� ��� �� �������� �����������������, ������ �������� ��� ����������
	// �������
	if( namePkg->GetChildPackageCount() == 1 )
	{	
		// ���� ������������� � ������� ������� ���������
		const Lexem &name = ((LexemPackage *)namePkg->GetChildPackage(0))->GetLexem();

		// ������������ �� ����� ��� ������ � ������� �����������		
		if( GetCurrentSymbolTable().IsClassSymbolTable() &&
			dynamic_cast<Identifier &>(GetCurrentSymbolTable()).GetName() == name.GetBuf() )
		{
			theApp.Error(name.GetPos(), 
					"'%s' - ������������ �� ����� ����� ��� ������ � ������� �����������",
					name.GetBuf().c_str() );
			return NULL;
		}

		NameManager nm( name.GetBuf(), &GetCurrentSymbolTable(), false);
		AmbiguityChecker achk(nm.GetRoleList(), name.GetPos(), true);

		if( (resultEnum = (EnumType *)achk.IsEnumType(true)) != NULL )
		{
			// ���� ������������� ������� �� ���������
			if( resultEnum->GetAccessSpecifier() != as )
			{
				theApp.Error(name.GetPos(), 
					"'%s' - ������������ �� ����� �������� ������������ �������",
					name.GetBuf().c_str() );
			
			}

			return resultEnum ;
		}
		
		else if( achk.IsTypeName(true) || achk.IsTemplateClass() || achk.IsNameSpace() )
		{
			theApp.Error( ParserUtils::GetPackagePosition(namePkg), 
				"'%s' - ��� ��� ������������ ��� ���, ��������� ����� ��� ������� ���������",
				name.GetBuf().c_str());
			return NULL;			
		}
		
		// ����� ������� ������������ � ��������� ��� � �������
		else
		{
			resultEnum = new EnumType(name.GetBuf(), &GetCurrentSymbolTable(), as);
			INTERNAL_IF( !GetCurrentSymbolTable().InsertSymbol(resultEnum) );
			return 	resultEnum ;		
		}			
	}

	// ����� ��� ����������������� � ��������� ������ �������� ��� �������������
	else
	{
		QualifiedNameManager qnm(namePkg);
		CharString cnam = ParserUtils::PrintPackageTree(namePkg);
		Position epos =  ParserUtils::GetPackagePosition(namePkg);
		AmbiguityChecker achk(qnm.GetRoleList(), epos, true);

		if( (resultEnum = const_cast<EnumType *>(achk.IsEnumType(true)) ) != NULL )
		{			
			// �����, ��������� ������ ��������������, ��������� �� �����������
			// � ���������� ������������
			stList = qnm.GetQualifierList();
			
			if( !defination )
				CheckerUtils::CheckAccess(qnm, *resultEnum, epos);
			return resultEnum;
		}

		// ����� ����� �� �������� � ������� ���������
		else
			theApp.Error(key.GetPos(), "'enum %s' - ������������ �� ���������",
							cnam.c_str() );				
	}

	return resultEnum;
}

	
