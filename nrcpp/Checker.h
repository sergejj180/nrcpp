// ��������� � �������-�������  - Checker.h


// �������� � Maker.h
struct TempObjectContainer ;

// �������� � Maker.h
struct TempOverloadOperatorContainer;

// �������� � Maker.h
struct TempCastOperatorContainer;

// �������� � Class.h
class ClassType;

// �������� � Object.h
class TypyziedEntity;

// �������� � Object.h
class FunctionPrototype;

// �������� � Body.h
class Operand;


// ������� ������������ ��� ��� ��������, ��� �� ��������� ������
namespace CheckerUtils
{	
	// ��������� ������������ ������ ����������� �����
	bool CheckDerivedTypeList( const TempObjectContainer &object );

	// ���������� �������� ������������� �������� ���� � �����������. 
	bool CheckRelationOfBaseTypeToDerived( TempObjectContainer &object,
							 bool declaration, bool expr = false ) ;

	// ���������, ����� �� ����� ���� ������� ��� ������� ������
	bool BaseClassChecker(const ClassType &cls, const SymbolTableList &stl, 
		const Position &errPos, PCSTR cname );

	// ��������� ����������� ����������� ������
	bool ClassDefineChecker( const ClassType &cls, const SymbolTableList &stl, 
		const Position &errPos );	

	// ���������, ���� ��� typedef, �������� �������, ������� ����� ����� 0
	const ClassType *TypedefIsClass( const ::Object &obj );

	// ��������� ���������� �����. ���� ��� �� �������� ������ ������, ��� �� �����������
	// �� �����������. ���� ��� ���������� - ������� ������
	void CheckAccess( const QualifiedNameManager &qnm, const Identifier &id, 
		const Position &errPos, const SymbolTable &ct = GetCurrentSymbolTable() );

	// �������� ������� ��� ������������������ �����, ��������� �����������
	// ������� ����� � ������������ � ���� ��������� ���� �������� �������,
	// ������� ��� ��� �������� ������ � ������ � CheckAccess, ����� ������� 0.
	// ������ ��������� 'ct' ������ ���� ��������� ������������� �� ���������
	// � ��������������, ���� ���������. ������ �������������� � 'qnm' �� ������
	// ���� ������.
	const ClassType *CheckQualifiedAccess( const QualifiedNameManager &qnm,
		const Position &errPos, const SymbolTable &ct );

	// ��������� ������������ ���������� ���������� �� ��������� � �������.
	// ���� ������ �������� 0, ������ ��������� ������������ ������ � ������
	void DefaultArgumentCheck( const FunctionPrototype &declFn, 
		const FunctionPrototype *fnInTable, const Position &errPos );
}


// �������� ������������ ���������� ����������� �������
class GlobalDeclarationChecker
{
	// ��������� ��������� �������, ����� ����� ����� 
	// �������� ���
	TempObjectContainer &object;

	// ���� ������ ����������������, ���������� � true. �� �����
	// �������� ��������� ��� ��� ��� � ����� ������ �����.
	// ���� ���� ������� � ������������� ������� � �������
	bool redeclared;

	// ���� ��������� ���� ���� ������ ��� ��������, ��������� � true
	bool incorrect;

	// ���� ��������� ��������� ����������, ���������� � true
	bool localDeclaration;

	// ������� ������� ��������
	void Check();


	// ������� ������, ���������� ����
	void Error( PCSTR msg, PCSTR arg );
		
public:
	// ������ ���������
	GlobalDeclarationChecker( TempObjectContainer &obj, bool ld = false ) 
		: object(obj), localDeclaration(ld) {
		redeclared = false;
		incorrect = false;
		Check();
	}

	// ���� ����������������, ������� � ����, ��� ���� ������
	// �� ����� ��������� � �������
	bool IsRedeclared() const {
		return redeclared;
	}

	// ���� ��� �������� ��������� ������
	bool IsIncorrect() const {
		return incorrect;
	}
};


// ��������� ������������ ���������
class ParametrChecker 
{
	// ������ ��� ����������� ���������� �������
	// ����� ����� ���� �������� ���������������� �������� ��� ���
	const FunctionParametrList &fnParamList;

