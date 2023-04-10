// ��������� ��� ���������� ������ ������ - MemberMaker.h
// ����������: ���������� ����������� ������������� ����� Maker.h ����� MemberMaker.h


// �������� � Class.h
class ClassType;

//  �������� � Maker.h
struct TempObjectContainer;

// �������� � Body.h
class Operand ;


// ��� ������������� �����
enum MemberInitializationType 
{
	MIT_NONE, MIT_PURE_VIRTUAL, MIT_DATA_MEMBER, MIT_BITFIELD
};


// ��������� ��� ���� ������� ���������� ������
class MemberDeclarationMaker : public DeclarationMaker
{
protected:
	// ����� ����������� ��� ���� ������� ���������� "������-������":
	// ������ �� �����, ���� �������� ��������
	ClassType &clsType;

	// ������� ������������ ������� � ���� ������
	ClassMember::AS curAccessSpec;

	// ��������� ���������, ������� ������������ ��� ���������� �������
	PTempObjectContainer toc;	

public:
	// ����������� ��� ������� ������, �������� ������������� �������
	// � ��������� ���������
	MemberDeclarationMaker( ClassType &ct, ClassMember::AS cas,
		const PTempObjectContainer &t ) 
		: clsType(ct), curAccessSpec(cas), toc(t) {
	}

	// ����������� ����� ��� ������������� �����
	virtual void Initialize( MemberInitializationType mit, const Operand &exp ) = 0;

private:
	// ��������� ������ � ������������ ������ ��� ������������� ����� ��
	// ������� �����
	virtual void Initialize( const ExpressionList & ) {
		INTERNAL( "'MemberDeclarationMaker::Initialize'"
			" - ����� �� ����� ���������� �� ������� ������");
	}
};


// ��������� �������-�����
class DataMemberMaker : public MemberDeclarationMaker
{
	// ��������� ������ ���������
	DataMember *targetDM;

	// ����� ��������� ������������ �������� �������� ����
	void CheckBitField( const Operand &exp );
		
	// ����� ��������� ������������ ������������� �������-����� ���������
	void CheckDataInit( const Operand &exp );

public:

	// ����������� ��� ������� ������, �������� ������������� �������
	// � ��������� ���������		
	DataMemberMaker( ClassType &ct, ClassMember::AS cas,
		const PTempObjectContainer &t ) : MemberDeclarationMaker(ct, cas, t), targetDM(0) {
	}

	// ������� �������� �������-�����, �������������� ����� ������ DeclarationMaker
	bool Make();

	// ����������� ����� ��� ������������� �����. ���������������� ����� ������,
	// ����������� ����� ������-�����
	void Initialize( MemberInitializationType mit, const Operand &exp ) ;

	// �������� �������������, ����� ���������� NULL, ����
	// ������������� �� ������
	const Identifier *GetIdentifier() const {
		return targetDM;
	}
};


// ��������� �������, ������� �� �������� ������������-��������� �������
class MethodMaker : public MemberDeclarationMaker
{
	// ��������� ������ ���������
	Method *targetMethod;

public:

	// ����������� ��� ������� ������, �������� ������������� �������
	// � ��������� ���������		
	MethodMaker( ClassType &ct, ClassMember::AS cas,
		const PTempObjectContainer &t ) : MemberDeclarationMaker(ct, cas, t), targetMethod(0) {
	}

	// ������� �������� ������ �������������� ����� ������ DeclarationMaker
	bool Make();

	// ����������� ����� ��� ������������� �����. ������������� ����� ����
	// ������ MIT_METHOD, exp ������ ���� ����� NULL
	void Initialize( MemberInitializationType mit, const Operand &exp ) ;

	// �������� �������������, ����� ���������� NULL, ����
	// ������������� �� ������
	const Identifier *GetIdentifier() const {
		return targetMethod;
	}
};


// ��������� ������������� ����������
class OperatorMemberMaker : public MemberDeclarationMaker
{
	// ��������� ������ ���������
	ClassOverloadOperator *targetOO;

