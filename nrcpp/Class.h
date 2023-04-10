// ���������� ���������� � �������� ����, ������, ������� - Class.h


#ifndef _CLASS_H_INCLUDE
#define _CLASS_H_INCLUDE


// �������� - ������� ���
// ���� ����� �������� ������ ��� ���� �������,
// ������� ����� ���� �������� ������
class BaseType
{
public:

	// ��������� ���� ������� �����
	enum BT {
		BT_NONE,
		BT_BOOL, BT_CHAR, BT_WCHAR_T, BT_INT, BT_FLOAT, BT_DOUBLE, BT_VOID,
		BT_CLASS, BT_STRUCT, BT_ENUM, BT_UNION,	
	};

	// ��������� ���� ������������� �����
	enum MSIGN	{
		MN_NONE, MN_SIGNED, MN_UNSIGNED, 
	};

	// ��������� ���� ������������� �������
	enum MSIZE	{
		MZ_NONE, MZ_LONG, MZ_SHORT,
	};

private:
	// ��� �������� ����, ��������� ����
	BT baseTypeCode;

	// ����������� ������� (�������� ������ ��� double, int)
	MSIZE sizeModifier;

	// ����������� ����� (�������� ������ ��� char, int)
	MSIGN signModifier;

public:
	
	// � ������������ ������ ���������� ��� ����
	BaseType( BT btc, MSIZE size = MZ_NONE, MSIGN sign = MN_NONE ) 
		: baseTypeCode(btc), sizeModifier(size), signModifier(sign) {				
	}

	// ���� ��� �� ����� ����� (�������� ������ ��� int � char)
	bool IsUnsigned() const {
		return signModifier == MN_UNSIGNED;
	}

	// ���� ��� ����� ���� (�������� ������ ��� int � char)
	bool IsSigned() const {
		return signModifier == MN_SIGNED;
	}

	// ���� ��� ����� ����������� ������� long (�������� ��� int, double)
	bool IsLong() const {
		return sizeModifier == MZ_LONG;
	}

	// ���� ��� ����� ����������� ������� short (�������� ��� int)
	bool IsShort() const {
		return sizeModifier == MZ_SHORT;
	}

	// ���� ������� ��� �������� ����������, �.�. bool, char, int, float, double, void,	
	bool IsBuiltInType() const {		
		return baseTypeCode == BT_INT || baseTypeCode == BT_CHAR ||
			baseTypeCode == BT_FLOAT || baseTypeCode == BT_DOUBLE ||
			baseTypeCode == BT_BOOL || baseTypeCode == BT_VOID;
	}

	// ���� ������� ��� �������� �������
	bool IsClassType() const {			
		return baseTypeCode == BT_CLASS || baseTypeCode == BT_STRUCT || 
			baseTypeCode == BT_UNION;
	}

	// ���� ������� ��� �������� ������������ �����
	bool IsEnumType() const {
		return baseTypeCode == BT_ENUM ;
	}

	// �������� ����������� �������
	MSIZE GetSizeModifier() const {
		return sizeModifier;
	}

	// �������� ����������� �����
	MSIGN GetSignModifier() const {
		return signModifier;
	}

	// �������� ��� �������� ����
	BT GetBaseTypeCode() const {
		return baseTypeCode;
	}

};


// ����� ������������ ����� ������������ ���
class EnumType : public Identifier, public BaseType, public ClassMember
{
	// ������������ �������, ���������� ���� ������������ ���
	// �������� ������ ������
	AS accessSpecifier;

	// ������ ��������
	EnumConstantList constantList;

	// ��� �������� �� �����������
	bool uncomplete;

public:
	// ����������� ������ ��� ������������� ����, ��� ������� ���������,
	// � ������ ��������
	EnumType( const nrc::CharString &name, SymbolTable *entry, AS as ) : 
	  Identifier(name, entry), BaseType(BT_ENUM), accessSpecifier(as)  {

		uncomplete = true;
	}

	// ���� ������������ ��� �� �������� ��������
	bool IsEmpty() const {
		return constantList.IsEmpty();
	}

	// ���� ��� �������� �� �����������
	bool IsUncomplete() const {

		// ��� �������� ����������� ������ ����� ����������
		// ����� Complete � �������� ���������
		return uncomplete;		
	}

