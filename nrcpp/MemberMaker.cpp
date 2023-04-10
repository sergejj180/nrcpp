// ���������� ��������� ���������� ������ ������ - MemeberMaker.cpp

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
#include "MemberMaker.h"
#include "Overload.h"

using namespace MakerUtils;


// ������� �������� �������-�����, �������������� ����� ������ DeclarationMaker
bool DataMemberMaker::Make()
{
	DataMemberChecker dmc(*toc);
	if( dmc.IsRedeclared() )
		return false;

	// ����� ������� ���� � ��������� ��� � �����
	targetDM = new DataMember( toc->name, &clsType, toc->finalType,
		toc->constQual, toc->volatileQual, toc->dtl, toc->ssCode < 0 ? ::Object::SS_NONE : 
		TypeSpecifierManager(toc->ssCode).CodeToStorageSpecifierObj(), curAccessSpec);


	// ���������, ����� ���� �� ���������������
	NameManager nm(toc->name, (SymbolTable*)&GetCurrentSymbolTable(), false);

	// ��������� �� ���������������
	RedeclaredChecker rchk(*targetDM, nm.GetRoleList(), toc->errPos, R_DATAMEMBER);
	
	// ���� ���� �� �������������, �������� ��� � �������
	if( !rchk.IsRedeclared() )
		GetCurrentSymbolTable().InsertSymbol(targetDM);
	else
	{
		delete targetDM;
		targetDM = const_cast<DataMember *>(
			dynamic_cast<const DataMember *>(rchk.GetPrevDeclaration()) );
		return false;
	}

	return true;
}

// ����������� ����� ��� ������������� �����. ���������������� ����� ������,
// ����������� ����� ������-�����
void DataMemberMaker::Initialize( MemberInitializationType mit, const Operand &exp )
{
	INTERNAL_IF( mit != MIT_NONE && mit != MIT_DATA_MEMBER && mit != MIT_BITFIELD );

	// ���������, ���� ������������� �����
	if( mit == MIT_DATA_MEMBER )
		CheckDataInit( exp );

	// ���������, ���� �������� �������
	else if( mit == MIT_BITFIELD )		
		CheckBitField( exp );
	
	// ����� ���� �� ������, ������
	else if( mit != MIT_NONE )
		INTERNAL( "'DataMemberMaker::Initialize' - �������� ��� �������������" );		
}


// ������� �������� ������ �������������� ����� ������ DeclarationMaker
bool MethodMaker::Make()
{
	MethodChecker mc(*toc);
	if( mc.IsRedeclared() )
		return false;

	targetMethod = new Method( toc->name, &clsType, toc->finalType,
		toc->constQual, toc->volatileQual, toc->dtl, toc->fnSpecCode == KWINLINE,
		toc->ssCode < 0 ? Function::SS_NONE : 
		TypeSpecifierManager(toc->ssCode).CodeToStorageSpecifierFn(),
		Function::CC_NON, curAccessSpec, false, toc->fnSpecCode == KWVIRTUAL, 
		false, Method::DT_USERDEFINED );
	
	// ���������, ����� ���� �� ���������������
	NameManager nm(toc->name, (SymbolTable*)&GetCurrentSymbolTable(), false);

	// ��������� �� ���������������
	RedeclaredChecker rchk(*targetMethod, nm.GetRoleList(), toc->errPos, R_METHOD);
	
	// ���� ����� �� �������������, �������� ��� � �������
	if( !rchk.IsRedeclared() )
	{
		CheckerUtils::DefaultArgumentCheck( 
			targetMethod->GetFunctionPrototype(), NULL, toc->errPos);

		GetCurrentSymbolTable().InsertSymbol(targetMethod);
	}

	else
	{
		const Method *meth = dynamic_cast<const Method *>(rchk.GetPrevDeclaration());
		if( meth )		
			theApp.Error(toc->errPos, "'%s' - ����� ��� ��������", toc->name.c_str());
			
		delete targetMethod;
		targetMethod = const_cast<Method *>(meth);
		return false;
	}
	
	return true;
}


