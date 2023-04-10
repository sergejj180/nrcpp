// ���������� ���������� ��� �������-������������� - Coordinator.cpp

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
#include "MemberMaker.h"
#include "Parser.h"
#include "Body.h"
#include "Coordinator.h"
#include "ExpressionMaker.h"


// ������ �������� ��������� �����, ��������� 
// �������� �����, ��������� �������� ����� ����� � �������
// ��������� �����
void DeclarationCoordinator::StoreMemberScope( const NodePackage &np ) const
{
	INTERNAL_IF( np.GetPackageID() != PC_QUALIFIED_NAME );
	memberQnm = new QualifiedNameManager(&np, &GetCurrentSymbolTable());

	// ��������� ������ ��������������
	memberStl = memberQnm->GetQualifierList();

	// ���� �������� ��������� ���, �������. ������� ���������� ���
	// �������� ������
	if( memberStl.IsEmpty() )
		return;

	// ����� ��� ������� ��������� �� ������ ��������� � �����
	// ������� ��
	
	// ���� ������ ������� ��������� ����������, ������ �� ������
	if( memberStl[0] == GetScopeSystem().GetFirstSymbolTable() )
		memberStl.PopFront();

	// ��������� ������� ���������
	GetScopeSystem().PushSymbolTableList(memberStl);
}


// ������������ ������� �� ����� ����������� ����� ��� ��� ������
void DeclarationCoordinator::RestoreScopeSystem() const
{
	// ����������� ��� �������, ������� ���� �������� � ���� 
	for( int i = 0; i<memberStl.GetSymbolTableCount(); i++ )
		::GetScopeSystem().DestroySymbolTable();
	memberStl.Clear();
}


// ��������������� ��������� ����������, ������� �� ������
// ��������� ��������� � ����� �� �� ��������� ������� ���������
PDeclarationMaker DeclarationCoordinator::Coordinate() const
{
	int ix = declarator->FindPackage(PC_QUALIFIED_NAME);
	INTERNAL_IF( ix < 0 );

	// ���������, ���� ���� ����������� �����, �.�. ��� ���������
	// ������ ����� ���������� ��������� ����������� �����
	if( static_cast<const NodePackage *>(declarator->GetChildPackage(ix))->
			GetChildPackageCount() > 1 )
	{
		// �������������� �������� ������� ��������� ����� (�����(�))
		// � ������� ��. � ����������� ������������ ��� ����������
		StoreMemberScope( *static_cast<const NodePackage *>(declarator->GetChildPackage(ix)) );
		return new MemberDefinationMaker(typeSpecList, declarator, *memberQnm);
	}

	// ����� ������� ��������� ��������� � �������� � ��� ����������
	PTempObjectContainer toc = 
		new TempObjectContainer ( 
			ParserUtils::GetPackagePosition(declarator->GetChildPackage(ix)),
			ParserUtils::PrintPackageTree((NodePackage*)declarator->GetChildPackage(ix))
		);

	// �������� ������ �������������� ����
	MakerUtils::AnalyzeTypeSpecifierPkg( typeSpecList, &*toc );

	// ����� ����������� ����������
	MakerUtils::AnalyzeDeclaratorPkg( declarator, &*toc );
	
	// �������� ������� ���
	MakerUtils::SpecifyBaseType( &*toc );
	
	// �������� ��������� - �������� �������������� ������������
	const NodePackage &np = 
		*(NodePackage *)((NodePackage*)declarator->GetChildPackage(ix))->GetChildPackage(0);
	if( np.GetPackageID() == PC_OVERLOAD_OPERATOR )
	{
		TempOverloadOperatorContainer tooc;
		MakerUtils::AnalyzeOverloadOperatorPkg(  np,  tooc);
		return new GlobalOperatorMaker(toc, tooc);
	}

	else if( np.GetPackageID() == PC_CAST_OPERATOR )
	{
		theApp.Error( toc->errPos, "��������� ���������� ����� ���� ������ ������� ������");
		return NULL;
	}

	else if( toc->dtl.IsFunction() && toc->ssCode != KWTYPEDEF )
		return new GlobalFunctionMaker(toc);
	else
		return new GlobalObjectMaker(toc);
}


