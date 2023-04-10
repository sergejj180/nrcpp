// ��������� �������-���������� - Maker.h


// ������-��������� ���������� ��� ����, ����� ��������� �������-����������
// ������� ������������ ����� ������ ����� ����������� �����. ������-���������
// �������� ��������� ������ ����� �������������� ������������ � �������������
// ���������. ������-��������� �������� �� ���� ����� ������, ��� �������
// ��� ����� ������, ����������� � ������� ���������� Package. �� ������
// ���������� ������� ������-���������. ������-��������� ��������� ������
// � ������ ��� ����������� ����������.
// �������������� ����������� ����������� � ����� �����, �� ������������
// ����� ������������ ������ �������, ��� �������� ������ � �������.
// ������-����������� ���������� �� �������������� ������� ���, ���
// ������ ����� ������� � ��������� ������ ������� ��������������.
// ����� �������, ���� �������������� ������ ������ �������� ����� �����-���������
// ���������� ������������ ��� ����������� ������ �������. ��������� �������
// ��� ������� ������ ������ ���� ����� ��������� � �����������������


// �������� � Object.h
class Identifier;

// �������� � Object.h
class EnumConstant;

// �������� � Parser.h
class NodePackage;

// �������� � Parser.h
class LexemPackage;

// �������� � Manager.h
class QualifiedNameManager;

// ��������� ����
struct TempObjectContainer;

// ��������� ����
struct TempOverloadOperatorContainer;

// ��������� ����
struct TempCastOperatorContainer;

// �������� � Body.h
class Operand;
typedef SmartPtr<Operand> POperand;
typedef vector<POperand> ExpressionList;


// ������� ������������ ��� ������������� ��������
namespace MakerUtils
{
	// ������� ����� ���������� �� ������ �� ��������� ���������,
	// ������������ ��� ����� ���������� ��� ���������� �������� � �������,
	// ��������� ��������, ��������� ��� ��������� �������������� �����������
	// ������
	bool AnalyzeTypeSpecifierPkg( const NodePackage *typeSpecList, 
			TempObjectContainer *tempObjectContainer, bool defineClassImplicity = true );

	// ������� ����� ���������� �� ������ � ������������
	// �� ��������� ��������� tempObjectContainer
	void AnalyzeDeclaratorPkg( const NodePackage *declarator, 
			TempObjectContainer *tempObjectContainer );

	// ������������� ����� � ������������� ���������� � ���������
	//  ��� �� ��������� ���������
	void AnalyzeOverloadOperatorPkg( const NodePackage &op, 
			TempOverloadOperatorContainer &tooc );


	// ������������� ����� � ���������� ���������� � ���������
	// ��� �� ��������� ���������
	void AnalyzeCastOperatorPkg( const NodePackage &op, TempCastOperatorContainer &tcoc );


	// �������� ������� ��� ���������� �������: 1. ���������� ���� �� ������� ���,
	// � ���� ��� �� �������� ��� �� ���������, 2. ���� ������� ��� ����� ���
	// ������� ���� typedef, ������������� 
	void SpecifyBaseType( TempObjectContainer *tempObjectContainer );

	
	// ��������� � ������� �������������� �������� ������, ����
	// ������� ����� �� ������, ���� �� �� ��������, ������������ 0
	PBaseClassCharacteristic MakeBaseClass( const NodePackage *bc, bool defaultIsPrivate );

	
	// ������� ����� ������� ���������, ���� ��� �� �������, ���������
	// �� � ���� �������� ���������, � ������ ����������� ��������.
	// � ��������� ���������� ����� � ������, ������� ����� ���� NULL,
	// ���� ��������� ���������� ��
	bool MakeNamepsaceDeclRegion( const NodePackage *nn );

	
	// ���������, ������� � �������� � ������� ������� ������� ���������
	// � ���������� ����������� ��� �������� � ��� ������� ��������� ���
	// ������� ������� ���������
	void MakeNamespaceAlias( const NodePackage *al, const NodePackage *ns );

	
	// ������� � ��������� ���������� ��������� ������� ���������
	void MakeUsingNamespace( const NodePackage *ns );


	// ������� ������������� �����
	void MakeFriendClass( const NodePackage *tsl );

	// ������� using-���������� ����
	void MakeUsingMember( const NodePackage *npkg, ClassMember::AS as );

	// ������� using-���������� �� ����
	void MakeUsingNotMember( const NodePackage *npkg );

	// ������� ��������� ������������ � �������� �� � ������� ������� ���������
	EnumConstant *MakeEnumConstant(
		const CharString &name, ClassMember::AS curAccessSpec,
		int lastVal, const Position &errPos, EnumType *enumType);
}