	// ���� ��� �������� ������ ������
	virtual bool IsClassMember() const {
		return accessSpecifier != NOT_CLASS_MEMBER;
	}

	// �������� ������������ �������
	virtual AS GetAccessSpecifier() const {
		return accessSpecifier;
	}

	// �������� ������ ��������
	const EnumConstantList &GetConstantList() const {
		return constantList;
	}

	
	// ��������� ���������� ���� ����� ������ �������� ������������
	// ��������� ���������� ����� ���� ����� ���������, �������
	// �������� ������������� ��� ����� ������
	void CompleteCreation( const EnumConstantList &cl ) {
		INTERNAL_IF( !uncomplete );
		constantList = cl;
		uncomplete = false;		// ��� �������� �����������
	}
};


// ��� ������������ - �������������� �������� ������
class BaseClassCharacteristic
{
	// ����������� ������������
	bool virtualDerivation;

	// ������������ ������� ��� ������������
	ClassMember::AS accessSpecifier;

	// ��������� �� �����. 
	const ClassType &pClass;

public:

	// ����������� ������ ������
	BaseClassCharacteristic( bool vd, ClassMember::AS as, const ClassType &p ) 
		: virtualDerivation(vd), accessSpecifier(as), pClass(p) {
	}

	// ���� ����������� ������������
	bool IsVirtualDerivation() const {
		return virtualDerivation;
	}

	// �������� ������������ ��������
	ClassMember::AS GetAccessSpecifier() const {
		return accessSpecifier;
	}

	// �������� ��������� �� �����
	const ClassType &GetPointerToClass() const {
		return pClass;
	}
};


// � ������� ������ �������� ���������������� ��������� 
typedef SmartPtr<BaseClassCharacteristic> PBaseClassCharacteristic;


// ������ ������� �������
class BaseClassList
{
	// ������ ������� �������
	vector<PBaseClassCharacteristic> baseClassList;

public:

	// ���� ������ ����
	bool IsEmpty() const {
		return baseClassList.empty();
	}

	// �������������� �� �������
	const PBaseClassCharacteristic &GetBaseClassCharacteristic( int ix ) const {
		INTERNAL_IF( ix < 0 || ix > GetBaseClassCount()-1 );
		return baseClassList[ix];
	}

	// �������� �������������� �� �������, ���� ��� ����������, ������� 0
	PBaseClassCharacteristic operator[]( int ix ) const {
		return (ix < 0 || ix > GetBaseClassCount()-1) ? 
			PBaseClassCharacteristic(NULL) : baseClassList[ix];
	}

	// ���������� ������� �������
	int GetBaseClassCount() const {
		return baseClassList.size();
	}

	// �������� ��������������, �� ����������� �����, ���������� ���
	// ���������� ������
	void AddBaseClassCharacteristic( const PBaseClassCharacteristic &bcc ) {
		baseClassList.push_back(bcc); 
	}

	// ���������, ������� �� ����� � ������
	int HasBaseClass( const ClassType *cls ) const {
		for( int i = 0; i<baseClassList.size(); i++ )
			if( &baseClassList[i]->GetPointerToClass() == cls )
				return i;

		return -1;
	}

	// �������� ������
	void ClearBaseClassList() {		
		baseClassList.clear();
	}
};


// � ������� ������ �������� ���������������� ��������� 
typedef SmartPtr<ClassMember> PClassMember;


// ������ ������ ������. ������� ������, ��� ����� �������� �������� ���������
// � ����� ��������� ������������� �����, ����� ������� ����� ��������� � �����
// ����:  ListOfIdentifierList
class ClassMemberList
{
	// ������ ������ � ���� ������������� ���������
	mutable ListOfIdentifierList memberList;

	// ������� ���������� ������ ������ ��� ����������,
	// ������������ ��� ���������������� �������
	vector<PClassMember> order;

public:

	// ���������� true, ���� ������ ����
	bool IsEmpty() const {
		return order.empty();
	};

	// �������� ���������� ������
	int GetClassMemberCount() const {
		return order.size() ;
	}	

	// �������� ���� �� �������, ������ �� ������ �������� �� �������
	// ������
	const PClassMember &GetClassMember( int ix ) const {
		INTERNAL_IF( ix < 0 || ix > GetClassMemberCount() - 1 );
		return order[ix];
	}

