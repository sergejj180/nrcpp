// ��������� ��� ����������� ���� ������� - Body.h


// �������� � ExpressionMaker.h
class ObjectInitElement;

// ��� - ������ ��������� ������������� �������
typedef list<ObjectInitElement> ObjectInitElementList;


// ����������� �����, ������������ ����� ��������� ��� ����������� ���������. 
// ���� ���������, ����� �������� ��������� � ��������� ���� �����. 
class Operand
{
public:

	// ����������� ���������� ��� �����������
	virtual ~Operand() {
	}

	// �������� �� ������� �����
	virtual bool IsTypeOperand() const {
		return false;
	}

	// �������� �� ������� ���������� (�������, �������� ��� ���������)
	virtual bool IsExpressionOperand() const {
		return false;
	}

	// �������� �� ������� �������� (�������������, �������, true, false, this)
	virtual bool IsPrimaryOperand() const {
		return false;
	}

	// �������� �� ������� ������� ������������� �������
	virtual bool IsOverloadOperand() const {
		return false;
	}

	// �������� �� ������� ���������
	virtual bool IsErrorOperand() const {
		return false;
	}

	// ������� ��� ���������. ��� OverloadOperand � IsErrorOperand,
	// ���������� ���������� ������ �����������
	virtual const TypyziedEntity &GetType() const = 0;
};


// ���������������� ��������� �� �������
typedef SmartPtr<Operand> POperand;


// �����-�����. ������������ ��� �������� ��������� � ���� ������.
// ��������� ���, ��� ���������� ��������������� ����� �����������
// � ���������� 
class ExpressionPackage : public Package
{
	// ���� ���������
	POperand expression;

public:

	// ����������� ��������� ��������� � ���������, ��� ������ PC_EXPRESSION
	ExpressionPackage( const POperand &exp ) 
		: expression(exp) {
	}

	// �����-���������
	bool IsExpressionPackage() const {
		return true;
	}

	// �������� ���������
	const POperand &GetExpression() const {
		return expression;
	}

	// �������� ������������� ������
	int GetPackageID() const {
		return PC_EXPRESSION;
	}

	// ������� ���������� ����������
	void Trace() const {
		cout << "<���������>\n";
	}
};


// ����������� �����, ������������ ����� ��������� ��� ���������� ���������. 
class Expression : public Operand
{
	// ���������� ��������� � ���������
	int operandCount;

	// ��� ��������� ������������� � ���������
	int operatorCode;

	// �������� �� ��������� lvalue
	bool lvalue;

	// ���� ��������� � �������, ������ � true
	bool inCramps;

	// �������� �����������, ����� ������ ����������� ������� ������
	// Expression, �� ����������� �������
protected:

	// � ������������ �������� ���������� � ���������
	Expression( int opCnt, int opCode, bool lv ) 
		: operandCount(opCnt), operatorCode(opCode), lvalue(lv), inCramps(false) {
	}

public:

	// �������� �� ������� ���������� (�������, �������� ��� ���������)
	bool IsExpressionOperand() const {
		return true;
	}

	// ���� ��������� �������� �������
	bool IsUnary() const {
		return operandCount == 1;
	}
	
	// ���� ��������� �������� ��������
	bool IsBinary() const {
		return operandCount == 2;
	}
	
	// ���� ��������� �������� ���������
	bool IsTernary() const {
		return operandCount == 3;
	}

	// ���� ��������� �������� ������� �������
	bool IsFunctionCall() const {
		return operatorCode == OC_FUNCTION;
	}

	// ���� ��������� new ��� new[]
	bool IsNewExpression() const {
		return operatorCode == KWNEW || operatorCode == OC_NEW_ARRAY;
	}

	// �������� �� ��������� lvalue
	bool IsLvalue() const {
		return lvalue;
	}

	// ���� ��������� ��������� � �������, ������� true
	bool IsInCramps() const {
		return inCramps;
	}

	// �������� ��� ���������
	int GetOperatorCode() const {
		return operatorCode;
	}

	// ������ ��������� ��� rvalue
	void SetRValue() {
		lvalue = false;
	}

	// ������, ��� ��������� � �������
	void SetCramps() {
		inCramps = true;
	}
};


// ������� ���������
class UnaryExpression : public Expression
{
	// ��������� �� ������� � ������� ���������
	POperand pOperand;

	// ���� true, �� �������� - ��� ����������� -�������
	bool postfix;

	// �������������� ��� ���������
	PTypyziedEntity pType;

public:

	// � ������������ ������ ������ � ���������
	UnaryExpression( int opCode, bool lv, bool pfix, const POperand &pop, 
					 const PTypyziedEntity &pt ) 
		: Expression(1, opCode, lv), postfix(pfix), pOperand(pop), pType(pt) {
	}


	// �������� �� ��������� �����������. ��������� � ������������ ����������� 
	// ++, -- ����� ���� ������������ �������� � ������������
	bool IsPostfix() const {
		return postfix;
	}

	// ������� �������
	const POperand &GetOperand() const {
		return pOperand;
	}

	// �������� ��� ���������
	const TypyziedEntity &GetType() const {
		return *pType;
	}
};


// �������� ���������
class BinaryExpression : public Expression
{
	// �������� ��������� ���������
	POperand pOperand1, pOperand2;

	// �������������� ��� ���������
	PTypyziedEntity pType;

public:

	// � ������������ ������ ������ � ���������
	BinaryExpression ( int opCode, bool lv, 
		const POperand &pop1, const POperand &pop2, const PTypyziedEntity &pt ) 

		: Expression(2, opCode, lv), pOperand1(pop1), pOperand2(pop2), pType(pt) {
	}

