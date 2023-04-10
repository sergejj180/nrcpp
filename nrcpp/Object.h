// ���������� ������� �������������, �������������� ��������, 
// ����������� ��� � ����������� �� ��� - Object.h


#ifndef _OBJECT_H_INCLUDE
#define _OBJECT_H_INCLUDE

#include <nrc.h>
using namespace nrc;

// ����� ����������� � SymbolTable.h
class SymbolTable;

// �������� � Class.h
class ClassType;

// �������� � Translator.h
class IdentifierGenerateData;


// ������� �����, ��� ���� ������� ��������� ����������������
// �������, �����, ����������� ������� ���������, �������
// ����� � ������������ ���. 
// ������ ����������� ������� ���������� ���, ����� ������ ���
// ����, ����� ��� ����������� �������� ����� ���� �������
// � ������� ��� ��������� �� ����� �������������
class Identifier
{
	// ��� ��������������
	nrc::CharString name;
	
	// ��������� �� ������� ��������, � ������� �����������
	// ��� ���
	const SymbolTable *pTable;

protected:
	// ��� �������������� ������������ ��� ��������� ����. ��������
	// ����� ������������ ����������� �����
	string c_name;

public:

	// ����������� �� ���������
	Identifier() { 
		pTable = NULL;
	}

	
	// ����������� � �������� ����������
	Identifier( const nrc::CharString &n, const SymbolTable *p )
		: name(n), pTable(p) {		
		
	}

	// ����������� ���������� ��� ����������� �������. ����������
	// ������ ������� ����������� � ���������
	virtual ~Identifier() {
	}

	// �������� �������� ��� ��������������
	const nrc::CharString &GetName() const { 
		return name; 
	}

	// �������� ����������������� ��� ��������������
	nrc::CharString GetQualifiedName() const;

	// �������� ��������� �� ������� ��������, � ������� ���������
	// �������������
	const SymbolTable &GetSymbolTableEntry() const { 
		INTERNAL_IF( pTable == NULL );
		return *pTable;
	}
	

	// �����, ������� ������ ���������� ������
	// ��� ����� ��������������, ������� ����� ��������������
	// ��� ��������� ����
	const string &GetC_Name() const {
		return c_name;
	}
}; 

	
// ������ ���������������. ������� �������� � ���-�������
typedef list<const Identifier *> IdentifierList;

// ������ �� ������� ���������������
typedef list<IdentifierList> ListOfIdentifierList;


// ��������� ��� ������ ������
class ClassMember
{
public:	
	// ����������� ����������, ��������� �.�. ����� ������ ������������
	// ������ ������ ��� ClassMember
	virtual ~ClassMember() {		
	}

	// ��������� ���� �������
	enum AS {
		NOT_CLASS_MEMBER, AS_PRIVATE, AS_PROTECTED, AS_PUBLIC, 
	};

	// �������� ������������ �������
	virtual AS GetAccessSpecifier() const {
		return NOT_CLASS_MEMBER;
	}

	// ����� ����������� ����� ��� ��������������� � ��� �������,
	// ������� ����� ���� ������� ������
	virtual bool IsClassMember() const = 0;
};


// ������������� ����������� � ������� using-���������
class UsingIdentifier : public Identifier, public ClassMember
{
	// ������������ ������� ���� ������������� �������� ������ 
	// ������
	AS accessSpecifier;

	// ��������� �� ������ �������������� ���������� ���������������
	const Identifier *pIdentifier;

public:
	// ����������� ������ ����������� ���������
	UsingIdentifier( const nrc::CharString &n, SymbolTable *p, 
		const Identifier *pid, AS as ) :
	  Identifier(n, p), pIdentifier(pid), accessSpecifier(as) {
	}

	// �������� �� ������������� ������ ������
	virtual bool IsClassMember() const {
		return accessSpecifier != NOT_CLASS_MEMBER;
	}

	// �������� ������� ��������� using-����������,
	// ����� ������ ��� �������
	const SymbolTable &GetDeclarativeRegion() const { 
		return GetSymbolTableEntry();
	}

	// �������� �������������� ����������
	const Identifier &GetUsingIdentifier() const {
		return *pIdentifier;
	}

	// �������� ������������ �������
	virtual AS GetAccessSpecifier() const {
		return accessSpecifier;
	}
};


// ����������������� �������������, � �������� ������������
// ������������� ��������. ������������ ��� ���� ���������� ���������
// T::m. ����� ������ ������, ���� ������������ � ���������� using
class UnspecifiedIdentifier : public Identifier, public ClassMember
{
	// ����� ���� �����
	bool type;

	// ������������ ������� ���� ����
	AS accessSpecifier;

public:
	// ����������� ������ ��������� ��������������
	UnspecifiedIdentifier( const nrc::CharString &n, SymbolTable *p, bool tp,
		AS as ) :
	  Identifier(n, p), type(tp), accessSpecifier(as) {
	}

	// ������������� �������� �������� �����
	bool IsTypename() const {
		return type;
	}

	// �������� �� ������������� ������ ������
	virtual bool IsClassMember() const {
		return accessSpecifier != NOT_CLASS_MEMBER;
	}

	// �������� ������������ �������
	virtual AS GetAccessSpecifier() const {
		return accessSpecifier;
	}
};


// �����
class Label : public Identifier
{
	// ������� � ������� ����������� �����
	Position definPos;

public:
	// ����������� � �������� ����� � ��������� �� ���� �������
	Label( const nrc::CharString &n, SymbolTable *p, const Position &dp ) 
		: Identifier(n,p), definPos(dp) {		
	}

