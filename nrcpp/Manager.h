// ���������� �������-���������� - Manager.h


// ������ ���������. ������������ ����� ������������� ����� ����� 
// �������� ������� � ��������� ���� ������� ������ �� ������������.
// ������ ��������� ��������� ���� ������ ��������-�����������.
// ������-��������� ����� ����� ������� ������������, ��� ���
// ��� ��� ������� ������������� �������������� ���������� ��
// �������, � �������� ���� ���������� ���������� ������ ������
// ��� �������.
// ������� �������� ���� ������������� ��������� �������: ��������
// ������ ����� ����������� �����, �������� ������ ����� ����������
// ����� (��������������� � ���� ������), �������������� ����������,
// ����� ��� ���������� ����������� ���� � ����� ������, �����������
// �� ��� ���, ��� � ����� �� ������, � ��.


// �������� � Parser.h
class NodePackage;

// �������� � Parser.h
class Package;

// �������� � Class.h
class TemplateClassType;


// ���� ����
enum Role 
{ 
	R_UNCKNOWN,					// ����������� ����, ������������, ���� ������ �� ������
	R_OBJECT,					// ������
	R_DATAMEMBER,				// ������-����
	R_PARAMETR,					// �������� �������
	R_ENUM_CONSTANT,			// ��������� ������������
	R_CLASS_ENUM_CONSTANT,		// ��������� ������������ ����������� � ������

	R_FUNCTION,					// �������
	R_METHOD,					// �����
	R_OVERLOAD_OPERATOR,		// ���������� ������������� ��������
	R_CLASS_OVERLOAD_OPERATOR,	// ������������� �������� ����������� ������ ������
	R_CONSTRUCTOR,				// ����������� ������

	R_CLASS_TYPE,				// class, struct
	R_ENUM_TYPE,				// enum
	R_UNION_CLASS_TYPE,			// union

	R_TEMPLATE_CLASS,					// ��������� �����
	R_TEMPLATE_CLASS_SPECIALIZATION,	// ������������� ���������� ������
	R_TEMPLATE_FUNCTION,				// ��������� �������
	R_TEMPLATE_FUNCTION_SPECIALIZATION,	// ������������� ��������� �������

	R_USING_IDENTIFIER,					// ������������� �� ������ ��, ����������� ��� using
	R_TEMPLATE_TYPE_PARAMETR,			// ��������� �������� ����
	R_TEMPLATE_TEMPLATE_PARAMETR,		// ��������� �������� �������
	R_TEMPLATE_NONTYPE_PARAMETR,		// ��������� �� ������� ��������

	R_NAMESPACE,						// ����������� ������� ���������
	R_NAMESPACE_ALIAS,					// ������� ����������� ������� ���������
};


// ���� - �������������, ����
typedef pair<Identifier *, Role> RolePair;

// ������ �� ���: �������������, ����
typedef list<RolePair> RoleList;


// ������� ����������
namespace ManagerUtils
{
	// �������� ��������� ������������� ������������� �������� �������
	CharString GetObjectStorageSpecifierName( ::Object::SS ss );


	// �������� ��������� ������������� ������������� �������
	CharString GetFunctionStorageSpecifierName( Function::SS ss );

	// �������� ������������ ������� � ���� �����
	PCSTR GetAccessSpecifierName( ClassMember::AS as );

	// �������� �������� ������� ��������� � ���� �����, ����
	// ��� �������� ���������������, ������� ���
	CharString GetSymbolTableName( const SymbolTable &st );
}


// ����� ������ ���������. ��������� � ������ ������������ ������ �����
// ������ using-���������� �� ��������� �� �������������
class SynonymList : public RoleList
{
public:
	// ����� using-�������������� �� ��������� �� ������������ �������������
	const UsingIdentifier *find_using_identifier( const Identifier *id ) const;
};


// �������� ����������� ����. ��� ���������� ����� ��� ���������� ����� ��� 
// ������������� �������� � ������ ���������. ������������� �������� 
// ����� ������ ��������� ��� ���� �����. ���� ����� �� ������� ��������� 
// ������ ����� ������� ����� � �������� ������� ���������, ���� � �������. 
// � ����� ������������� ���������� ����� ���: ���������� ����, 
// ������ �� ���������� ����, ��������� �� ��������� ��� - ������ ����, 
// �������� �� ��������� ��� - ������ ������, � ��.
class NameManager
{
	// ����������� ���
	CharString queryName;

	// ������ ����� ����������� �����
	RoleList roleList;
	
	// ������ ������������ ���������������-���������. �������� ��������
	// �������� ����������� ������� ��������� � using-�������������.
	// �������� ���������� � ���� ������, �.�. ��������������, �������
	// ��� ���������� ����� ���� � ������ �����. ���� �������� ����������
	// ��� �������� �������
	SynonymList synonymList;

