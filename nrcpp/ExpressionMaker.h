// ��������� ��� ������������� ��������� - ExpressionMaker.h


// �������������� ������� ��� ������������� ���������
namespace ExpressionMakerUtils
{
	// ����� - ������������ �������� CorrectObjectInitialization,
	// ������ �������� ��������, �������� �����������, ������� ���. ���
	// �������������
	class InitAnswer {
		bool isCorrect;						// ���� ������������ �������������
		const ConstructorMethod *ictor;		// ����������� ������������ ��� �������������
	public:
		InitAnswer( bool flag ) 
			: isCorrect(flag), ictor(NULL) {
		}

		// ����� � �������������. isCorrect ������ true � ���� ������
		InitAnswer( const ConstructorMethod &ctor )
			: isCorrect(true), ictor(&ctor) {
		}

		// ������� �����������
		const ConstructorMethod *GetCtor() const {
			return ictor;
		}

		// ������� ����
		operator bool() const {
			return isCorrect;
		}
	};

	// ��������� �������� ����, ������������ � ���������� ���� 'int ()',
	// � ���������� ���� ������� �������
	POperand MakeSimpleTypeOperand( int tokcode );

	// ������� ������� ������� � ���������, ���. � ���������� ����,
	// typeid, new
	POperand MakeTypeOperand( const NodePackage &np );

	// ��������, �������� �� ��� �����
	bool IsIntegral( const TypyziedEntity &op );

	// ��������, �������� �� ��� ��������������
	bool IsArithmetic( const TypyziedEntity &op );

	// ���������, ���� ��� �������� ���������� rvalue
	bool IsRvaluePointer( const TypyziedEntity &op );

	// ��������, �������� �� ������� ����������������
	bool IsInterpretable( const POperand &op, double &rval );

	// ������ �����������, �������� �� ��� ��������� ��� ��������
	bool IsClassType( const TypyziedEntity &type ) ;

	// ���������, �������� �� �������, �������������� lvalue, 
	// ���� ��� ������� ������ � ���������� false
	bool IsModifiableLValue( const POperand &op, const Position &errPos, PCSTR opName );

	// ����� ��������� ��������� ��������������� ����� � ��������� ���������.
	// �������� ������ ���������� ����� � ����������� �����, ��� ����� ��������������
	bool CompareTypes( const TypyziedEntity &type1, const TypyziedEntity &type2 );

	// ��������, ������ �� ��������� � �� ������� � �� void
	bool IsScalarType( const TypyziedEntity &type );

	// ���� ��� �������������� ��� ���������
	inline bool IsArithmeticOrPointer( const TypyziedEntity &type ) {
		return IsArithmetic(type) || IsRvaluePointer(type);
	}

	// ������� true, ���� ��� �������� ��������, ������� �� �������, ����������
	// �� ������� ��� ���������� �� ����-�������
	bool IsFunctionType( const TypyziedEntity &type );

	// ��������, �������� �� ������� lvalue
	bool IsLValue( const POperand &op );

	// ��������, �������� �� ��� �����������. �.�. ����������
	// �� ����� ������� ������, ���� ���������, ���� ������� ���
	bool IsConstant( const TypyziedEntity &op );

	// ������� ����� �������
	POperand MakeFunctionCall( POperand &fn, PExpressionList &params );

	// ��������� ����������� ������������ �� ���������, ������������ �����������, 
	// ����������� �� ����������. ������������ ��� ��������� ��� �������������
	// �������
	bool ObjectCreationIsAccessible( const ClassType &cls, 
		const Position &errPos, bool ctor, bool copyCtor, bool dtor );

	// �������������� � rvalue, ����������, ����� ��� ��� ��������� �������� 
	// � �� void. ����� ���������, ����� ������� ��� Primary ��� Expression
	bool ToRValue( const POperand &val, const Position &errPos );

	// �������������� � rvalue ��� �����������. ����������� ������������� �����,
	// ���� ����������� ��� ������ ��� ���������
	bool ToRvalueCopy( const POperand &val, const Position &errPos );

	// �������������� ���� � ������ ��� ������������� ����. � ������
	// ���� ������������� ��������, ���������� �������, ����� ������� ������
	// � ���������� NULL. ������������ ��� �������� ���������. �����
	// ��������� ����������� �������������� �������� � rvalue
	bool ToIntegerType( POperand &op, const Position &errPos, const string &opName );

	// �������������� ���� � ��������������� ����
	bool ToArithmeticType( POperand &op, const Position &errPos,const string &opName );

	// �������������� ���� � ���� ���������
	bool ToPointerType( POperand &op, const Position &errPos, const string &opName );

	// �������������� ���� � ��������� ���� 
	bool ToScalarType( POperand &op, const Position &errPos, const string &opName );
		
	// �������������� � ��������������� ���� ��� ���������
	bool ToArithmeticOrPointerType( POperand &op, 
		const Position &errPos, const string &opName );

	// ������� ���������� ��� �� ����, ��� ������� ��� ��� �������������� �
	// ���������� ��������� � ���� ����� ��������� ��������
	TypyziedEntity *DoResultArithmeticOperand( const TypyziedEntity &op1,
		const TypyziedEntity &op2 );

	// ���������� ����� ����, ���� ��� �������� ���������, ������ ������
	TypyziedEntity *DoCopyWithIndirection( const TypyziedEntity &type );

	// ���������, ���� �������������  ���� ������, 
	// ������������ ��� this, ����� ������� ������ � ������� -1. ���� �������� this
	// �� �����, ������� 1, ����� 0.
	int CheckMemberThisVisibility( const POperand &dm, 
		const Position &errPos, bool printError = true );

	// ��������� ������������ ������������� �������. ������������ ������
	// ��� � ������ ���������������. �� ���������������� ��������, ����� ���
	// ������� �� �����������
	InitAnswer CorrectObjectInitialization( const TypyziedEntity &obj,
		const PExpressionList &initList, bool checkDtor, const Position &errPos );

	// ��������� ������������ ������� �������� �� ��������� ��� ���������
	bool CorrectDefaultArgument( const Parametr &param,
		const Operand *defArg, const Position &errPos );
}


// ��������� 'this'. ���� 'this' � ������� ������� ���������
// ����������, ������� ������ � ���������� 'errorOperand'
class ThisMaker
{
	// ������� ��� ������ ������
	const Position &errPos;

	// ������� ������ ��� 'this'
	const TypyziedEntity *MakeThis( const Method &meth ) const;
		
public:
	// ������ �������
	ThisMaker( const Position &ep ) 
		: errPos(ep) {
	}

	// �������� ��������� 'this', ���� ��������.
	POperand Make();
};


// ��������� ���������. �����, ������������, ����������, ���������,
// ������
class LiteralMaker
{
	// ������� � ���������
	const Lexem &literalLxm;

	// ��������� ���������� ������� � ����� �����
	int CharToInt( PCSTR chr, bool wide ) const;

public:
	// ������ �������
	LiteralMaker( const Lexem &lxm ) 
		: literalLxm(lxm) {
	}

	// ������� PrimaryOperand � �������������� ��������� ���������
	// �� ������ �������
	POperand Make();
};


// ��������� �������� �� ������ ������ � ���������������.
// ����������� ������ ����� ���� ���� PrimaryOperand, ����
// ������������� �������� �����������, ���� TypeOperand,
// ���� ������������� �������� �����, ���� OverloadOperand, ����
// ������������� �������� ������������� �������� � �����������
// ������������, ���� ErrorOperand, ���� ������������� ��������
// ������������� ��� �� ������ �����. ����� ����������� �����������
// ��������������.
class IdentifierOperandMaker
{
	// ����� � ���������������
	const NodePackage &idPkg;
	
	// ������� � ������� ��������� ������������� � �����
	Position errPos;

	// ��� ��������������, ��� ��� ������ � ���������
	CharString name;
	
	// ����, ������� ��������� �� ��, ��� �� ������� ��������
	// ������ � ������ ���� ������������� �� ������. ������������
	// ��� ������ ������������� ����������
	bool noErrorIfNotFound;

	// ���� ��������������� ���� ������������� �� ������
	mutable bool notFound;

	// ������� ���������, ������������ ������� ������ �������������,
	// ����� �� ����������
	mutable const SymbolTable *curST;

	// ������, � ������ ���� ��������� ���� � ����� ����� �������� '.' ��� '->'
	const TypyziedEntity *object;

	// ������� ����������, ��� �������������� ������ ������ � ����������.
	// � ������ ���� ��� ����������������� ��� �������� ����������,
	// ��� �� ���������
	void MakeTempName( const CharString &nam ) const;

	// �������� ��������� �� ������, �������� �������� �����. 
	bool ExcludeDuplicates( RoleList &idList ) const;

	// ������� ������� �� ��������� ���������������� ������
	POperand MakeOperand( const QualifiedNameManager &qnm ) const;

	// ���������, ����������� �� 'srcId' ������������� 'destId'
	// �� ������� ����� ��������. ���� destId ����������� ������
	// 'V', ������� �������� ����������� �� ��������� � curST,
	// � srcId ����������� � ������ 'B' �������� ������ 'curST',
	// � 'B' ��������� 'V', �� ������������ true
	bool HideVirtual( const Identifier *destId, const Identifier *srcId ) const;

public:

	// �������� ������ ������� ���������, ��� �������
	IdentifierOperandMaker( const NodePackage &ip, 
		const SymbolTable *cst = NULL, bool neinf = false ) ;

	// �������� ����� � ���������������. ���� ������� ��������� ������,
	// ������ ����� ������ ������������� � ���. ������������ � ���������
	// � '.'  � '->'
	IdentifierOperandMaker( const NodePackage &ip, const TypyziedEntity *obj ) ;
	