	// �������� ������� � ������� ����������� �����
	const Position &GetDefinPos() const { 
		return definPos;
	}
};


// ������� ����� ��� 5 �������������� ����������� �����
class DerivedType
{
public:

	// ��������� ���� ������������ ����
	enum DT { 
		DT_ARRAY, DT_POINTER, DT_POINTER_TO_MEMBER, DT_REFERENCE, 
		DT_FUNCTION_PROTOTYPE 			
	};

private:
	// ��� ������������ ����
	DT  derivedTypeCode;

public:

	// ����������� � �������� ���� ��� ��� ����
	DerivedType( DT dtc ) {
		derivedTypeCode = dtc;
	}


	// ����������� ����������
	virtual ~DerivedType() { }

	// �������� ��� ������������ ����
	DT GetDerivedTypeCode() const {		
		return derivedTypeCode;
	}

	// ����������� ����� ��������� ������� � ������ ������������ ����
	virtual int GetDerivedTypeSize() const = 0;
};


// ���������������� ��������� �� ����������� ���
typedef SmartPtr<DerivedType> PDerivedType;


// ������ ����������� �����
class DerivedTypeList
{
	// ������ ���������� �� ������� ����������� �� DerivedType
	vector<PDerivedType> derivedTypeList;

public:

	// ������� true, ���� ������ ����
	bool IsEmpty() const {
		return derivedTypeList.empty();
	}

	// ���� ������ ����������� ����� ���������� � �������
	bool IsFunction() const {
		return !derivedTypeList.empty() && 
			derivedTypeList[0]->GetDerivedTypeCode() == DerivedType::DT_FUNCTION_PROTOTYPE;
	}

	// ���� ������ ����������� ����� ���������� � �������
	bool IsArray() const {
		return !derivedTypeList.empty() && 
			derivedTypeList[0]->GetDerivedTypeCode() == DerivedType::DT_ARRAY;
	}

	// ���� ������ ����������� ����� ���������� � ���������
	bool IsPointer() const {
		return !derivedTypeList.empty() && 
			derivedTypeList[0]->GetDerivedTypeCode() == DerivedType::DT_POINTER;
	}

	// ���� ������ ����������� ����� ���������� � ��������� �� ����
	bool IsPointerToMember() const {
		return !derivedTypeList.empty() && 
			derivedTypeList[0]->GetDerivedTypeCode() == DerivedType::DT_POINTER_TO_MEMBER;
	}

	// ���� ������ ����������� ����� ���������� � �������
	bool IsReference() const {
		return !derivedTypeList.empty() && 
			derivedTypeList[0]->GetDerivedTypeCode() == DerivedType::DT_REFERENCE;
	}


	// �������� ���������� ����������� �����
	int GetDerivedTypeCount() const { 
		return derivedTypeList.size();
	}

	// �������� ����������� ��� �� �������,
	// ������ ������� �� �������, ���������� ���������� ������
	const PDerivedType &GetDerivedType( int ix ) const {
		INTERNAL_IF( ix < 0 || ix > derivedTypeList.size()-1 );
		return derivedTypeList[ix];
	}

	// �������� ����������� ��� �� �������, ����
	// ������ ������� �� �������, ������������ 0
	const PDerivedType operator[]( int ix ) const {
		return (ix < 0 || ix > derivedTypeList.size()-1) ? 
			PDerivedType(NULL) : derivedTypeList[ix];
	}

	// �������� ������ ������
	const PDerivedType GetHeadDerivedType() const {
		return this->operator[](0);
	}

	// �������� ����� ������
	const PDerivedType GetTailDerivedType() const {
		return IsEmpty() ? NULL : this->operator[](derivedTypeList.size()-1);
	}

	// �������� ����������� ��� � ������
	void PushHeadDerivedType( PDerivedType dt ) {
		derivedTypeList.insert(derivedTypeList.begin(), dt);
	}

	// ������� ����������� ��� � ������
	void PopHeadDerivedType() {
		derivedTypeList.erase(derivedTypeList.begin());
	}

	// �������� ����������� ��� � ������
	// ����� �� �����������, ������� ����� ���������� ������ ���
	// ���������� ������
	void AddDerivedType( PDerivedType dt ) {
		derivedTypeList.push_back(dt);
	}

	// ������������ ������ ����������� ����� � ��������
	void AddDerivedTypeList( const DerivedTypeList &dtl ) {
		derivedTypeList.insert( derivedTypeList.end(),
			dtl.derivedTypeList.begin(), dtl.derivedTypeList.end() );
	}

	// ������������ ����������� ������ ����� � cv-������������� �������
	// ������������ ����, ������ ���� �� '*', 'ptr-to-member', '()', 
	// ��� ���� ���� ����������� ��� ��������� ���������
	bool AddDerivedTypeListCV( const DerivedTypeList &dtl, bool c, bool v );

	// �������� ������ � ������������� ������
	void ClearDerivedTypeList() {
		derivedTypeList.clear();
	}
};


// ������������� �������� "������". ���� ������ ������� �� �����,
// �� ����� -1
class Array : public DerivedType
{
	// ��������� �� ��������� ����������� ������ �������
	// ���� � ������� �� ����� ������, ��������� == NULL
	int size;

public:
	// �������� �������
	Array(int sz = -1) : DerivedType(DT_ARRAY), size(sz) {		
	}

	
	// ������� true, ���� ������ ������� �� �����
	bool IsUncknownSize() const { 
		return size < 1;
	}

	// �������� ������ �������. �������� ���������� ������, ����
	// ������ �� �����. � ������� ������� ������� ���������, ����� ��
	// ������ ������
	int GetDerivedTypeSize() const {		
		return size;
	}
	