	// �������� ����, ���� ������ ������� �� ������� - ������� NULL
	PClassMember operator[]( int ix ) const {
		return (ix < 0 || ix > GetClassMemberCount() - 1) ? 
			PClassMember(NULL) : order[ix];
	}

	// �������� ��� ����� � ������ name � ���� ������ �����,
	// ���� ������ ����� ���, ������� ������ ������
	IdentifierList *FindMember( const CharString &name ) const;
 
	// �������� ���� � ������. ��� ���� ���� ������ �������������
	// ������ �� ������, �� ���������, � ���� ������, ���� �����������
	// � ����� ����� ������
	void AddClassMember( PClassMember cm );

	// �������� ������ ������ ������������� ������������� ������
	void ClearMemberList();
};


// ������������ �������� ����. ������ ����� ���� ���� �����,
// ���� �������, ���� ��������� �������� (������� ����� ����� �������� � �������)
class ClassFriend
{
	union
	{
		// ��������� �� ����� ��� ��������� ��������
		ClassType *pClass;

		// ��������� �� �������
		Function *pFunction;
	};

	// �������������� ����, ������ ���� ����
	bool isClass;

public:

	// ������� ����� ���� �����
	ClassFriend( ClassType *p ) : pClass(p), isClass(true) {
	}

	// ������� ����� �������
	ClassFriend( Function *p ) : pFunction(p), isClass(false) {
	}

	// ���� ���� �������� �������
	bool IsClass() const {
		return isClass;
	}

	// �������� �������, �� ������ ��� ������� ��� � ������������ ����
	// ������ �������
	const Function &GetFunction() const {
		INTERNAL_IF( isClass );
		return *pFunction;
	}

	// �������� �����, �� ��� ������� ��� � ������������ ��� �����
	// �����
	const ClassType &GetClass() const {
		INTERNAL_IF( !isClass );
		return *pClass;
	}
};


// ������ ������ ������
class ClassFriendList
{
	// ������ ������
	vector<ClassFriend> friendList;

public:

	// ���������� true, ���� ������ ����
	bool IsEmpty() const {
		return friendList.empty();
	};

	// �������� ���������� ������ ������
	int GetClassFriendCount() const {
		return friendList.size() ;
	}	

	// �������� ����� �� �������, ������ �� ������ �������� �� �������
	// ������
	const ClassFriend &GetClassFriend( int ix ) const {
		INTERNAL_IF( ix < 0 || ix > GetClassFriendCount() - 1 );
		return friendList[ix];
	}

	// �������� �����, ���� ������ ������� �� ������� - ������� NULL
	const ClassFriend *operator[]( int ix ) const {
		return (ix < 0 || ix > GetClassFriendCount() - 1) ? NULL : &friendList[ix];
	}

	// �������� ����� ������. ������ ����� �����, ������
	void AddClassFriend( ClassFriend cf ) {
		friendList.push_back(cf);
	}

	// �������� ������ ������ ������������� ������������� ������
	void ClearMemberList() {
		friendList.clear();
	}

	// ���������� ������ ����� �� ���������, ���� ����� ��� ������� -1
	int FindClassFriend( const Identifier *p ) const; 
};


// ��� - ������ ������������� ������
typedef list<const ConstructorMethod *> ConstructorList;

// ��� - ������ ������������� ����������
typedef list<const ClassCastOverloadOperator *> CastOperatorList;

// ��� - ������ ����������� �������
typedef list<const Method *> VirtualFunctionList;


// �����
class ClassType : public Identifier, public SymbolTable, public BaseType, public ClassMember
{
	// ������������ �������, ����� �������������, ���� ����� ��
	// ������ � ������ �����
	ClassMember::AS	accessSpecifier;

	// ������ ������� �������
	BaseClassList baseClassList;

	// ������ ������ ������
	ClassMemberList memberList;

	// ������ ������ ������
	ClassFriendList friendList;

	// ������ (����������) ������������� ������. �������� ��������,
	// �.�. ��������� ��� ���������� ���� � ��������������� �������
	ConstructorList constructorList;
	
	// ������ ���������� ����������
	CastOperatorList *castOperatorList;