	// ������� �������. �������������� �������� �������������
	// �� �������������, ����������� � �������������.
	POperand Make();

	// ���� ������������� �� ������, ������� true
	bool IsNotFound() const {
		return notFound ;
	}
};


// ��������� ��� ����������������, �������� � ���� ����� ����������� ������,
// ������� ������������ ��� ��������������
class Caster
{
public:
	// ��������� ��������������	
	enum CC { 
		CC_EQUAL,				// ������ �����.
		CC_QUALIFIER,			// �������������� ��������������, 
		CC_INCREASE,			// ������������ ����,
		CC_STANDARD,			// ����������� ��������������, 
		CC_USERDEFINED,			// �������������� ������������ ������������� 
		CC_NOCAST				// �������������� ����������,
	} castCategory;

	// ����������� ����������, ��� ����������� ������
	virtual ~Caster()  { 
	}

	// ���������������� ��������������. ��������� ��������� �����������
	// �������������� ������ ���� � ������, � ����� ������� �����������
	// ���������� ��� ����������� ��������������
	virtual void ClassifyCast() = 0;

	// ��������� ����������� ����������� �������������� ������ ����
	// � ������
	virtual bool IsConverted() const = 0;

	// ��������� ���������� ��������������, ������� 
	// ���������� ������ ���������. ������� �������� ��� ������ ������,
	// ����� ��� ������������� ��� explicit-�����������
	virtual void DoCast( const POperand &destOp, POperand &srcOp, const Position &errPos ) = 0;

	// ������� ��������� �� ������
	virtual const CharString &GetErrorMessage( ) const = 0;

	// �������� ��������� ��������������, ���������� ��� ����������
	// ���������������
	virtual CC GetCastCategory() const = 0;
};


// ���������������� ��������� �� ���������������
typedef SmartPtr<Caster> PCaster;


// ���������� �� ��������� ���� � ��������. �������� ������ ���������
// ������ � ������ ���� ����� �++-���������� (�� ������������ � �������)
class ScalarToScalarCaster : public Caster
{	
	// ���������� ��� �� �������� ������� ������������� � ������
	// ���������
	TypyziedEntity destType, srcType;

	// �������� ���������, �� ������ �������� �� ������� ���������
	const TypyziedEntity &origDestType, &origSrcType;

	// ��������� �� ������, � ������ �� ������������� 
	// (����� �������������� ����������)
	CharString errMsg;

	// ���� ��������������� � true, ���� �������������� ��������
	bool isConverted;

	// ���� srcType �������� � destType, ������ � ������ �����������
	bool isCopy;
	
	// ���� ���� �������������� �� ������������ � �������
	bool derivedToBase;

	// �������������� ���. � ������ �����������, ������ ������. � ���������
	// ������ ����������. � ������ ���������� ����������� - NULL
	const TypyziedEntity *resultType;

	// ����������� � true, ���� ������ ������� rvalue
	bool isRefConv1, isRefConv2;	

	// ����� �������� ���������� ��� �� ���� �� �������� �����
	const TypyziedEntity *GetBiggerType() ;

	// ��������� ����������� �������������� �� ������� � ���������,
	// �� ������� � ��������� �� �������, � �� ������ � ��������� �� ���� �������,
	// ���� ���������
	bool StandardPointerConversion( TypyziedEntity &type, const TypyziedEntity &orig );

	// �������������� ������������ � ��������, ���� ��������,
	// ������������� � ������� 0. ���������� -1, ���� ��������������
	// ������ � �������, ����� > 0.
	int DerivedToBase( const TypyziedEntity &base, const TypyziedEntity &derived, bool ve ) ;

	// �������������� ������������ � ��������, ������ ������ ��� ��������
	int DerivedToBase( const ClassType &base, const ClassType &derived, bool ve );

	// ���������� ��������� �������������� ��� �������������� �����,
	// ����������� ������� ����� � ������ �����������
	void SetCastCategory( const BaseType &bt1, const BaseType &bt2 );

	// ��������� �������������� ��� ���������, ����� ��������� � ������ ���������.
	// �������������� �� ������ � ��������
	void MakeReferenceConversion( POperand &pop, int op );

	// ��������� �������������� �� ������������ ������ � �������
	// ����� ��������� � ������ ���������. �������� ������ ������� (derived),
	// � ���� ������ (base)
	void MakeDerivedToBaseConversion( const TypyziedEntity &baseClsType,
		const POperand &base, POperand &derived );

	// �������� ������������, � ������ �������� ��������������. �����
	// �������� ������ ��� �����������. ������ ������� ������ ���� �����
	// ��������������, ��� ������. � ������, ���� ������������ 'src',
	// ������ ��� 'dest' ������ false
	bool QualifiersCheck( TypyziedEntity &dest, TypyziedEntity &src );
	

public:
	// � ������������ �������� ��� ���� � ���� �����������. ���� ����
	// ����������, ������ srcType, ������� ��������� � destType, �
	// ��������� ������ ��� ���� ����� �������� � ������
	ScalarToScalarCaster( const TypyziedEntity &dest, const TypyziedEntity &src, bool ic ) 
		: destType(dest), srcType(src), origDestType(dest), origSrcType(src), isCopy(ic) {

		isConverted = false;
		resultType = NULL;		
		castCategory = CC_NOCAST;
		isRefConv1 = destType.GetDerivedTypeList().IsReference();
		isRefConv2 = srcType.GetDerivedTypeList().IsReference();
		derivedToBase = false;
	}
	
	// �������� ��� 'resultType' ���� ��������� ������, ����. ��
	// ����� ����������
	~ScalarToScalarCaster() {
		if( resultType != &destType && resultType != &srcType )
			delete resultType;
	}

	// ���������������� ��������������. ��������� ��������� �����������
	// �������������� ������ ���� � ������, � ����� ������� �����������
	// ���������� ��� ����������� ��������������
	void ClassifyCast();

	// ��������� ���������� ��������������, ������� 
	// ���������� ������ ���������. ����������� �� srcOp � destOp, ���
	// ���� ������ ���������� ������ srcOp
	void DoCast( const POperand &destOp, POperand &srcOp, const Position &errPos  ) ;

	// ���� �������������� �������� - ������� true
	bool IsConverted() const {
		return isConverted;
	}

	// ������� ��������� �� ������
	const CharString &GetErrorMessage( ) const {
		return errMsg;
	}


	// ���� ���� �������������� �� ������������ ������ � �������
	bool IsDerivedToBase() const {
		return derivedToBase;
	}

	// ������� �������������� ���
	const TypyziedEntity *GetResultType() const {
		return resultType;
	}

	// �������� ��������� ��������������, ���������� ���
	// ���������� ���������������
	CC GetCastCategory() const {
		return castCategory;
	}
};


// ���������������� ���������
typedef SmartPtr<ScalarToScalarCaster> PScalarToScalarCaster;


// �������� ���������� �� ���������� ���� � ������� ��������� �������������� ������.
// ����� �������� ��� ���������� ���, ��� � ��������� (���������, ��������������)
class OperatorCaster : public Caster
{
public:
	// ��������� ��������������
	enum ACC { ACC_NONE, ACC_TO_INTEGER, ACC_TO_ARITHMETIC, 
		ACC_TO_POINTER, ACC_TO_ARITHMETIC_OR_POINTER, ACC_TO_SCALAR, };

private:
	// ��������� ���������� �������������
	ACC category;

	// ���������� ��� �� �������� ������� ������������� � ������
	// ���������
	const TypyziedEntity &clsType, &scalarType;

	// ��������� �� ������, � ������ �� ������������� 
	// (����� �������������� ����������)
	CharString errMsg;

	// ���� ��������������� � true, ���� �������������� ��������
	bool isConverted;

	// ���� ���� ����������, ������ ���������� �����������
	bool isCopy;

	// ����������� �������� ��� �������������� �� ���������� � ��������.
	// ����� ���� NULL
	const ClassCastOverloadOperator *castOperator;

	// �������� ���������������, �� ��������������� ���� ��������� ��������������
	// � �������� ���
	PScalarToScalarCaster scalarCaster;

	// ��������� ��������� ���������� �������������� � �����������
	// ���������� 0, ���� �������� �������� ��� ��������������, ����������
	// 1 ���� �������� �� �������� � -1, ���� �������� ������������ � ������
	// ���������� (Cast Operator Strategy)

	// ��������� ������������ � ��������� � �������� ���������������� ���������,
	// �� ������������ ���������� ������������ ��������
	int CVcmp( const ClassCastOverloadOperator &cur ); 

	// ��������� �����. ���� ��������� � ���������� �����, ������� �� �����
	// ��������� ����� � ������� ScalarToScalarCaster
	int COSCast( const ClassCastOverloadOperator &ccoo );

	// ���������, �������� �� ��� ��������� ��������������
	int COSArith( const ClassCastOverloadOperator &ccoo );

	// ���������, �������� �� ��� ��������� ����������
	int COSPointer( const ClassCastOverloadOperator &ccoo );

	// ���������, �������� �� ��� ��������� �����
	int COSIntegral( const ClassCastOverloadOperator &ccoo );

	// ���������, �������� �� ������� �������� �����
	int COSScalar( const ClassCastOverloadOperator &ccoo );

	// ���������, �������� �� ������� �������������� ��� ����������
	int COSArithmeticOrPointer( const ClassCastOverloadOperator &ccoo );

public:
	// ������ ����� ����������, � ����������� ����
	OperatorCaster( const TypyziedEntity &dest, const TypyziedEntity &src, bool ic ) 
		: category(ACC_NONE), clsType(src.GetBaseType().IsClassType() ? src : dest),
		scalarType(&src == &clsType ? dest : src), isCopy(ic), 
		isConverted(false), scalarCaster(NULL), castOperator(NULL) {

		castOperator = NULL;				
	}