	// ������� ��������� � ������� ������� ����������� ������ ����� ������� ����� 
	// ������ ����� ������������ � � ������������� �������� ���������
	// (��� ������� - �������, ��� ������ - using-�������). ���� �������
	// ��������� �� ������ - NULL, ����������� ����� �� ������� �����.
	// �� ����
	const SymbolTable *bindTable;

	// ����������� ����� ����� � �� ��������� ��������,
	// ��� ������� - ������� ������, ��� ������ - using �������
	bool watchFriend;

public:

	// ����������� ��������� ������, � ��������� ������ �����
	// �������� �������. 
	// qn - ��� (������), bt - ���� ������, ������� ��������� � ������� �������
	// ������ ���, watchFriend - ����������� ����� ����� � �� ��������� ��������,
	// ��� ������� - ������� ������, ��� ������ - using �������
	NameManager( const CharString &qn, const SymbolTable *bt = NULL, bool watchFriend = true ); 

	// ���� � ����� ���� ����, �.�. ��� ��������� - ������� true
	bool IsUnique() const {
		return GetRoleCount() == 1;
	}

	// ���� ��� �������� �����
	bool IsTypeName() const ;

	// ���� ��� �������� ����� typedef
	bool IsTypedef() const;

	// �������� ��� ���� ��� ��������� �����. ����� ��� ���������, ��� ��� ����� ������
	// ��������� ��������� ����� (����. ������������� �������, ��������� �-���,
	// �����). ���������� false, ���� ����� ���
	const RoleList &GetRoleList( ) const {
		return roleList;
	}		

	// �������� ���������� ����� ������� �����
	int GetRoleCount() const {
		return roleList.size();
	}

	// �������� ������ ���������
	const SynonymList &GetSynonymList() const {
		return synonymList; 
	}

	// �������� ���� ��������������
	static Role GetIdentifierRole( const Identifier *id ) ;
};


// �������� ��������� ����. ������� ������ NameManager, � ��� 
// ���� ��������, ��� ���������� ������������ ��� ���������� �����, 
// ������� ���������������� ����� ��� ����������� ��������� ��������� 
// � ������������ � ���� ������.
class QualifiedNameManager
{
	// ������ �� �������� ���������, ������� ���������� ������������ �����
	// ������� ��������� ����������� � ��� �������, � ������� ��� �����������
	// � ������
	SymbolTableList qualifierList;

	// ������ ����� ������ ���������� �����, ����������, ������� � ���������������
	RoleList roleList;

	// ������ ��������� ��� ����������� �������� ������� � using-��������������
	SynonymList synonymList;

	// �����, ������� �������� �������� � ���������
	const NodePackage *queryPackage;

	// ������� ��������� � ������� ������� ����������� ����� ������� ����� 
	// ����� ���� NULL, ����� ����� ������������ � �������
	const SymbolTable *bindTable;

	// ���������, �������� �� ��� �������� ���������. ���� �������� - ���������� 
	// ��������� �� ���, � ��������� ������ - NULL
	const SymbolTable *IsSymbolTable( const NameManager &nm ) const;

	// ���������� ��� ������. ����� ����� ����� ��� NAME, PC_OVERLOAD_OPERATOR,
	// PC_CAST_OPERATOR, PC_DESTRUCTOR. � ��������� ���� ������� ���������� 
	// �������-��������� ��� ��������� ����������� ����� ��������������
	CharString GetPackageName( const Package &pkg );

public:
	// ������� �������� ���������� �����,
	// ����� ������ ������� ���������, � ������� ������� ������ ���. 	 
	// ��� ������� ������������ ��� ������ ����� ��������� ����, ���� �����
	// �������������� � ��� ���������, � ������ ���� np �������� ������
	// ���� ��� �����. np - ������ ����� ��������� PC_QUALIFIED_NAME
	QualifiedNameManager( const NodePackage *np, const SymbolTable *bt = NULL );

	// �������� ������ ��������������
	const SymbolTableList &GetQualifierList() const {
		return qualifierList;
	}

	// �������� ������ �����
	const RoleList &GetRoleList() const {
		return roleList;
	}

	// �������� ������ ���������
	const SynonymList &GetSynonymList() const {
		return synonymList; 
	}

	// �������� ���������� �����
	int GetRoleCount() const {
		return roleList.size();
	}

		// ���� � ����� ���� ����, �.�. ��� ��������� - ������� true
	bool IsUnique() const {
		return GetRoleCount() == 1;
	}

	// ���� ��� �������� �����
	bool IsTypeName() const ;

	// ���� ��� �������� ����� typedef
	bool IsTypedef() const;
};