	// ��������� ��������� ���������� ���������� �� ���������
	TempOverloadOperatorContainer tooc;

public:

	// ����������� ��� ������� ������, �������� ������������� �������
	// � ��������� ���������		
	OperatorMemberMaker( ClassType &ct, ClassMember::AS cas,
		const PTempObjectContainer &t, const TempOverloadOperatorContainer &tc )
		: MemberDeclarationMaker(ct, cas, t), targetOO(0), tooc(tc) {
	}

	// ������� �������� �������������� ���������,
	// �������������� ����� ������ DeclarationMaker
	bool Make();

	// ����������� ����� ��� ������������� ������. ������������� ����� ����
	// ������ MIT_METHOD, exp ������ ���� ����� NULL
	void Initialize( MemberInitializationType mit, const Operand &exp ) ;

	// �������� �������������, ����� ���������� NULL, ����
	// ������������� �� ������
	const Identifier *GetIdentifier() const {
		return targetOO;
	}
};


// ��������� ���������� ����������
class CastOperatorMaker : public MemberDeclarationMaker
{
	// ���������
	ClassCastOverloadOperator *targetCCOO;

	// ���������� � ����������� �� ���������
	TempCastOperatorContainer tcoc;

public:

	// ����������� �������� ���������
	CastOperatorMaker( ClassType &ct, ClassMember::AS cas,
		const PTempObjectContainer &t, const TempCastOperatorContainer &tc )
		: MemberDeclarationMaker(ct, cas, t), targetCCOO(0), tcoc(tc) {
	}

	// ������� �������� ��������� ����������,
	// �������������� ����� ������ DeclarationMaker
	bool Make();

	// ����������� ����� ��� ������������� ��������� ����������. ������������� ����� ����
	// ������ MIT_METHOD, exp ������ ���� ����� NULL
	void Initialize( MemberInitializationType mit, const Operand &exp ) ;

	// �������� �������������, ����� ���������� NULL, ����
	// ������������� �� ������
	const Identifier *GetIdentifier() const {
		return targetCCOO;
	}
};


// ��������� �������������
class ConstructorMaker : public MemberDeclarationMaker
{	
	// ��������� ������ ���������
	ConstructorMethod *targetCtor;

public:

	// ����������� ��� ������� ������, �������� ������������� �������
	// � ��������� ���������		
	ConstructorMaker( ClassType &ct, ClassMember::AS cas,
		const PTempObjectContainer &t ) : MemberDeclarationMaker(ct, cas, t), targetCtor(0) {
	}

	// ������� �������� ������������, �������������� ����� ������ DeclarationMaker
	bool Make();

	// ����������� ����� ��� ������������� ������������. ������������� 
	// ������������ ����������
	void Initialize( MemberInitializationType mit, const Operand &exp ) ;

	// �������� �������������, ����� ���������� NULL, ����
	// ������������� �� ������
	const Identifier *GetIdentifier() const {
		return targetCtor;
	}
};


// ��������� ������������
class DestructorMaker : public MemberDeclarationMaker
{
	// ��������� ������ ���������
	Method *targetDtor;

public:

	// ����������� ��� ������� ������, �������� ������������� �������
	// � ��������� ���������		
	DestructorMaker( ClassType &ct, ClassMember::AS cas,
		const PTempObjectContainer &t ) : MemberDeclarationMaker(ct, cas, t), targetDtor(0) {
	}

	// ������� �������� ������������, �������������� ����� ������ DeclarationMaker
	bool Make();

	// ����������� ����� ��� ������������� �����������. ������������� ����� ����
	// ������ MIT_METHOD, exp ������ ���� ����� NULL
	void Initialize( MemberInitializationType mit, const Operand &exp ) ;

	// �������� �������������, ����� ���������� NULL, ����
	// ������������� �� ������
	const Identifier *GetIdentifier() const {
		return targetDtor ;
	}
};