	// ������ ����������� ���������� ��� ���������� �������������
	OperatorCaster( ACC acc, const TypyziedEntity &ct )
		: category(acc), clsType(ct), scalarType(ct), isConverted(false),
		  scalarCaster(NULL), castOperator(NULL) {		
	}

	// ����������� ������������ ��� �������������� �� clsType � ������������ ���������,
	// scalarType ������������ ��� ���������������. ������������ ��� ��������������
	// �� ���������� � � ��������� �� �������. � st �������� ������ ����������
	OperatorCaster( ACC acc, const TypyziedEntity &ct, const TypyziedEntity &st )
		: category(acc), clsType(ct), scalarType(st), isConverted(false),
		  scalarCaster(NULL), castOperator(NULL) {		
	}

	// ������� �������� ������ ������� ���������� �������� ��� ��������������
	// � �������� ��� ��� ���������
	void ClassifyCast();

	// ��������� ���������� ��������������, ������� 
	// ���������� ������ ���������. SrcOp - ��������� ���, �������,
	// �������� �������� ����������, destOp - ��� � �������� �������� ���������.
	// � srcOp ����� ����������� � ���� destOp ���������
	void DoCast( const POperand &destOp, POperand &srcOp, const Position &errPos  ) ;

	// ���������������� ��������������
	// ���� �������������� ����������
	bool IsConverted() const {
		return isConverted;
	}

	// ������� ��������� �� ������
	const CharString &GetErrorMessage( ) const {
		return errMsg;
	}

	// ������� ��������, ����� ���� NULL
	const ClassCastOverloadOperator *GetCastOperator() const {
		return castOperator;
	}

	// �������� ��������� ��������������, ���������� ��� ����������
	// ���������������
	CC GetCastCategory() const {
		return isConverted ? CC_USERDEFINED : CC_NOCAST;
	}
};


// ���������� �� ���������� ���� � ��������. ����������� ����� ������
// ������������ ������. ������ ��������������� ����� ����������� �����������
// ��� ���������� ��������� ����
class ConstructorCaster : public Caster
{	
	// ��� ����: ���� ���������, ������ ��������
	const TypyziedEntity &lvalue, &rvalue;

	// ��������� �� ������, � ������ �� ������������� 
	// (����� �������������� ����������)
	CharString errMsg;

	// ���� ��������������� � true, ���� �������������� ��������
	bool isConverted;

	// ���� ���������� � true, ������ ������������ ����� ���������� �
	// ����� ������������ explicit-�����������
	bool explicitCast;

	// ����� ���������� false, ���� ��������� ���������� ����
	// ������ ����������� ������������. � ������ ���� �����������
	// �������� ��� ��������������, �������� �����. ���� ������
	bool AnalyzeConstructor( const ConstructorMethod &srcCtor );
	
	// ����������� ����������� ��� �������������� �� ��������� � ���������.
	// ����� ���� NULL
	const ConstructorMethod *constructor;

	// �������� ���������������, �� ��������������� ���� ��������� ��������������
	// � �������� ���
	PScalarToScalarCaster scalarCaster;

	// �������� ����������� ������������ ��� �����������
	bool CheckCtorAccess(  const POperand &destOp, const Method &meth   ) ;

public:
	// ������ ��� ����. rv ������ ����� �������� ���, � lv ���������. ������
	// ������ ������������� �������� ��� � ��������� � ������� ������������
	// ������.
	ConstructorCaster( const TypyziedEntity &lv, const TypyziedEntity &rv, bool ec ) 
		: lvalue(lv), rvalue(rv), constructor(NULL), scalarCaster(NULL), explicitCast(ec) {
		isConverted = false;
	}
		
	// ���������������� ��������������. ��������� ��������� �����������
	// �������������� ������ ���� � ������, � ����� ������� �����������
	// ���������� ��� ����������� ��������������
	void ClassifyCast();

	// ��������� ���������� ��������������, ������� 
	// ���������� ������ ���������. SrcOp - ��������� ���, �������,
	// �������� �������� ����������, destOp - ��� � �������� �������� ���������.
	// � srcOp ����� ����������� � ���� destOp ���������
	void DoCast( const POperand &destOp, POperand &srcOp, const Position &errPos  ) ;

	// ���� �������������� �������� - ������� true
	bool IsConverted() const {
		return isConverted;
	}

	// ������� ��������� �� ������
	const CharString &GetErrorMessage( ) const {
		return errMsg;
	}

	// ������� �����������, ����� ���� NULL
	const ConstructorMethod *GetConstructor() const {
		return constructor;
	}

	// �������� ��������� ��������������, ���������� ��� ����������
	// ���������������
	CC GetCastCategory() const {
		return isConverted ? CC_USERDEFINED : CC_NOCAST;
	}
};


// ���������������� ��������� �� �������������� ���������������
typedef SmartPtr<ConstructorCaster> PConstructorCaster;

// ���������������� ��������� �� ����������� ���������������
typedef SmartPtr<OperatorCaster> POperatorCaster;


// ���������� �� ���������� ���� � ���������. ��� ���������� �� ������ ����������
// � ������ ����� ������������ �����������, �������� ����������, � �����
// �������������� �� ������������ ������ � �������
class ClassToClassCaster : public Caster
{	
	// ��� ����, ��� ���������
	const TypyziedEntity &cls1, &cls2;

	// ��������� �� ������, � ������ �� ������������� 
	// (����� �������������� ����������)
	CharString errMsg;

	// ���� ��������������� � true, ���� �������������� ��������
	bool isConverted;

	// ���� ���������� � true, ������ ������������ ����� ���������� �
	// ����� ������������ explicit-�����������
	bool explicitCast;

	// ��������� ��������������, ����� ����: 
	// CC_NOCAST, CC_EQUAL, CC_STANDARD, CC_USERDEIFNED
	CC category;

	// �����, ������� ��������� �������������� - ����������� ��� �������� ��������������.
	// ����� ������������, � ������ �������������� �������� ���� � �����������
	const Method *conversionMethod;

	// ����� ���������������, ������� �������� ����� ���������������,
	// ���� cls1, ���� cls2
	const ClassType *conversionClass;
	
	// ���������������. ����� ���� ��������, � ������ �������������� ������������
	// ������ � ��������, �����������, � ������ �������������� ������� ��������
	// � ������ � ������� ��������� �������������� ������� � ��������������,
	// ���� � ������ �������� ���� ����������� ����������� ������
	PCaster caster;

public:
	// ������ ��� ����. rv ������ ����� �������� ���, � lv ���������. ������
	// ������ ������������� �������� ��� � ��������� � ������� ������������
	// ������.
	ClassToClassCaster( const TypyziedEntity &c1, const TypyziedEntity &c2, bool ec ) 
		: cls1(c1), cls2(c2), caster(NULL), explicitCast(ec) {
		isConverted = false;
		conversionMethod = NULL;
		conversionClass = NULL;
		category = CC_NOCAST;
	}
		
	// ������������� ���� ��������� ��� � �������. �������������� �������� ����� ������:
	// ������ �� ����������� ����� ������������� � �������,
	// � ������� ��������� ����������, � ������� ������������, 
	void ClassifyCast();

	// ��������� ���������� ��������������, �� srcOp � destOp,
	// ��� ���� ���������� srcOp
	void DoCast( const POperand &destOp, POperand &srcOp, const Position &errPos  ) ;

	// ���� �������������� �������� - ������� true
	bool IsConverted() const {
		return isConverted;
	}

	// ������� ��������� �� ������
	const CharString &GetErrorMessage( ) const {
		return errMsg;
	}

	// �������� �����, ������� ��������� ��������������
	const Method *GetConversionMethod() const {
		return conversionMethod;
	}

	// �������� ��������� ��������������, ���������� ��� ����������
	// ���������������
	CC GetCastCategory() const {
		return category;
	}
};


// �������� ���������� �� ������ ���� � ������. ������������ � ���������
// �����������, � ����� � ����� ������ ���������. ��� �������, ��������
// ��������� ���� ���������������, ������� �������� ��� ���� �����
class AutoCastManager
{
	// ��� ����
	const TypyziedEntity &destType, &srcType;

	// ���� �����������
	bool isCopy;

	// ���� ���������� � true, ������ ����� ����� ���������� ����
	// � ����� ������������ explicit-�����������
	bool explicitCast;

	// ���������������, ������ ��� �������� ���������� �����������,
	// ��� ��������� ��� �� ������ ���������� �����
	PCaster caster;

public:

	// ������ ��� ����, ���� �����������, ���� ������ ����������
	AutoCastManager( const TypyziedEntity &dt, const TypyziedEntity &st, 
		bool ic, bool ec = false )
		: destType(dt), srcType(st), isCopy(ic), caster(NULL), explicitCast(ec) {
	}

	// ������� ���������������
	PCaster &RevealCaster();
};


// ����������� ���������� �������. �� ������ ������ ������� � ������ ����������,
// �������� ������������ �������, ������� �������� ��������� ��� ��������� ������.
// ���� ����� ������� ���������, ������ �����. ��������� �� ������. ������
// ��������� ����� ���� ��� �� ������ �����.
class OverloadResolutor
{
	// ������� ������� ��������
	const Function *candidate;

	// ������ ���������������� ���������� ���������� � ����������
	typedef list<PCaster> CasterList;
	CasterList candidateCasterList;