	// �������� ������ �������, ���������������� ������������, 
	// ���  GetDerivedTypeSize, ����� �������� � �������� ���� Array
	int GetArraySize() const {
		return size;
	}

	// ������ ������ ��� �������������
	void SetArraySize( int sz ) {
		size = sz;
	}
};


// ������������� �������� "���������"
class Pointer : public DerivedType
{
	// ������������� ���������
	bool constQualifier, volatileQualifier;

public:

	// ����������� � �������� ��������������
	Pointer( bool cq, bool vq ) 
		: DerivedType(DT_POINTER), constQualifier(cq), volatileQualifier(vq) {
	}

	// �������� �������� ������������ �������������
	bool IsConst() const {
		return constQualifier;
	}

	// �������� �������� ������������� volatile
	bool IsVolatile() const {
		return volatileQualifier;
	}

	// �������� cv-������������ ���������
	int CV_Qualified() const {
		return (int)constQualifier + volatileQualifier;
	}
		
	// �������� ������ ���������, �������������� ����������� �����
	// �������� ������
	int GetDerivedTypeSize() const {
		return DEFAULT_POINTER_SIZE;
	}
};


// ������������� �������� "��������� �� ���� ������"
class PointerToMember : public DerivedType
{
	// ��������� �� ����� � �������� ����������� ���������
	const ClassType *pClass;

	// ������������� ��������� �� ����
	bool constQualifier, volatileQualifier;

public:

	// ����������� � �������� ���������� ��������� �� ����
	PointerToMember( const ClassType *p, bool cq, bool vq ) 
		: DerivedType(DT_POINTER_TO_MEMBER), pClass(p), constQualifier(cq), 
		volatileQualifier(vq) {

		INTERNAL_IF( pClass == NULL );
	}

	// �������� �������� ������������ �������������
	bool IsConst() const {
		return constQualifier;
	}

	// �������� �������� ������������� volatile
	bool IsVolatile() const {
		return volatileQualifier;
	}

	// �������� cv-������������ ���������
	int CV_Qualified() const {
		return (int)constQualifier + volatileQualifier;
	}

	// �������� ��������� �� ����� � �������� ����������� ���������
	const ClassType &GetMemberClassType() const {
		return *pClass;
	}
	
	// �������� ������ ���������, �������������� ����������� �����
	// �������� ������
	int GetDerivedTypeSize() const {
		return DEFAULT_POINTER_TO_MEMBER_SIZE;
	}
};


// ������������� �������� "������"
class Reference : public DerivedType
{
public:

	// ������ ���
	Reference() : DerivedType(DT_REFERENCE) {
	}


	// �������� ������ ������
	int GetDerivedTypeSize() const {
		return DEFAULT_REFERENCE_SIZE;
	}
};


// �������� �����, ��� ��� ��������� ��������������� ���������� 
// �������������� �������� � ��������
class FunctionPrototype;


// ���� ����� ��������� ��� ���������� �������������� ��������,
// ��� ����� ����������� � ������ BaseType.h
class BaseType;


// ����� �������������� ��������, �������� ������� ��� ����
// �������, ������� �������� ���
class TypyziedEntity
{
	// ��������� �� ������� ���. ����� ���� ���������� �����, 
	// �������, �������������, ��������� ���������� ����, ���������
	// ���������� �������
	BaseType *baseType;

	// cv-�������������
	bool constQualifier, volatileQualifier;

	// ������ ����������� ����� �������������� ��������
	DerivedTypeList derivedTypeList;

public:	

	// � ������������ ������ ���������� ��������� ��������������� ��������
	TypyziedEntity( BaseType *bt, bool cq, bool vq, const DerivedTypeList &dtl ) {		
		baseType = bt;
		constQualifier = cq;
		volatileQualifier = vq;
		derivedTypeList = dtl;

		INTERNAL_IF( baseType == NULL );
	}

	// ���������� ����������� ������ ������� ������� ����������� �����
	virtual ~TypyziedEntity() {
		derivedTypeList.ClearDerivedTypeList();
	}

	// �������� �������� ������������ �������������
	bool IsConst() const {
		return constQualifier;
	}

	// �������� �������� ������������� volatile
	bool IsVolatile() const {
		return volatileQualifier;
	}

	// �������� cv-������������ �������������� ��������
	int CV_Qualified() const {
		return (int)constQualifier + volatileQualifier;
	}

	// ������� ����������
	// ---
	// ���� ������ ����� ��� Literal, ������� �������� ����������� �������
	// TypyziedEntity. � ������ Literal ��� ������� ������ ����������������
	// � ���������� true
	virtual bool IsLiteral() const {
		return false;
	}

	// ���� ������ ����� ��� Object
	virtual bool IsObject() const {
		return false;
	}

	// ���� ������ ����� ��� EnumConstant
	virtual bool IsEnumConstant() const {
		return false;
	}

	// ���� ������ ����� ��� Function
	virtual bool IsFunction() const {
		return false;
	}

	// ���� ������ ����� ��� Parametr
	virtual bool IsParametr() const {
		return false;
	}

	// ���� ������������ �������������� ��������
	virtual bool IsDynamicTypyziedEntity() const {
		return false;
	}

	// ���� ������ ����� ��� - �� �������������� ��������� ��������
	virtual bool IsNonTypeTemplateParametr() const {
		return false;
	}

	// �������� ������� ���
	const BaseType &GetBaseType() const {
		return *baseType;
	}

	// �������� ������ ����������� �����
	const DerivedTypeList &GetDerivedTypeList() const {
		return derivedTypeList;
	}