// ����������� ����� ��� ������������� �����. ������������� ����� ����
// ������ MIT_PURE_VIRTUAL, exp ������ ���� ����� NULL
void MethodMaker::Initialize( MemberInitializationType mit, const Operand &exp )
{		
	if( mit == MIT_PURE_VIRTUAL )	
		targetMethod->SetAbstract();	// ������ ������������ ������
	
	else if( mit == MIT_BITFIELD )
		theApp.Error( toc->errPos, "������� ���� �� ����� ���� �������" );

	else if( mit != MIT_NONE )
		INTERNAL( "'MethodMaker::Initialize' - �������� ��� �������������" );

	// ��������� ��������� ����������� �������
	VirtualMethodChecker( *targetMethod, toc->errPos );
}


// ������� �������� �������������� ���������,
// �������������� ����� ������ DeclarationMaker
bool OperatorMemberMaker::Make()
{
	ClassOperatorChecker coc(*toc, tooc);

	targetOO = new ClassOverloadOperator( toc->name, &clsType, toc->finalType,
		toc->constQual, toc->volatileQual, toc->dtl, toc->fnSpecCode == KWINLINE,
		toc->ssCode < 0 ? Function::SS_NONE : 
		TypeSpecifierManager(toc->ssCode).CodeToStorageSpecifierFn(),
		Function::CC_NON, curAccessSpec, false, toc->fnSpecCode == KWVIRTUAL, 
		tooc.opCode, tooc.opString, Method::DT_USERDEFINED
		);
	
	// ���������, ����� �������� �� ���������������
	NameManager nm(toc->name, (SymbolTable*)&GetCurrentSymbolTable(), false);

	// ��������� �� ���������������
	RedeclaredChecker rchk(*targetOO, nm.GetRoleList(), toc->errPos, R_CLASS_OVERLOAD_OPERATOR);
	
	// ���� ����� �� �������������, �������� ��� � �������
	if( !rchk.IsRedeclared() )
	{
		// ���� �������� �� �������, ��� ����� ���������� � ��������� ��������������
		// ������, ������ ���
		if( !toc->dtl.IsFunction() )
		{
			delete targetOO;
			targetOO = NULL;
			return false;
		}

		// ����� ��������� ��������� �� ��������� � ��������� ������
		else
		{
			CheckerUtils::DefaultArgumentCheck( 
				targetOO->GetFunctionPrototype(), NULL, toc->errPos);
			GetCurrentSymbolTable().InsertSymbol(targetOO);		
		}
	}

	else
	{
		const ClassOverloadOperator *coo = 
				dynamic_cast<const ClassOverloadOperator *>(rchk.GetPrevDeclaration());
		if( coo )
			theApp.Error(toc->errPos, "'%s' - ������������� �������� ��� ��������", 
				toc->name.c_str());

		delete targetOO;
		targetOO = const_cast<ClassOverloadOperator *>(coo);
		return false;
	}
	
	return true;
}


// ����������� ����� ��� ������������� ������. ������������� ����� ����
// ������ MIT_PURE_VIRTUAL, exp ������ ���� ����� NULL
void OperatorMemberMaker::Initialize( MemberInitializationType mit, const Operand &exp )
{	
	if( mit == MIT_PURE_VIRTUAL )
		targetOO->SetAbstract();	// ������ ������������ ������
	
	else if( mit == MIT_BITFIELD )
		theApp.Error( toc->errPos, "������� ���� �� ����� ���� �������" );

	else if( mit != MIT_NONE )
		INTERNAL( "'OperatorMemberMaker::Initialize' - �������� ��� �������������" );

	// ��������� ��������� ������������ ���������
	VirtualMethodChecker( *targetOO, toc->errPos );
}