	// ������ ����������������, ������� ���� �������� ��
	// ������ ViableFunction, �� ����� �������������� ��� ����������
	// �������� ����������� ������� ����� ����
	CasterList viableCasterList;

	// ��������� �������� � ������ ������������� ������
	CharString errMsg;

	// ���������� � true, ���� ������� ������������. ���� ��� ����������
	// ������� �� ���������������
	bool ambigous;

	// ���������� ������ ������������� �������
	// (overload function list)
	const OverloadFunctionList &ofl;

	// ���������� ������ ��������� ����������� ����������
	// (actual parametr list)
	const ExpressionList &apl;

	// ������, ����� ������� ���������� �����. ����� ���� NULL
	const TypyziedEntity *object;

	// ����� �������� ������������ �������, ������� �������� ��� ������ ����������
	void PermitUnambigousFunction();

	// ������� true, ���� ������� 'fn' ����� ��������� 'pcnt' ����������
	bool CompareParametrCount( const Function &fn, int pcnt );

	// ���������, �������� �� ������� ���������� (����������) ��� 
	// ������, �� ������ ���������� ������ ����������.
	bool ViableFunction( const Function &fn );

	// ���������� 'fn' � ������� ���������� � ���������, ����� 
	// �� ������� �������� �������� ��� ������ ����������, ����
	// �������� ���, �������� ��������������� � ���������� false
	bool SetBestViableFunction( const Function &fn );

public:

	// ������ ������ ������� � ������ ����������. ������
	// ������� ������ ��������� ���� �� ���� �������. ������ �������������� ����������
	// ����� ���� ��������� �� ������, ����� ������� ���������� �������
	OverloadResolutor( const OverloadFunctionList &fl, const ExpressionList &pl, 
		const TypyziedEntity *obj = NULL )
		: ofl(fl), apl(pl), candidate(NULL), object(obj), ambigous(false) {

		INTERNAL_IF( fl.empty() );
		PermitUnambigousFunction();
	}

	// ���� ������� ������������
	bool IsAmbigous() const {
		return ambigous;
	}

	// �������� ����������� �������, ������� ����� ������� � ��������
	// ������� ����������. ����� ���������� NULL, ���� ������� ������������,
	// ���� ������ ��� ���������� �������
	const Function *GetCallableFunction() const {
		return candidate;
	}

	// �������� ��������� �� ������
	const CharString &GetErrorMessage() const {
		return errMsg;
	}

	// ��������� ���������� ������� ��������� � ������ � ������������ ����
	void DoParametrListCast( const Position &errPos );
};


// ����� ������������ ��� ������������� ��������� ��� ������ 
// �������������� ��������� �� ��������� ���� ��������� � �������
// ���������. 
// ����������� ������ ������ �������� ������ ����������, ����
// ������ ������ � ������ ���� �������� �� ������.
class OverloadOperatorFounder
{
	// ���� �������� ������ ��������������� � true
	bool found;

	// ���� �������� ������������, ���������� � true
	bool ambigous;

	// ������ ���������� ������� ������� � ������,
	// ���� �� ���� �������� �� ������ - ������ ����
	OverloadFunctionList clsOperatorList;

	// ������ ���������� ������� ������� � �� ��������� ��,
	// ���� �� ���� �������� �� ������ - ������ ����
	OverloadFunctionList globalOperatorList;

	// �������� �����, ������� ���������� ������������� ���
	// ������ ����������
	void Found( int opCode, bool evrywhere, const ClassType *cls, const Position &ep  );

public:
	// � ������������ �������� ��������� ��� ������ � ����� ��
	// ���������� ������, ������� ��������� �����
	OverloadOperatorFounder( int oc, bool evrywhere, const ClassType *cls, const Position &ep  )
		: found(false), ambigous(false) {
		Found(oc, evrywhere, cls, ep);
	}

	// ���� �������� ������, ������� true
	bool IsFound() const {
		return found;
	}

	// ���� ������������
	bool IsAmbigous() const {
		return ambigous;
	}

	// ������� ������ ������������� ����������. ������ ����� ���� ������
	const OverloadFunctionList &GetClassOperatorList() const {
		return clsOperatorList;
	}

	// ������� ������ ���������� ������������� ����������
	const OverloadFunctionList &GetGlobalOperatorList() const {
		return globalOperatorList;
	}
};


// �� ������ ���� ��������, ������� ������� ������������� �������� �
// ������ �� ������ ��� �����
class UnaryOverloadOperatorCaller
{
	// �������
	const POperand &right;

	// ��� �������� ���������
	int opCode;

	// ������� ��� ������ ������
	const Position &errPos;

	// ����������� ����, ������� ������������ ����� 0
	// ������������ ��� �������� ������������ ���������
	static POperand null;

public:
	// ������ ���������� ��� ������ ���������
	UnaryOverloadOperatorCaller( const POperand &r, int op, const Position &ep )
		: right(r), opCode(op), errPos(ep) {
	}

	// ������� ����������� �����, ���� NULL, ���� ������������� �������� �������
	// ����������
	POperand Call() const;
};


// �� ������ ����� ���� ���������, �������� ����� ������������� ��������
// �� ������ ���� 'op'
class BinaryOverloadOperatorCaller
{
	// ��� �������� 
	const POperand &left, &right;

	// ��� ���������
	int opCode;

	// ������� ��� ������ ������
	const Position &errPos;

public:
	// � ������������ ������ ���������, ������� ���������� ��� ������
	// ���������
	BinaryOverloadOperatorCaller( const POperand &l, const POperand &r, int op, 
		const Position &ep ) 
		: left(l), right(r), opCode(op), errPos(ep) {
	}

	// ������� ����������� �����, ���� NULL, ���� ������������� �������� �������
	// ����������
	POperand Call() const;
};


// �������������. ��������� ������ ������� ��� ����. ��������� � ��������
// ��������� �������������� �������� � ��������� �� ������. ������ �����������
// � ������ ������������ �������� ������ ������ (����������� �������, ������� ������).
// ������ ������� ������ �� ��������� ����� 4. � ������ ���� ������� ��� �����������,
// �������� ������� ��� ������������� �����, ���������� -1, ��� ���� ����� ���������
// ����� ������
class SizeofEvaluator
{
	// ���, ������ �������� ����������� 
	const TypyziedEntity &type;

	// � ������ ������������� ������ � ���� ����� �����������
	// ������� ������ � ���� ������
	mutable PCSTR errMsg;

	// ���������� ������ �������� ����
	int GetBaseTypeSize( const BaseType &bt ) const;

	// ���������� ������ ���������, ������ ��� ������������
	int EvalClassSize( const ClassType &cls ) const;

public:
	// ������ ����������, ����������� ��� ���������� ������� ����
	SizeofEvaluator( const TypyziedEntity &t )
		: type(t), errMsg("") {
	}

	// ��������� ������ ����, ���� ��� �����������, ������� -1
	int Evaluate() const;

	// �������� ������� ������, � ������ ���� ������ �� ��������
	PCSTR GetErrorMessage() const {
		return errMsg;
	}
};


// ������������� ������� ���������
class UnaryInterpretator
{
	// ������� 
	const POperand &cnst;

	// ��������
	int op;

	// ������� ��� ������ ������
	const Position &errPos;

public:
	
	// � ������������ ������ ���������. ������������� ����� �� ����,
	// ���� ������� �� ����������� ��� �������� �� ����� ���� ���������������
	UnaryInterpretator( const POperand &c, int op_, const Position &ep )
		: cnst(c), op(op_), errPos(ep) {
	}

	// ����������������. ���� ������������� ���������� ������� NULL,
	// ����� ��������� �� ����� �������
	POperand Interpretate() const;
};


// ������������� �������� ���������
class BinaryInterpretator
{
	// �������1 � �������2
	const POperand &cnst1, &cnst2;

	// ��������
	int op;

	// ������� ��� ������ ������
	const Position &errPos;
	
public:
	
	// � ������������ ������ ���������. ������������� ����� �� ����,
	// ���� ������� �� ����������� ��� �������� �� ����� ���� ���������������
	BinaryInterpretator( const POperand &c1, const POperand &c2, int op_, const Position &ep )
		: cnst1(c1), cnst2(c2), errPos(ep), op(op_) {
	}

	// ����������������. ���� ������������� ���������� ������� NULL,
	// ����� ��������� �� ����� �������
	POperand Interpretate() const;

	// �����, ������� ��������� �� ������ ���� ���������
	static POperand MakeResult( const BaseType &bt1, const BaseType & bt2, double res );
};


// ������������� �������� ��������� '?:'
class TernaryInterpretator
{
	// �������1, �������2, �������3
	const POperand &cnst1, &cnst2, &cnst3;

public:
	
	// � ������������ ������ ���������. ������������� ����� �� ����,
	// ���� ������� �� ����������� ��� �������� �� ����� ���� ���������������
	TernaryInterpretator( const POperand &c1, const POperand &c2, const POperand &c3 )
		: cnst1(c1), cnst2(c2), cnst3(c3) {
	}

	// ����������������. ���� ������������� ���������� ������� NULL,
	// ����� ��������� �� ����� �������
	POperand Interpretate() const;
};


// ����� ������������� ������ ���������
class ExpressionPrinter
{
	// ������ � ����������
	string resultStr;

	// ����������� ���������
	string PrintExpression( const POperand &expr );

	// ����������� �������� ���������
	string PrintBinaryExpression( const BinaryExpression &expr );

public:
	// ������ �������
	ExpressionPrinter( const POperand &e ) 	{
		resultStr = PrintExpression(e);
	}