	// �������� ��������� ������������� ����
	CharString GetTypyziedEntityName( bool printName = true ) const;

private:

	// �������� ���������� ����������� ���� � ��������� �� � �����
	void PrintPointer( string &buf, int &ix, bool &namePrint ) const ;

	// �������� ����������� ����������� ���� � ��������� � �����
	void PrintPostfix( string &buf, int &ix ) const;
};


// ����� - ������������ �������������� ��������. ������������
// � ���������� ��� �������� ��� ���������������, ������� �����������
// �������� ���� ���. �������� ���� � ����������� ������ ���������� �����������
class DynamicTypyziedEntity : public TypyziedEntity
{
	// �������������� ��������
	const TypyziedEntity &original;

public:
	// ����������� ����� ��� � TypyziedEntity, ������ ��������� ������ �� ��������������
	// ��������
	DynamicTypyziedEntity( const BaseType &bt, bool cq, bool vq, 
		const DerivedTypeList &dtl, const TypyziedEntity &orig ) 
		: TypyziedEntity((BaseType*)&bt, cq, vq, dtl), original(orig) {		
	}

	// ����������� ��� ��������
	DynamicTypyziedEntity( const TypyziedEntity &cop, const TypyziedEntity &orig )
		: TypyziedEntity(cop),  original(orig) {		
	}

	// �������� ��������
	const TypyziedEntity &GetOriginal() const {
		return original;
	}

	// ���� ������������ �������������� ��������
	bool IsDynamicTypyziedEntity() const {
		return true;
	}
};


// ����� ����������� � ������ Body
class Operand;


// ����� ������������ �������� - �������� �������
class Parametr : public Identifier, public TypyziedEntity
{
	// ��������� ����������� �������� ��������� �� ���������,
	// ����� ���� NULL
	const Operand *defaultValue;

	// �������� ����� ����� ������������ �������� register
	bool registerStorageSpecifier;

public:

	// � ������������ �������� ��������� ������
	Parametr( BaseType *bt, bool cq, bool vq, const DerivedTypeList &dtl, 
		const nrc::CharString &n, SymbolTable *p, const Operand *dv, bool rss ) : 
			Identifier(n, p), TypyziedEntity(bt, cq, vq, dtl) {

		defaultValue = dv;
		registerStorageSpecifier = rss;

		// ������ �-���. ���� ��� ������������� ��� ���������, ������ ������
		// ����� ��������� ��� ����
		c_name = n[0] == '<' ? "" : n.c_str();
	}

	// � ����������� ������������ �������� �� ���������
	~Parametr() ;

	// ���� ������ ����� ��� Parametr
	bool IsParametr() const {
		return true;
	}

	// ����� �� �������� �������� �� ���������. � ����� ������, ����
	// ��������� ������
	bool IsHaveDefaultValue() const {
		return defaultValue != NULL;
	}
	
	// ���� �������� ����� ������������ �������� register
	bool IsHaveRegister() const {
		return registerStorageSpecifier;
	}

	// ���� � ��������� �� ������ ���
	bool IsUnnamed() const {
		return GetName().empty();
	}

	// ���� �������� ���������
	bool IsReferenced() const {
		return GetDerivedTypeList().IsReference();
	}

	// �������� �������� �� ���������
	const Operand *GetDefaultValue() const {
		return defaultValue;
	}

	// ������ �������� �� ��������� � ������ ���������������
	void SetDefaultValue( const Operand *dv ) {
		INTERNAL_IF( defaultValue != NULL );
		defaultValue = dv;
	}
};


// ���������������� ��������� �� �������������� ��������
typedef SmartPtr<TypyziedEntity> PTypyziedEntity;

// ���������������� ��������� �� ��������
typedef SmartPtr<Parametr> PParametr;


// ������ ������������ �������� �������������� ��������
class FunctionThrowTypeList
{
	// ������ ���������� �� ������� ���� TypyziedEntity
	vector<PTypyziedEntity> throwTypeList;

public:

	// ������� true, ���� ������ ����
	bool IsEmpty() const {
		return throwTypeList.empty();
	}

	// �������� ���������� ����������� �����
	int GetThrowTypeCount() const { 
		return throwTypeList.size();
	}

	// �������� ��� �������������� �������� �� �������,
	// ������ ������� �� �������, ���������� ���������� ������
	const PTypyziedEntity &GetThrowType( int ix ) const {
		INTERNAL_IF( ix < 0 || ix > throwTypeList.size()-1 );
		return throwTypeList[ix];
	}

	// �������� ����������� ��� �� �������, ����
	// ������ ������� �� �������, ������������ 0
	const PTypyziedEntity operator[]( int ix ) const {
		return (ix < 0 || ix > throwTypeList.size()-1) ? 
					PTypyziedEntity(NULL) : throwTypeList[ix];
	}

	// �������� ��� � ������
	// ����� �� �����������, ������� ����� ���������� ������ ���
	// ���������� ������
	void AddThrowType( PTypyziedEntity &dt ) {
		throwTypeList.push_back(dt);
	}

	// �������� ������ � ������������� ������
	void ClearThrowTypeList() {
		throwTypeList.clear();
	}
};


// ������ ���������� �������
class FunctionParametrList
{
	// ������ ���������� �� ������� ���� Parametr
	vector<PParametr> parametrList;

public:

	// ������� true, ���� ������ ����
	bool IsEmpty() const {
		return parametrList.empty();
	}

	// �������� ���������� ����������� �����
	int GetFunctionParametrCount() const { 
		return parametrList.size();
	}

