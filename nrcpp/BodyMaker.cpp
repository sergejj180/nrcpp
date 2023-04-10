// ���������� ���������� ���� ������� - BodyMaker.cpp

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
#include "Body.h"
#include "ExpressionMaker.h"
#include "MemberMaker.h"
#include "Coordinator.h"
#include "Overload.h"
#include "BodyMaker.h"


// ������� ������������� ������� �� ������ ��������� � �����������.
// ������� ������������� �������������
PObjectInitializator BodyMakerUtils::MakeObjectInitializator( 
		const PExpressionList &initList, const DeclarationMaker &dm )
{
	static PExpressionList emptyList = new ExpressionList;

	// ���� ������������� �������, ���������� �������������, ����� ������� NULL
	if( const GlobalObjectMaker *gom = dynamic_cast<const GlobalObjectMaker *>(&dm) )
		return new ConstructorInitializator( 
			initList.IsNull() ? emptyList : initList, gom->GetConstructor() );

	// ���� ������������� ������������ ����� ������
	else if( const MemberDefinationMaker *mdm = dynamic_cast<const MemberDefinationMaker *>(&dm) )
		return new ConstructorInitializator( 
			initList.IsNull() ? emptyList : initList, mdm->GetConstructor() );
	// ����� 
	return NULL;
}


// ������� ������������� �� ������ �������������
PObjectInitializator BodyMakerUtils::MakeObjectInitializator( const PListInitComponent &il )
{
	return new AgregatListInitializator( il );
}


// ��������� ���������� ������� ��� ����������� if, for, switch, while
// �� ������ ������� � ����������� � ���������������
PInstruction BodyMakerUtils::MakeCondition( 
		const NodePackage &rpkg, const POperand &iator, const Position &errPos )
{
	INTERNAL_IF( rpkg.GetPackageID() != PC_DECLARATION || rpkg.GetChildPackageCount() != 2 );
	const NodePackage *tsl = static_cast<const NodePackage*>(rpkg.GetChildPackage(0)),
		*decl =static_cast<const NodePackage*>(rpkg.GetChildPackage(1));
	
	AutoDeclarationCoordinator dcoord(tsl, const_cast<NodePackage *>(decl));
	PDeclarationMaker dmak = dcoord.Coordinate();
	
	// ������ ����������
	INTERNAL_IF( dmak.IsNull() );
	dmak->Make();

	// ������������� �������, ������ ��� ���������
	PObjectInitializator objIator = NULL;
	PExpressionList	initList = new ExpressionList;
	initList->push_back(iator);
	dmak->Initialize( *initList ),

	// ��������� ����� ������� NULL, ���� ������������� �� �������� ��������			
	objIator = BodyMakerUtils::MakeObjectInitializator(initList, *dmak);
	return new DeclarationInstruction(
		*dynamic_cast<const TypyziedEntity *>(dmak->GetIdentifier()), objIator, errPos);
}


// ���������, ����� �������� ����������� ����������������� � ��� bool.
// ����������� ����� ���� ����������, ���� ���������. � ������ ����������,
// ������� ��������� �������� ������� �� ������ ����������� � ��� ���������
void BodyMakerUtils::ValidCondition( const PInstruction &cond, PCSTR cnam, bool toInt )
{
	INTERNAL_IF( cond.IsNull() || 
		!(cond->GetInstructionID() == Instruction::IC_DECLARATION ||
		  cond->GetInstructionID() == Instruction::IC_EXPRESSION) );

	// ������� ������
	const Position &errPos = cond->GetPosition();

	// ���� ���������, ��������� ��� ��������
	if( cond->GetInstructionID() == Instruction::IC_EXPRESSION )
	{
		const POperand &exp = static_cast<const ExpressionInstruction&>(*cond).GetExpression();
		// ���� ������� �� �������� ����������, ������� ������
		if( !(exp->IsExpressionOperand() || exp->IsPrimaryOperand()) )
		{
			if( !exp->IsErrorOperand() )				
				theApp.Error(errPos, "'%s' - ��������� ������ ���� ���� 'bool'", cnam);
			return;
		}

		// ���������, ����� ��� ��� ��������
		toInt
		? ExpressionMakerUtils::ToIntegerType(const_cast<POperand&>(exp), errPos, cnam)		 
		: ExpressionMakerUtils::ToScalarType(const_cast<POperand&>(exp), errPos, cnam);
	}

	// ����� ����������
	else
	{
		const TypyziedEntity &declarator = 
			static_cast<const DeclarationInstruction&>(*cond).GetDeclarator();

		// ������ �� ����������� �������� ������� � ���������
		POperand prim = new PrimaryOperand(true, declarator);
		toInt
		? ExpressionMakerUtils::ToIntegerType(prim, errPos, cnam)
		: ExpressionMakerUtils::ToScalarType(prim, errPos, cnam);
	}
}