	// ��������� ��������� ���������, ����� ����� ����� 
	// �������� ���
	TempObjectContainer &parametr;

	// ���� ��������� ���� ���� ������ ��� ��������, ��������� � true
	bool incorrect;

	// ������� ������� �������� ���������, ��������� �������� ������
	// ������� 
	void Check();

public:

	// ����������� ����� �� �������� �������� ����� �������������
	ParametrChecker( TempObjectContainer &prm, const FunctionParametrList &fp ) 
		: parametr(prm), fnParamList(fp) {

		// ��������� �� ����� ���� redeclared, �.�. ���� ����
		// ��� �������� �������� � ����� ������, �� �������������
		// ������ ��� ����������, ��� ���������� ��������� �������
		incorrect = false;
		Check();
	}

	// ���� ��� �������� ��������� ������
	bool IsIncorrect() const {
		return incorrect;
	}
};


// �������� ���� throw-������������
class ThrowTypeChecker  
{
	// ��������� ��������� ����
	TempObjectContainer &throwType;

	// ������� ������� �������� ����, ��������� �������� ������
	// ������� 
	void Check();

public:

	// ����������� ����� �� �������� �������� ����� �������������
	ThrowTypeChecker( TempObjectContainer &tt ) 
		: throwType(tt)  {

		Check();
	}
};


// �������� catch-����������
class CatchDeclarationChecker
{
	// ��������� ��������� ����
	TempObjectContainer &toc;

	// ������� ������� �������� ����, ��������� �������� ������
	// ������� 
	void Check();

public:

	// ����������� ����� �� �������� �������� ����� �������������
	CatchDeclarationChecker( TempObjectContainer &ct ) 
		: toc(ct)  {

		Check();
	}
};


// �������� ���������� �������-�����
class DataMemberChecker
{
	// ��������� ��������� �� ������� �������� ����
	TempObjectContainer &dm;

	// ��������������� � true, ���� ���� ������������
	bool redeclared;

	// ��������������� � true, ���� ���� ����������� ������������
	bool incorrect;

	// ����� �������� ����� ����� �������� ��� � �������
	void Check();

	// ���������� true, ���� ������ �������� ����������
	bool ConstantMember();

	// ���������� ��-NULL, ���� ����� �������� �� ����������� �-���,
	// �-��� �����������, ����������, �������� �����������
	PCSTR HasNonTrivialSMF( const ClassType &cls );

public:
	// � ������������ ���������������� ����
	DataMemberChecker( TempObjectContainer &m ) : dm(m), redeclared(false), incorrect(false) {
		Check();
	}

	// ���� �������������
	bool IsRedeclared() const {
		return redeclared;
	}

	// ���� �����������
	bool IsIncorrect() const {
		return incorrect;
	}

};


// �������� ���������� �������-�����
class MethodChecker
{
	// ��������� ��������� �� ������� �������� �����
	TempObjectContainer &method;

	// ��������������� � true, ���� ����� ������������
	bool redeclared;

	// ��������������� � true, ���� ����� ����������� ������������
	bool incorrect;

	// ����� �������� ������ ����� �������� ��� � �������
	void Check();
	

public:
	// � ������������ ���������������� ����
	MethodChecker( TempObjectContainer &m ) : method(m), redeclared(false), incorrect(false) {
		Check();
	}

	// ���� �������������
	bool IsRedeclared() const {
		return redeclared;
	}

	// ���� �����������
	bool IsIncorrect() const {
		return incorrect;
	}

};


// �������� �������������� ��������� ������
class ClassOperatorChecker
{
	// ��������� ��������� � ������� ���������� ���������� � ���
	TempObjectContainer &op;

	// ��������� ��������� � ����� ��������� � ������
	const TempOverloadOperatorContainer &tooc;

	// ���� �������� ������������� ������ ������
	bool redeclared;

	
	// ����� ��������
	void Check();

	// ����� ������������ true, ���� �������� �������� �����
	bool IsInteger( const Parametr &prm ) const;

public:

	// ������ ���������
	ClassOperatorChecker( TempObjectContainer &_op, const TempOverloadOperatorContainer &t )
		: op(_op), tooc(t), redeclared(false) {
		Check();
	}

	// ���� �������������
	bool IsRedeclared() const {
		return redeclared;
	}
};


// �������� ��������� ���������� ����
class CastOperatorChecker
{
	// ��������� ��������� � ������� ���������� ���������� � ���
	TempObjectContainer &cop;

	// ��������� ���������, ������� �������� ���������� �� ���������
	const TempCastOperatorContainer &tcoc;


	// ����� ��������
	void Check();

	// ���� �������� �� �������� ��������, ���������� � true
	bool incorrect;

public:
	
	// ������ ���������
	CastOperatorChecker( TempObjectContainer &op, const TempCastOperatorContainer &t )
		: cop(op), tcoc(t), incorrect(false)  {
		Check();
	}

	// ������� true ���� �������� �� �������� ��������
	bool IsIncorrect() const {
		return incorrect;
	}
};


// �������� ������������
class ConstructorChecker
{
	// ��������� ��������� � ������� ���������� ���������� � ���
	TempObjectContainer &ctor;

	// ������ �� ����� � ������� ����������� �����������
	const ClassType &cls;

	// ���� �������������� � true, ���� ����������� ������ �� �����
	// ����������� ����������� � �������
	bool incorrect;

	// ����� ��������
	void Check();

public:

	// ������������� ��������� ���������
	ConstructorChecker( TempObjectContainer &c, const ClassType &cl );
		
	// ������� �� ��������� ����������� � �������
	bool IsIncorrect() const {
		return incorrect;
	}
};


// ��������� ������������ ����������� �������������� ���������
class GlobalOperatorChecker
{
	// ��������� ��������� � ������� ���������� ���������� � ���
	TempObjectContainer &op;

	// ��������� ��������� � ������� ���������� ���������� �� ���������
	const TempOverloadOperatorContainer &tooc;

	// ������� ��������
	void Check();

	// ����� ������������ true, ���� �������� �������� �����
	bool IsInteger( const Parametr &prm ) const;

	// ���������, ����� �������� ��� �������, ������� �� �����, �������������,
	// ������� �� ������������
	bool IsCompoundType( const Parametr &prm ) const;

public:

	// �������� ��������� ���������
	GlobalOperatorChecker( TempObjectContainer &_op, const TempOverloadOperatorContainer &tc ) 
		: op(_op), tooc(tc) {

		Check();
	}
};


// �������� ������� � ������
class AccessControlChecker
{
	// ������� ��������� ��� ������� ������� ��������� ����������� �������
	const SymbolTable &curST;

	// ����� ����� ������� ������������ ������ � �����
	const ClassType &memberCls;

	// ��� ���� �����, ������� ����������� �� �����������
	const ClassMember &member;	

	// ��������������� � true, ���� ���� ��������
	bool accessible;

	// �������� �������, ������� ��������� �������� ������ ������
	void Check();

public:

	// � ������������ ����������� ������� ������� ���������, �� ������ ������� �����������
	// ������, �����, ����� ������� ������������ ������ � �����, � ��� ����
	AccessControlChecker( const SymbolTable &ct, const ClassType &mcls, const ClassMember &cm )
		: curST(ct), memberCls(mcls), member(cm), accessible(false) {

		Check();
	}

	// ���������� true, ���� ���� ��������
	bool IsAccessible() const {
		return accessible;
	}

private:

	// ��������������� ���������, ����������� �������������� 
	// ������������ ������� �����
	struct RealAccessSpecifier
	{
		// ������������ ����� ��������� ��� 4 �������� ����������������
		// ����� AS. � ��������� ������, ����� �������� NOT_CLASS_MEMBER,
		// ��������, ��� � ����� ��� ������� (�.�. �� ���������
		// � �������� ������� ������)
		ClassMember::AS realAs;

		// ��� ����
		const ClassMember *pMember;

		// ����� � �������� ����������� ����,
		// �������� ��� ������ ����� ����� ������� � ��������� ������
		const ClassType *pClass;

		// ���� ���������������, ���� ����� ������� � ���������
		// ������� ��������
		bool isClassFound;