	// �������� ��� �������������� �������� �� �������,
	// ������ ������� �� �������, ���������� ���������� ������
	const PParametr &GetFunctionParametr( int ix ) const {
		INTERNAL_IF( ix < 0 || ix > parametrList.size()-1 );
		return parametrList[ix];
	}

	// �������� ����������� ��� �� �������, ����
	// ������ ������� �� �������, ������������ 0
	const PParametr operator[]( int ix ) const {
		return (ix < 0 || ix > parametrList.size()-1) ? PParametr(NULL) : parametrList[ix];
	}

	// �������� ��� � ������
	// ����� �� �����������, ������� ����� ���������� ������ ���
	// ���������� ������
	void AddFunctionParametr( PParametr dt ) {
		parametrList.push_back(dt);
	}

	// �������� ������ � ������������� ������
	void ClearFunctionParametrList() {
		parametrList.clear();
	}

	// ����� ���������� ������ ���������, ���� � ������ ������� �������� � ������ name
	int HasParametr( const nrc::CharString &name ) const {
		int i = 0;
		for( vector<PParametr>::const_iterator p = parametrList.begin(); 
			 p != parametrList.end(); p++, i++ )
			if( (*p)->GetName() == name ) 
				return i;
		return -1;
	}
};


// ������ ������� �� ��������� (�����������)
extern const unsigned int defualt_function_size ;


// ������������� �������� "�������� �������"
class FunctionPrototype : public DerivedType
{
	// ������������� ��������� �������
	bool constQualifier, volatileQualifier;

	// ������ ���������� �������
	FunctionParametrList parametrList;

	// ������ ������������ �������� �������������� ��������
	FunctionThrowTypeList throwTypeList;

	// true, ���� ������� ����� ���������� �������������� ��������
	bool canThrowExceptions;

	// true, ���� �������� ����� '...'
	bool haveEllipse;

public:

	// ����������� � �������� ���������� ���������
	FunctionPrototype( bool cq, bool vq, const FunctionParametrList &fpl, 
		const FunctionThrowTypeList &fttl, bool cte, bool he ) 
		: DerivedType(DT_FUNCTION_PROTOTYPE), constQualifier(cq), volatileQualifier(vq) {

		parametrList = fpl;
		throwTypeList = fttl;
		canThrowExceptions = cte;
		haveEllipse = he;
	}

	
	// �������� �������� ������������ �������������
	bool IsConst() const {
		return constQualifier;
	}

	// �������� �������� ������������� volatile
	bool IsVolatile() const {
		return volatileQualifier;
	}

	// �������� cv-������������ ���������
	int CV_Qualified() const {
		return (int)constQualifier + volatileQualifier;
	}

	// ����� �� ������� '...'
	bool IsHaveEllipse() const {
		return haveEllipse;
	}


	// ����� �� ������� ���������� �������������� ��������
	bool CanThrowExceptions() const {
		return canThrowExceptions;
	}


	// �������� ������ ����������
	const FunctionParametrList &GetParametrList() const {
		return parametrList;
	}

	// �������� ������ �������������� ��������
	const FunctionThrowTypeList &GetThrowTypeList() const {
		return throwTypeList;
	}


	// �������� ������ ��������� �������
	int GetDerivedTypeSize() const {
		return DEFUALT_FUNCTION_SIZE;
	}
};


// ���������� �������: ����������, �����, ������������, ���������
class Literal : public TypyziedEntity
{
	// �������� �������� 
	nrc::CharString value;

public:

	// ����������� � �������� ���������� ��������
	Literal( BaseType *bt, bool cq, bool vq, const DerivedTypeList &dtl, 
		const nrc::CharString &v ) : TypyziedEntity(bt, cq, vq, dtl) {

		value = v;
	}

	// �������� �������� ��������
	const nrc::CharString &GetLiteralValue() const {
		return value;
	}

	// ���� ����� ���������, char, int � ��������������. ���������������,
	// ��� char, wchar_t ��� ��������� � ���� ������ �����
	bool IsIntegerLiteral() const;

	// ���� ������������ 
	bool IsRealLiteral() const;

	// ���� ��������� �������
	bool IsStringLiteral() const;

	// ���� wide-������
	bool IsWideStringLiteral() const;

	// ������ �������� ���������
	virtual bool IsLiteral() const {
		return true;
	}
};


// ����� ������������ �������� ������. �������� �������� ��������
// ����� �++, ��� ������� ���������� ������, ���� �� ���������� 
// ��� ������� � ������� �������������� �������� extern, typedef
class Object : public Identifier, public TypyziedEntity, public ClassMember
{
public:
	// ������������� �������������� �������� ��� ������� � ������� �����
	enum SS {
		SS_AUTO, SS_REGISTER, SS_EXTERN, SS_STATIC, SS_TYPEDEF, 
		SS_BITFIELD, SS_MUTABLE, SS_NONE,
	};

private:

	// ������������ ��������
	SS storageSpecifier;

	// ���������������� �������� � ������ ���� ������������� �����������
	// � ������� ����� ��� ������
	const double  *pInitialValue;

	// ���������� � true, ���� ������������ ���������� � ������� 
	// ������ ��� "extern C"
	bool clinkSpec;

public:

	// ����������� ������ ��������� �������
	Object( const nrc::CharString &name, SymbolTable *entry, BaseType *bt,
		bool cq, bool vq, const DerivedTypeList &dtl, SS ss, bool ls = false );

	// ������ �� ����� �������� ������ ������
	virtual bool IsClassMember() const {
		return false;
	}