	// ������� ��� ��������1
	const POperand &GetOperand1() const {
		return pOperand1;
	}

	// ������� ��� ��������2
	const POperand &GetOperand2() const {
		return pOperand2;
	}

	// �������� ��� ���������
	const TypyziedEntity &GetType() const {
		return *pType;
	}
};

// ��������� ���������
class TernaryExpression : public Expression
{
	// �������� ���������� ���������
	POperand pOperand1, pOperand2, pOperand3;

	// �������������� ��� ���������
	PTypyziedEntity pType;

public:

	// � ������������ ������ ������ � ���������
	TernaryExpression ( int opCode, bool lv, const POperand &pop1, 
		const POperand &pop2, const POperand &pop3, const PTypyziedEntity &pt ) 
		: Expression(3, opCode, lv), pOperand1(pop1), pOperand2(pop2), pOperand3(pop3), pType(pt)
	{
	}

	// ������� ��� ��������1
	const POperand &GetOperand1() const {
		return pOperand1;
	}

	// ������� ��� ��������2
	const POperand &GetOperand2() const {
		return pOperand2;
	}

	// ������� ��� ��������3
	const POperand &GetOperand3() const {
		return pOperand3;
	}
	
	// �������� ��� ���������
	const TypyziedEntity &GetType() const {
		return *pType;
	}
};


// ������ ���������
typedef vector<POperand> ExpressionList;

// ��� - ���������������� ��������� �� ������ ��������� (���������)
typedef SmartPtr< ExpressionList > PExpressionList;


// ��������� ����� �������
class FunctionCallExpression : public Expression
{
	// ��������� �� �������. ����� ���� PrimaryOperand, OverloadOperand,
	// TypeOperand. � �������� ������ ����� ������� ���������� ���������� ����
	POperand pFunction;

	// ������ ���������� �������, ����� ���� ������
	PExpressionList parametrList;

	// �������������� ��� ���������
	PTypyziedEntity pType;

public:

	// ������ ������� � ������ ����������. ������ ����� ���� ������. �������
	// ����� ���� ����� ���������, � ��� ����� ������������� � �����
	FunctionCallExpression( bool lv, const POperand &pfn, const PExpressionList &pel,
							const PTypyziedEntity &pt ) 
		: Expression(-1, OC_FUNCTION, lv), pFunction(pfn), parametrList(pel), pType(pt) {
	}

	// ������� ��������� �� �������. � ����� ������� ������� ������ ����
	// �������� � ������������ ����� ��������� ��� �������� �������
	const POperand &GetFunctionOperand() const {
		return pFunction;
	}

	// ������� ������ ����������
	const PExpressionList &GetParametrList() const {
		return parametrList;
	}

	// �������� ��� ���������
	const TypyziedEntity &GetType() const {
		return *pType;
	}
};


// ��������� new ��� new[]. ������������ ��� ������������� new-���������
class NewExpression : public Expression
{
	// ����������� ����� ������� ��������� ������
	POperand newOperatorCall;

	// ������ ���������������, ������� ������������ ��� �������������
	// ���������� �������
	PExpressionList initializatorList;

	// �������������� ���
	PTypyziedEntity pType;

	// ��� ��������� new ��� new[]
	int opCode;

public:

	// � ������������ ������ �����. ���������� ��� ��������
	NewExpression( int opc, const POperand &call, const PExpressionList &il,
				const PTypyziedEntity &pt ) 
		: Expression(-1, opc, false), newOperatorCall(call), initializatorList(il), pType(pt) {

		INTERNAL_IF( !(opc == KWNEW || opc == OC_NEW_ARRAY) );
	}

	// ������� ��������� �� �������. � ����� ������� ������� ������ ����
	// �������� � ������������ ����� ��������� ��� �������� �������
	const POperand &GetNewOperatorCall() const {
		return newOperatorCall;
	}

	// ������� ������ ����������
	const PExpressionList &GetInitializatorList() const {
		return initializatorList;
	}

	// �������� ��� ���������
	const TypyziedEntity &GetType() const {
		return *pType;
	}
};


// ��������� �������. ���� � �������� �������� ��������� �������� ������,
// ��� ��������� ������������� � ��������� �������. ���������� ������
// ���� ��������� ����� ��������
class ErrorOperand : public Operand
{
	// ������������ ��������� ���������� ��������
	static POperand errorOperand;

	// �������� ����������� ������������� �������� �������
	// ��� ������ ������
	ErrorOperand() { }

	// ����������� ����� �� Operand. �������� ���������� ������
	const TypyziedEntity &GetType() const {
		INTERNAL( "'ErrorOperand::GetType' - ������������ �����" );
		return *(TypyziedEntity *)0;
	}

public:

	// �������� �� ������� ���������
	bool IsErrorOperand() const {
		return true;
	}

	// ������� ������������� ���������� ��������
	static const POperand &GetInstance() {
		if( errorOperand.IsNull() )
			errorOperand = new ErrorOperand;

		return errorOperand;
	}
};


// �������� �������
class PrimaryOperand : public Operand
{
	// ��������� �� �������������� �������. ��� ����� ����
	// ������, �������, true, false, this. � ������ ����
	// ����� ������������� �������, ������� OverloadOperand,
	// ���� ������� ������������ ��������� PrimaryOperand
	const TypyziedEntity &pType;

	// ����� ���� lvalue, ���� ��� �������������
	bool lvalue;
	
public:
	// ������ ���������� �� ��������
	PrimaryOperand( bool lv, const TypyziedEntity &pt )
		: lvalue(lv), pType(pt) {
	}