// ������� �������� ��������� ����������,
// �������������� ����� ������ DeclarationMaker
bool CastOperatorMaker::Make()
{
	// ��������� ��������
	CastOperatorChecker coc(*toc, tcoc);

	// ���� �������� �� �������� �������� �����
	if( coc.IsIncorrect() )
		return false;

	targetCCOO = new ClassCastOverloadOperator( toc->name, &clsType, toc->finalType,
		toc->constQual, toc->volatileQual, toc->dtl, toc->fnSpecCode == KWINLINE,
		toc->ssCode < 0 ? Function::SS_NONE : 
		TypeSpecifierManager(toc->ssCode).CodeToStorageSpecifierFn(),
		Function::CC_NON, curAccessSpec, false, toc->fnSpecCode == KWVIRTUAL, 
		tcoc.opCode, tcoc.opString, *tcoc.castType, Method::DT_USERDEFINED);

	// ���������, ����� ���� �� ���������������
	NameManager nm(toc->name, (SymbolTable*)&GetCurrentSymbolTable(), false);

	// ��������� �� ���������������
	RedeclaredChecker rchk(*targetCCOO, nm.GetRoleList(), toc->errPos, R_CLASS_OVERLOAD_OPERATOR);
	
	// ���� ���� �� �������������, �������� ��� � �������
	if( !rchk.IsRedeclared() )
		GetCurrentSymbolTable().InsertSymbol(targetCCOO);
	else
	{
		delete targetCCOO;	
		theApp.Error(toc->errPos, "'%s' - �������� ���������� ��� ��������", 
				toc->name.c_str());

		targetCCOO = const_cast<ClassCastOverloadOperator *>(
			dynamic_cast<const ClassCastOverloadOperator *>(rchk.GetPrevDeclaration()) );
	
		return false;
	}

	return true;
}


// ����������� ����� ��� ������������� ������. ������������� ����� ����
// ������ MIT_PURE_VIRTUAL, exp ������ ���� ����� NULL
void CastOperatorMaker::Initialize( MemberInitializationType mit, const Operand &exp )
{	
	if( mit == MIT_PURE_VIRTUAL )
		targetCCOO->SetAbstract();	// ������ ������������ ������

	else if( mit == MIT_BITFIELD )
		theApp.Error( toc->errPos, "������� ���� �� ����� ���� �������" );

	else if( mit != MIT_NONE )
		INTERNAL( "'CastOperatorMaker::Initialize' - �������� ��� �������������" );

	// ��������� ��������� ������������ ���������
	VirtualMethodChecker( *targetCCOO, toc->errPos );	
}


// ������� �������� ������������, �������������� ����� ������ DeclarationMaker
bool ConstructorMaker::Make()
{
	ConstructorChecker cm(*toc, clsType);

	if( cm.IsIncorrect() )
		return false;

	// ���� � ������������ �� ������ ������, ������
	if( toc->dtl.IsFunction() && toc->dtl.GetDerivedTypeCount() != 2 )
		toc->dtl.AddDerivedType(new Reference);

	targetCtor = new ConstructorMethod( toc->name, &clsType, toc->finalType,
		toc->constQual, toc->volatileQual, toc->dtl, toc->fnSpecCode == KWINLINE,
		toc->ssCode < 0 ? Function::SS_NONE : 
		TypeSpecifierManager(toc->ssCode).CodeToStorageSpecifierFn(),
		Function::CC_NON, curAccessSpec, toc->fnSpecCode == KWEXPLICIT, Method::DT_USERDEFINED );

	// ���������, ����� ���� �� ���������������
	NameManager nm(toc->name, (SymbolTable*)&GetCurrentSymbolTable(), false);

	// ��������� �� ���������������
	RedeclaredChecker rchk(*targetCtor, nm.GetRoleList(), toc->errPos, R_CONSTRUCTOR);
	
	// ���� ���� �� �������������, �������� ��� � �������
	if( !rchk.IsRedeclared() )
	{
		CheckerUtils::DefaultArgumentCheck( 
			targetCtor->GetFunctionPrototype(), NULL, toc->errPos);
		GetCurrentSymbolTable().InsertSymbol(targetCtor);		
	}

	else
	{
		delete targetCtor;	
		theApp.Error(toc->errPos, "'%s' - ����������� ��� ��������", 
				toc->name.c_str());

		targetCtor = const_cast<ConstructorMethod *>(
			dynamic_cast<const ConstructorMethod *>(rchk.GetPrevDeclaration()) );
		return false;
	}
	return true;
}