	// ������ ����������� �������
	VirtualFunctionList *virtualFunctionList;

	// ��������� �� ���������� ������
	const Method *destructor;

	// ���� ����� �� ��������� ��������, true
	bool uncomplete;

	// ���������� ����������� ������� ������������ � ������,
	// � ��� ����� � � �������. ���� ���������� ������� = 0,
	// ����� �� �������� �����������
	short abstractMethodCount;

	// ���������� ����������� ������� � ������. �������������
	// �� Method::SetVirtual
	short virtualMethodCount;

	// ���� ����� �����������, �.�. ���� ������� ����� ���
	// ��� ����� �������� ����-�� ���� ����������� �������
	bool polymorphic;

	// ���������� � true, ���� ����� ������� ����������� vm-�������,
	// ��� ����������, ����� � ������ ����������� ����� ����������� �������,
	// ������� ��� ��� � ��������
	bool madeVmTable;

	// ��������� �����, ������� ���������� � ��������������� ������
	friend class ClassParserImpl;

public:

	// ����������� � �������� �������������� ����������, �.�. ������
	// ������ ��������� ��� ��� ����������, � �� �����������
	ClassType( const nrc::CharString &name, SymbolTable *entry, BT bt, AS as );

	// ����������� ������ ������� �������
	~ClassType() {
		delete castOperatorList;
	}

	// ���� ����� ����������� (����� ���� �� ���� ����������� �������)
	bool IsAbstract() const {
		return abstractMethodCount != 0;
	}

	// ���� ����� �����������
	bool IsPolymorphic() const {
		return polymorphic;
	}

	// ���� ����� �����������, �������� ������ ������� �������
	bool IsDerived() const {
		return !baseClassList.IsEmpty();
	}

	// ���� ����� �� ��������� ��������
	bool IsUncomplete() const {
		return uncomplete;
	}

	// ���� ����� ��������� 
	bool IsLocal() const {
		const SymbolTable *st = &GetSymbolTableEntry();
		while(st->IsClassSymbolTable())
			st = &(static_cast<const ClassType &>(GetSymbolTableEntry()).GetSymbolTableEntry());
		
		return st->IsFunctionSymbolTable() || st->IsLocalSymbolTable();
	}

	// ���� ������� �������� ������	
	virtual bool IsClassSymbolTable() const {
		return true;
	}

	// ���� ������ vm-������� (����� ��� ����������� ������� � ��������� ����)
	bool IsMadeVmTable() const {
		return madeVmTable;
	}

	// �������� ������������ �������
	virtual AS GetAccessSpecifier() const {
		return accessSpecifier;
	}


	// ���� ����� �������� ������������ �������
	virtual bool IsClassMember() const {
		return accessSpecifier != NOT_CLASS_MEMBER;
	}

	// �������� ������ ������
	const ClassMemberList &GetMemberList() const {
		return memberList;
	}

	// �������� ������ ������� �������
	const BaseClassList &GetBaseClassList() const {
		return baseClassList;
	}

	// �������� ������ ������ ������
	const ClassFriendList &GetFriendList() const {
		return friendList;
	}

	// ������� ������ �������������, ����� ���������� NULL
	const ConstructorList &GetConstructorList() const {
		return constructorList;
	}

	// �������� ������ ���������� ����������
	const CastOperatorList *GetCastOperatorList() const {
		return castOperatorList;
	}

	// �������� ����������, ����� ���������� NULL
	const Method *GetDestructor() const {
		return destructor;
	}

	// ��������� ���������� ����������� ������� � ������
	void DecreaseAbstractMethods() {
		INTERNAL_IF( !abstractMethodCount );
		abstractMethodCount--;
	}

	// ������ ����� ��� �����������
	void MakeVmTable() {
		polymorphic = true;		
		madeVmTable = true;
	}

	// ������� ���������� ����������� �������
	int GetVirtualFunctionCount() const {
		return virtualMethodCount;
	}

	// ��������� ������� ����������� ������� ������
	void IncVirtualFunctionCount() {
		virtualMethodCount++;
	}