	// ������� ������, ���������� �������������� ���������,
	// ������ ���� ��� �� �������� ���������������. � ���� ������
	// �������� ������� ������ ������
	~PrimaryOperand() {
		if( dynamic_cast<const Identifier *>(&pType) == NULL ) 
			delete &pType;		
	}

	// �������� �� ������� �������� (�������������, �������, true, false, this)
	bool IsPrimaryOperand() const {
		return true;
	}

	// �������� �� ��������� lvalue
	bool IsLvalue() const {
		return lvalue;
	}

	// �������� ��� ��������
	const TypyziedEntity &GetType() const {
		return pType;
	}

	// ������ ��������� ��� rvalue
	void SetRValue() {
		lvalue = false;
	}
};


// �������-���
class TypeOperand : public Operand
{
	// ��������� �� �������������� �������
	const TypyziedEntity &pType;
	
public:
	// ������ ���������� �� ��������
	TypeOperand( const TypyziedEntity &pt )
		: pType(pt) {
	}

	// ����������� ������ ������� �����. 
	// ��� ������ ��������� �����������
	~TypeOperand() {
		delete &pType;
	}

	// �������� �� ������� �����
	bool IsTypeOperand() const {
		return true;
	}

	// �������� ��� ��������
	const TypyziedEntity &GetType() const {
		return pType;
	}
};


// ��������� �� ������� �������
typedef SmartPtr<TypeOperand> PTypeOperand;


// ������ ������������� �������
typedef vector<const Function *> OverloadFunctionList;


// ������������� �������
class OverloadOperand : public Operand
{
	// ������ ������������� �������
	OverloadFunctionList overloadList;
		
	// ����������� ����� �� Operand. �������� ���������� ������
	const TypyziedEntity &GetType() const {
		INTERNAL( "'OverloadOperand::GetType' - ������������ �����" );
		return *(TypyziedEntity *)0;
	}

	// ���������� ������������� �������� �������� lvalue. �� ���
	// ������ ������, �� ���������� rvalue
	bool lvalue;

public:

	// �������� ������ ������������� �������
	OverloadOperand( const OverloadFunctionList &ovl )
		:   overloadList(ovl), lvalue(true)  {		
	}

	// ���� rvalue
	bool IsLvalue() const {
		return lvalue;
	}

	// �������� �� ������� ������� ������������� �������
	bool IsOverloadOperand() const {
		return true;
	}

	// ������� ������ ������������� �������
	const OverloadFunctionList &GetOverloadList() const {
		return overloadList;
	}

	
	// ������ ��������� ��� rvalue
	void SetRValue() {
		lvalue = false;
	}
};


// ���������� ����������� ������������� �������
//
// ������� ����� ��� ����������� �������������. �����������
// ����� ���: ��������� � ������. ������ ��� ����, ����� ����������
// �������� ����������� � ������� �������������. 
class InitComponent
{
	// ���� ��������� �������� ������, ���������� � true
	bool isAtom;

public:
	// ����������� � ������ ���������� � ����������
	InitComponent( bool isa ) 
		: isAtom(isa) {
	}

	// ���������� ��� ����������� ����������� ��������
	virtual ~InitComponent() = 0;

	// ������� �������
	virtual const Position &GetPosition() const = 0;

	// ���� ��������� ����
	bool IsAtom() const {
		return isAtom;
	}

	// ���� ��������� ������
	bool IsList() const {
		return !isAtom;
	}
};


// ������� ��������� �������������. ������� ������� ���������.
class AtomInitComponent : public InitComponent
{
	// ������ �� ���������
	POperand pExpr;

	// ������� ��� ������ ������
	Position errPos;

public:
	// ������ ��������� � �������
	AtomInitComponent( const POperand &pe, const Position &ep ) 
		: pExpr(pe), errPos(ep), InitComponent(true) {
	}

	// ����������� ����������� �����
	~AtomInitComponent() {
	}

	// ������� ���������
	const POperand &GetExpression() const {
		return pExpr;
	}

	// ������� �������
	const Position &GetPosition() const {
		return errPos;
	}
};


// ������ �������������, �������� ���������� �������������,
// ��������� ��� ���������. ����� ���� ������.
class ListInitComponent : public InitComponent
{	
public:
	// ��� ������
	typedef list<const InitComponent *> ICList;

private:
	// ������ ����������� �������������
	ICList icList;

	// ���������� � true, ���� ������ �����������, � �������� ������
	// ��� ���������������
	bool isDone;

	// ������� ������ ������	
	Position errPos;

public:
	// �� ���������
	ListInitComponent(const Position &ep) 
		: errPos(ep), isDone(false), InitComponent(false) {
	}

	// ���������� ������ ������� ������������
	~ListInitComponent() {
		for( ICList::iterator p = icList.begin(); p != icList.end(); ++p )
			delete *p;
		icList.clear();
	}

	// �������� ������ �����������
	const ICList &GetICList() const {
		return icList;
	}

	// �������� ��������� � ������
	void AddInitComponent( const InitComponent *ic ) {
		// ������ ��������� ���� ������ �����������
		INTERNAL_IF( isDone );	
		icList.push_back(ic);
	}

	// ��������� ���������� ������
	void Done() {
		isDone = true;
	}

	// ������� �������
	const Position &GetPosition() const {
		return errPos;
	}
};


// ���������������� ��������� �� ������ �������������
typedef SmartPtr<ListInitComponent> PListInitComponent;


// ����������� ����� �������������� �������. ���������������� ����� ��������,
// ������������� ������� ��������� (�������������), ������������� ���������� �������
class ObjectInitializator
{
public:
	// ����������� �����
	virtual ~ObjectInitializator() = 0;
	