// ��������������� ��������� ����������, ������� �� ������
// ��������� ��������� � ����� �� �� ��������� ������� ���������
PDeclarationMaker AutoDeclarationCoordinator::Coordinate() const
{
	int ix = declarator->FindPackage(PC_QUALIFIED_NAME);
	INTERNAL_IF( ix < 0 );
	Position errPos = ParserUtils::GetPackagePosition(declarator->GetChildPackage(ix));

	// ���������, ���� ���� ����������� �����, �.�. ��� ���������
	// ������ ��� �������� �������
	if( static_cast<const NodePackage *>(declarator->GetChildPackage(ix))->
			GetChildPackageCount() > 1 )
	{
		theApp.Error(errPos,
			"���������� ����� ���������� � ��������� ������� ���������");
		return NULL;
	}

	// ����� ������� ��������� ��������� � �������� � ��� ����������
	PTempObjectContainer toc = 
		new TempObjectContainer ( 
			errPos,
			ParserUtils::PrintPackageTree((NodePackage*)declarator->GetChildPackage(ix))
		);

	// �������� ������ �������������� ����
	MakerUtils::AnalyzeTypeSpecifierPkg( typeSpecList, &*toc );

	// ����� ����������� ����������
	MakerUtils::AnalyzeDeclaratorPkg( declarator, &*toc );
	
	// �������� ������� ���
	MakerUtils::SpecifyBaseType( &*toc );
	
	// �������� ��������� - �������� �������������� ������������
	const NodePackage &np = 
		*(NodePackage *)((NodePackage*)declarator->GetChildPackage(ix))->GetChildPackage(0);
	if( np.GetPackageID() == PC_OVERLOAD_OPERATOR )
	{
		TempOverloadOperatorContainer tooc;
		MakerUtils::AnalyzeOverloadOperatorPkg(  np,  tooc);
		return new GlobalOperatorMaker(toc, tooc);
	}

	else if( np.GetPackageID() == PC_CAST_OPERATOR )
	{
		theApp.Error( toc->errPos, "��������� ���������� ����� ���� ������ ������� ������");
		return NULL;
	}

	else if( toc->dtl.IsFunction() && toc->ssCode != KWTYPEDEF )
		return new GlobalFunctionMaker(toc);
	else
		return new GlobalObjectMaker(toc, true);
}