// �������� ���������� �����. �������� � ������: bool, char, int, float, double, void,
// short int, long int, long double. ������� �������������� - ��������� ������� ���� 
// BaseType , ���������� ������ ����������� ����.
class ImplicitTypeManager
{
	// ��� �������� ����
	BaseType::BT baseTypeCode;

	// ����������� ����� 
	BaseType::MSIGN modSign;

	// ����������� �������
	BaseType::MSIZE modSize;

public:

	// �������� ���������� �����. � ���������� ��� ����, ��� �������������,
	// ���� ������������ �� ������ ��� ����� -1
	ImplicitTypeManager( int lcode, int msgn = -1, int msz = -1 );

	// ����������� ��� ��� ���������� ����
	ImplicitTypeManager( const BaseType &bt );

	// ����������� ��� ������������ ����
	ImplicitTypeManager( BaseType::BT btc, BaseType::MSIZE mz = BaseType::MZ_NONE,
		BaseType::MSIGN mn = BaseType::MN_NONE ) : baseTypeCode(btc), modSize(mz), modSign(mn)
	{
	}

	// �� ����, ���������� ��������� �� ��� ��������� ������� ���
	const BaseType &GetImplicitType() ; 

	// �������� ���
	BaseType::BT GetImplicitTypeCode() const {
		return baseTypeCode;
	}
	

	// �������� ������ ����
	int GetImplicitTypeSize() const ;

	// �������� ��������� ������������� �����
	CharString GetImplicitTypeName() const ;
};


// �������� �������������� ����, ������� ������������ ��� ��������
// �������-���������� �� ������
class TypeSpecifierManager
{
	// ��������� ������ �������������� (type specifier group)
	enum {
		TSG_UNCKNOWN,		// �����������, �������� �� ���������
		TSG_BASETYPE,		// ������� ��� (bool, char, wchar_t, int, float, double, void)
		TSG_CLASSSPEC,		// ������������ ������ (enum, class, union, struct)
		TSG_CVQUALIFIER,	// cv-������������ (const, volatile)
		TSG_SIGNMODIFIER,	// ����������� ����� (unsigned, signed)
		TSG_SIZEMODIFIER,	// ����������� ������� (short, long)
		TSG_STORAGESPEC,	// ������������ �������� (auto, extern, static, 
							// register, mutable, typedef)
		TSG_FRIEND,			// ������������ ������
		TSG_FUNCTIONSPEC,	// ������������ ������� (inline, virtual, explicit)		
	} group;

	// ��� �������������
	int code;

public:

	// ����������� ���������� � ����� ������ ��������� ���
	TypeSpecifierManager( int c );

	// ���� ��� �������� ����������
	bool IsUncknown() const {
		return group == TSG_UNCKNOWN;
	}

	// ���� ��� �������� ������� �����
	bool IsBaseType() const {
		return group == TSG_BASETYPE;
	}

	// ���� ��� �������� ������� �����
	bool IsClassSpec() const {
		return group == TSG_CLASSSPEC;
	}
	
	// ���� ��� �������� cv-��������������
	bool IsCVQualifier() const {
		return group == TSG_CVQUALIFIER;
	}

	// ���� ��� ��������  ������������� �����
	bool IsSignModifier() const {
		return group == TSG_SIGNMODIFIER;
	}

	// ���� ��� ��������  ������������� �������
	bool IsSizeModifier() const {
		return group == TSG_SIZEMODIFIER;
	}
	
	// ���� ��� ��������  ������������� �������
	bool IsStorageSpecifier() const {
		return group == TSG_STORAGESPEC;
	}

	// ���� ��� �������� ������
	bool IsFriend() const {
		return group == TSG_FRIEND;
	}

	// ���� ��� �������� �������������� �������
	bool IsFunctionSpecifier() const {
		return group == TSG_FUNCTIONSPEC;
	}

	// --- ��� ������ CodeToXXX() ����������� ����� ����� � ��� ��� ��������
	//	   � ����������. ���� ������ �� �����. ����, ���������� ���������� ������	
	// ������� ������� ���, � ������ ���� ������������ �������� ������� �����	
	BaseType::BT CodeToBaseType() const ;

	// ������� ������������ ������
	BaseType::BT CodeToClassSpec() const ;

	// ������� ����������� �����
	BaseType::MSIGN CodeToSignModifier() const ;

	// ������� ����������� �������
	BaseType::MSIZE CodeToSizeModifier() const ;

	// ������� ������������ �������� �������
	::Object::SS CodeToStorageSpecifierObj() const ;
	
	// ������� ������������ �������� �������
	Function::SS CodeToStorageSpecifierFn() const ;

	// �������� ��� ����
	CharString GetKeywordName() const {
		return group == TSG_UNCKNOWN ? "<uncknown>" : ::GetKeywordName(code);
	}