	// �������� ������ � ������������ ����������
	const string &GetExpressionString() const {
		return resultStr;
	}

	// �������� ��������� ������������� ��������� �� ����
	static PCSTR GetOperatorName( int op );
};	


// �������� ������������ ������������� ��������� ���������� �� ���������
class DefaultArgumentChecker
{
	// ����������� ��������
	const Parametr &parametr;

	// �������� �� ���������
	const POperand &defArg;

	// ������� ��� ������ ������
	const Position &errPos;

public:

	// � ������������ �������� ���������� ��� ��������
	DefaultArgumentChecker( const Parametr &prm, const POperand &da, const Position &ep ) 
		: parametr(prm), defArg(da), errPos(ep) {
	}

	// ��������, � ��������� ���� �� �������� � ������ ������, ����
	// error operand � ������ �������
	const POperand &Check() const;
};


// ��������� �� ����������
class AgregatController;
typedef SmartPtr<AgregatController> PAgregatController;


// ��������� ��� ������� ������������ ���������
class AgregatController
{
public:
	// ��� �������� ����������� ������� ����� ��-�� �� �������
	virtual ~AgregatController() {
	}

	// ����� �������� ��������������� ������� - "���������� ���������".
	// ��������� � ����� ����� ������ ������������ � ������, �����
	// ��������������� ������, ��� ���������
	struct NoElement {
		const Position errPos;
		NoElement( const Position &ep ) : errPos(ep) {}
	};

	// ����� �������� ��������������� ������� - "��� �� �������� ���������".
	struct TypeNotArgegat {
		const TypyziedEntity &type;
		TypeNotArgegat( const TypyziedEntity &t ) : type(t) {}
	};

	
	// ���������� ���������� ������ ���������� ����� �������������,
	// ���� ��� ������� �������� ���������. ���� �� �������� ������� this.
	// ��������� ���������� ���������� ������ ����� ����������� �������������
	virtual AgregatController *DoList( const ListInitComponent &ilist ) = 0;

	// ��������� ����� (���������) � ������. ���������� ���������� ����������
	// ����� ��������������, ���� ������� ���������������� ������� �������� 
	// ���������. ����, ���� � ������� ����������� ��������� �������� 
	// (������-����� � ���������, ������ � �������), ���������� ������������
	// ����������, �� ������ ���� �� ����, ����� ������. � � ��������� ������,
	// ���� ������� ���������������� ������� �� �������� ���������, �� ��
	// ����������� ������� ������� �� ������������� � ������������ NULL.
	// NULL - ������������ ������ � ������ ���� ������� ������� ����� ����������������
	virtual AgregatController *DoAtom( const AtomInitComponent &iator, bool ) = 0;

	// ��������� ����������, � ������ ���� ��� �������� ������ ��������. ����������
	// ���������� ������ ��������� ����������� ������������� �� ��������� ��� ����
	// ����� ��������� (��� ��������� ���������� �����, ��� ������� ���� ������� �������).
	virtual void EndList( const Position &errPos ) = 0;

protected:
	// ��������� ������������� �� ��������� ��� ���������� ����,
	// ���������� ����� �������� ������ �������� � ��������
	void DefaultInit( const TypyziedEntity &type, const Position &errPos );
};


// ���������� ������������� �������
class ArrayAgregatController : public AgregatController
{
	// ����������� ��� ��������, ���� ������ ���� T, �� elementType,
	// ����� ��� T
	PTypyziedEntity elementType;

	// ������ �� ������ �������, ��� ��������� �������
	Array &pArray;

	// �������������� ������ �������, ����� ���� < 1, � ������
	// ���� �� ����������
	const int arraySize;

	// ���������� ��� ������������������ ���������, ���������� ��� ������
	// ������ DoList ��� DoAtom
	int initElementCount;

	// ��������� �� ������������ ����������
	const AgregatController *parentController;

public:
	// � ������������ �������� ��� ����. ��������, � ���������
	// �� ������������ ����������, � ����� ������ �� ������ ������� 
	// ��� ��������� ��� ������� � ������ �������������
	ArrayAgregatController( const PTypyziedEntity &et, 
		const AgregatController *parent, Array &ar ) 

		: elementType(et), parentController(parent), initElementCount(0),
		  pArray(ar), arraySize(pArray.GetArraySize()) {
		
	}

	// ���������� ������
	AgregatController *DoList( const ListInitComponent &ilist );

	// ���������� ���������, ���� ������� ����������� ��� ����� ����������,
	// � ������ ���� ��������� �� �������� ��� ������� ������� �������
	AgregatController *DoAtom( const AtomInitComponent &iator, bool endList );

	// ���������� ��������� - ����� ������, ��������� ������������� �� ���������
	// ��� ������ ��������, ���� ������������ ���� ��� �������� ����������������
	// ���� ������ ������� ����������
	void EndList( const Position &errPos ) ;
};


// ���������� ������������� ���������
class StructureAgregatController : public AgregatController
{
	// ��������� �� ������������ ����������
	const AgregatController *parentController;

	// ��������� �� ��� ����� (���������)
	const ClassType &pClass;
		
	// ������� ������ ������������ �������� � ������ ������ ������,
	// ���������� -1
	int curDMindex;

	// �������� ��-�� �� ��������� ���������������� �� ����������� ������-����,
	// ���� ������ ��� � ���������, ���������� -1
	const DataMember *NextDataMember( ) ;

public:
	// � ������������ �������� ���������������� ������, � ���������
	// �� ������������ ����������, ��������� ���������� ������� ����� ��
	// �������
	StructureAgregatController( const ClassType &pc, const AgregatController *parent ) 
		: pClass(pc), parentController(parent), curDMindex(-1)  {
	}

	// ���������� ������
	AgregatController *DoList( const ListInitComponent &ilist );

	// ���������� ���������, ���� ������� ����������� ��� ����� ����������,
	// � ������ ���� ��������� �� �������� ��� ������� ������� �������
	AgregatController *DoAtom( const AtomInitComponent &iator, bool endList );

	// ���������� ��������� - ����� ������, ��������� ������������� �� ���������
	// ��� ���������� ������
	void EndList( const Position &errPos ) ;
};


// ���������� ������������� �����������
class UnionAgregatController : public AgregatController
{
	// ��������� �� ������������ ����������
	const AgregatController *parentController;

	// ��������� �� ��� ����� (���������)
	const UnionClassType &pUnion;

	// ���������� � true, ���� ������ ���� ��� ��� �������
	bool memberGot;

	// �������� ������ ������ ����, ���� �� ��� �� �������
	const DataMember *GetFirstDataMember();

public:
	// � ������������ �������� ���������������� ������, � ���������
	// �� ������������ ����������, ��������� ���������� ������� ����� ��
	// �������
	UnionAgregatController( const UnionClassType &pu, const AgregatController *parent ) 
		: pUnion(pu), parentController(parent), memberGot(false) {
	}

	// ���������� ������
	AgregatController *DoList( const ListInitComponent &ilist );

	// ���������� ���������, ���� ������� ����������� ��� ����� ����������,
	// � ������ ���� ��������� �� �������� ��� ������� ������� �������
	AgregatController *DoAtom( const AtomInitComponent &iator, bool endList );

	// ���������� ��������� - ����� ������, ��������� ������������� �� ���������
	// ��� ���������� ������
	void EndList( const Position &errPos ) ;

	// ������� � ���������� ��������
	bool IncCounter() {
		return false;
	}
};


// ������������ ��������� ����������� �� ������ ������������� �
// �������� �� �����. �����������
class ListInitializationValidator
{
	// ��������� �� ������� ����������
	AgregatController *pCurrentController;

	// ������ �������������
	const ListInitComponent &listInitComponent;

	// ������� ����� ������ ��� ������ ������
	const Position &errPos;

	// ������ �� ���������������� ������
	::Object &object;

	// ������ ��������� ��������-������������. ������������
	// ��� �������� ��������� ������������, ����������� ��� ������
	// MakeNewController
	static list<AgregatController *> allocatedControllers;

	// ������� ����� ���������� ��������,��� �������� ����. ���
	// ������ ���� ����� ���������
	static AgregatController *MakeNewController( 
		const TypyziedEntity &elemType, const AgregatController *prntCntrl );

	// ���������, �������� �� ��� ���������. ���� ��������� (�����) -
	// �� ����� ���� ���������, ���������� ���������� ���� ClassType 
	static bool IsAgregatType( const TypyziedEntity &type );

	// ����� ���������� true, ���� ��� �������� �������� ���� char ��� wchar_t,
	// � ������� �������� ��������� ���������
	static bool IsCharArrayInit( const TypyziedEntity &type, const POperand &iator );

	// ����������� �����, �������� �� ������ �������������, � ������
	// ��������� ��������� �������� �������� ��� ���������
	void InspectInitList( const ListInitComponent &ilist );

public:
	// ������ ������ �� ������ � ������� ����� ������
	ListInitializationValidator( const ListInitComponent &lic, 
		const Position &ep, ::Object &ob ) 
		: listInitComponent(lic), errPos(ep), pCurrentController(NULL), object(ob) {
		INTERNAL_IF( !allocatedControllers.empty() ); 
	}

	// ����������� ������ ������� �������������
	~ListInitializationValidator() {
		for( list<AgregatController *>::iterator p = allocatedControllers.begin();
			 p != allocatedControllers.end(); p++ )
			delete *p;
		allocatedControllers.clear();
	}

	// ��������� ���������� ������������� �������
	void Validate();

	// ��������� ������ ��� ������� � ����������� ������� ��������� ���������
	friend class ArrayAgregatController;
	friend class StructureAgregatController;
	friend class UnionAgregatController;
};