// ��������� ��������� � ������� ������������ ��������� �� ������
// ����������. � �������� �����, ���� ���� �� ������ ��� �������� < 0
struct TempObjectContainer
{
public:
	// ������� 3 ������������� ������������ ����:
	// ���������� ���, ��������� ��� (����� ��� ������������), 
	// ������� ��� typedef. � ���� 3 ������ �������������, ����
	// ����� �������� ��� �������������
	class VariantType
	{
	public:
		virtual ~VariantType() {
		}

		virtual bool IsImplicit() const {
			return false;
		}

		virtual bool IsCompound() const {
			return false;
		}

		virtual bool IsSynonym() const {
			return false;
		}
	};

	// ���������� ���
	class ImplicitType : public VariantType 
	{
	public:
		// ��� �������� ����
		BaseType::BT baseTypeCode;

		// ������������ �������� ����
		BaseType::MSIGN signMod;

		// ����������� �������
		BaseType::MSIZE sizeMod;

		// ���������� ���
		ImplicitType( BaseType::BT bt = BaseType::BT_NONE ) 
			: baseTypeCode(bt), signMod(BaseType::MN_NONE),
			sizeMod(BaseType::MZ_NONE)  {
		}

		bool IsImplicit() const {
			return true;
		}
	};

	// ����� ��� ������������ - ��������� ���
	class CompoundType : public VariantType
	{
		// ��������� �� ������� ��� (����� ��� ������������)
		const BaseType *baseType;

	public:
		// 
		CompoundType( const BaseType &bt ) : baseType(&bt) {
		}

		// �������� ������� ���
		const BaseType *GetBaseType() const {
			return baseType;
		}

		bool IsCompound() const {
			return true;
		}
	};

	// typedef
	class SynonymType : public VariantType
	{
		// ������ ����� �����, ������ ���� ������ ���� typedef
		const ::Object &typedefName;

	public:
		// �� ���� ��������� ����� � ������, �� ������ - ������ �����
		SynonymType( const ::Object &tname ) : typedefName(tname) {			
		}

		// �������� ��� typedef
		const ::Object &GetTypedefName() const {
			return typedefName;
		}

		bool IsSynonym() const {
			return true;
		}
	};

public:
	// ���
	CharString name;

	// ������ ����� �����, � ����� ��� ������� ���������,
	// ����� ��� ���������, ��� ���� ����������� ������������
	// ������� �����
	QualifiedNameManager *nameManager;
	
	// �� ����� ������� ���������� ������������ baseType,
	// �.�. ��������� ����������� 3 ������������� ������� �����.
	// ����� �������, ������� ��� ����������� �� ������ baseType �
	// ������������� � finalType
	union
	{
		// ��������������� ����� ������� ���������� �������� ����:
		// ����������, ���������, typedef
		VariantType *baseType;

		// ������� ��� ����� ���������� ���� ���������
		BaseType *finalType;
	};


	// �������������, ���� ���� ����������� � true
	bool constQual, volatileQual;

	// ������ ����������� �����
	DerivedTypeList dtl;

	// ������������ ��������, ���� �� ����� ����� -1
	int ssCode;

	// ������������ �������, � �������� �� ������ ����������
	int fnSpecCode;

	// ������������ ������
	bool friendSpec;

	// ������������ ���������� ����� �. ���� ����� - true
	bool clinkSpec;

	// ������� �����, ��� ������ ������
	Position errPos;

	// ������� ������������ �������, ���� ���������� �������� ������ ������,
	// �.�. �������� ����� ������
	ClassMember::AS	curAccessSpec;

	// ����������� ��������� �����, ��� ���������� � ��� ����� � 
	// ������������� nameManager
	TempObjectContainer( const NodePackage *qualName, 
			ClassMember::AS cas = ClassMember::NOT_CLASS_MEMBER );

	// ����������� ��� ��������, ������� �� ����� ��������� ���������
	// �����, ���� ����� �� ����� ����� �����
	TempObjectContainer( const Position &ep, const CharString &n, 
		ClassMember::AS cas = ClassMember::NOT_CLASS_MEMBER );

	// ������� ������ ����������� �����, ������� ������� ������
	~TempObjectContainer();
};


// ��������� ��������� ��� ������������� ����������
struct TempOverloadOperatorContainer
{
	// ��� ���������
	int opCode;

	// ��������� ������������� ���������
	CharString opString;

	// ������ ��� �������������� ���������
	CharString opFullName;

	// ������������� ����
	TempOverloadOperatorContainer() : opCode(-1) {
	}
};	


// ��������� ��������� ��� ���������� ����������
struct TempCastOperatorContainer
{
	// ��� ���������
	int opCode;