	// ���� ������ ����� ��� Object
	virtual bool IsObject() const {
		return true;
	}

	// ������� true, ���� ������ ����� ���������������� ��������
	bool IsHaveInitialValue() const {
		return pInitialValue != NULL;
	}

	// ������� true, ���� ������ ����� ������������ ��������� ����� C
	bool IsCLinkSpecification() const {
		return clinkSpec;
	}

	// �������� ������������ ��������
	SS GetStorageSpecifier() const {
		return storageSpecifier;
	}	

	// �������� ��������� �� ���������������� ��������, �����
	// ���� NULL
	const double *GetObjectInitializer() const {
		return pInitialValue;
	}

	// ������ ������������� �������. ������ �������� ����������,
	// ��� �������� �������� �������� ����
	void SetObjectInitializer( double dv, bool bf = false ) {
		INTERNAL_IF( pInitialValue != NULL );
		pInitialValue = new double(dv);
		if( bf )
			storageSpecifier = SS_BITFIELD;
	}
};


// ����� ������������ �������� - ������-���� ������ 
class DataMember : public ::Object
{
	// ������������ ������� � �������-�����
	AS	accessSpecifier;

public:
	// ������-����
	DataMember( const nrc::CharString &name, SymbolTable *entry, BaseType *bt,
		bool cq, bool vq, const DerivedTypeList &dtl, SS ss, AS as ) 
		: Object(name, entry, bt, cq, vq, dtl, ss, false ), accessSpecifier(as) {
	}
	
	// ���� ������-���� �������� �����������
	bool IsStaticMember() const {
		return GetStorageSpecifier() == SS_STATIC;
	}

	// ���� ������ ���� �������� ������� �����
	bool IsBitField() const {
		return GetStorageSpecifier() == SS_BITFIELD;
	}

	// �������� ������ ������
	bool IsClassMember() const {
		return true;
	}
	
	// �������� ������������ �������
	virtual AS GetAccessSpecifier() const {
		return accessSpecifier;
	}

};


// ����� ����������� � Statement.h
class FunctionBody;


// �������� �������. �������, ��� ������, � �������� � ������
// ������ ����������� ����� ���������� ����������� ��� FUNCTION_PROTOTYPE.
// ������� ����� ��������� ����
class Function : public Identifier, public TypyziedEntity, public ClassMember
{
public:
	// ��������� ������������� �������� �������
	enum SS {
		SS_STATIC, SS_EXTERN, SS_TYPEDEF, SS_NONE
	};

	// ��������� ������� ������ �������
	enum CC {
		CC_PASCAL, CC_CDECL, CC_STDCALL, CC_NON
	};

private:

	// ������������ inline
	bool inlineSpecifier;

	// ������������ �������� �������
	SS storageSpecifier;
	
	// ������� ������. ���� ������� ������ ��� "extern C", ��
	// ������� ������ ������ CC_CDECL
	CC callingConvention;

	// ���������� � true, ���� � ������� ���� ����
	bool isHaveBody;

public:
	// ����������� ������ ����������� ��������� �������
	Function( const nrc::CharString &name, SymbolTable *entry, BaseType *bt,
		bool cq, bool vq, const DerivedTypeList &dtl, bool inl, 
		SS ss, CC cc ) ;

	// ����������� ���������� ��� ����������� �������
	virtual ~Function() {
	}

	// ���� ������� �������������, ������������� ��� ����������� ���������,
	// ������ ���� ������� ��������� ��� inline, ���������� true
	bool IsInline() const {
		return inlineSpecifier;
	}

	// ������� �� ���������� ��������
	bool IsProcedure() const; 

	// ���� ������� ����� ����
	bool IsHaveBody() const {
		return isHaveBody;
	}

	// ���� �������, �� ����� ���� �������� ������������� ����������
	virtual bool IsOverloadOperator() const {
		return false;
	}
		
	// ���� ������� �� ����� ���� �������� ���������
	virtual bool IsTemplate() const {
		return false;
	}

	// ���� ������� �� ����� ���� �������� �������������� ��������� �������
	virtual bool IsTemplateSpecialization() const {
		return false;
	}
	// ����� ��� �������
	virtual bool IsFunction() const {
		return true;
	}

	// �� ���� ������
	virtual bool IsClassMember() const {
		return false;
	}	

	// �������� ������������ �������� �������
	SS GetStorageSpecifier() const {
		return storageSpecifier;
	}

	// �������� ������� ������
	CC GetCallingConvention() const {
		return callingConvention;
	}

	// �������� �������� �������
	const FunctionPrototype &GetFunctionPrototype() const {
		return 
			static_cast<const FunctionPrototype &>(*GetDerivedTypeList().GetHeadDerivedType());				
	}

	// ������ ���� �������, ���� ����� ���������� ������ ���� ���
	void SetFunctionBody( ) {
		INTERNAL_IF( isHaveBody );
		isHaveBody = true;
	}
};


// ����� ������
class Method : public Function
{
public:
	// ��� ���������� ������: ��������� �������������, �������������
	// ������������, ����������� ����� (������������ ��� ������������,
	// �������������, ���������� �����������), � ����������� �����,
	// ������������ ������ ��� ��������� �����������, ������� �� ����� ����
	// ������������
	enum DT { DT_USERDEFINED, DT_IMPLICIT, DT_TRIVIAL, DT_UNAVAIBLE };
	
	// ����� ��������� ���������� ��������� ��� ������������ ������
	// ����� ��������� ������ � 
	class VFData {
		// ������ � ����������� �������
		int vtIndex;
	
		// ����� �������� ����������� �������, ���� ����� ���,
		// ������ �� ����������� �����
		const ClassType &rootVfCls;