	// �������� ����������� ������� � ������
	void AddVirtualFunction( const Method &vf ) {
		INTERNAL_IF( !vf.IsVirtual() );
		if( !virtualFunctionList )
			virtualFunctionList = new VirtualFunctionList;
		virtualFunctionList->push_back(&vf);
	}

	// ������� ������ ����������� ������� ������
	const VirtualFunctionList &GetVirtualFunctionList() const {
		static VirtualFunctionList vfl;
		return virtualFunctionList ? *virtualFunctionList : vfl;
	}
	
	// �������� ������� �����
	void AddBaseClass( const PBaseClassCharacteristic &bcc );

	// ����� ����� � ������, � ����� � ������� �������, � ������ ��������� ������,
	// ���������� ������ ������, � ��������� ������, ������ ����. 
	// friend-���������� ������� �� �������� � ������� � ������ �� ����������
	bool FindSymbol( const nrc::CharString &name, IdentifierList &out ) const ;


	// ����� ������ ������ ������, ��� ����� ������� �������
	bool FindInScope( const nrc::CharString &name, IdentifierList &out ) const ;

	// ������� ����� � �������
	bool InsertSymbol( Identifier *id ) ;


	// ������� ��� �������
	void ClearTable() ;
};


// �����������
class UnionClassType : public ClassType
{
	// ��������� ������������� �������� ��� �����������
	::Object::SS	storageSpecifier;

	// ���� ����������� ���������, �.�. ��� ����� � ���
	// ���������� �������� ����� ����������� �����������
	bool anonymous;

public:
	// ����������� � �������� �������������� ����������, �.�. ������
	// ������ ��������� ��� ��� ����������, � �� �����������
	UnionClassType( const nrc::CharString &name, SymbolTable *entry, 
		AS as, bool a, ::Object::SS ss ) 
	
		: ClassType(name, entry, BT_UNION, as), anonymous(a), storageSpecifier(ss) {		
	 }

	// ���� ����������� ����������
	bool IsAnonymous() const {
		return anonymous;
	}

	// ���� ����������� �����������
	bool IsStaticUnion() const {
		return storageSpecifier == ::Object::SS_STATIC;
	}

	// �������� ������������ ������� �����������, ������
	// ������������ ����� �������������
	::Object::SS GetStorageSpecifier() const {
		return storageSpecifier;
	}
};


// ����������� ����� �������������� �������� "��������� ��������",
// ��������� �������� ����� ��� �������������, ��������� �������� ����,
// ��������� �������� �� ����, ��������� �������� �������
class TemplateParametr : public Identifier
{
public:
	// ������������� ���������� ���������
	enum TP	{ TP_TYPE, TP_NONTYPE, TP_TEMPLATE };

private:
	// ��� ��������� (������������)
	TP templateParametrType;

public:
	// � ������������ �������� ������������� ���������� ���������
	TemplateParametr( const nrc::CharString &name, SymbolTable *entry, TP tpt ) : 
	  Identifier(name, entry), templateParametrType(tpt) {
	}


	// ����������� �����, ���������� true, ���� ��������� ��������
	// ����� �������� �� ���������
	virtual bool IsHaveDefaultValue() const = 0;

	// ���������� �����, ���������� true, ���� ��������� ��������
	// ��������������. ���������� true ������ ���� ��������� ��������
	// �������� ������ TemplateClassSpecialization
	virtual bool IsSpecialized() const = 0;

	// �������� ������������� ���������� ���������
	TP GetTemplateParametrType() const {
		return templateParametrType;
	}
};

// � ������� ������ �������� ���������������� ��������� 
typedef SmartPtr<TemplateParametr> PTemplateParametr;


// ������ ��������� ����������
class TemplateParametrList
{
	// ������ ��������� ����������
	vector<PTemplateParametr> templateParametrList;

public:
	// ������� true, ���� ������ ����
	bool IsEmpty() const {
		return templateParametrList.empty();
	}

	// �������� ���������� ��������� ���������� ������������
	int GetTemplateParametrCount() const { 
		return templateParametrList.size();
	}

	// �������� ��������� �������� �� �������,
	// ���� �� ����������
	const PTemplateParametr &GetTemplateParametr( int ix ) const {
		INTERNAL_IF( ix < 0 || ix > templateParametrList.size()-1 );
		return templateParametrList[ix];
	}