// �������� ������������� ���������� ��� ��������� ����������. ��������
// ��� � ����� ���������������, ��� � � �����������
class InitializationValidator
{
	// ������ ���������� ������������ �������
	const PExpressionList &expList;	

	// ������ �� ���������������� ������. ������ �� �����������,
	// �.�. ��� ���������� ����� ����������, ����� ��� ����������������
	// �������� ��� ������ �������
	::Object &object;

	// ������� ��� ������ ������
	const Position &errPos;

	// � ������, ���� ������������� ���� �������������, ��������� ���.
	// ���������� ���������� ��� ���������
	const ConstructorMethod *ictor;

	// ��������� ������������� ������������� ��� ����� ���������,
	// ��� ��� ��������
	void ValidateCtorInit();

public:
	// ������ ������� ����������
	InitializationValidator( const PExpressionList &el, ::Object &obj, const Position &ep )
		: expList(el), object(obj), ictor(NULL), errPos(ep) {
	}

	// ��������� �������������
	void Validate() {
		ValidateCtorInit();
	}

	// �������� �����������, ����� ���������� NULL, ���� �������������
	// ��������� ��� ������������, ���� ���� �����������
	const ConstructorMethod *GetConstructor() const {
		return ictor;
	}
};


// ��������� �������� ������� ������������� ������� ���
// ������ ������ ������������� ������������
class ObjectInitElement
{
public:
	// ��� �������������� ��������������
	enum IV { IV_DATAMEMBER, IV_VIRTUALBC, IV_DIRECTBC };

private:
	// ���������������� ������-���� ��� �����
	const Identifier &id;

	// ��������� �������������, ����� ���� NULL
	PExpressionList expList;
	
	// ������������� ��������������: ������-����, ����������� �������
	// �����, ������ ������� �����
	IV iv;

	// ���������� ����� ������������� ��������
	unsigned int orderNum;

public:
	
	// ������ � ������������ �����. ����������.
	ObjectInitElement( const Identifier &id_, const PExpressionList &el, IV iv_, unsigned on )
		: id(id_), expList(el), iv(iv_), orderNum(on) {
	}

	// �������� ������������
	const Identifier &GetIdentifier() const {
		return id;
	}

	// �������� ������ ���������
	const PExpressionList &GetExpressionList() const {
		return expList;
	}

	// �������� ����� �������������
	unsigned GetOrderNum() const {
		return orderNum;
	}

	// ������� true, ���� ������������� ������������� ������ ����
	bool IsDataMember() const {
		return iv == IV_DATAMEMBER;
	}

	// ������� true, ���� ������������� - ����������� ������� �����
	bool IsVirtualBaseClass() const {
		return iv == IV_VIRTUALBC;
	}

	// ������� true, ���� ������������� - ������ ������� �����
	bool IsDirectBaseClass() const {
		return iv == IV_DIRECTBC;
	}

	// ������� true, ���� ������� ������������������
	bool IsInitialized() const {
		return !expList.IsNull();
	}

	// ������ ������ ���������, ��������� ������ � ������ ����
	// ���� ������ �� �����
	void SetExpressionList( const PExpressionList &el ) {
		INTERNAL_IF( !expList.IsNull() );
		expList = el;
	}

	// ������ ���������� ����� �������������, ���������� � ������
	// ���� ������������ ������ ������������� ����
	void SetOrderNum( unsigned on ) {
		orderNum = on;
	}

	// �������� ��������� ��� ������ ��������������
	bool operator==( const ObjectInitElement &oie ) const {
		return &id == &oie.GetIdentifier();
	}

	// �������� ��������� ��� ������ ��������������
	bool operator!=( const ObjectInitElement &oie ) const {
		return &id != &oie.GetIdentifier();
	}

	void operator=(const ObjectInitElement &oie ) const {
		return *this = oie;
	}
};


// ��� - ������ ��������� ������������� �������
typedef list<ObjectInitElement> ObjectInitElementList;


// ��������� ������������ ������������� ������� �������
// ������������� ������������
class CtorInitListValidator
{
	// ������ �� �����������
	const ConstructorMethod &pCtor;

	// ������ �� ���������������� �����
	const ClassType &pClass;

	// ������� ��� ������ ������
	Position errPos;

	// ������ ��������� �������������. ������� �����������
	// ���� ����������, ������� ������ ���� �������������������,
	// ����� ���� ��������� � ��� ��������������, ������� ������ ����
	ObjectInitElementList oieList;

	// ������� ���������� ���� ������������������ ���������, ���
	// ���� ����� ����� ����� ������������� ��������� �������� �
	// ���������� �������
	unsigned explicitInitCounter;
	
	// ������ ����� �� �������� �������, ����� ����������� ������
	void SelectVirtualBaseClasses( const BaseClassList &bcl, unsigned &orderNum );

	// ��������� ������ ���������� ���������� �������������,
	// ������������ �������� ��������, ������� �������� ��������, 
	// �������������� �������-�������
	void FillOIEList( );

public:
	// ������ ����������� ��� �������� �������������
	CtorInitListValidator( const ConstructorMethod &pc, const Position &ep )
		: pCtor(pc), pClass( static_cast<const ClassType &>(pc.GetSymbolTableEntry()) ),
		errPos(ep), explicitInitCounter(0) {

		FillOIEList( );		// �������� ������ ���������������� ���������
	}

	// �������� ���� ���������������� �������, ���� �� �� ������������ � ������,
	// ������� ������. ����� ������� ������ ���� ������������ ��� ����� ��������
	// � ������, ��� �������� ���������������. 
	void AddInitElement( const POperand &id, 
		const PExpressionList &expList, const Position &errPos, unsigned &orderNum ) ;

	// ��������� �������������� ��������, ����� ���� ��� ���� ������ ������.
	// ��������� �������������������� ��������. ������ ���������� �������
	// �������������
	void Validate();

	// ���������� ������ ������������� ��� ����������
	const ObjectInitElementList &GetInitElementList() const {
		return oieList;
	}
};


// ����� ���� ������ ��������� �������, �������� � ��������� ���������.
// ������ ����� ����� ����������� ��� ��������� ��������� � ���������


// ����� ���������� ���� - ( type )
class TypeCastBinaryMaker
{
	// ��� � �������� ���������� ���������
	const POperand &left;

	// ���� ���������
	const POperand &right;

	// ��� ������ ������
	const Position &errPos;

	// ��������� ���������� ������������ � ��������� � ���������� ����� ���
	PTypyziedEntity UnsetQualification();

	// ��������� ���������� � ������� static_cast. ������� 0 � ������ ������,
	// ������� 1 � ������ �������, -1 � ������ ���� ���������� ����� ����������
	int CheckStaticCastConversion( const TypyziedEntity &expType );

	// ��������� ���������� � ������� reinterpret_cast. ������� true � ������ ������	
	bool CheckReinterpretCastConversion( const TypyziedEntity &expType );

public:

	// ������ ��� �������� � ��������
	TypeCastBinaryMaker( const POperand &tp, const POperand &exp, int op, const Position &ep  )
		: left(tp), right(exp), errPos(ep) {

		INTERNAL_IF( op != OC_CAST );
		INTERNAL_IF( !tp->IsTypeOperand()  || 
			(!exp->IsExpressionOperand() && !exp->IsPrimaryOperand()) );
	}

	// ������� ���������. ����� ������� ErrorOperand, ���� ���������� ����������
	POperand Make();
};


// ����� �������  - ( )
class FunctionCallBinaryMaker
{
	// �������, ���� ������ �������, ���� ���, ���� ������� � ����� �������
	const POperand &fn;

	// ������ ����������
	const PExpressionList &parametrList;

	// ��� ������ ������
	const Position &errPos;

	// ���� ����� �������, ���������� � true
	bool implicit;

	// ����, ��������������� � true, ���� ����������, ��� ��� �������
	// ������� �� ��������� ��� ������. ��� ����� ���� ������������ �����.
	// ���� ������� ������������, ���� �� ���������������
	mutable bool noFunction;

	// �����, �������� ��������� ������� �� ������ � ��������� �� �����������.
	// ���� ������� ������������ ��� �� �������, ������� NULL
	const Function *CheckBestViableFunction( const OverloadFunctionList &ofl,
		const ExpressionList &pl, const TypyziedEntity *obj = NULL ) const;

	// ���� 'fn' - ��� ������, ������� ��� ���� ������������� ��������� '()',
	// ���� �������� ���������� � ���� ��������� �� �������. ���� ����������
	// �������, ������ ����� �������, ����� NULL
	POperand FoundFunctor( const ClassType &cls ) const;
	
	// ��������� ����������� ����������� � ����������� ������� ���������.
	// ��������� ����� ����������� ����������� � ���������� ���� ��������
	static void CheckParametrInitialization( const PExpressionList &parametrList, 
		const FunctionParametrList &formalList, const Position &errPos ) ;

	// �������� ����������� ����� ��� ��������
	void CheckParametrInitialization( const FunctionParametrList &formalList ) const {
		CheckParametrInitialization(parametrList, formalList, errPos);
	}

	// ��������� ������������� �������, ����� ��� ����� ������ �
	// ������������ ������ �������� ����������
	friend 	ExpressionMakerUtils::InitAnswer ExpressionMakerUtils::CorrectObjectInitialization( 
		const TypyziedEntity &obj,const PExpressionList &initList, bool checkDtor, 
		const Position &errPos );

	// ��������� �������� �� ������� ������ �������� ������, ���� ���
	// ���������� �������� ��� this, � ��������� �� �� ������������ � ��������
	void CheckMemberCall( const Method &m ) const;

public:

	// ������ ��� �������� � ��������. ��������� �������� ��������� �� �������
	// ����� ���� true. ���� ����� �������, ��������� ���������� NULL, � ������
	// ���������� �����. ������� � �� ������� ������. ������������ ��� �������
	// ������ ������������� ����������
	FunctionCallBinaryMaker( const POperand &f, const PExpressionList &pl, 
		int op, const Position &ep, bool imp = false )

		: fn(f), parametrList(pl), errPos(ep), implicit(imp), noFunction(false) {

		INTERNAL_IF( op != OC_FUNCTION );
	}

	// ������� ���������. ����� ������� ErrorOperand, ���� ���������� ����������
	POperand Make();

};


// ��������� �������� ������� - [ ]
class ArrayBinaryMaker
{
	// ������� �����
	const POperand &left;

	// ������� ������
	const POperand &right;

	// ��� ������ ������
	const Position &errPos;

public:

	// ������ ��� �������� � ��������
	ArrayBinaryMaker( const POperand &l, const POperand &r, int op, const Position &ep )
		: left(l), right(r), errPos(ep) {

		INTERNAL_IF( op != OC_ARRAY );
	}

	// ������� ���������. ����� ������� ErrorOperand, ���� ���������� ����������
	POperand Make();
};


// �������� ����� ������ - .  ->
class SelectorBinaryMaker
{
	// ������� �����
	const POperand &left;

	// ������� ������ (��� � ���� ������)
	const NodePackage &idPkg;

	// ��������
	int op;

	// ��� ������ ������
	const Position &errPos;

	// ��������, ����� ���� ����������� ������� ������, ��������
	// �������������� ������ ��� ��������� � ������������������ �����
	bool CheckMemberVisibility( const Identifier &id, const ClassType &objCls );

public:

	// ������ ��� �������� � ��������
	SelectorBinaryMaker( const POperand &l, const NodePackage &id, int op_, const Position &ep )
		: left(l), idPkg(id), op(op_), errPos(ep) {

		INTERNAL_IF( op != ARROW && op != '.' );
	}

	// ������� ���������. ����� ������� ErrorOperand, ���� ���������� ����������
	POperand Make();
};


// ���������� �� - !
class LogicalUnaryMaker
{
	// ������� ������ �� ���������
	const POperand &right;

	// ��� ������ ������
	const Position &errPos;

public:

	// ������ ��� �������� � ��������
	LogicalUnaryMaker( const POperand &r, int op, const Position &ep )
		: right(r), errPos(ep) {

		INTERNAL_IF( op != '!' );
	}

	// ������� ���������. ����� ������� ErrorOperand, ���� ���������� ����������
	POperand Make();
};


// ��������� ��� - ~
class BitReverseUnaryMaker
{
	// ������� ������ �� ���������
	const POperand &right;

	// ��� ������ ������
	const Position &errPos;

public:

	// ������ ������� � ��������
	BitReverseUnaryMaker( const POperand &r, int op, const Position &ep )
		: right(r), errPos(ep) {

		INTERNAL_IF( op != '~' );
	}

	// ������� ���������. ����� ������� ErrorOperand, ���� ���������� ����������
	POperand Make();
};


// ��������, �������� ���������� ��� ������ � ������ - % << >> ^ | &
class IntegralBinaryMaker
{
	// ������� �����
	const POperand &left;

	// ������� ������
	const POperand &right;

	// ��������
	int op;

	// ��� ������ ������
	const Position &errPos;

public:

	// ������ ��� �������� � ��������
	IntegralBinaryMaker( const POperand &l, const POperand &r, int op_, const Position &ep )
		: left(l), right(r), op(op_), errPos(ep) {

		INTERNAL_IF( op != '%' && op != '^' && op != '|' && op != '&' && 
			op != LEFT_SHIFT && op != RIGHT_SHIFT );
	}

	// ������� ���������. ����� ������� ErrorOperand, ���� ���������� ����������
	POperand Make();
};


//  �������� ��������� � ������� - *  /
class MulDivBinaryMaker
{
	// ������� �����
	const POperand &left;

	// ������� ������
	const POperand &right;

	// ��������
	int op;

	// ��� ������ ������
	const Position &errPos;

public:

	// ������ ��� �������� � ��������
	MulDivBinaryMaker( const POperand &l, const POperand &r, int op_, const Position &ep )
		: left(l), right(r), op(op_), errPos(ep) {

		INTERNAL_IF( op != '*' && op != '/' );
	}

	// ������� ���������. ����� ������� ErrorOperand, ���� ���������� ����������
	POperand Make();
};


// ������� ���� � ����� -   +  - 
class ArithmeticUnaryMaker
{
	// ������� ������
	const POperand &right;

	// ��������
	int op;

	// ��� ������ ������
	const Position &errPos;

public:

	// ������ �������� � ��������
	ArithmeticUnaryMaker( const POperand &r, int op_, const Position &ep )
		: right(r), op(op_), errPos(ep) {

		INTERNAL_IF( op != '+' && op != '-' );
	}

	// ������� ���������. ����� ������� ErrorOperand, ���� ���������� ����������
	POperand Make();
};


// ����������� �������������� �������� - throw
class ThrowUnaryMaker
{
	// ������� ������
	const POperand &right;

	// ��� ������ ������
	const Position &errPos;

public:

	// ������ �������� � ��������
	ThrowUnaryMaker( const POperand &r, int op, const Position &ep )
		: right(r), errPos(ep) {

		INTERNAL_IF( op != KWTHROW );
	}

	// ������� ���������. ����� ������� ErrorOperand, ���� ���������� ����������
	POperand Make();
};


// ������������� - *			
class IndirectionUnaryMaker	
{
	// ������� ������
	const POperand &right;

	// ��� ������ ������
	const Position &errPos;

public:

	// ������ �������� � ��������
	IndirectionUnaryMaker( const POperand &r, int op, const Position &ep )
		: right(r), errPos(ep) {

		INTERNAL_IF( op != '*' );
	}

	// ������� ���������. ����� ������� ErrorOperand, ���� ���������� ����������
	POperand Make();
};


// ����������� � ���������� ��������� � ��������� -  ++ --
class IncDecUnaryMaker
{
	// ������� ������ ��� �����, � ����������� �� ���� ����� �������,
	// ����������� ��� ����������
	const POperand &right;

	// ��� ��������� ����� ���� INCREMENT, DECREMENT � �������������
	// �������, � ������ � ��������������, ��������, ��� ��������� ����������,
	// �.�. ����� ���������
	int op;

	// ��� ������ ������
	const Position &errPos;

public:

	// ������ �������� � ��������
	IncDecUnaryMaker( const POperand &r, int op_, const Position &ep )
		: right(r), op(op_), errPos(ep) {

		INTERNAL_IF( abs(op) != INCREMENT && abs(op) != DECREMENT );
	}

	// ������� ���������. ����� ������� ErrorOperand, ���� ���������� ����������
	POperand Make();
};


// ������ ������ -  &
class AddressUnaryMaker
{
	// ������� ������
	const POperand &right;

	// ��� ������ ������
	const Position &errPos;

public:

	// ������ �������
	AddressUnaryMaker( const POperand &r, int op, const Position &ep )
		: right(r), errPos(ep) {

		INTERNAL_IF( op != '&' );
	}

	// ������� ���������. ����� ������� ErrorOperand, ���� ���������� ����������
	POperand Make();
};


// �������� �������� -  +
class PlusBinaryMaker
{
	// ������� �����
	const POperand &left;

	// ������� ������
	const POperand &right;

	// ��� ������ ������
	const Position &errPos;

public:

	// ������ ��� �������� � ��������
	PlusBinaryMaker( const POperand &l, const POperand &r, int op, const Position &ep )
		: left(l), right(r), errPos(ep) {

		INTERNAL_IF( op != '+' );
	}

	// ������� ���������. ����� ������� ErrorOperand, ���� �������� ����������
	POperand Make();
};


// �������� ��������� -  -
class MinusBinaryMaker
{
	// ������� �����
	const POperand &left;

	// ������� ������
	const POperand &right;

	// ��� ������ ������
	const Position &errPos;

public:

	// ������ ��� �������� � ��������
	MinusBinaryMaker( const POperand &l, const POperand &r, int op, const Position &ep )
		: left(l), right(r), errPos(ep) {

		INTERNAL_IF( op != '-' );
	}

	// ������� ���������. ����� ������� ErrorOperand, ���� �������� ����������
	POperand Make();
};


// �������� ��������� -  <   <=   >=  >  ==  !=
class ConditionBinaryMaker
{
	// ������� �����
	const POperand &left;

	// ������� ������
	const POperand &right;

	// ��������
	int op;

	// ��� ������ ������
	const Position &errPos;

public:

	// ������ ��� �������� � ��������
	ConditionBinaryMaker( const POperand &l, const POperand &r, int op_, const Position &ep )
		: left(l), right(r), op(op_), errPos(ep) {

		INTERNAL_IF( op != '<' && op != '>' && op != EQUAL && op != NOT_EQUAL &&
			op != GREATER_EQU && op != LESS_EQU );
	}

	// ������� ���������. ����� ������� ErrorOperand, ���� �������� ����������
	POperand Make();
};


// 	���������� �������� - &&   ||
class LogicalBinaryMaker
{
	// ������� �����
	const POperand &left;

	// ������� ������
	const POperand &right;

	// ��������
	int op;

	// ��� ������ ������
	const Position &errPos;

public:

	// ������ ��� �������� � ��������
	LogicalBinaryMaker( const POperand &l, const POperand &r, int op_, const Position &ep )
		: left(l), right(r), op(op_), errPos(ep) {

		INTERNAL_IF( op != LOGIC_AND && op != LOGIC_OR );
	}