// ����������� ����� ��� ������������� ������������. ������������� 
// ������������ ����������
void ConstructorMaker::Initialize( MemberInitializationType mit, const Operand &exp )
{		
	if( mit == MIT_PURE_VIRTUAL )
		theApp.Error( toc->errPos, 
			"'%s' - ����������� �� ����� ����� ������������� �����-����������� �������",
			toc->name.c_str());

	else if( mit == MIT_BITFIELD )
		theApp.Error( toc->errPos, "������� ���� �� ����� ���� �������" );

	else if( mit != MIT_NONE )
		INTERNAL( "'ConstructorMaker::Initialize' - �������� ��� �������������" );
}


// ������� �������� ������������, �������������� ����� ������ DeclarationMaker
bool DestructorMaker::Make()
{
	// ���������� ������ ���� ��������, ��� �������������� ���� � 
	// �������������� ����������� �����. ����������
	// �� ���������� ��������. ���������� ������ ����������� ��� ����������.
	if( !toc->dtl.IsFunction() || toc->dtl.GetDerivedTypeCount() != 1 )
		theApp.Error(toc->errPos, "'%s' - ���������� ������ ���� ��������", 
				toc->name.c_str());

	// ������� ���, cv-�������������, ������������� ��������, friend-������.
	// ������ ������������� � �����������
	if( toc->finalType != NULL || toc->constQual || toc->volatileQual ||
		toc->friendSpec || toc->fnSpecCode == KWEXPLICIT )
		theApp.Error(toc->errPos, "������������� ���� � ���������� �����������", 
				toc->name.c_str());

	// ���������� ������ ����������� ��� ���������� � ��� cv-��������������
	if( toc->dtl.IsFunction() )
	{
		const FunctionPrototype &fp = static_cast<const FunctionPrototype&>(
			*toc->dtl.GetHeadDerivedType());

		if( fp.GetParametrList().GetFunctionParametrCount() != 0 ||
			fp.IsHaveEllipse() )
			theApp.Error(toc->errPos, "'%s' - ���������� ������ ����������� ��� ����������", 
				toc->name.c_str());

		if( fp.IsConst() || fp.IsVolatile() )
			theApp.Error(toc->errPos, "'%s' - ���������� �� ����� ��������� cv-�������������", 
				toc->name.c_str());
	}

	// ��� ����������� ������ ��������� � ������ ������
	INTERNAL_IF( toc->name.at(0) != '~' );
	if( toc->name != ('~' + clsType.GetName()) )
	{
		theApp.Error(toc->errPos, "'%s' - ��� ����������� �� ��������� � ������ ������", 
			toc->name.c_str());
		return false;
	}

	// ���������, ���������� ������ ���� ���� �� ���� ������
	NameManager nm(toc->name, (SymbolTable*)&GetCurrentSymbolTable(), false);
	if( nm.GetRoleCount() != 0 )
	{
		theApp.Error(toc->errPos, "'%s' - ���������� ��� ��������", 
			toc->name.c_str());
		return false;
	}

	// ���� ������� ��� �� ����� - ������ void
	if( toc->finalType == NULL )
		toc->finalType = (BaseType*)&ImplicitTypeManager(BaseType::BT_VOID).GetImplicitType();

	// �������� ������� ����������
	targetDtor = new Method(toc->name, &clsType, toc->finalType,
		toc->constQual, toc->volatileQual, toc->dtl, toc->fnSpecCode == KWINLINE,
		toc->ssCode < 0 ? Function::SS_NONE : 
		TypeSpecifierManager(toc->ssCode).CodeToStorageSpecifierFn(),
		Function::CC_NON, curAccessSpec, false, toc->fnSpecCode == KWVIRTUAL, 
		true, Method::DT_USERDEFINED);

	GetCurrentSymbolTable().InsertSymbol(targetDtor);
	return true;
}


