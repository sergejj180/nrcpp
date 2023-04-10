// ��������� ���� �������, ��������� � ������ - BodyMaker.h


// ������� ��� ��������� ���� �������
namespace BodyMakerUtils
{
	// ������� ������������� ������� �� ������ ��������� � �����������.
	// ������� ������������� �������������
	PObjectInitializator MakeObjectInitializator( 
		const PExpressionList &initList, const DeclarationMaker &dm );

	// ������� ������������� �� ������ �������������
	PObjectInitializator MakeObjectInitializator( const PListInitComponent &il );

	// ��������� ���������� ������� ��� ����������� if, for, switch, while
	// �� ������ ������� � ����������� � ���������������
	PInstruction MakeCondition( const NodePackage &decl, 
				const POperand &iator, const Position &errPos );

	// ���������, ����� �������� ����������� ����������������� � �������� ��� ����� ���
	// ����������� ����� ���� ����������, ���� ���������. � ������ ����������,
	// ������� ��������� �������� ������� �� ������ ����������� � ��� ���������.
	// ���� toInt, ���������, ��� �������������� ������ ������������ � ����� ���,
	// � ��������� ������ � ��������
	void ValidCondition( const PInstruction &cond, PCSTR cnam, bool toInt = false );

	// ��������� ������� ������ ��� ���������
	void ValidCondition( const POperand &exp, PCSTR cnam, const Position &ep );

	// ������� ������� ��������� ��� ����� ����������� ������������� 
	// �� ����������, ���� ������� ���������� true
	bool MakeLocalSymbolTable( const Construction &cur );

	// ��������� ������� �������� ����������, ������� ������� ������ �������
	template <class Component>
	inline Component *SimpleComponentMaker( const Position &ep ) {
		return new Component(ep);
	}

	// ��������� ������� �������� ����������, ������� ������� 
	// ������������ ����������� � �������
	template <class Cons>
	inline Cons *SimpleConstructionMaker( Construction &parent, const Position &ep ) {
		return new Cons(&parent, ep);
	}

	// �������� �����������: if, switch, while
	template <class Cons>
	inline Cons *ConditionConstructionMaker( const PInstruction &cond, 
		Construction &parent, const Position &ep ) {	
		return new Cons(cond, &parent, ep);
	}

	// ������� for-�����������, � ��������� ��������� ���� ����
	inline ForConstruction *ForConstructionMaker( const ForInitSection &fic, 
		const PInstruction &cond, const POperand &iter, Construction &ppc, const Position &ep ) {
		if( !cond.IsNull() )
			BodyMakerUtils::ValidCondition( cond, "for" );	
		return new ForConstruction(fic, cond, iter, &ppc, ep);;
	}

	// ��������� ���������
	inline ExpressionInstruction *ExpressionInstructionMaker( 
		const POperand &exp, const Position &ep ) {

		return new ExpressionInstruction(exp, ep);
	}

	// ��������� ����������
	inline DeclarationInstruction *DeclarationInstructionMaker( const TypyziedEntity &dator, 
		const PObjectInitializator &iator, const Position &ep ) {
		// ���������� ������ ���� �������� ��� ��������
		INTERNAL_IF( &dator == NULL ||
			!(dator.IsObject() || dator.IsFunction()) ); 

		return new DeclarationInstruction(dator, iator, ep);
	}

	// ��������� ���������� ������
	inline ClassDeclarationInstruction *ClassInstructionMaker( 
		const ClassType &cls, const Position &ep ) {
		return new ClassDeclarationInstruction(cls, ep);
	}

	// ��������� ���������� ���������� ������
	inline ClassDeclarationInstruction *ClassDeclarationInstructionMaker( 
		const ClassType &cls, const Position &ep ) {
		return new ClassDeclarationInstruction(cls, ep);			
	}

	// ��������� ������� �����
	SimpleLabelBodyComponent *SimpleLabelMaker( 
		const Label &lab, const BodyComponent &nc, FunctionBody &fnBody, const Position &ep );
		
	// ��������� case, � ���������
	CaseLabelBodyComponent *CaseLabelMaker( const POperand &exp, 
		const BodyComponent &childBc, const Construction &cur, const Position &ep );