	// �������� ��� ������ �� ������� �����
	CharString GetGroupNameRU() const ;

	// �������� ��� ������ �� ����������
	CharString GetGroupNameENG() const;
};


// x������������ ����� 'Base' �� ��������� � ������ 'Derived' ��
// ����� ����������: �������������, �������������, �����������,
// � ����� �������� �� 'Base' ������� ������� ��� 'Derived'
class DerivationManager
{
	// ���� ����� 'Base' �������� ��������� ��� 'Derived', ���������� � true
	bool accessible;

	// ���� ����� 'Base' �������� ����������� ��� 'Derived', ���������� � true
	bool virtualDerivation;

	// ���������� ������� ������� 'Base' ��� 'Derived'
	int baseCount;

	// ����� ������������� ����� 'base' �� ��������� � 'derived'
	void Characterize( const ClassType &base, const ClassType &curCls, bool ac );

public:

	// � ������������ �������� ��� ������ � ����� �� �����������
	// �������������� ������ 'Base' �� ��������� � 'Derived'
	DerivationManager( const ClassType &base, const ClassType &derived )
		: accessible(false), virtualDerivation(false), baseCount(0) {

		Characterize( base, derived, true );
	}

	// �������, � ��� ������ ���� ���� � �������� 'base'
	bool IsBase() const {
		return baseCount > 0;
	}

	// �����������, ���� ����� ���� ��� ����� ���������
	bool IsUnambigous() const {
		return baseCount == 1 || virtualDerivation;
	}

	// ����������� ������������
	bool IsVirtual() const {
		return virtualDerivation;
	}

	// �����������
	bool IsAccessible() const {
		return accessible;
	}
};


// �������� ����������� �������-������, ��� ��������� ������
// ������� �-�� �� ���������, �-�� �����������, ����������, �������� �����������
class SMFManager
{
public:
	// ���� - �-�� �� ����� � ���� �������������, ���� �����
	// ������������, ���� ���������� � true
	typedef pair<const Method *, bool> SmfPair;

private:
	// ������ ������������ ������� ������
	SmfPair ctorDef, ctorCopy, dtor, copyOperator;

	// ����� ��� �������� ����������� �����
	const ClassType &pClass;

	// ������-���������, ��� ���������� ������� ����. ������� �����
	// ���� �-�� �� ��������� ������ true
	bool IsDefaultConstructor( const Method &meth, const ClassType &cls ) const {
		const FunctionParametrList &pl = meth.GetFunctionPrototype().GetParametrList();
		return pl.GetFunctionParametrCount() == 0 ||
			   pl.GetFunctionParametr(0)->IsHaveDefaultValue();
	}

	// ���� �-�� �����������, ������ true
	bool IsCopyConstructor( const Method &meth, const ClassType &cls ) const {
		const FunctionParametrList &pl = meth.GetFunctionPrototype().GetParametrList();
		if( pl.IsEmpty() )
			return false;

		const Parametr &prm = *pl.GetFunctionParametr(0);
		if( &prm.GetBaseType() == &cls && prm.GetDerivedTypeList().IsReference() &&
			prm.GetDerivedTypeList().GetDerivedTypeCount() == 1 )		
			return pl.GetFunctionParametrCount() == 1 ||
				pl.GetFunctionParametr(1)->IsHaveDefaultValue();
		return false;		
	}	

	// ���� �������� �����������
	bool IsCopyOperator( const Method &meth, const ClassType &cls ) const {
		const FunctionParametrList &pl = meth.GetFunctionPrototype().GetParametrList();
		if( pl.GetFunctionParametrCount() != 1 )
			return false;

		const Parametr &prm = *pl.GetFunctionParametr(0);	
		return ( &prm.GetBaseType() == &cls && prm.GetDerivedTypeList().IsReference() &&
			prm.GetDerivedTypeList().GetDerivedTypeCount() == 1 );
	}

	// ������� ������ ���� ������� ������� ������
	void FoundSMF();

public:
	// ������ �����
	SMFManager( const ClassType &pcls )
		: pClass(pcls), ctorDef(NULL, false), ctorCopy(NULL, false),
		  dtor(NULL, false), copyOperator(NULL, false) {

		FoundSMF();
	}

	// ������� ����������� �� ���������
	const SmfPair &GetDefaultConstructor() const {
		return ctorDef;
	}

	// ������� ����������� �����������
	const SmfPair &GetCopyConstructor() const {
		return ctorCopy;
	}

	// ������� ����������
	const SmfPair &GetDestructor() const {
		return dtor;
	}

	// ������� �������� �����������
	const SmfPair &GetCopyOperator() const {
		return copyOperator;
	}
};