	// ������� true, ���� ����� ���������� �� ������ ConstructorInitializator
	virtual bool IsConstructorInitializator() const {
		return false;
	}

	// ������� true, ���� ����� ���������� �� ������ AgregatListInitializator
	virtual bool IsAgregatListInitializator() const {
		return false;
	}
};


// ���������������� ��������� �� ������������� �������
typedef SmartPtr<ObjectInitializator> PObjectInitializator;


// ������������� �������������� �������. ������������� �������������,
// ��������� ������ ���������, ���� ������ ������, �� �� NULL � ��-��
// �� �����������, ���� NULL, ���� ��� �� ���������
class ConstructorInitializator : public ObjectInitializator
{
	// ������ ���������������, ����� ���� ������, �� �� NULL
	PExpressionList expList;

	// ��������� �� �����������, ������� ���������� ��� �������������,
	// ����� ���� NULL, ���� ��� �� ���������
	const ConstructorMethod *pCtor;

public:
	// ������ ������ ��������� � �����������
	ConstructorInitializator( const PExpressionList &el, const ConstructorMethod *pc )
		: expList(el), pCtor(pc) {
		INTERNAL_IF( expList.IsNull() );
	}

	// ������� ������ ���������������
	const PExpressionList &GetExpressionList() const {
		return expList;
	}

	// ������� ��������� �� �����������, ������� ���������� ��� ������������� �������,
	// ����� ���� NULL
	const ConstructorMethod *GetConstructor() const {
		return pCtor;
	}

	// ������� true, ���� ����� ���������� �� ������ ConstructorInitializator
	bool IsConstructorInitializator() const {
		return true;
	}
};


// ������������� �������������� �������. ������������� ���������� �������
class AgregatListInitializator : public ObjectInitializator
{
	// ��������� �� ������ �������������
	PListInitComponent initList;

public:
	// ������ ������ �������������
	AgregatListInitializator( const PListInitComponent &il )
		: initList(il) {
		INTERNAL_IF( initList.IsNull() );
	}

	// ������� true, ���� ����� ���������� �� ������ AgregatListInitializator
	bool IsAgregatListInitializator() const {
		return true;
	}
};


// ����� ����������� ���������� ���� �������
//
// ������� ����� ��� ���� ����������� �������
class BodyComponent
{
public:
	// ���� ���������� ����������� �������
	enum BCC { BCC_INSTRUCTION, BCC_CONSTRUCTION, BCC_LABEL, BCC_ADDITIONAL_OPERATION };

private:
	// ��� ���������� ��� �������������
	BCC bodyComponentId;

	// �������
	Position errPos;

public:
	// ������ ��� � �������
	BodyComponent( BCC bci, const Position &ep )
		: bodyComponentId(bci), errPos(ep) {
	}

	// ����������� ���������� ��� ������������ ������
	virtual ~BodyComponent() = 0;

	// ������� �������
	const Position &GetPosition() const {
		return errPos;
	}

	// ������� ��� ����������
	BCC GetComponentID() const {
		return bodyComponentId;
	}
};


// ��������� �� ��������� �������
typedef SmartPtr<BodyComponent> PBodyComponent;


// ������� ����� ��� ����������. ����������� �������� ���������,
// ������ ���������, ����������, ���������� ������, ���� ����������
class Instruction : public BodyComponent
{
public:
	// ���� ����������
	enum IC { 
		IC_EXPRESSION, IC_EMPTY, IC_DECLARATION, 
		IC_CLASS_DECLARATION, IC_DECLARATION_BLOCK
	};

private:
	// ��� ����������
	IC instructionId;

public:
	// ������ ��� ���������� � �������
	Instruction( IC ic, const Position &ep )
		: instructionId(ic), BodyComponent(BCC_INSTRUCTION, ep) {
	}

	// ����������� �����
	virtual ~Instruction() = 0;

	// ������� ��� ����������
	IC GetInstructionID() const {
		return instructionId;
	}
};


// ��������� �� ����������
typedef SmartPtr<Instruction> PInstruction;


// ������� ����� ��� �����������. ������������ �������� ���������,
// ������� �������� � ���� ������ �������������
class Construction : public BodyComponent
{
public:
	// ���� �����������
	enum CC {  
		CC_IF, CC_ELSE, CC_WHILE, CC_FOR, CC_DOWHILE, 
		CC_SWITCH, CC_TRY, CC_CATCH, CC_COMPOUND 
	};

private:
	// ��� �����������
	CC constructionId;

	// ��������� �� ������������ �����������
	const Construction *parentConstruction;

public:
	// ������ ��� ������������, ��������� �� ������������ ����������� � �������
	Construction( CC cc, const Construction *pc, const Position &ep )
		: constructionId(cc), parentConstruction(pc), BodyComponent(BCC_CONSTRUCTION, ep) {
	}

	// ������� ��� �����������
	CC GetConstructionID() const {
		return constructionId;
	}

	// ������� ������������ ����, ����� ���������� NULL, ���� �����������
	const Construction *GetParentConstruction() const {
		return parentConstruction;
	}

	
	// �������� �������� ��������� � ������� �����������. ������� ���������,
	// ��� ����� ����� ���������� ������ ������� ��� ���� �����������, �����
	// CompoundConstruction
	virtual void AddChildComponent( const BodyComponent *childComponent ) = 0;
	
	// ������� �������� ���������, ����� ���������� NULL, ���� �� ��� �� �����.
	// ���� ����������� ���������, ����� ������ �������� � ���������� ������
	virtual const BodyComponent *GetChildComponent() const = 0;
};


