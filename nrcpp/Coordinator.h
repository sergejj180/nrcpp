// ���������� ���������� � �������-������������� - Coordinator.h


// ������-������������ - ��� ���������� ��� ��������-�����������. 
// ������������ ����������, ����� ����� ��������� ������������ ��� 
// �������� ������, ����� �������� ������� �������, ����� ��� �������
// ������� ��������� � ��. ������������ ����� ��������� �� ��� 
// �������� �����: ������������ ���������� � ������������ 
// ���������. � ������ ������ ��� ���������� ������������ ���������,
// ������� ������ ������� � ��������� �� ��������� ����������. ��
// ������ ������ ����� ���������� ��������� ������������, � �����
// ����������� ������������ � ��������� �����������.


// ��� - ���������������� ��������� �� ���������
typedef SmartPtr<DeclarationMaker> PDeclarationMaker;


// �����-����������� ��� ����������
class DeclarationCoordinator
{
	// ������� �����, ������� ������������ ��� ����������� 
	// ������ ������������
	const NodePackage *typeSpecList, *declarator;
	
	// ������� ���������, ������� ����������� � ������� �� ���
	// ����������� ����� ������ ��� ����������� ��
	mutable SymbolTableList memberStl;

	// �������� �����, ����������� ��� �������� ��������� �����
	mutable QualifiedNameManager *memberQnm;

	// ������ �������� ��������� �����, ��������� 
	// �������� �����, ��������� �������� ����� ����� � �������
	// ��������� �����
	void StoreMemberScope( const NodePackage &np ) const;

public:
	// � ������������ ������� 2 ������ ��� �������:
	// 1. ������ �������������� �������, 2. ���������� (���, ������ ����������� �����)
	DeclarationCoordinator( const NodePackage *tsl, const NodePackage *dcl ) 
		: typeSpecList(tsl), declarator(dcl), memberQnm(NULL) {
		INTERNAL_IF( typeSpecList == NULL || declarator == NULL );
	}

	// � ����������� ������������ �������� �����
	~DeclarationCoordinator() {
		delete memberQnm;
		RestoreScopeSystem();
	}

	// ��������������� ��������� ����������, ������� �� ������
	// ��������� ��������� � ����� �� �� ��������� ������� ���������
	PDeclarationMaker Coordinate() const;

	// ������������ ������� �� ����� ����������� ����� ��� ��� ������
	void RestoreScopeSystem() const;
};


// ����������� ��������� ����������
class AutoDeclarationCoordinator
{
	// ������� �����, ������� ������������ ��� ����������� 
	// ������ ������������
	const NodePackage *typeSpecList, *declarator;
	
public:
	// � ������������ ������� 2 ������ ��� �������:
	// 1. ������ �������������� �������, 2. ���������� (���, ������ ����������� �����)
	AutoDeclarationCoordinator( const NodePackage *tsl, const NodePackage *dcl ) 
		: typeSpecList(tsl), declarator(dcl) {
		INTERNAL_IF( typeSpecList == NULL || declarator == NULL );
	}

	// ��������������� ��������� ����������, ������� �� ������
	// ��������� ��������� � ����� �� �� ��������� ������� ���������
	PDeclarationMaker Coordinate() const;
};


// ��� - ���������������� ��������� �� ��������� ����� ������
typedef SmartPtr<MemberDeclarationMaker> PMemberDeclarationMaker;


// �����-����������� ��� ���������� ������ ������
class MemberDeclarationCoordinator
{
	// ������ ����� � ���������� ��� ������������ ��������� ���������
	// � ����������� ���������
	const NodePackage *typeSpecList, *declarator;

	// ������ �� �����, ���� �������� ��������
	ClassType &clsType;

	// ������� ������������ ������� � ���� ������
	ClassMember::AS curAccessSpec;

public:
	// � ������������ �������� ������, � ����� ����� � 
	// ������� ������������ �������
	MemberDeclarationCoordinator( const NodePackage *tsl, const NodePackage *dcl,
		ClassType &ct, ClassMember::AS cas ) : typeSpecList(tsl), declarator(dcl), 
											   clsType(ct), curAccessSpec(cas) 
	{
		INTERNAL_IF( typeSpecList == NULL || declarator == NULL );
	}

	// ���������� ��������� ���������� 
	PMemberDeclarationMaker Coordinate();	
};