		// ����� � C-����� ������� '(���)', ������� ������������
		// ��� ���������� ��������� �� ������� � v-������� ��� ������
		string castBuf;

	public:
		// ������ ���������� �����
		VFData( int vtIndex, const Method &thisMeth );

		// ������� ������
		int GetVTableIndex() const {
			return vtIndex;
		}

		// ������� ����� �������� ����������� �������
		const ClassType &GetRootVfClass() const {
			return rootVfCls;
		}

		// ������� ����� �-���� �������
		const string &GetCastBuf() const {
			return castBuf;
		}
	};

private:
	// ��� ����������
	DT declarationType;

	// ������������ ������� � ������
	AS accessSpecifier;

	// ���� ����� ����������� - true
	bool abstractMethod;

	// ���� ����� ����������� - true
	bool virtualMethod;

	// ���� ����� �������� ������������ - true
	bool destructorMethod;

	// ��������� �� ���������� ��������� ��� ������������ ������
	const VFData *vfData;

public:

	// ������ ��������� ������
	Method( const nrc::CharString &name, SymbolTable *entry, BaseType *bt,
		bool cq, bool vq, const DerivedTypeList &dtl, bool inl, 
		SS ss, CC cc, AS as, bool am, bool vm, bool dm, DT dt);

	// ����������� ������ ������� ����������� ������������ ������
	~Method();

	// ���� ����� �����������
	bool IsStaticMember() const {
		return GetStorageSpecifier() == SS_STATIC;
	}		

	// ���� ����� �����������
	bool IsAbstract() const {
		return abstractMethod;
	}

	// ���� ����� ����������� (�� ����������� � ���� �����������)
	bool IsVirtual() const {
		return virtualMethod;
	}
	
	// ���� ����� �������� ������������
	bool IsDestructor() const {
		return destructorMethod;
	}

	// ���� ����� �������� �������������
	bool IsUserDefined() const {
		return declarationType == DT_USERDEFINED;
	}

	// ���� ����� �����������
	bool IsTrivial() const {
		return declarationType == DT_TRIVIAL;
	}

	// ���� ����� �� ��������
	bool IsUnavaible() const {
		return declarationType == DT_UNAVAIBLE;
	}

	// ���� ����� ������������ ������������, �� ������� �����
	// ��������� (�� �����������)
	bool IsImplicit() const {
		return declarationType == DT_IMPLICIT;
	}

	// ���� ����� �������� �������������. ������� ��������������� � ConstructorMethod
	virtual bool IsConstructor() const {
		return false;
	}

	//  ���� ������
	virtual bool IsClassMember() const {
		return true;
	}

	// �������� ������������ �������
	virtual AS GetAccessSpecifier() const {
		return accessSpecifier;
	}

	// �������� ���������� � ����������� �������. ����� ���� NULL
	const VFData &GetVFData() const {
		INTERNAL_IF( vfData == NULL );
		return *vfData;
	}

	// ������ ������������� ������, �.�. ��� ����� ���������� �� �����
	// ����� ��������������� ������
	void SetVirtual( const Method *rootMeth );

	// ������ ������������� ������, �.�. ������������� ����������
	// ����� ���������� '= 0'
	void SetAbstract( ) {
		abstractMethod = true;
	}
};


// ����������� - �������� �������� Method
class ConstructorMethod : public Method
{
	// ������������ ������ ������
	bool explicitSpecifier;

public:
	// ������ ��������� ������������
	ConstructorMethod( const nrc::CharString &name, SymbolTable *entry, BaseType *bt,
		bool cq, bool vq, const DerivedTypeList &dtl, bool inl, 
		SS ss, CC cc, AS as, bool es, DT dt );

	// ���� ����������� ������� ������ ������
	bool IsExplicit() const {
		return explicitSpecifier;
	}

	// ����� �������� �������������
	virtual bool IsConstructor() const {
		return true;
	}
};


// ������������� �������� �� ���� ������
class OverloadOperator : public Function
{
	// ��� ���������
	int opCode;

	// ��� ���������. � ������ ��������� ���������� ����������� ������
	// ��� ����
	nrc::CharString opName;

public:

	// ������ ��������� ������������
	OverloadOperator( const nrc::CharString &name, SymbolTable *entry, BaseType *bt,
		bool cq, bool vq, const DerivedTypeList &dtl, bool inl, 
		SS ss, CC cc, int opc, const nrc::CharString &opn );

	// ���� �������� �������� ���������� ����������
	bool IsCastOperator() const {
		return opCode == 'c';
	}

	// �������� ��� ���������
	int GetOperatorCode() const {
		return opCode;
	}

	// �������� ��� ���������
	const nrc::CharString &GetOperatorName() const {
		return opName;
	}

	// �������� ������������� ����������
	virtual bool IsOverloadOperator() const {
		return true;
	}
};


// ���� ����������, ������� ������� �� ���������� ������,
enum OverloadOperatorCode
{
	OC_NEW_ARRAY,
	OC_DELETE_ARRAY,
	OC_FUNCTION,
	OC_ARRAY,
	OC_CAST,
};


// ���� ����������, ������� ������������ ��� ��������� ����,
// ��� ������������� ��� �����������, ������� ���� �� �������� � �++
enum GeneratorOperatorCode
{
	GOC_DERIVED_TO_BASE_CONVERSION = 1300,
	GOC_BASE_TO_DERIVED_CONVERSION,
	GOC_REFERENCE_CONVERSION,
};


// ������������� �������� ������
class ClassOverloadOperator : public Method
{
	// ��� ���������
	int opCode;