// ��������� ������� ������ ��� ���������
void BodyMakerUtils::ValidCondition( const POperand &exp, PCSTR cnam, const Position &ep ) 
{
	// ���� ������� �� �������� ����������, ������� ������
	if( !(exp->IsExpressionOperand() || exp->IsPrimaryOperand()) )
	{
		if( !exp->IsErrorOperand() )
			theApp.Error(ep, "'%s' - ��������� ������ ���� ���� 'bool'", cnam);
		return;
	}

	// ���������, ����� ��� ��� ��������
	ExpressionMakerUtils::ToScalarType(const_cast<POperand&>(exp), ep, cnam);
}


// ������� ������� ��������� ��� ����� ����������� ������������� 
// �� ����������. 
bool BodyMakerUtils::MakeLocalSymbolTable( const Construction &cur )
{
	// ���� ������� switch, while, for, catch, else
	if( cur.GetConstructionID() == Construction::CC_IF    ||
		cur.GetConstructionID() == Construction::CC_FOR   ||
		cur.GetConstructionID() == Construction::CC_WHILE ||		
		cur.GetConstructionID() == Construction::CC_CATCH ||
		cur.GetConstructionID() == Construction::CC_ELSE  ||
		cur.GetConstructionID() == Construction::CC_DOWHILE )
		return false;

	GetScopeSystem().MakeNewSymbolTable( new LocalSymbolTable(GetCurrentSymbolTable()) );
	return true;
}


// ��������� case, � ���������
CaseLabelBodyComponent *BodyMakerUtils::CaseLabelMaker( const POperand &exp, 
			const BodyComponent &childBc, const Construction &cur, const Position &ep )
{
	// ��������� ���� ���������. ��������� ������ ���� ����� �����������
	double ival = 0;
	if( !ExpressionMakerUtils::IsInterpretable(exp, ival)  ||
		!ExpressionMakerUtils::IsIntegral(exp->GetType()) )
		theApp.Error( ep, "'case' - ��������� ������ ���� ����� � ����������");

	// ������� switch-����������� � ��������
	const Construction *fnd = &cur;
	while( fnd )
	{
		if( fnd->GetConstructionID() == Construction::CC_SWITCH )
			break;
		fnd = fnd->GetParentConstruction();
	}

	if( !fnd )	
		theApp.Error( ep, "case ��� switch" );

	// ����� ��������� ��������, ��� �� ����� ����� ���
	else
	{
		const SwitchConstruction &sc = static_cast<const SwitchConstruction &>(*fnd);
		for( LabelList::const_iterator p = sc.GetLabelList().begin();
			 p != sc.GetLabelList().end(); p++ )
		{
			if( (*p)->GetLabelID() == LabelBodyComponent::LBC_CASE &&
				static_cast<const CaseLabelBodyComponent &>(**p).GetCaseValue() == ival )
			{
				theApp.Error(ep, "'%d' - case-����� ��� ������", (int)ival);
				break;
			}
			
		}

		// ������ ����� ��� switch
		CaseLabelBodyComponent *cbc = new CaseLabelBodyComponent(ival, childBc, ep);
		const_cast<SwitchConstruction &>(sc).AddLabel(cbc);
		return cbc;
	}

	// ����� ���������� �����, ���� �� �� NULL
	return new CaseLabelBodyComponent(ival, childBc, ep);
}


// ��������� default, � ���������
DefaultLabelBodyComponent *BodyMakerUtils::DefaultLabelMaker( 
		const BodyComponent &childBc, const Construction &cur, const Position &ep )
{
	// ������� switch-����������� � ��������
	const Construction *fnd = &cur;
	while( fnd )
	{
		if( fnd->GetConstructionID() == Construction::CC_SWITCH )
			break;
		fnd = fnd->GetParentConstruction();
	}

	if( !fnd )	
		theApp.Error( ep, "default ��� switch" );

	// ����� ��������� ��������, ��� �� ����� ����� ���
	else
	{
		const SwitchConstruction &sc = static_cast<const SwitchConstruction &>(*fnd);
		for( LabelList::const_iterator p = sc.GetLabelList().begin();
			 p != sc.GetLabelList().end(); p++ )
		{
			if( (*p)->GetLabelID() == LabelBodyComponent::LBC_DEFAULT )
			{
				theApp.Error(ep, "default-����� ��� ������");
				break;
			}
			
		}

		// ������ ����� ��� switch
		DefaultLabelBodyComponent *dlbc = new DefaultLabelBodyComponent(childBc, ep);
		const_cast<SwitchConstruction &>(sc).AddLabel(dlbc);
		return dlbc;
	}

	// ����� ���������� �����, ���� �� NULL
	return new DefaultLabelBodyComponent(childBc, ep);
}