		// � ������������ �������� ��������� ��������
		RealAccessSpecifier( ClassMember::AS as, const ClassMember *p )
			: realAs(as), pMember(p), isClassFound(false) {

			pClass = static_cast<const ClassType *>(
				&dynamic_cast<const Identifier *>(pMember)->GetSymbolTableEntry());
		}
	};
	

	// ����������� �������, �������� �� ����� ������ ������� ������� � 
	// �������� ������������ ������� ����� � ����������� �� �������������
	// ������������ �� �������:  public-(public, protected, no_access), 
	// protected-(protected, protected, no_access), private-(no_access, no_access, no_access).
	// ������� ����������� ��� �������� ��� ��� �������� �������� ����� ����
	// �������� �� ���������� �����, � ���� ������ ���������� �������� ��������� ����
	void AnalyzeClassHierarhy( RealAccessSpecifier &ras, 
		ClassMember::AS curAS, const ClassType &curCls, int level );

	
	// ������� ���������� true, ���� d �������� ����������� ������� b
	bool DerivedFrom( const ClassType &d, const ClassType &b );
};


// ��������, �������� �� ����� ����������� � ���� ��������, �� ���������
// �� ���� ������������ �������� ����������� �������
class VirtualMethodChecker
{
	// ����������� �����, �� ��������� �������� ����������� ��������
	Method &method;

	// ����: ����� ���� �����, ������������� ��������, �������� 
	// �������� ���������� ��� ����������
	Role destRole;

	// �������, � ������� ������� �������� ������ � ������ �� �������������
	const Position &errPos;

	// ��� - ������ ����������� �������
	typedef list<const Method *> VML;

	// ������ ����������� ������� �� ������� �������
	VML vml;

	// ����������� �����, ��������� ���������� ������, ��������,
	// ������� ��������� �� ��������� � ��������� �������
	void FillVML( const ClassType &curCls );

	// ���� ��������� � ������ 'vm', ����� �� ��� � 'method',
	// ������� true. ��� ���� 'vm' ������ ���� �����������
	bool EqualSignature( const Method *vm );

	// �������� �-��� ��������� ��������
	void Check();

	// ��������� �������� ������������ ���������
	// ������� �������
	void CheckDestructor( const ClassType &curCls );

	// ����� ������� ��� ������ ������ � ������ ����������� �������
	class VMFunctor {
		// ��� ������
		string mname;
	public:
		// ������ ��� ������
		VMFunctor( const string &mn )
			: mname(mn) {
		}

		// �������� ����� � ���
		bool operator()( const Method *m ) const {
			return mname == m->GetName().c_str();
		}
	};

public:

	// � ������������ �������� ��������� � ����� ���� ��������.
	// ���� ����� ��������, ��� ������� �����������, �� ����� ������
	// �������������. ���� ����� ��������, ��� ����� �����������
	// �����-����������� ����� �� �������� ������, ����� ��������
	// ������� ����������� ������� ������.
	VirtualMethodChecker( Method &dm, const Position &ep )
		: method(dm), errPos(ep), destRole(R_UNCKNOWN) {
				
		// ��������� ��������, � ���� ��������� ������ �����. �����
		Check();

		// ���� ����� ��������� � � ���� ��� �����. � ������� �������,
		// ������� ����������� �������, ���� ��� �� ������� � ������ 
		// ������������� ������
		if( method.IsVirtual())
		{
			ClassType &mcls = ((ClassType &)method.GetSymbolTableEntry());
			if( vml.empty() )
			{				
				if( mcls.GetVirtualFunctionCount() == 0 )
					mcls.MakeVmTable();
				method.SetVirtual(NULL);
			}

			mcls.AddVirtualFunction(method);
		}

		// ��������� �������� - ���� ����� ��������� ������������ ������
		// ����� �������� ����������� � �� �������� ����������� - �������
		// ������
		if( method.IsAbstract() && !method.IsVirtual() )
			theApp.Error( errPos, 
				"'%s' - �����-����������� ����� ������ ����������� �� �������������� 'virtual'",
				method.GetName().c_str());
	}
};