	// ��������� ������������� ���������
	CharString opString;

	// ������ ��� �������������� ���������
	CharString opFullName;

	// �������������� ��������, ������� ������������� ���������� ���
	PTypyziedEntity castType;

	// ������������� ���� � ����
	TempCastOperatorContainer() : opCode(-1), castType(NULL) {
	}
};


// ��������� ��� ������� ���������� ����������. �������� ������������ ����������� �����
// ��� �������� ����������. ������ ��� ������������� ��������-��������������
class DeclarationMaker
{
public:
	virtual ~DeclarationMaker( ) { }

	// ���� ������������� �� ���������, ������� false
	virtual bool Make() = 0;

	// ���������������� ������, � ������� ����� ���� ������� ������
	virtual void Initialize( const ExpressionList & ) = 0;

	// �������� �������������, ����� ���������� NULL, ����
	// ������������� �� ������
	virtual const Identifier *GetIdentifier() const = 0;
};


// ���������������� ���������
typedef SmartPtr<TempObjectContainer> PTempObjectContainer;


// ��������� ���������� ��������. ��� ����������� ��������� ����������
// ������� ����������� � ���������� ������� ��������� � ����������� ������� ���������
class GlobalObjectMaker : public DeclarationMaker
{
	// ��������� ���������, ������� ������������ ��� ���������� �������
	PTempObjectContainer tempObjectContainer;

	// �������������, ������� ���������� �������
	::Object *targetObject;

	// ��������������� � true, ���� ������ ����������������
	bool redeclared;

	// ���������� � true, ���� ��������� ����������
	bool localDeclaration;

	// ���� ������ ����������������� �������������, ������ ���
	const ConstructorMethod *ictor;

public:
	// � ������������ ������ ��������� ���������,������� �����
	// �������������� ��� ���������� �������
	GlobalObjectMaker( const PTempObjectContainer &toc, bool ld = false ) ;

	// ��������� ������������ �������� ������, ������� ���������� �� ��������� ���������,
	// ������� ������-���������, ���������� ������� �������� �������-���������,
	// �������� ��������� � ������� 
	bool Make();

	// ���������������� ������ ���������� 
	void Initialize( const ExpressionList & );

	// �������� �������������, ����� ���������� NULL, ����
	// ������������� �� ������. ������ ��������� ����� Make
	const Identifier *GetIdentifier() const {
		return targetObject;
	}

	// ������� �����������
	const ConstructorMethod *GetConstructor() const {
		return ictor;
	}
};


// ��������� ���������� �������, �������� ��������� GlobalObjectMaker,
// �� ����������� ����, ��� ������� ������ ���� Function
class GlobalFunctionMaker : public DeclarationMaker
{
	// ��������� ���������, ������� ������������ ��� ���������� �������
	PTempObjectContainer tempObjectContainer;

	// �������, ������� ���������� �������
	Function *targetFn;

	// ��������������� � true, ���� ��� ���������� ���� ������������� ������
	bool errorFlag;

public:
	// � ������������ ������ ��������� ���������,������� �����
	// �������������� ��� ���������� �������
	GlobalFunctionMaker( const PTempObjectContainer &toc ) ;

	// ������� ������� �� ��������� ���������
	bool Make();

	// ������� ������, ��� ������������� ������� ���������
	void Initialize( const ExpressionList &il ) {		
		if( &il != NULL )
			theApp.Error( tempObjectContainer->errPos,
				"������������� ������� �����������" );
	}

	// �������� �������������, ����� ���������� NULL, ����
	// ������������� �� ������. ������ ��������� ����� Make
	const Identifier *GetIdentifier() const {
		return targetFn;
	}
};


// ��������� �������������� ���������
class GlobalOperatorMaker :  public DeclarationMaker
{
	// ��������� ���������, ������� ������������ ��� ���������� ���������
	PTempObjectContainer tempObjectContainer;

	// ��������� � ����������� �� ���������
	TempOverloadOperatorContainer tooc;

	// ��������, ������� ���������� �������
	OverloadOperator *targetOP;

public:
	// � ������������ ������ ��������� ���������,������� �����
	// �������������� ��� ���������� ���������
	GlobalOperatorMaker( const PTempObjectContainer &toc, 
		const TempOverloadOperatorContainer &tc ) ;

	// ������� �������� �� ��������� ���������
	bool Make();

	// ������� ������, ��� ������������� ������� ���������
	void Initialize( const ExpressionList &il ) {		
		if( &il != NULL )
			theApp.Error( tempObjectContainer->errPos,
				"������������� ������������� ���������� �����������" );
	}

	// �������� �������������, ����� ���������� NULL, ����
	// ������������� �� ������. ������ ��������� ����� Make
	const Identifier *GetIdentifier() const {
		return targetOP;
	}
};