// ������� ����� ��� �����. ����� � ��������� �������������� ���������
// ������ ����������� ��������� � ���� ���������
class LabelBodyComponent : public BodyComponent
{
public:
	// ���� �����
	enum LBC { LBC_LABEL, LBC_CASE, LBC_DEFAULT };

private:
	// ��� �����
	LBC labelId;

	// ��������� �� ��������� ���������, ������� ����� �����
	const BodyComponent *nextComponent;

public:
	// ������ ��� ����� � �������
	LabelBodyComponent( const BodyComponent &nc, LBC lbc, const Position &ep )
		: labelId(lbc), nextComponent(&nc), BodyComponent(BCC_LABEL, ep) {
	}

	// ����������� �����
	virtual ~LabelBodyComponent() = 0;

	// ������� ��� �����
	LBC GetLabelID() const {
		return labelId;
	}

	// ������� ��������� ��������� ������� ����� �����, � ���������
	// �������������� ��������� ������ ���� �� NULL
	const BodyComponent &GetNextComponent() const {
		return *nextComponent;
	}
};


// �������������� ��������. �� �������� � ���� ��������������, �������
// �� �������� �������������
class AdditionalOperation : public BodyComponent
{
public:
	// ���� �������������� ��������
	enum AOC { AOC_RETURN, AOC_CONTINUE, AOC_BREAK, AOC_ASM, AOC_GOTO };

private:
	// ��� �������������� ��������
	AOC aoc;

public:
	// ������ ��� �������� � �������
	AdditionalOperation( AOC aoc_, const Position &ep )
		: aoc(aoc_), BodyComponent(BCC_ADDITIONAL_OPERATION, ep) {
	}

	// ����������� �����
	virtual ~AdditionalOperation() = 0;

	// ������� ��� ��������
	AOC GetAOID() const {
		return aoc;
	}
};


// ��������� �������� �����������
class ExpressionInstruction : public Instruction
{
	// �������, �������� ��� �������������� ���������, �� ����� ���� NULL
	POperand pExpr;

public:
	// ������ ������� � �������
	ExpressionInstruction( const POperand &pe,  const Position &ep ) 
		: pExpr(pe), Instruction(IC_EXPRESSION, ep) {
		INTERNAL_IF( pExpr.IsNull() );
	}

	// ������� ���������
	const POperand &GetExpression() const {
		return pExpr;
	}
};


// ������ ����������, ';'
class EmptyInstruction : public Instruction
{
public:
	// ������ �������
	EmptyInstruction( const Position &ep ) 
		: Instruction(IC_EMPTY, ep) {		
	}
};


// ���������� �������� �����������
class DeclarationInstruction : public Instruction
{
	// ������ �� ������ ��� �������, �������������� ��������
	// ����������� ������ ���� ���� �������� ���� ��������
	const TypyziedEntity &declarator;

	// �������������, ����� ���� NULL, ����� ���������������,
	// ��� ������ ���������������� �� ���������
	PObjectInitializator initializator;

public:
	// ������ ����������, ������������� � �������
	DeclarationInstruction( const TypyziedEntity &d, 
		const PObjectInitializator &iator, const Position &ep ) 
		: declarator(d), initializator(iator), Instruction(IC_DECLARATION, ep) {		

		// ���������� ������ ���� �������� ��� ��������
		INTERNAL_IF( !(d.IsObject() || d.IsFunction()) );
	}

	// ���� �� ����� �������������, ������� true
	bool IsNoInitializator() const {
		return initializator.IsNull();
	}

	// ������� ����������
	const TypyziedEntity &GetDeclarator() const {
		return declarator;
	}

	// ������� �������������
	const PObjectInitializator &GetInitializator() const {
		return initializator;
	}
};


// ���������� ������
class ClassDeclarationInstruction : public Instruction
{
	// ����������� �����. ������� ������, ��� ����� ����� ����
	// �� ��������� �����������
	const ClassType &pClass;

public:
	// ������ ����� � �������
	ClassDeclarationInstruction( const ClassType &cls, const Position &ep )
		: pClass(cls), Instruction(IC_CLASS_DECLARATION, ep) {
	}
	
	// ������� �����
	const ClassType &GetClass() const {
		return pClass;
	}
};


// ���� ����������. ����� ���� ���������� ������ � ��������� ���������� ��������.
// � ������ ���� ���������� ���� � ������, ������������ DeclarationInstruction
class DeclarationBlockInstruction : public Instruction
{
	// ������ ����������
	InstructionList declBlock;

public:
	// ������ ������ ����������
	DeclarationBlockInstruction( const InstructionList &il, const Position &ep ) 
		: declBlock(il), Instruction(IC_DECLARATION_BLOCK, ep) {
	}

	// �������� ���� ����������
	const InstructionList &GetDeclarationBlock() const {
		return declBlock;
	}
};


// ������� �����
class SimpleLabelBodyComponent : public LabelBodyComponent
{
	// ������ �� �����
	Label label;

public:
	// ������ ����� � �������
	SimpleLabelBodyComponent( const Label &lab, const BodyComponent &nc, const Position &ep )
		: label(lab), LabelBodyComponent(nc, LBC_LABEL, ep) {
	}

	// ������� �����
	const Label &GetLabel() const {
		return label;
	}
};


// case-�����
class CaseLabelBodyComponent : public LabelBodyComponent
{
	// �������� �����
	int caseValue;

public:
	// ������ �������� � �������
	CaseLabelBodyComponent( int cv, const BodyComponent &nc, const Position &ep )
		: caseValue(cv), LabelBodyComponent(nc, LBC_CASE, ep) {
	}