// ����������� ����� ��� ������������� ������. ������������� ����� ����
// ������ MIT_PURE_VIRTUAL, exp ������ ���� ����� NULL
void DestructorMaker::Initialize( MemberInitializationType mit, const Operand &exp )
{
	INTERNAL_IF( mit == MIT_DATA_MEMBER );
	if( mit == MIT_PURE_VIRTUAL )
		targetDtor->SetAbstract();	// ������ ������������ ������

	else if( mit == MIT_BITFIELD )
		theApp.Error( toc->errPos, "������� ���� �� ����� ���� �������" );

	else if( mit != MIT_NONE )
		INTERNAL( "'DestructorMaker::Initialize' - �������� ��� �������������" );

	// ��������� ��������� ������������ �����������
	VirtualMethodChecker( *targetDtor, toc->errPos );
}


// ������� �������� ��������� �������, 
// �������������� ����� ������ DeclarationMaker
bool FriendFunctionMaker::Make()
{
	// ������� ������������ ������ ��� ��������
	INTERNAL_IF( !toc->friendSpec );
	toc->friendSpec = false;

	if( isOperator )
		GlobalOperatorChecker g(*toc, tooc); 
	else
		GlobalDeclarationChecker g(*toc); 

	if( toc->ssCode != -1 )
		theApp.Error(toc->errPos, "'%s' - ������������� %s �� friend-����������",
			toc->name.c_str(), GetKeywordName(toc->ssCode));

	// ���������, ����� ���� �� ���������������
	SymbolTable *destST = const_cast<SymbolTable *>(&::GetScopeSystem().GetGlobalSymbolTable());
	NameManager nm(toc->name, destST, true);

	// ������� ������ ��� �������� �� ������������������
	if( isOperator )
		targetFn = new OverloadOperator(toc->name, destST,
			toc->finalType,	toc->constQual, toc->volatileQual, 
			toc->dtl, toc->fnSpecCode == KWINLINE,
			toc->ssCode < 0 ? Function::SS_NONE : 
			TypeSpecifierManager(toc->ssCode).CodeToStorageSpecifierFn(),
				Function::CC_NON, tooc.opCode, tooc.opString );

	else
		targetFn = new Function(toc->name, destST,
			toc->finalType,	toc->constQual, toc->volatileQual, 
			toc->dtl, toc->fnSpecCode == KWINLINE,
			toc->ssCode < 0 ? Function::SS_NONE : 
			TypeSpecifierManager(toc->ssCode).CodeToStorageSpecifierFn(), Function::CC_NON  );
							
	// ��������� �� ���������������
	RedeclaredChecker rchk(*targetFn, nm.GetRoleList(), toc->errPos, 
		isOperator ? R_OVERLOAD_OPERATOR : R_FUNCTION);

	// ���� �������������, ��������� ������� ��� ��������� ������
	if( rchk.IsRedeclared() )
	{
		Function *prevFn = const_cast<Function *>(
			dynamic_cast<const Function *>(rchk.GetPrevDeclaration()) );
		if( prevFn != NULL )
			CheckerUtils::DefaultArgumentCheck( targetFn->GetFunctionPrototype(), 
				&prevFn->GetFunctionPrototype(), toc->errPos);
		delete targetFn;
		targetFn = prevFn;	
	}
	
	// ����� ��������� �� � ��������� ���������� ������� ���������
	else	
	{
		CheckerUtils::DefaultArgumentCheck( targetFn->GetFunctionPrototype(), NULL, toc->errPos);
		INTERNAL_IF( !destST->InsertSymbol(targetFn));	
	}


	// � �������� ����� ��������� ������� � ������ ������ ������
	const_cast<ClassFriendList &>(clsType.GetFriendList()).
		AddClassFriend( ClassFriend(targetFn) );

	return true;
}


// ����������� ����� ��� ��������� �������. ������������� ����������
void FriendFunctionMaker::Initialize( MemberInitializationType mit, const Operand &exp )
{	
	if( mit != MIT_NONE )
		theApp.Error( toc->errPos,
			"'%s' - � ������������� ������� �� ����� ���� ��������������",
			toc->name.c_str());
}