// ������� ������������ �� �����
class EnumTypeMaker
{
	// ������ �������������� ����, ��������� �� ������� ������
	// ���� KWENUM � ���, ���� ������ ����
	NodePackage *typePkg;

	// ������������ �������, ���� ����� �������� ������
	ClassMember::AS as;

	// ��������� ������ ���������
	EnumType *resultEnum;

	// ������ �������������� �������������, �� ������ ������ � ������
	// ���� ����� � ������ ������ �������� ����������������� ���
	SymbolTableList stList;

	// ���������� � true, ���� ����� ������������. ���������� ��� 
	// �������� �������, ���� true, �������� �� �����������
	bool defination;

public:
	//  �-��� ��������� ����� � ��������� ��� ������������
	EnumTypeMaker( NodePackage *np, ClassMember::AS a, bool def = false );

	// ������� �����, ���� �� ��� �� ������, � ����� ��������� 
	// ����������� ��� ��������
	EnumType *Make();

	// �������� ������ �������� ���������, �������� �������������� �����
	const class SymbolTableList &GetQualifierList() const {
		return stList;
	}
};


// ������� ����� �� ����� � �����. ���� ����� ��� ���������� 
// ���������� ���. ���� ����� ����������, ������� ��� ���� ��� � ������������
// � ������ �������. ���� ����� �� ����������, ���� ���������� ���
// � ����� ������ - ���������� NULL
class ClassTypeMaker
{
	// ������ �������������� ����, ��������� �� ������� ������
	// ���� ���� ������ � ���, ���� ������ ����
	NodePackage *typePkg;

	// ������������ �������, ���� ����� �������� ������
	ClassMember::AS as;

	// ��������� ������ ���������
	ClassType *resultCls;

	// ������ �������������� ������, �� ������ ������ � ������
	// ���� ����� � ������ ������ �������� ����������������� ���
	SymbolTableList stList;

	// ������� ��������� � ������� ����������� �����
	SymbolTable &destST;

	// ���������� � true, ���� ����� ������������. ���������� ��� 
	// �������� �������, ���� true, �������� �� �����������
	bool defination;

public:
	//  �-��� ��������� ����� � ��������� ��� ������������
	ClassTypeMaker( NodePackage *np, ClassMember::AS a, 
		SymbolTable &d = GetCurrentSymbolTable(), bool def = false );

	// ������� �����, ���� �� ��� �� ������, � ����� ��������� 
	// ����������� ��� ��������
	ClassType *Make();

	// �������� ������ �������� ���������, �������� �������������� �����
	const class SymbolTableList &GetQualifierList() const {
		return stList;
	}


	// ������ ��������
private:

	// ������� ����� �� ����� � �� �����
	void MakeUncompleteClass();

	// ������� ���������� �����
	void MakeUnnamedClass();
};


// ��������� ��������� �������. �������� ������ ������� � �������� � �������� 
// ������ ����������, ������ �������������� ��������
class FunctionPrototypeMaker
{
	// ����������� ������ ���������� �������
	FunctionParametrList parametrList;

	// ������ �������������� ��������
	FunctionThrowTypeList throwTypeList;

	// true - ���� �� ��������� ����� � ���������, �.�.
	// ����� ��������� �������� �� � ����������, � � ���������
	bool noNames;

	// ������������� �����
	const NodePackage &protoPkg;

	// ������� �������� � �������� ��� � ������ �� ������
	void MakeParametr( const NodePackage &pp, int pnum );

	// ������� ��� throw-������������
	void MakeThrowType( const NodePackage &tt );

	// ������� ������ throw-������������
	void MakeThrowSpecification( const NodePackage &ts );

	// ������������� �������
	bool constQual, volatileQual;

	// '...'
	bool ellipse;

	// ���� ������� ����� ���������� �������������� ��������,
	// ���������� � true
	bool canThrow;
		
public:
	// ����������� ��������� ����� � ���� ����
	FunctionPrototypeMaker( const NodePackage &pp, bool nn = false ) ;

	// ����� ��������� �������� ������� �� ������ � ������������ ���
	// � �������� ���������� ������ �������
	FunctionPrototype *Make();
};


// ��������� catch-����������, ���������� ������
class CatchDeclarationMaker
{
	// ������
	const NodePackage &typeSpec, &declPkg;

	// ������� 
	Position errPos;

public:
	// ������ ������
	CatchDeclarationMaker( const NodePackage &ts, const NodePackage &dp, const Position &ep )
		: typeSpec(ts), declPkg(dp), errPos(ep) {
	}

	// ��������� catch-����������, ������� ������
	::Object *Make();
};