	// ��� ���������. � ������ ��������� ���������� ����������� ������
	// ��� ����
	nrc::CharString opName;

public:

	// ����������� ������ ��� ��������� ����� �� ��������
	ClassOverloadOperator(
		const nrc::CharString &name, SymbolTable *entry, BaseType *bt,
		bool cq, bool vq, const DerivedTypeList &dtl, bool inl, 
		SS ss, CC cc, AS as, bool am, bool vm, 
		int opc, const nrc::CharString &opn, DT dt );

	// ���� �������� �������� ���������� ����������
	virtual bool IsCastOperator() const {
		return false;
	}

	// �������� ��� ���������
	int GetOperatorCode() const {
		return opCode;
	}

	// �������� ��� ���������
	const nrc::CharString &GetOperatorName() const {
		return opName;
	}

	// ����������� ����������� ������� ��� ���������� ���������������
	virtual bool IsOverloadOperator() const {
		return true;
	}	
};


// ������������� �������� ���������� ����
class ClassCastOverloadOperator : public ClassOverloadOperator
{
	// ��� � �������� �������� ���� ������������� ��������
	TypyziedEntity castType;

public:
	// ����������� ��������� ��� ���������
	ClassCastOverloadOperator( 	const nrc::CharString &name, SymbolTable *entry, BaseType *bt,
		bool cq, bool vq, const DerivedTypeList &dtl, bool inl, 
		SS ss, CC cc, AS as, bool am, bool vm, 
		int opc, const nrc::CharString &opn, const TypyziedEntity &ctp, DT dt );

	// ���� �������� �������� ���������� ����������
	bool IsCastOperator() const {
		return true;
	}

	// �������� ��� ����������
	const TypyziedEntity &GetCastType() const {
		return castType;
	}
};


// ����� ����������� � ������ Class.h
class EnumType;


// ����� �������������� ��������� ������������, ������� �� ��������
// ������ ������
class EnumConstant : public Identifier, public TypyziedEntity, public ClassMember
{
	// �������� ��������� ������������
	int value;

public:

	// ����������� ������ �� ��������� ��������� ������������, �������
	// �� ����������
	EnumConstant( const nrc::CharString &name, SymbolTable *entry, 
		int v, EnumType *pEnumType );

	// ����������� ���������� ��� ����������� �������
	virtual ~EnumConstant() { 
	}


	// ��������� �� �������� ������ ������, ��� ����� ���������� �����
	// ClassEnumConstant
	virtual bool IsClassMember() const {
		return false;
	}

	// ���� ������ ����� ��� EnumConstant
	virtual bool IsEnumConstant() const {
		return true;
	}

	// �������� ����� �������� ���������
	// ������� ����� ��������, ������ ���� �������� �������� ����������
	int GetConstantValue() const {
		return value;
	}

	// �������� ������������ ��� � �������� ����������� ���������
	const EnumType &GetEnumType() const ;
};


// ��������� ������������ ������������� ������
class ClassEnumConstant : public EnumConstant
{
	// ������������ ������� ���������
	AS	accessSpecifier;

public:

	// ������ ��������� ���������, � ����� ������������ �������
	ClassEnumConstant( const nrc::CharString &name, SymbolTable *entry, 
		int val, EnumType *pEnumType, AS as ) : EnumConstant(name, entry, val, pEnumType),
		accessSpecifier(as) {
	}

	// �������� ������ ������
	virtual bool IsClassMember() const {
		return true;
	}

	// �������� ������������ ������
	virtual AS GetAccessSpecifier() const {
		return accessSpecifier;
	}
};


// ���������������� ���������
typedef SmartPtr<EnumConstant> PEnumConstant;

// ������ �������� ������������. ����� ����� ��������� ��� ��������� ������������,
// ��� � ��������� ������������, ������� �������� ������� ������
class EnumConstantList
{
	// ������ ���������� �� ������� ���� EnumConstant ��� ����������� �� ����
	vector<PEnumConstant> enumConstantList;

public:

	// ������� true, ���� ������ ����
	bool IsEmpty() const {
		return enumConstantList.empty();
	}

	// �������� ���������� �������� ������������
	int GetEnumConstantCount() const { 
		return enumConstantList.size();
	}

	// �������� ��������� ������������ �� �������,
	// ���� ��� ����������
	const PEnumConstant &GetEnumConstant( int ix ) const {
		INTERNAL_IF( ix < 0 || ix > enumConstantList.size()-1 );
		return enumConstantList[ix];
	}

	// �������� ��������� ������������, ���� ��� �� ����������
	// ������������ NULL
	PEnumConstant operator[]( int ix ) const {
		return (ix < 0 || ix > enumConstantList.size()-1) ? NULL : enumConstantList[ix];
	}

	// �������� ��������� � ������
	// ����� �� �����������, ������� ����� ���������� ������ ���
	// ���������� ������
	void AddEnumConstant( PEnumConstant dt ) {
		enumConstantList.push_back(dt);
	}

	// �������� ������ � ������������� ������
	void ClearEnumConstantList() {
		enumConstantList.clear();
	}

	// ����� ���������� ������ ��������� ������������, 
	// ���� � ������ ������� ��������� ������������ � ������ name
	int HasEnumConstant( const nrc::CharString &name ) const {
		int i = 0;
		for( vector<PEnumConstant>::const_iterator p = enumConstantList.begin(); 
			 p != enumConstantList.end(); p++, i++ )
			if( (*p)->GetName() == name ) 
				return i;
		return -1;
	}
};



#endif // end  _OBJECT_H_INCLUDE