// ��������� ��������� �������, ������� �� ����������� � �������
// ��������� ������, � ����������� � ��������� ���������� ��,
// � ����� � ������ ������ ������
class FriendFunctionMaker : public MemberDeclarationMaker
{
	// ��������� ������ ���������
	Function *targetFn;

	// ���� ����������� ��������, ���������� � true
	
	bool isOperator;

	// ���������� �� ���������, ���� ����������� ��������� ��������
	TempOverloadOperatorContainer tooc;

public:

	// ����������� ��� ������� �������
	FriendFunctionMaker( ClassType &ct, ClassMember::AS cas,
		const PTempObjectContainer &t ) 
		: MemberDeclarationMaker(ct, cas, t), isOperator(false), targetFn(0) {

	}

	// ����������� ��� �������������� ���������
	FriendFunctionMaker( ClassType &ct, ClassMember::AS cas,
		const PTempObjectContainer &t, const TempOverloadOperatorContainer &tc ) 
		: MemberDeclarationMaker(ct, cas, t), isOperator(true), tooc(tc), targetFn(0) {

	}

	// ������� �������� ��������� �������, 
	// �������������� ����� ������ DeclarationMaker
	bool Make();

	// ����������� ����� ��� ������������� �������. ������������� ����������	
	void Initialize( MemberInitializationType mit, const Operand &exp ) ;

	// �������� �������������, ����� ���������� NULL, ����
	// ������������� �� ������
	const Identifier *GetIdentifier() const {
		return targetFn;
	}
};


// ��������� ����������� ������ ������ ��� ��� � ���������� ������� ���������
class MemberDefinationMaker : public DeclarationMaker
{
	// ����� � �����������. � ������� �� ������ ����������,
	// ��������� ����������� ���������� ������ �������� ���������� �� ������
	const NodePackage *tsl, *decl;

	// �������������� �������������
	Identifier *targetID;

	// ��������� ��������� � ������� ������������ ����������, ���������� ����� 0
	PTempObjectContainer toc;

	// ���� �������������� ������� �����������, ������ ����:
	// R_OBJECT, R_DATAMEMBER, R_FUNCTION,					
	// R_METHOD, R_OVERLOAD_OPERATOR, R_CLASS_OVERLOAD_OPERATOR,	
	// R_CONSTRUCTOR. ���������� R_UNCKNOWN
	Role idRole;

	// �������� ����� �����, ���������� �� ������������
	const QualifiedNameManager &memberQnm;

	// ����������� ������� ���������� ������� ��� � ����������� �����,
	// � ������ ����, ��� � �������������, ������������, ���������� ����������
	// ������� ��� �� �������� ���� � ����������� �������������
	void SpecifyBaseType( const NodePackage *np, const SymbolTable &st );

	// ���������� ���� ������������ ��������������. ������, �����������
	// ������������ ������������, ��������� ���� ������ ���������.
	// ������ ���� ����-�� ���� ��������� ����
	bool InspectIDRole( const RoleList &rl ) ;
	
	// ���� ������ ����������������� �������������, ������ ���
	const ConstructorMethod *ictor;

public:

	// ����������� ��������� �����
	MemberDefinationMaker( const NodePackage *t, const NodePackage *d, 
		const QualifiedNameManager &qnm ) 

		: tsl(t), decl(d), memberQnm(qnm), targetID(0), toc(NULL), idRole(R_UNCKNOWN), 
		ictor(NULL) {
		
		INTERNAL_IF( tsl == NULL || decl == NULL );
	}

	// ���������������� ������ ���������� 
	void Initialize( const ExpressionList & );

	// ��������� �����������
	bool Make();

	// ������� ��������� �� ������������ �������������, ����� ���� 0
	const Identifier *GetIdentifier() const {
		return targetID;
	}

	// ������� �����������
	const ConstructorMethod *GetConstructor() const {
		return ictor;
	}
};