// ����������� ������� ���������, ��������� ��������������� ��������
// ������������ ���������, � ����� ��������� ������������� ������������� 
// ��������� ��� ������ �������������� ���������. ����� ����������
// ���� �������� �������� ��������� ������� ��������� �� ��������� ���������.
// ����������� ������ ������������ �������� ����� �������
class UnaryExpressionCoordinator
{
	// ������� ��� ������ ������
	const Position &errPos;

	// ������� �������� ���������
	const POperand &right;

	// ��� ���������
	int op;

public:
	// ������ ���������� ��� �����������
	UnaryExpressionCoordinator( const POperand &r, int op_, const Position &ep ) 
		: right(r), op(op_), errPos(ep) {
	}

	// ��������������� � ��������� ���������
	POperand Coordinate() const;
};


// ����������� �������� ���������. �� ��������� � ������� �������������,
// �������� ���������, �.�. ��� ���������� �������� ��� ������� �������� �
// ������� ���������� ������ ����� ����� ������ ����� ���������
template <class Maker>
class BinaryExpressionCoordinator
{
	// ������� ��� ������ ������
	const Position &errPos;

	// ����� ������� ��������� ���������
	const POperand &left;

	// ������ ������� ��������� ���������
	const POperand &right;

	// ��� ���������
	int op;

public:
	// ������ ���������� ��� �����������
	BinaryExpressionCoordinator( const POperand &l, const POperand &r, 
		int op_, const Position &ep ) 

		: left(l), right(r), op(op_), errPos(ep) {
	}

	// ��������������� � ��������� ���������
	POperand Coordinate() const {

		// ��������� ������ ��������������
		INTERNAL_IF( left.IsNull() || right.IsNull() );

		// ������� ���������, ���� ��������� ��������, ������� ��� ��
		if( left->IsErrorOperand() || right->IsErrorOperand() )
			return ErrorOperand::GetInstance();		

		// ������� �� ����� ���� �����, ������ ���� ��� �� �������� ����������
		if( right->IsTypeOperand() || 
			(left->IsTypeOperand() && 
			 !(op == OC_CAST || op == KWSTATIC_CAST || op == KWDYNAMIC_CAST ||
			   op == KWCONST_CAST || op == KWREINTERPRET_CAST) ) )
		{
			theApp.Error(errPos, "��� �� ����� ���� ��������� � ���������");
			return ErrorOperand::GetInstance();
		}

		// �� ����� ���� �������������� ��������
		else if( left->IsOverloadOperand() || right->IsOverloadOperand() )
		{
			theApp.Error(errPos, "������������� ������� �� ����� ���� ��������� � ���������");
			return ErrorOperand::GetInstance();
		}

		// ���������, ���� ����� ������������� ������-����, ����� 
		// ��������� � ���� ���� ����� 'this'
		if( left->IsPrimaryOperand() )
			ExpressionMakerUtils::CheckMemberThisVisibility(left, errPos);
		if( right->IsPrimaryOperand() )
			ExpressionMakerUtils::CheckMemberThisVisibility(right, errPos);		

		// ����� �������� ��������� ������� �� ������������������
		POperand rval = BinaryInterpretator(left, right, op, errPos).Interpretate();
		if( !rval.IsNull() )
			return rval;

		// �������� ��������� ����� �������������� ��������� ��� ���� ���������
		rval = BinaryOverloadOperatorCaller(left, right, op, errPos).Call();
		if( !rval.IsNull() )
			return rval;

		// ����� ������ ���������, ��������� ��� ����� ��� ���������
		// ��������
		return Maker(left, right, op, errPos).Make();
	}
};


// ����������� ��������� ���������. ��������� ��������� ���� '?:',
// ����������� ��������� ��� ��������������� �������� ������������ ���������
// � �������� ���������� �������������
class TernaryExpressionCoordinator
{
	// ������� ��� ������ ������
	const Position &errPos;

	// �������
	const POperand &cond;

	// ����� ������� �� ':'
	const POperand &left;

	// ������ ������� �� ':'
	const POperand &right;

	// ��� ���������
	int op;

public:
	// ������ ���������� ��� �����������
	TernaryExpressionCoordinator( const POperand &c, const POperand &l, const POperand &r, 
		int op_, const Position &ep ) 

		: cond(c), left(l), right(r), op(op_), errPos(ep) {
	}

	// ��������������� � ��������� ���������
	POperand Coordinate() const;
};