	// ������� ���������. ����� ������� ErrorOperand, ���� ���������� ����������
	POperand Make();
};


// �������� �������� -  ?:
class IfTernaryMaker
{
	// ������� 
	const POperand &cond;

	// ������� ����� �� ':'
	const POperand &left;

	// ������� ������ �� ':'
	const POperand &right;

	// ��� ������ ������
	const Position &errPos;

public:

	// ������ ��� �������� � ��������
	IfTernaryMaker( const POperand &c, const POperand &l, const POperand &r, 
		int op, const Position &ep )

		: cond(c), left(l), right(r), errPos(ep)  {

		INTERNAL_IF( op != '?' );
	}

	// ������� ���������. ����� ������� ErrorOperand, ���� ���������� ����������
	POperand Make();
};


// ���������� -  =   +=   -=   *=   /=   %=   >>=   <<=  |=  &=  ^=
class AssignBinaryMaker
{
	// ������� �����
	const POperand &left;

	// ������� ������
	const POperand &right;

	// ��������
	int op;

	// ��� ������ ������
	const Position &errPos;

	// �����, ��������� ��������� ���� �������� ���������;
	// -1 - ������, 0 - ��������� ��������, 1 - ������� ��������� � �����
	int CheckOperation( const string &opName );

public:

	// ������ ��� �������� � ��������
	AssignBinaryMaker( const POperand &l, const POperand &r, int op_, const Position &ep )
		: left(l), right(r), op(op_), errPos(ep) {

		INTERNAL_IF( op != '=' && op != MUL_ASSIGN && op != DIV_ASSIGN && op != PERCENT_ASSIGN &&
			op != PLUS_ASSIGN && op != MINUS_ASSIGN &&  op != LEFT_SHIFT_ASSIGN && 
			op != RIGHT_SHIFT_ASSIGN && op != AND_ASSIGN &&  op != XOR_ASSIGN &&
			op != OR_ASSIGN );
	}

	// ������� ���������. ����� ������� ErrorOperand, ���� ���������� ����������
	POperand Make();
};


// ������� -  ,
class ComaBinaryMaker
{
	// ������� �����
	const POperand &left;

	// ������� ������
	const POperand &right;

	// ��� ������ ������
	const Position &errPos;

public:

	// ������ ��� �������� � ��������
	ComaBinaryMaker( const POperand &l, const POperand &r, int op, const Position &ep )
		: left(l), right(r), errPos(ep) {

		INTERNAL_IF( op != ',' );
	}

	// ������� ���������. ����� ������� ErrorOperand, ���� ���������� ����������
	POperand Make();
};


// ��������� ������ -  new  new[]
class NewTernaryMaker
{
	// sizeof ��������� ������������ ������ ���������� ������
	POperand allocSize;

	// ��� ���������� ������
	POperand allocType;

	// ������ ��������� ����������. ����� ���� NULL
	const PExpressionList &placementList;

	// ������ ���������������. ����� ���� NULL
	const PExpressionList &initializatorList;

	// ���� �������� ����������, ���������� � true. new ��� new[],
	// ������� � ����������� �� �������� ����
	bool globalOp;

	// ��� ������ ������
	const Position &errPos;

	// ����� ��� ��� ������ ������������� ���������, ������� �������������
	// ����� � ����� � ��������� sizeof, ������� �������� �� ��������� ������
	void MakeSizeofExpression( const NodePackage &typePkg );

	// ����� ���������� ����� ��������� new � ������ �����
	POperand MakeNewCall( bool array, bool clsType );

public:
	// ������ ���������� ��������� � ��������
	NewTernaryMaker( const NodePackage &typePkg, const PExpressionList &pl, 
		const PExpressionList &il, bool glob, const Position &ep ) 
		
		: allocType(ExpressionMakerUtils::MakeTypeOperand(typePkg)), placementList(pl), 
		initializatorList(il), allocSize(NULL), globalOp(glob), errPos(ep) {

		// ��� �������� ������ ���� �� �������. ������ ���������� �
		// ��������������� ����� ���� ������
		INTERNAL_IF( placementList.IsNull() || initializatorList.IsNull() );
		
		// ������ allocSize, �� ������ ������. ����� ���� ErrorOperand
		if( allocType->IsTypeOperand() )
			MakeSizeofExpression( typePkg );
	}

	// ������� ���������. ����� ������� ErrorOperand, ���� ���������� ����������
	POperand Make();
};


// ������������ ������ -  delete  delete[]	
class DeleteUnaryMaker
{
	// ��������� ������
	const POperand &right;

	// ��� ��������� delete ��� delete[]
	int op;

	// ��� ������ ������
	const Position &errPos;

	// ����� ������ ������ ��������� delete
	POperand MakeDeleteCall(  bool clsType  );

public:

	// ������ ������� � ��� ���������
	DeleteUnaryMaker( const POperand &r, int op_, const Position &ep )
		: right(r), op(op_), errPos(ep) {

		INTERNAL_IF( abs(op) != KWDELETE && abs(op) != OC_DELETE_ARRAY );
	}

	// ������� ���������. ����� ������� ErrorOperand, ���� ���������� ����������
	POperand Make();
};


// ������ � ��������� �� ���� -  .*   ->*
class PointerToMemberBinaryMaker
{
	// ������� �����
	const POperand &left;

	// ������� ������
	const POperand &right;

	// ��� ���������
	int op;

	// ��� ������ ������
	const Position &errPos;

public:

	// ������ ��� �������� � ��������
	PointerToMemberBinaryMaker( const POperand &l, const POperand &r, 
		int op_, const Position &ep )

		: left(l), right(r), op(op_), errPos(ep) {

		INTERNAL_IF( op != DOT_POINT && op != ARROW_POINT );
	}

	// ������� ���������. ����� ������� ErrorOperand, ���� ���������� ����������
	POperand Make();
};


// ������������� ���� - typeid		
class TypeidUnaryMaker
{
	// ��������� ������
	const POperand &right;

	// ��� ������ ������
	const Position &errPos;

public:

	// ������ ������� � ��� ���������
	TypeidUnaryMaker( const POperand &r, int op, const Position &ep )
		: right(r), errPos(ep) {

		INTERNAL_IF( op != KWTYPEID );
	}

	// ������� ���������. ����� ������� ErrorOperand, ���� ���������� ����������
	POperand Make();
};


// ������������ ���������� ���� - dynamic_cast
class DynamicCastBinaryMaker
{
	// ��� � �������� ���������� ��������� (������ �������)
	const POperand &left;

	// ������� ������
	const POperand &right;

	// ��� ������ ������
	const Position &errPos;

public:

	// ������ ��� �������� � ��������
	DynamicCastBinaryMaker( const POperand &l, const POperand &r, int op, const Position &ep )
		: left(l), right(r), errPos(ep) {

		INTERNAL_IF( op != KWDYNAMIC_CAST );
	}


	// ������� ���������. ����� ������� ErrorOperand, ���� ���������� ����������
	POperand Make();
};


// ����������� ���������� ���� - static_cast
class StaticCastBinaryMaker	
{
	// ��� � �������� ���������� ��������� (������ �������)
	const POperand &left;

	// ������� ������
	const POperand &right;

	// ��� ������ ������
	const Position &errPos;

	// ��������� ��������������. ������� -1, ���� �������������� ����������,
	// ������� 1, ���� �������������� �������� � ������� ������� right, �.�.
	// ��������� ��� �� ���������, ������� 0, � ������ ���� ������� ���������
	// ���������
	int CheckConversion( );

	// ���������� true, ���� �������� �������������� �� lvalue B � cv D&,
	// ���� �� B * � D *
	bool BaseToDerivedExist( const TypyziedEntity &toCls, const TypyziedEntity &fromCls );

public:

	// ������ ��� �������� � ��������
	StaticCastBinaryMaker( const POperand &l, const POperand &r, int op, const Position &ep )
		: left(l), right(r), errPos(ep) {

		INTERNAL_IF( op != KWSTATIC_CAST );
	}

	// ������� ���������. ����� ������� ErrorOperand, ���� ���������� ����������
	POperand Make();
};


// ���������� ������ ����� - reinterpret_cast
class ReinterpretCastBinaryMaker
{
	// ��� � �������� ���������� ��������� (������ �������)
	const POperand &left;

	// ������� ������
	const POperand &right;

	// ��� ������ ������
	const Position &errPos;

public:

	// ������ ��� �������� � ��������
	ReinterpretCastBinaryMaker( const POperand &l, const POperand &r, 
		int op, const Position &ep )

		: left(l), right(r), errPos(ep) {

		INTERNAL_IF( op != KWREINTERPRET_CAST );
	}

	// ������� ���������. ����� ������� ErrorOperand, ���� ���������� ����������
	POperand Make();
};


// ����� ������������� � ���� - const_cast
class ConstCastBinaryMaker
{
	// ��� � �������� ���������� ��������� (������ �������)
	const POperand &left;

	// ������� ������
	const POperand &right;

	// ��� ������ ������
	const Position &errPos;

public:

	// ������ ��� �������� � ��������
	ConstCastBinaryMaker( const POperand &l, const POperand &r, int op, const Position &ep )
		: left(l), right(r), errPos(ep) {

		INTERNAL_IF( op != KWCONST_CAST );
		INTERNAL_IF( !(left->IsTypeOperand() && 
			(right->IsExpressionOperand() || right->IsPrimaryOperand()) ) );
	}

	// ������� ���������. ����� ������� ErrorOperand, ���� ���������� ����������
	POperand Make();
};