	// ������� �������� case-�����
	int GetCaseValue() const {
		return caseValue;
	}
};


// default-�����
class DefaultLabelBodyComponent : public LabelBodyComponent
{
public:
	// ������ �������
	DefaultLabelBodyComponent( const BodyComponent &nc, const Position &ep )
		: LabelBodyComponent(nc, LBC_DEFAULT, ep) {
	}
};


// else �����������
class ElseConstruction : public Construction
{
	// �������� ��������� ����������� else
	const BodyComponent *childElseComponent;

public:
	// ������ ���������� �������, ������������ �����������, �������
	ElseConstruction( const Construction *pc, const Position &ep )
		: childElseComponent(NULL), Construction(CC_ELSE, pc, ep) {
	}

	// ������ �������� ���������
	void AddChildComponent( const BodyComponent *childComponent ) {
		INTERNAL_IF( childElseComponent != NULL );
		childElseComponent = childComponent ;
	}

	// ������� �������� ��������� 
	const BodyComponent *GetChildComponent() const {
		return childElseComponent;
	}
};


// if �����������
class IfConstruction : public Construction
{
	// �������� ��������� ����������� if
	const BodyComponent *childIfComponent;

	// ��������� �� else �����������, ����� ���� NULL
	const ElseConstruction *elseConstruction;

	// ���������� �������, ���� ��������� ���� ����������
	PInstruction condition;

public:
	// ������ ���������� �������, ������������ �����������, �������
	IfConstruction( const PInstruction &cond, const Construction *pc, const Position &ep )
		: condition(cond), Construction(CC_IF, pc, ep), 
		childIfComponent(NULL), elseConstruction(NULL) {
	}

	// ������ �������� ���������
	void AddChildComponent( const BodyComponent *childComponent ) {
		INTERNAL_IF( childIfComponent != NULL );
		childIfComponent = childComponent ;
	}

	// ������ else-�����������
	void SetElseConstruction( const ElseConstruction *ec ) {
		INTERNAL_IF( elseConstruction != NULL );
		elseConstruction = ec;
	}

	// ������� else
	const ElseConstruction *GetElseConstruction() const {
		return elseConstruction;
	}

	// ������� �������� ��������� if
	const BodyComponent *GetChildComponent() const {
		return childIfComponent;
	}

	// ������� �������
	const PInstruction &GetCondition() const {
		return condition;
	}
};


// while �����������
class WhileConstruction : public Construction
{
	// �������� ��������� ����������� while
	const BodyComponent *childWhileComponent;

	// ���������� �������, ���� ��������� ���� ����������
	PInstruction condition;

public:
	// ������ ���������� �������, ������������ �����������, �������
	WhileConstruction( const PInstruction &cond, const Construction *pc, const Position &ep )
		: condition(cond), childWhileComponent(NULL), Construction(CC_WHILE, pc, ep) {
	}

	// ������ �������� ���������
	void AddChildComponent( const BodyComponent *childComponent ) {
		INTERNAL_IF( childWhileComponent != NULL );
		childWhileComponent = childComponent ;
	}

	// ������� �������� ��������� 
	const BodyComponent *GetChildComponent() const {
		return childWhileComponent;
	}

	// ������� �������
	const PInstruction &GetCondition() const {
		return condition;
	}
};


// ������ ����������
typedef list<PInstruction> InstructionList;


// ������ ������������� for-�����������
class ForInitSection
{
	// ����� ���� ������ ����������, ���� ���� ���������,
	// ���� ������ ������ ������
	InstructionList initList;

public:
	// �� ���������
	ForInitSection( const InstructionList &il )
		: initList(il) {
	}

	// ���� ������ ������, ������� true
	bool IsEmptySection() const {
		return initList.empty();
	}

	// ������� ������ ����������
	const InstructionList &GetInitList() const {
		return initList;
	}
};


// for �����������
class ForConstruction : public Construction
{
	// �������� ��������� ����������� for
	const BodyComponent *childForComponent;

	// ���������� �������������, ����� ���� ���������, �����
	// ���� ������ ����������, ����� ������ �������������
	ForInitSection init;

	// ���������� �������, ����� �������������  (NULL)
	PInstruction condition;

	// ��������� ��������, ����� �������������  (NULL)
	POperand iteration;

public:
	// ������ ���������� �������������, �������, ��������, ������������ �����������, �������
	ForConstruction( const ForInitSection &in, const PInstruction &cond, const POperand &iter,
		const Construction *pc, const Position &ep )
		: init(in), condition(cond), iteration(iter),
		childForComponent(NULL), Construction(CC_FOR, pc, ep) {
	}

	// ������ �������� ���������
	void AddChildComponent( const BodyComponent *childComponent ) {
		INTERNAL_IF( childForComponent != NULL );
		childForComponent = childComponent ;
	}

	// ������� �������� ��������� 
	const BodyComponent *GetChildComponent() const {
		return childForComponent;
	}

	// ������� ������ �������������
	const ForInitSection &GetInitSection() const {
		return init;
	}

	// ������� ���������� �������
	const PInstruction &GetCondition() const {
		return condition;
	}

	// ������� ��������� ��������
	const POperand &GetIteration() const {
		return iteration;
	}
};

// do-while �����������
class DoWhileConstruction : public Construction
{
	// �������� ��������� ����������� do-while
	const BodyComponent *childDoWhileComponent;

	// ���������� �������
	POperand condition;

public:
	// ������ ���������� �������, ������������ �����������, �������
	DoWhileConstruction( const Construction *pc, const Position &ep )
		: condition(NULL), childDoWhileComponent(NULL), Construction(CC_DOWHILE, pc, ep) {
	}