// ��������������� �������� ��� ������ �����
class LabelPredicat
{
	// ��� �����
	const CharString &lname;

public:
	// ������ ���
	LabelPredicat( const CharString &lname ) 
		: lname(lname) {
	}

	// ��������
	bool operator()( const Label &lbc ) {
		return lbc.GetName() == lname;
	}
};

// ��������� ������� �����
SimpleLabelBodyComponent *BodyMakerUtils::SimpleLabelMaker( 
	const Label &lab, const BodyComponent &nc, FunctionBody &fnBody, const Position &ep ) 
{
	const FunctionBody::DefinedLabelList &pll = fnBody.GetDefinedLabelList();	
	SimpleLabelBodyComponent *nl = new SimpleLabelBodyComponent(lab, nc, ep);

	if( find_if(pll.begin(), pll.end(), LabelPredicat(lab.GetName()) ) != pll.end() )
		theApp.Error(ep, "'%s' - ����� ��� ���������", lab.GetName().c_str());
	else
		fnBody.AddDefinedLabel(lab);
	return nl;
}


// ��������������� �������, ��������� ������������ ������������� �������� ��� return
static void ValidateReturnValue( const POperand &rval, 
				const TypyziedEntity &rtype, const Position &ep )
{
	// ��������, ���� �������������� �� ���������� � ���������,
	// ������ �-�� ����������� ������ ���� ��������
	if( !(rtype.GetBaseType().IsClassType() && rtype.GetDerivedTypeList().IsEmpty()) )
		return;
		
	// ����� ��������� ���������
	const ClassType &cls = static_cast<const ClassType&>(rtype.GetBaseType());
	
	// ��������� �� ���� �� ��� ��������������
	const POperand &exp = rval;
	if( exp->IsExpressionOperand() &&
		static_cast<const Expression&>(*exp).IsFunctionCall() )
	{
		const FunctionCallExpression &fnc = 
			static_cast<const FunctionCallExpression&>(*exp);
		if( &fnc.GetFunctionOperand()->GetType().GetBaseType() == &cls	&&
			dynamic_cast<const ConstructorMethod *>(
			&fnc.GetFunctionOperand()->GetType()) != NULL )
			return;
	}
	
	// � ��������� ������, ���������, ����� � ������� ��� �������� �-��� �����������
	// � ����������
	ExpressionMakerUtils::ObjectCreationIsAccessible(cls, ep, false, true, true);
}


// ��������� return-��������. ����������� �� ����� ���������� ��������, �����
// ��� ����������
ReturnAdditionalOperation *BodyMakerUtils::ReturnOperationMaker( 
		const POperand &rval, const Function &fn, const Position &ep )
{
	bool procedure = fn.GetBaseType().GetBaseTypeCode() == BaseType::BT_VOID &&
		fn.GetDerivedTypeList().GetDerivedTypeCount() == 1;

	// ���� ����� �����������, �� ������������� �������� �� ������ ����
	if( fn.IsClassMember() && static_cast<const Method &>(fn).IsConstructor() )
	{
		if( !rval.IsNull() )
			theApp.Error( ep, "����������� �� ����� ����� ������������� ��������" );
	}

	// ��������� �����������, ������� ������ ���������� void
	else if( rval.IsNull() )
	{
		if( !procedure )
			theApp.Error( ep, "return ������ ���������� ��������" );
	}

	// ����� ���������� ��� ������������� �������� � ��� �������
	else if( !rval->IsErrorOperand() )
	{
		// �������� �� �������������� ��
		PTypyziedEntity rtype = new TypyziedEntity(fn);
		const_cast<DerivedTypeList&>(rtype->GetDerivedTypeList()).PopHeadDerivedType();

		// �������� ����
		PCaster pc = AutoCastManager( *rtype, rval->GetType(), true ).RevealCaster();
		pc->ClassifyCast();
		if( !pc->IsConverted() )
		{
			if( pc->GetErrorMessage().empty() )
				theApp.Error(ep, 
					"'return' - ���������� ������������� '%s' � '%s'",
					rval->GetType().GetTypyziedEntityName(false).c_str(),
					rtype->GetTypyziedEntityName(false).c_str());
			else
				theApp.Error(ep, "'return' - %s", pc->GetErrorMessage().c_str());			;
		}

		else
		{
			const TypyziedEntity &rt = *rtype.Release();
			POperand fnOp = new PrimaryOperand(true, rt);
			pc->DoCast(fnOp, const_cast<POperand&>(rval), ep);

			// ��������� ����������� �-�� �����������, ���� ���������
			ValidateReturnValue(rval, rt, ep);
		}
	}
	
	// ���������� ���������
	return new ReturnAdditionalOperation(rval, ep);
}