	// ��������� default, � ���������
	DefaultLabelBodyComponent *DefaultLabelMaker( 
		const BodyComponent &childBc, const Construction &cur, const Position &ep );

	// ��������� asm
	inline AsmAdditionalOperation *AsmOperationMaker( 
							const CharString &lit, const Position &ep ) {
		return new AsmAdditionalOperation(lit.c_str(), ep);
	}

	// ��������� else, � ���������
	ElseConstruction *ElseContructionMaker( const Construction &cur, const Position &ep );

	// ��������� break-�������� � ������������� ��������� 
	BreakAdditionalOperation *BreakOperationMaker( Construction &ppc, const Position &ep );

	// ��������� continue-��������
	ContinueAdditionalOperation *ContinueOperationMaker( Construction &ppc, const Position &ep );
	
	// ��������� return-��������. ����������� �� ����� ���������� ��������, �����
	// ��� ����������
	ReturnAdditionalOperation *ReturnOperationMaker( 
		const POperand &rval, const Function &fn, const Position &ep );

	// ��������� goto-��������
	GotoAdditionalOperation *GotoOperationMaker( 
		const CharString &labName, FunctionBody &fnBody, const Position &ep );

	// ��������� ������ ����������. ����� ���� ���� ����������, ���� ���������
	Instruction *InstructionListMaker( const InstructionList &insList, const Position &ep );
}


// ���������� �����������. ������ �� ������ ������� �����������, ���������
// �������� ��������� � ��������� �� �������� ������������� ���� �������
class ConstructionController
{
	// ������� ����������� � ���� �������������� �������
	const Construction *pCurrentConstruction;

public:
	// ������ ������� �����������
	ConstructionController( const Construction &pCur )
		: pCurrentConstruction(&pCur) {
	}

	// ������ ������� ����������� �����. ��������� ��� �������� ������
	// ��������� ������� ��������� ��� �����
	void SetCurrentConstruction( const Construction *pCur ) {
		INTERNAL_IF( pCur == NULL );
		pCurrentConstruction = pCur;
	}

	// �������� ������� �����������
	Construction &GetCurrentConstruction() const {
		return const_cast<Construction &>(*pCurrentConstruction); 
	}
};


// ��������� catch-����������� � ���������
class CatchConstructionMaker
{
	// ��������������� ������
	const PTypyziedEntity &catchObj;

	// ������������ try-�����������
	Construction &parent;

	// �������
	const Position &ep;

	// ���������, �������� �� ����������� �����������
	bool EqualCatchers( const CatchConstruction &cc1, const CatchConstruction &cc2 ) const;

public:
	// ��������� ������ (����� ���� 0, � ������ ...), 
	// ������������ ����������� (try) � �������
	CatchConstructionMaker( const PTypyziedEntity &catchObj, Construction &parent,
		const Position &ep ) 
		: catchObj(catchObj), parent(parent), ep(ep) {
		INTERNAL_IF( parent.GetConstructionID() != Construction::CC_TRY );
	}

	// �������
	CatchConstruction *Make();
};


// �������� ���� ������� ����� ���������. ��������� ��� �����, � �������
// ���� ��������� ����� goto. ��������� ����� goto, case, default �� �������������
// ����� ����������
class PostBuildingChecks
{
	// ������ �� ���� �������
	const FunctionBody &body;

	// ����� ��������, ������������ ��� ������ ����� � ������
	class LabelPr {
		// ��� ������� �����
		const string &lname;

	public:
		// ������ ���
		LabelPr( const string &ln )
			: lname(ln) {
		}

		// ���� ��� �������, ������� true
		bool operator()( const Label &lab ) {
			return lname == lab.GetName().c_str();
		}
	};

	// ��������, ����� ��� �����, � ������� ���� ��������� �����
	// goto, ���� ���������
	void CheckLabels( const FunctionBody::DefinedLabelList &dll, 
		const FunctionBody::QueryLabelList &qll );

public:
	// ������ ����
	PostBuildingChecks( const FunctionBody &body )
		: body(body) {
	}

	// ��������� ��������
	void DoChecks() {
		CheckLabels(body.GetDefinedLabelList(), body.GetQueryLabelList());
	}
};