	// ������ �������� ���������
	void AddChildComponent( const BodyComponent *childComponent ) {
		INTERNAL_IF( childDoWhileComponent != NULL );
		childDoWhileComponent = childComponent ;
	}

	// ������ ���������� �������, ����� ���������� while
	void SetCondition( const POperand &cond ) {
		INTERNAL_IF( !condition.IsNull() );
		condition = cond;
	}


	// ������� �������� ��������� 
	const BodyComponent *GetChildComponent() const {
		return childDoWhileComponent;
	}

	// ������� �������
	const POperand &GetCondition() const {
		return condition;
	}
};


// ��� ������ �����
typedef list<const LabelBodyComponent *> LabelList;


// switch �����������
class SwitchConstruction : public Construction
{
	// �������� ��������� ����������� while
	const BodyComponent *childSwitchComponent;

	// ���������� �������, ���� ��������� ���� ����������
	PInstruction condition;	

private:
	// ������ �����, ��� �������� �� �������������. ����� �����
	// ���� ������ case ��� default
	LabelList caseLabs;
	
public:
	// ������ ���������� �������, ������������ �����������, �������
	SwitchConstruction( const PInstruction &cond, const Construction *pc, const Position &ep )
		: condition(cond), childSwitchComponent(NULL), Construction(CC_SWITCH, pc, ep) {
	}

	// ������ �������� ���������
	void AddChildComponent( const BodyComponent *childComponent ) {
		INTERNAL_IF( childSwitchComponent != NULL );
		childSwitchComponent = childComponent ;
	}

	// �������� case ��� defaul-����� � ������ �����
	void AddLabel( const LabelBodyComponent *lab ) {
		caseLabs.push_back(lab);
	}

	// ������� ������ ����� ��� ��������
	const LabelList &GetLabelList() const {
		return caseLabs;
	}

	// ������� �������� ��������� 
	const BodyComponent *GetChildComponent() const {
		return childSwitchComponent;
	}

	// ������� �������
	const PInstruction &GetCondition() const {
		return condition;
	}
};


// catch-�����������, ����� ���� ������ ������ try-catch �����������
class CatchConstruction : public Construction
{
	// ��������� �� ����������� ���, ������. ���� NULL,
	// ������ ������������ '...'
	PTypyziedEntity catchType;

	// ��������� �� �������� ��������� �����������, ������ ���� �� NULL
	const BodyComponent *childCatchConstruction;
	
	// �������
	const Position errPos;

public:
	// ������ ������, �������� ����������� � �������
	CatchConstruction( const PTypyziedEntity &ct, const Construction *pc, const Position &ep )
		: catchType(ct), Construction(CC_CATCH, pc, ep), childCatchConstruction(NULL) {
	}

	// ���� ������������� ��� ��������� "catch(...)",
	// ���������� true
	bool IsCatchAll() const {
		return catchType.IsNull();
	}

	// ������� ��������������� ���
	const PTypyziedEntity &GetCatchType() const {
		return catchType;
	}

	// ������� �������� ��������� 
	const BodyComponent *GetChildComponent() const {
		return childCatchConstruction;
	}

	// ������ �������� ���������
	void AddChildComponent( const BodyComponent *childComponent ) {
		INTERNAL_IF( childCatchConstruction != NULL );
		childCatchConstruction = (childComponent);
	}

};


// ������ catch-�����������
typedef list<const CatchConstruction *> CatchConstructionList;


// try-catch �����������
class TryCatchConstruction : public Construction
{
	// �������� ��������� ����������� try, ������ ��������� �����������
	const Construction *childTryCatchComponent;

	// ������ catch-�����������
	CatchConstructionList catchList;

public:
	// ������ ���������� �������, ������������ �����������, �������
	TryCatchConstruction( const Construction *pc, const Position &ep )
		: childTryCatchComponent(NULL), Construction(CC_TRY, pc, ep) {
	}

	// ������ �������� ���������
	void AddChildComponent( const BodyComponent *childComponent ) {
		INTERNAL_IF( childTryCatchComponent != NULL || childComponent == NULL ||
			childComponent->GetComponentID() != BCC_CONSTRUCTION );
		childTryCatchComponent = static_cast<const Construction *>(childComponent);
	}

	// ������� �������� ��������� 
	const BodyComponent *GetChildComponent() const {
		return childTryCatchComponent;
	}

	// �������� catch-�����������
	void AddCatchConstruction( const CatchConstruction &cc ) {
		catchList.push_back(&cc);
	}

	// ������� ������ catch-�����������
	const CatchConstructionList &GetCatchList() const {
		return catchList;
	}
};


// ������ ����������� �������
typedef list<PBodyComponent> BodyComponentList;


// ��������� �����������
class CompoundConstruction : public Construction
{
	// �������� ������ ����������� �������
	BodyComponentList componentList;

	// ���������� � true, ���� ��������� ����������� ����������� ������
	// ��� ���������� �����. � ������ ��������� ��� �����������
	bool mnemonic;

public:
	// ������ ������������ �����������, �������
	CompoundConstruction( const Construction *pc, const Position &ep, bool mn = false )
		: Construction(CC_COMPOUND, pc, ep), mnemonic(mn) {
	}

	// ������� true, ���� ��������� ����������� ������ � ������� ������
	bool IsMnemonic() const {
		return mnemonic;
	}

	// ����� ��������� �������� ��������� � ������ �����������
	void AddChildComponent( const BodyComponent *childComponent ) {
		componentList.push_back(const_cast<BodyComponent *>(childComponent));
	}