// ���������� ��������� ���������� 
PMemberDeclarationMaker MemberDeclarationCoordinator::Coordinate()
{
	int ix = declarator->FindPackage(PC_QUALIFIED_NAME);
	const NodePackage *np = ix >= 0 ? (const NodePackage *)declarator->GetChildPackage(ix) : 0;
	Position ep;
	CharString name;
	
	
	ep = ParserUtils::GetPackagePosition(ix < 0 ? typeSpecList : np);
	name = ix < 0 ? "<��� �����>" : ParserUtils::PrintPackageTree(np);

	// ���������� ������ ����������� ��������������
	if( declarator->IsNoChildPackages() )
	{
		theApp.Error(ep, "�������� ���������� � ���������� �����");
		return NULL;
	}

	// ����� ���������, ����� ��� �� ���� �����������������
	if( np && np->GetChildPackageCount() > 1 )
	{
		theApp.Error(ep, "'%s' - ������ ��������� ����������������� ��� ������ ������",
			name.c_str());
		return NULL;
	}

	// ��������� ��������� ����������, ���������� �� ��������� ��������� ������
	if( np && np->GetChildPackage(0)->GetPackageID() == PC_CAST_OPERATOR )
	{		
		TempCastOperatorContainer tcoc;
		MakerUtils::AnalyzeCastOperatorPkg(*(const NodePackage *)np->GetChildPackage(0), tcoc);		

		// ����������� ��� ��������, ������� �� ����� ��������� ���������
		// �����, ���� ����� �� ����� ����� �����
		PTempObjectContainer toc = new TempObjectContainer( ep, tcoc.opFullName, curAccessSpec );
		
		// �������� ������ �������������� ����
		MakerUtils::AnalyzeTypeSpecifierPkg( typeSpecList, &*toc );

		// ����� ����������� ����������
		MakerUtils::AnalyzeDeclaratorPkg( declarator, &*toc );
	
		// �������� ������� ���, �� ������ ���� ������ �� ������
		if( !typeSpecList->IsNoChildPackages() )
		{
			// ���� ���������� ���� �����, � ��� 'virtual',
			// �� ��� ���������, ����� ������� ������� ������
			if( typeSpecList->GetChildPackageCount() == 1 && 
				typeSpecList->GetChildPackage(0)->GetPackageID() == KWVIRTUAL )
				toc->fnSpecCode = KWVIRTUAL;

			// ����� ��������
			else
				MakerUtils::SpecifyBaseType( &*toc );	
		}

		return new CastOperatorMaker(clsType, curAccessSpec, toc, tcoc);		
	}

	// ����������� ��� ��������, ������� �� ����� ��������� ���������
	// �����, ���� ����� �� ����� ����� �����
	PTempObjectContainer toc = new TempObjectContainer( ep, name, curAccessSpec );

	// �������� ������ �������������� ����
	MakerUtils::AnalyzeTypeSpecifierPkg( typeSpecList, &*toc );

	// ����� ����������� ����������
	MakerUtils::AnalyzeDeclaratorPkg( declarator, &*toc );
	
	// �������� ������� ���, �� ������ ���� ��� �� ���������� � �� �����������
	if( np														 && 
		(np->GetChildPackage(0)->GetPackageID() == PC_DESTRUCTOR &&
		 (typeSpecList->IsNoChildPackages() || (typeSpecList->GetChildPackageCount() == 1 &&
		  typeSpecList->GetChildPackage(0)->GetPackageID() == KWVIRTUAL) )	||
		 toc->name == clsType.GetName() )
		 )
		;

	else
		MakerUtils::SpecifyBaseType( &*toc );

	// ���� ����� ��� - ������ ��� ������ ���������� ������������,
	// ������ ������. ���� ��� ��������� � ������ ������, ������ ���
	// �� ������ ���������� ������������. � ����� ������ ��������
	// ��������� ������������
	if( ix < 0 || toc->name == clsType.GetName() )
	{
		// ��� ������������ �� ������ ��������� � ������ ������. � ������������
		// �� ������ ���� ����� ������, ������� ������ ���
		toc->name = ('.' +
			string(clsType.GetName().c_str()) ).c_str();	
		return new ConstructorMaker(clsType, curAccessSpec, toc);
	}

	// ����� ���� ��������� ����� - PC_OVERLOAD_OPERATOR, ����������
	// ��������� �������������� ���������
	else if( np->GetChildPackage(0)->GetPackageID() == PC_OVERLOAD_OPERATOR )
	{
		TempOverloadOperatorContainer tooc;
		MakerUtils::AnalyzeOverloadOperatorPkg(
			*(const NodePackage *)np->GetChildPackage(0), tooc);
		toc->name = tooc.opFullName;

		// ����� ���� friend-����������, ����� ������� ������� ���������
		// ������������� ����������, � ��������� ������ ������� ���������
		// ����������-������
		if( toc->friendSpec )
			return new FriendFunctionMaker(clsType, curAccessSpec, toc, tooc) ;			
		else
			return new OperatorMemberMaker(clsType, curAccessSpec, toc, tooc) ;
	}

	// ����� ���� ��������� ����� - PC_DESTRUCTOR, ����������
	// ��������� �����������
	else if( np->GetChildPackage(0)->GetPackageID() == PC_DESTRUCTOR )
		return new DestructorMaker(clsType, curAccessSpec, toc);

	// ����� ���� ������ ����������� ����� ���������� � �������, ������
	// ��� ���� �����, ���� ��������� �������
	else if( toc->dtl.IsFunction() && toc->ssCode != KWTYPEDEF )
	{
		// ���� ����� ������������ ������ - ������ ���������� ���������
		// ��������� �������
		if( toc->friendSpec )
			return new FriendFunctionMaker(clsType, curAccessSpec, toc);
		else
			return new MethodMaker(clsType, curAccessSpec, toc);
	}

	// ����� ���������� ��������� �������-�����
	else 
		return new DataMemberMaker(clsType, curAccessSpec, toc);		
}