// ��������� break-�������� � ������������� ��������� 
BreakAdditionalOperation *BodyMakerUtils::BreakOperationMaker( 
								Construction &ppc, const Position &ep )
{
	const Construction *fnd = &ppc;
	while( fnd )
	{
		Construction::CC cc = fnd->GetConstructionID();
		if( cc == Construction::CC_SWITCH || cc == Construction::CC_FOR ||
			cc == Construction::CC_WHILE  || cc == Construction::CC_DOWHILE )
			break;
		fnd = fnd->GetParentConstruction();
	}

	if( !fnd )	
		theApp.Error( ep, "break ��� ����� ��� switch" );
	return new BreakAdditionalOperation(ep);
}


// ��������� continue-�������� � ������������� ��������� 
ContinueAdditionalOperation *BodyMakerUtils::ContinueOperationMaker( 
								Construction &ppc, const Position &ep )
{
	const Construction *fnd = &ppc;
	while( fnd )
	{
		Construction::CC cc = fnd->GetConstructionID();
		if( cc == Construction::CC_FOR    ||
			cc == Construction::CC_WHILE  || cc == Construction::CC_DOWHILE )
			break;
		fnd = fnd->GetParentConstruction();
	}

	if( !fnd )	
		theApp.Error( ep, "continue ��� �����" );
	return new ContinueAdditionalOperation(ep);
}


// ��������� goto-��������
GotoAdditionalOperation *BodyMakerUtils::GotoOperationMaker( 
		const CharString &labName, FunctionBody &fnBody, const Position &ep )
{
	// ������ ���� ������� ��������� � �����
	fnBody.AddQueryLabel( labName.c_str(), ep );
	return new GotoAdditionalOperation(labName.c_str(), ep);
}


// ���������, �������� �� ����������� �����������
bool CatchConstructionMaker::EqualCatchers( 
					const CatchConstruction &cc1, const CatchConstruction &cc2 ) const
{
	if( cc1.GetCatchType().IsNull() || cc2.GetCatchType().IsNull() )	
		return cc1.GetCatchType().IsNull() && cc2.GetCatchType().IsNull();
	
	ScalarToScalarCaster stsc(*cc1.GetCatchType(), *cc2.GetCatchType(), false);
	stsc.ClassifyCast();
	return stsc.IsConverted() && stsc.GetCastCategory() == Caster::CC_EQUAL;
}


// �������
CatchConstruction *CatchConstructionMaker::Make()
{
	TryCatchConstruction &tcc = static_cast<TryCatchConstruction &>(parent);	
	CatchConstruction *cc = new CatchConstruction(catchObj, &parent, ep);

	// ���������, ����� try �� �������� ���������� ������������
	for( CatchConstructionList::const_iterator p = tcc.GetCatchList().begin(); 
		 p != tcc.GetCatchList().end(); p++ )
		if( EqualCatchers( **p, *cc ) )
		{
			theApp.Error(ep, "catch-���� � ����� '%s' ��� ������������",
				catchObj.IsNull() ? "..." : catchObj->GetTypyziedEntityName(false).c_str());
			break;
		}
	tcc.AddCatchConstruction( *cc );
	return cc;
}


// ��������� ������ ����������. ����� ���� ���� ����������, ���� ���������
Instruction *BodyMakerUtils::InstructionListMaker( 
				const InstructionList &insList, const Position &ep )
{
	// ���� ������ ������, ���� ������, �� � ����� ������ ������� �������
	// �� NULL, ���������� ������ ����������
	if( insList.empty() )
		return SimpleComponentMaker<EmptyInstruction>(ep);

	// ���� ���������� ����, ���������� �� ����
	else if( insList.size() == 1 )
		return const_cast<PInstruction &>(insList.front()).Release();

	// ����� ���������� ���������, ��������� ���� ����������
	else
		return new DeclarationBlockInstruction(insList, ep);
}


// ��������, ����� ��� �����, � ������� ���� ��������� �����
// goto, ���� ���������
void PostBuildingChecks::CheckLabels( const FunctionBody::DefinedLabelList &dll, 
		const FunctionBody::QueryLabelList &qll )
{
	for( FunctionBody::QueryLabelList::const_iterator p = qll.begin(); p != qll.end(); p++ )
		if( find_if(dll.begin(), dll.end(), LabelPr( (*p).first ) ) == dll.end() )
			theApp.Error( (*p).second, "'%s' - ����� �� ���������",
				(*p).first.c_str() );
}