	// �� ����� ����������, �.�. �� �������� ��������� ����������, � �������� ������
	const BodyComponent *GetChildComponent() const {
		INTERNAL( "'CompoundConstruction::GetChildComponent' �� ����� ����������" );
		return NULL;
	}

	// ������� ������ ����������� �������
	const BodyComponentList &GetBodyComponentList() const {
		return componentList;
	}
};


// �������� return, �������� ���������
class ReturnAdditionalOperation : public AdditionalOperation
{
	// ������������ ��������, ����� ���� NULL. ���� NULL,
	// ������ return ��� ���������
	POperand pExpr;

public:
	// ������ ��������� � �������
	ReturnAdditionalOperation( const POperand &exp, const Position &ep )
		: pExpr(exp), AdditionalOperation( AOC_RETURN, ep ) {
	}

	// ���� return ��� ���������, ������� true
	bool IsNoExpression() const {
		return pExpr.IsNull();
	}

	// ������� ���������
	const POperand &GetExpression() const {
		return pExpr;
	}
};


// �������� continue
class ContinueAdditionalOperation : public AdditionalOperation
{
public:
	// ������ �������
	ContinueAdditionalOperation( const Position &ep )
		: AdditionalOperation( AOC_CONTINUE, ep ) {
	}
};


// �������� break
class BreakAdditionalOperation : public AdditionalOperation
{
public:
	// ������ �������
	BreakAdditionalOperation( const Position &ep )
		: AdditionalOperation( AOC_BREAK, ep ) {
	}
};


// �������� asm, ������ ��������� ������� � �����������
class AsmAdditionalOperation : public AdditionalOperation
{
	// ��������� ������� �������� ���-����������
	string stringLiteral;

public:
	// ������ ��������� ������� � �������
	AsmAdditionalOperation ( const string &sl, const Position &ep )
		: stringLiteral(sl), AdditionalOperation( AOC_ASM, ep ) {
	}

	// ������� ��������� �������
	const string &GetStringLiteral() const {
		return stringLiteral;
	}
};


// �������� goto, ������ ��� �����
class GotoAdditionalOperation : public AdditionalOperation
{
	// ��� �����
	string labelName;

public:
	// ������ ��� ����� � �������
	GotoAdditionalOperation( PCSTR labelName, const Position &ep )
		: labelName(labelName), AdditionalOperation( AOC_GOTO, ep ) {
	}

	// ������� ��� �����
	const string &GetLabelName() const {
		return labelName;
	}
};


// ���� �������. �������� ������ ���������� � ������ ���������������
// ����������
class FunctionBody
{
	// �������, � ������� ����������� ����
	const Function &pFunction;

	// ����������� ������������� ����, ����� ���� ���������, ����� ���� try-����
	Construction *construction;

public:
	// ������ ������������� �����
	typedef pair<string, Position> QueryLabel;
	typedef list< QueryLabel > QueryLabelList;

	// ������ �����������
	typedef list<Label> DefinedLabelList;

private:
	// ������ ������������� �����
	QueryLabelList queryLabelList;

	// ������ ����������� �����
	DefinedLabelList definedLabelList;

public:

	// ������ ������� � ������� ��������� �����������
	FunctionBody( const Function &pFn, const Position &ccPos ) 
		: pFunction(pFn), construction( NULL ) {
	}

	// ������� ��������� �����������
	~FunctionBody() {
		delete construction;
	}
		
	// ����������� �����, ������ ���������������� � ConstructorFunctionBody
	virtual bool IsConstructorBody() const {
		return false;
	}

	// �������� �������
	const Function &GetFunction() const {
		return pFunction;
	}

	// �������� ������������� �����
	void AddQueryLabel( const string &ql, const Position &pos ) {		
		queryLabelList.push_back( QueryLabel(ql, pos) );
	}

	// �������� ����������� �����, �������������� ������� ���������,
	// �� ��������� �� ��� ���
	void AddDefinedLabel( const Label &lbc ) {
		definedLabelList.push_back(lbc);
	}

	// �������� ������ ����������� �����
	const DefinedLabelList &GetDefinedLabelList() const {
		return definedLabelList;
	}

	// �������� ������ ����������� �����
	const QueryLabelList &GetQueryLabelList() const {
		return queryLabelList;
	}

	// ������ �������� ����������� ����, �������� ����� ������ ���� ���
	void SetBodyConstruction( Construction *rootConstr ) {
		INTERNAL_IF( construction != NULL );
		construction = rootConstr;
	}

	// ������� ��������� ����������� ������������� ����
	const Construction &GetBodyConstruction() const {
		return *construction;
	}

	// ������� ����������� ������������� ����, ��� ���������
	Construction &GetBodyConstruction() {
		return *construction;
	}
};


// ���� ������������, ����������� �� FunctionBody
// �����. ������ ������ ����������, �������� ����� ������ 
// �������������
class ConstructorFunctionBody : public FunctionBody
{
	// ������ ������������� ������������, ����� ���� �������� ��������
	ObjectInitElementList *oieList;

public:
	// ������ �������, ������� ������ �������������
	ConstructorFunctionBody( const Function &pFn, const Position &ccPos );
		
	// ������� ������ ������������� ������������
	~ConstructorFunctionBody();

	// ����������� �����
	bool IsConstructorBody() const {
		return true;
	}

	// ������ ������ ������������� ������������
	void SetConstructorInitList( const ObjectInitElementList &ol );

	// ������� ������ �������������
	const ObjectInitElementList &GetConstructorInitList() const {
		return *oieList;
	}
};