	// �������� ��������� ��������, ���� ��� �� ����������
	// ������������ NULL
	const PTemplateParametr operator[]( int ix ) const {
		return (ix < 0 || ix > templateParametrList.size()-1) ? NULL : templateParametrList[ix];
	}

	// �������� ��������� �������� � ������
	// ����� �� �����������, ������� ����� ���������� ������ ���
	// ���������� ������
	void AddTemplateParametr( PTemplateParametr tp ) {
		templateParametrList.push_back(tp);
	}

	// �������� ������ � ������������� ������
	void ClearTemplateParametrList() {
		templateParametrList.clear();
	}

	// ����� ���������� ������ ���������� ���������, 
	// ���� � ������ ������� ��������� �������� � ������ name
	int HasTemplateParametr( const nrc::CharString &name ) const {
		int i = 0;
		for( vector<PTemplateParametr>::const_iterator p = templateParametrList.begin(); 
			 p != templateParametrList.end(); p++, i++ )
			if( (*p)->GetName() == name ) 
				return i;
		return -1;
	}
};



// ��������� �������� ����. ������� ����� ������������� ���������� ���������
// ���� ��� �������������
class TemplateTypeParametr : public TemplateParametr
{
	// �������� ��������� �� ���������, ���� �� ������ - ����� 0
	// �������������� ��������, �.�. ����� ���� ������� ��� � 
	// ������������� � ������������
	const TypyziedEntity *defaultValue;
		
public:
	// � ����������� ��������� ����������� �������� ��� �������
	TemplateTypeParametr( const nrc::CharString &name, SymbolTable *entry,
		const TypyziedEntity *dv ) : 
		
		TemplateParametr(name, entry, TP_TYPE), 
		defaultValue(dv) 	
	{			
	}

	// ���� �������� ����� �������� �� ���������		
	virtual bool IsHaveDefaultValue() const {
		return defaultValue != NULL;
	}


	// ���������� �����, ���������� true, ���� ��������� ��������
	// ��������������. 
	virtual bool IsSpecialized() const {
		return false;		// ???
	}
};


// ��������� �������� �������
class TemplateTemplateParametr :  public TemplateParametr
{
public:
	
	// � ����������� ��������� ����������� �������� ��� �������
	TemplateTemplateParametr( const nrc::CharString &name, SymbolTable *entry ) : 
		TemplateParametr(name, entry, TP_TEMPLATE)	{			
	}

	// ���� �������� ����� �������� �� ���������		
	virtual bool IsHaveDefaultValue() const {
		return false;	//defaultValue != NULL;
	}


	// ���������� �����, ���������� true, ���� ��������� ��������
	// ��������������. 
	virtual bool IsSpecialized() const {
		return false;		// ???
	}
};


// �� ������� ��������� ��������. ������������ �������������� ��������
class TemplateNonTypeParametr : public TemplateParametr
{
	// �������� ��������� �� ���������, ���� �� ������ - ����� 0
	const TypyziedEntity *defaultValue;
		
	// ��� ���������
	TypyziedEntity parametrType;

public:
	// � ����������� ��������� ����������� �������� ��� �������
	TemplateNonTypeParametr( const nrc::CharString &name, SymbolTable *entry,
		BaseType *bt, bool cq, bool vq, const DerivedTypeList &dtl, const TypyziedEntity *dv ) : 

		TemplateParametr(name, entry, TP_NONTYPE), 
		parametrType(bt, cq, vq, dtl),
		defaultValue(dv) 	
	{			
	}


	// ���� �������� ����� �������� �� ���������		
	virtual bool IsHaveDefaultValue() const {
		return defaultValue != NULL;
	}

	// ���������� �����, ���������� true, ���� ��������� ��������
	// ��������������. 
	virtual bool IsSpecialized() const {
		return false;		// ???
	}
};


// ��������� �����
class TemplateClassType : public Identifier, public ClassMember, public SymbolTable
{
	// ������������ �������
	AS accessSpecifier;

public:

	// � ����������� ��������� ����������� �������� ��� �������
	TemplateClassType( const nrc::CharString &name, SymbolTable *entry,	AS as ) 
		: Identifier(name, entry), accessSpecifier(as) {
	}
	
};


#endif	// end _CLASS_H_INCLUDE