// ����������� ������� ���������� ������� ��� � ����������� �����,
// � ������ ����, ��� � �������������, ������������, ���������� ����������
// ������� ��� �� �������� ���� � ����������� �������������
void MemberDefinationMaker::SpecifyBaseType( const NodePackage *np, const SymbolTable &st )
{
	// �������� ��������� ���
	const NodePackage *last = static_cast<const NodePackage *>(np->GetLastChildPackage());

	// ������ ���� ���, ��������, ����������
	register int pid = last->GetPackageID();
	bool special = pid == PC_DESTRUCTOR || pid == PC_CAST_OPERATOR ||idRole == R_CONSTRUCTOR;
	INTERNAL_IF( pid != NAME && pid != PC_OVERLOAD_OPERATOR && !special );

	// ���� ������� ��� �����, ���� ����� �� ����������, ����������� ��� �������� ����������
	if( toc->baseType != NULL || !special )		
	{
		if( special ) 		
			theApp.Error(toc->errPos, 
				"'%s' - ������� ��� �� ����� ���������� ��� ����������� %s", 
				toc->name.c_str(), pid == PC_DESTRUCTOR ? "�����������" : 
				(pid == PC_CAST_OPERATOR ? "��������� ����������" : "������������"));
			
		MakerUtils::SpecifyBaseType( &*toc );
		return;	
	}
		

	// ���� ������� ��� �� �����, �� �������� ��� �� �����, �.�.
	// � ������������, �������������, ���������� ���������� ��� ����������� 
	// ������������� � ���� �� ��������. �����. (��� �����, void, ��� ���������� ��� �-���)

	// ���� ����������, ������� ��� �� ��������� void
	if( pid == PC_DESTRUCTOR )
		toc->finalType = 
			const_cast<BaseType*>(&ImplicitTypeManager(BaseType::BT_VOID).GetImplicitType());	

	// ���� �����������
	else if( idRole == R_CONSTRUCTOR )
	{
		// � ������������ �� ��������� ��� ������ � �������� �� �����������
		INTERNAL_IF( !st.IsClassSymbolTable() );
		const ClassType &cls = static_cast<const ClassType&>(st);
		toc->finalType = const_cast<ClassType *>(&cls);
	}

	// ����� ���� �������� ����������
	else if( pid == PC_CAST_OPERATOR )
	{
		// ����� ������ ��������� �� ��������� �.�. ����� ��� �������� ���
		TempCastOperatorContainer tcoc;
		AnalyzeCastOperatorPkg( *last, tcoc );									
		CastOperatorChecker(*toc, tcoc);
	}

	// ����� ����������� ������
	else
		INTERNAL( "����������� ��� ��������������" );
}


// ���������� ���� ������������ ��������������. ������, �����������
// ������������ ������������, ��������� ���� ������ ���������.
// ������ ���� ����-�� ���� ��������� ����. ���������� false
// ���� ���� �� ����������
bool MemberDefinationMaker::InspectIDRole( const RoleList &rl )
{
	if( rl.empty() )
		return false;
	
	for( RoleList::const_iterator p = rl.begin(); p != rl.end(); p++ )
	{
		register Role r = (*p).second;
		if( r == R_CLASS_TYPE || r == R_UNION_CLASS_TYPE || r == R_ENUM_TYPE )
			continue;

		// � ��������� ������ ��������� ������������ ����
		INTERNAL_IF( idRole != R_UNCKNOWN && idRole != r );
		if( r != R_OBJECT && r != R_DATAMEMBER &&
			!(r >= R_FUNCTION && r <= R_CONSTRUCTOR ) )
		{
			theApp.Error( toc->errPos, "'%s' - ���� �� ����� ������������ � ������ �����",
				toc->name.c_str() );
			return false;
		}

		// ���� ���� �� ������, ������
		if( idRole == R_UNCKNOWN )
			idRole = r;
	}

	// ���� ���� �� ������, ������ ���� ������ ������
	if( idRole == R_UNCKNOWN )
	{
		theApp.Error( toc->errPos, 
			"'%s' - ���� �� �������� �������� ��� ��������", toc->name.c_str() );
		return false;
	}

	return true;
}