// ��������������� � ��������� ������� ���������
POperand UnaryExpressionCoordinator::Coordinate() const
{
	// ��������� ������ ��������������
	INTERNAL_IF( right.IsNull() );

	// ������� ���������, ���� ��������� ��������, ������� ��� ��
	if( right->IsErrorOperand() )
		return right;

	// ���� ������� ���, ������� ������
	if( right->IsTypeOperand() && op != KWSIZEOF && op != KWTYPEID )
	{
		theApp.Error(errPos, "��� �� ����� ���� ��������� � ���������");
		return ErrorOperand::GetInstance();
	}

	// ���� ������� ������������� �������, �������� ������ ����������
	// ��������� '&'
	if( right->IsOverloadOperand() && op != '&' )
	{
		theApp.Error(errPos, 
			"'%s' - ������������� ������� �� ����� ���� ��������� � ���������",
			static_cast<const OverloadOperand &>(*right).GetOverloadList().
			front()->GetName().c_str());
		return ErrorOperand::GetInstance();
	}

	// ���������, ���� �� ����� ����, �� �� ������ �������������� ������ �����
	// this, �� ����������� ��������� '&'.
	if( right->IsPrimaryOperand() && op != '&' )
		ExpressionMakerUtils::CheckMemberThisVisibility(right, errPos);

	// ����� �������� ��������� ������� �� ������������������
	POperand rval = UnaryInterpretator(right, op, errPos).Interpretate();
	if( !rval.IsNull() )
		return rval;

	// �������� ������� ������������� ��������, ���� ����������,
	// ��������� ������������� � ����� �������
	rval = UnaryOverloadOperatorCaller(right, op, errPos).Call();
	if( !rval.IsNull() )
		return rval;

	// ����� ��������� ���������� ���������
	switch( op )
	{
	case '!':  return LogicalUnaryMaker(right, op, errPos).Make();
	case '~':  return BitReverseUnaryMaker(right, op, errPos).Make();
	case '+':  
	case '-':  return ArithmeticUnaryMaker(right, op, errPos).Make();
	case '*':  return IndirectionUnaryMaker(right, op, errPos).Make();
	case INCREMENT:
	case DECREMENT:
	case -INCREMENT:
	case -DECREMENT: return IncDecUnaryMaker(right, op, errPos).Make();
	case '&':		 return AddressUnaryMaker(right, op, errPos).Make();
	case KWDELETE:
	case OC_DELETE_ARRAY:  
	case -KWDELETE:
	case -OC_DELETE_ARRAY:
		return DeleteUnaryMaker(right, op, errPos).Make();
	case KWTYPEID:  return TypeidUnaryMaker(right, op, errPos).Make();
	case KWTHROW:	return ThrowUnaryMaker(right, op, errPos).Make();

		// ����� ����������� ������� ��������
	default:
		INTERNAL( "'UnaryExpressionCoordinator::Coordinate': ����������� ������� ��������");
	}

	return NULL;		// kill warning
}


// ��������� ��������������� �������� ������������ ���������
// � ��������� ���
POperand TernaryExpressionCoordinator::Coordinate() const
{
	// ��������� ������ ��������������
	INTERNAL_IF( cond.IsNull() || left.IsNull() || right.IsNull() );

	// ������ ������� ������ ���� ����������
	if( cond->IsErrorOperand() || left->IsErrorOperand() || right->IsErrorOperand() )
		return ErrorOperand::GetInstance();

	// ���� ������� ���, ������� ������
	if( cond->IsTypeOperand() || left->IsTypeOperand() || right->IsTypeOperand() )
	{
		theApp.Error(errPos, "��� �� ����� ���� ��������� � ���������");
		return ErrorOperand::GetInstance();
	}

	// ���� ������� ������������� �������
	if( cond->IsOverloadOperand() || left->IsOverloadOperand() || right->IsOverloadOperand() )
	{
		theApp.Error(errPos, 
			"������������� ������� �� ����� ���� ��������� � ���������");
		return ErrorOperand::GetInstance();
	}

	// �������� �� ������� 'this' � ������� ������� ���������, ����
	// ������� �������� ������������� ������-������ ������
	if( left->IsPrimaryOperand() )
		ExpressionMakerUtils::CheckMemberThisVisibility(left, errPos);
	if( right->IsPrimaryOperand() )
		ExpressionMakerUtils::CheckMemberThisVisibility(right, errPos);		
	if( cond->IsPrimaryOperand() )
		ExpressionMakerUtils::CheckMemberThisVisibility(cond, errPos);		

	// ����� �������� ��������� ������� �� ������������������
	POperand rval = TernaryInterpretator(cond, left, right).Interpretate();
	if( !rval.IsNull() )
		return rval;

	// ����� ������ ���������
	return IfTernaryMaker(cond, left, right, op, errPos).Make();
}