// ��������� �����������
bool MemberDefinationMaker::Make()
{
	// ������� ��� ���������� ����� �������������, �������
	// �����. �����������
	int ix = decl->FindPackage(PC_QUALIFIED_NAME);
	INTERNAL_IF( ix < 0 );

	const NodePackage *np = static_cast<const NodePackage *>(decl->GetChildPackage(ix));
	INTERNAL_IF( np->GetChildPackageCount() <= 1 );

	// ����� ������� ��������� ��������� � �������� � ��� ����������
	toc = new TempObjectContainer(
		ParserUtils::GetPackagePosition(np), ParserUtils::PrintPackageTree(np) );

	// ���� �����. �� �������, �����
	if( memberQnm.GetRoleCount() == 0 )
	{
		theApp.Error(toc->errPos, "'%s' - ���� �� ������", toc->name.c_str() );
		return false;
	}

	// �������� ���� ��������������
	if( !InspectIDRole( memberQnm.GetRoleList() ) )
		return false;

	// �������� ������ �������������� ����
	MakerUtils::AnalyzeTypeSpecifierPkg( tsl, &*toc );

	// ����� ����������� ����������
	MakerUtils::AnalyzeDeclaratorPkg( decl, &*toc );

	// �������� ������� ��� ����������� �������
	const SymbolTable *st = 
		memberQnm.GetQualifierList()[memberQnm.GetQualifierList().GetSymbolTableCount()-1];
	SpecifyBaseType( np , *st );

	// ������ ���������, ����� �� ���� �������� �������������� � �����������
	if( toc->fnSpecCode != -1 || toc->ssCode != -1 || toc->friendSpec )
		theApp.Error(toc->errPos, "'%s' - %s ����������� ��� ����������� �����", 
			toc->name.c_str(), toc->friendSpec ? "friend" : 
			GetKeywordName(toc->ssCode != -1 ? toc->ssCode : toc->fnSpecCode));
		
	// ������ �������� ���������, �����. ����������� � ����������
	RedeclaredChecker rchk( 
		TypyziedEntity(toc->finalType, toc->constQual, toc->volatileQual, toc->dtl),
		memberQnm.GetRoleList(), toc->errPos, idRole);

	if( !rchk.IsRedeclared() )
	{
		theApp.Error(toc->errPos, "'%s' - ���� �� ������", toc->name.c_str() );
		return false;
	}

	// ����� ������������� ������, � ���� �� �������, �������� ��������� �� ���������
	else	
	{
		targetID = const_cast<Identifier *>(rchk.GetPrevDeclaration());	
		if( Function *prevFn = dynamic_cast<Function *>(targetID) )
		{
			INTERNAL_IF( !toc->dtl.IsFunction() );
			CheckerUtils::DefaultArgumentCheck( static_cast<const FunctionPrototype&>(
				*toc->dtl.GetDerivedType(0)), &prevFn->GetFunctionPrototype(), toc->errPos);

			// ����� ��������, ����� �� ���������� ����� ���������������
			// ������������
			if( prevFn->IsClassMember() && 
				!static_cast<const Method *>(prevFn)->IsUserDefined() )
				theApp.Error(toc->errPos, 
					"'%s' - ���������� ���������� ����� ��������������� ������������",
					prevFn->GetQualifiedName().c_str() );
		}
	}

	// ���������, ����� �� ����� �������� ����� ����������
	if( st->IsClassSymbolTable() )
	{
		if( ::Object *ob = dynamic_cast<::Object *>(targetID) )
		{
			if( ob->GetStorageSpecifier() != ::Object::SS_STATIC )
				theApp.Error(toc->errPos, 
					"'%s' - �� ����������� ���� �� ����� ������������",
					toc->name.c_str() );
			// ��������� �� �������������
		}
	}
	
	else if( st->IsNamespaceSymbolTable() )
		;
	// ����� ������
	else
		theApp.Error(toc->errPos, 
			"����� ���������� ������ ����� ������ � ����� ����������� ������� ���������",
			toc->name.c_str() );

	INTERNAL_IF( targetID == NULL );
	return true;
}

