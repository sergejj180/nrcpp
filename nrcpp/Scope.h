// ��������� � ������� �������� - Scope.h


#ifndef _SCOPE_H_INCLUDE
#define _SCOPE_H_INCLUDE


// ����� ����������� � ������ TypyziedEntity.h
class Identifier;

// ������������ � ������ GeneralSymbolTable
class NameSpace;

// �������� � Object.h
class Function;

	
// ������ ���������������. ������� �������� � ���-�������
typedef list<const Identifier *> IdentifierList;

// ������ �� ������� ���������������
typedef list<IdentifierList> ListOfIdentifierList;


// ������������ ��� ������
class IdentifierListFunctor 
{
	// ������� ���
	const CharString &name;

public:
	// ������ ������� �������������
	IdentifierListFunctor( const CharString &nam )
		: name(nam) {
	}

	// �������
	bool operator() ( const IdentifierList &il ) const;		
};


// ���-�������
class HashTab
{
	// ����������� ����������� �������
	ListOfIdentifierList *table;

	// ������ �������
	unsigned int size;

	// ������� ������������, ���������� ������ �� �����
	unsigned Hash( const CharString &key ) const;

public:
	// �������� �������
	HashTab( unsigned htsz );

	// ���������� ���������� �������
	~HashTab();

	// ����� �������
	IdentifierList *Find( const CharString &key ) const;

	// �������� ������� � �������
	unsigned Insert( const Identifier *id );

	// �������� ������� ��������
	void Clear() {
		table->clear();
	}
};


// ��������� � ������� ��������. � ����� ���������,
// ������� ������� ��������� � ������� �������� ������������, 
// ������� ����� ������� ������ ����� ������� ��� ����
// ���������, ������� ������������ ����� ������� ���������
class SymbolTable
{
public:
	// ����� �������. � ��������� ����� ������ �������������� ���
	// �������������� �� ������ ������� ���������, � ����� �� ���������.
	// ���������� true, ���� ���� �� ���� ������ ������
	virtual bool FindSymbol( const nrc::CharString &name, 
		IdentifierList &out ) const = 0;


	// ����� FindSymbol ���������� SymbolTable - ����� ������ � ������
	// ������� ���������, � ����� � ������������ �������� (using). ���
	// �������, � ����������� �������� ���������, ��� �� ������ ���������,
	// ��� ��� ��� ����� �������� ����� ��, ��������� ����������
	// ������ ������ ���� ��. � ���� ����� � ���������� ������� ClassType, 
	// GeneralSymbol � NameSpace �������������� �������������� ����� - FindInScope,
	// ������� ���� ������ ������ � ����� �� � ������� ������. ��� 
	// ��������� �� , ����� FindInScope ���������� ������ FindSymbol
	virtual bool FindInScope( const nrc::CharString &name, IdentifierList &out ) const {
		return FindSymbol(name, out);
	}


	// ������� �������. ������� ������ ������� �������� �� ������, �������
	// � ��������� ������� ��������� ������� ������� ��������� � ��� �� 
	// ������, � � ���������� ������� ��������� ������� ������������� �������
	// ��� ���� ��������� ������. ������� ���� ������� �����������, �������
	// ���������� true, � ��������� ������ false. ���������� ������� 
	// ����������� ����������� ��������
	virtual bool InsertSymbol( Identifier *id ) = 0;


	// ������� ��� �������
	virtual void ClearTable() = 0;

	// ������-����������, 
	// ������� ��������� ��� ���������� ��� ������� ���������.
	// ���� �� ����� ���� � ���������� �������� ���������
	virtual bool IsGlobalSymbolTable() const {
		return false;
	}

	// ���� ������� �������� ���������
	virtual bool IsLocalSymbolTable() const {
		return false;
	}

	// ���� ������� �������� �����������	
	virtual bool IsNamespaceSymbolTable() const {
		return false;
	}

	// ���� ������� �������� ������	
	virtual bool IsClassSymbolTable() const {
		return false;
	}

	// ������� �������� �������
	virtual bool IsFunctionSymbolTable() const {
		return false;
	}
};


// ������ �������� ���������. ������������ � GeneralSymbolTable,
// � ����������� �� ���� ��� �������� using-�������� ���������.
// ���� ��� ������ ��� ���������� ��� ������������ ��
class SymbolTableList
{
	// ������ ���������� �� ������� ���������
	vector<const SymbolTable *> symbolTableList;

public:

	// �����
	bool IsEmpty() const {
		return symbolTableList.empty();
	}

	// �������� ������� ���������
	void AddSymbolTable( const SymbolTable *st ) {
		symbolTableList.push_back(st);
	}

	// ������� ������ ������� ��������� � ������
	void PopFront() {
		if( !symbolTableList.empty() )
			symbolTableList.erase( symbolTableList.begin() );
	}

	// �������� ���������� �������� ��������� � ������
	int GetSymbolTableCount() const {
		return symbolTableList.size();
	}

	// �������� ������� ��������� �� �������,
	// ���� ��� ����������
	const SymbolTable &GetSymbolTable( int ix ) const {
		INTERNAL_IF( ix < 0 || ix > symbolTableList.size()-1 );
		return *symbolTableList[ix];
	}

	// �������� ������� ���������, ���� ��� �� ����������
	// ������������ NULL
	const SymbolTable *operator[]( int ix ) const {
		return (ix < 0 || ix > symbolTableList.size()-1) ? NULL : symbolTableList[ix];
	}

	// ��������� ������� ������� ���������
	int HasSymbolTable( const SymbolTable *ptab ) const {
		int i = 0;
		for( vector<const SymbolTable *>::const_iterator p = symbolTableList.begin(); 
			 p != symbolTableList.end(); p++, i++ )

			// ���� ��������� �� ������� ��������� �����
			if( *p == ptab )
				return i;
		return -1;
	}

	// �������� ������
	void Clear() {
		symbolTableList.clear();
	}
};


// ���������� ������� ���������, � ����� ������� �����
// ��� ����������� ������� ���������
class GeneralSymbolTable : public SymbolTable
{
	// ���� ������� ��������, ������������� � ���� ���-�������,
	// ��������� �����������
	HashTab	*hashTab;

	// ������ ������������ �������� ���������, ����� ���� ������
	SymbolTableList	usingList;

	// ��������� �� ����������� ������� ���������, ��� ����������
	// ����� ����� ����
	const SymbolTable *parent;

	// � ������������ �������� ������ ���-�������, ���������
	// �� ������������ ��. � ������ ���� pp == NULL,
	// ���������, ��� ��������� ���������� ������� ���������, �����
	// ��������� ����������� ��. ��������, �.�. ������ �����������
	// ������ NameSpace � TranslationUnit
	GeneralSymbolTable( unsigned htsz, SymbolTable *pp ) {
		parent = pp == NULL ? this : pp;
		hashTab = new HashTab(htsz);		
	}

	// ����������� ����� ����� ��������� �������
	friend class NameSpace;
	
	// ������ ���������� ������� ���������� ��
	friend class TranslationUnit;

public:
	
	// ���� ������� �������� ����������, �.�. � ��� ��� ������������ �� 
	// � parent ��������� ��� �� ����
	virtual bool IsGlobalSymbolTable() const {
		return parent == this;
	}

	// �������� ��������� �� ������������ ������� ���������
	const SymbolTable &GetParentSymbolTable() const {
		return *parent;
	}

	// �������� using-������� ���������, ������� ����� ��������������
	// ������������� ��� ������
	void AddUsingNamespace( NameSpace *ns );

	// �������� ������ ��������� �������� ���������
	const SymbolTableList &GetUsingList() const {
		return usingList;
	}

	// ����� ������� � ��������� ��� ���������� ������� ���������,
	// ���� � ����� ������� ��������� � ����� � ���������� ����������
	// ���������� ����� � ��������� �������� ��������� (using). ��� ����
	// ������� ��������������, ����� ������� �� ������������ �.�. 2 �������
	// ����� ���� ���������� �� ��������� ���� � �����. ���� �� ���� ��
	// ���� �� ������� - ������������ ����� ������ name � NULL ����������
	virtual bool FindSymbol( const nrc::CharString &name, IdentifierList &out ) const;


	// ���������� ����� ��� ����� using-��������, ������ ���������� (��� ���������) ��
	virtual bool FindInScope( const nrc::CharString &name, IdentifierList &out ) const ;


	// ������� ������� �������
	virtual bool InsertSymbol( Identifier *id ) ;

	// ������� ������, ���������� ���������� ��� ������ � ������ ������������
	// �������� ���������. ������ ����������������� � NameSpace
	bool FindSymbolWithUsing( const CharString &name,
							 SymbolTableList &tested, IdentifierList &out ) const;
	// ������� ��� �������
	virtual void ClearTable() {
		hashTab->Clear();
	}
};


// ������� �������� �������
class FunctionSymbolTable : public SymbolTable
{
	// �������
	const Function &pFunction;
	
	// ������������ ������� ���������
	const SymbolTable &parentST;
	
	// ������ ������������ �������� ���������, ����� ���� ������
	SymbolTableList	usingList;

	// ������ ��������� ���������������
	ListOfIdentifierList localIdList;

	// ������� ������, ���������� ���������� ��� ������ � ������ ������������
	// �������� ���������. ������ ����������������� � NameSpace
	bool FindSymbolWithUsing( const CharString &name,
					   SymbolTableList &tested, IdentifierList &out ) const;
public:

	// � ������������ �������� ��������� �� ������� � ��������� �� 
	// ������������ ������� ���������
	FunctionSymbolTable( const Function &fn, const SymbolTable &p )
		: pFunction(fn), parentST(p) {
	}

	// ������� �������� �������
	bool IsFunctionSymbolTable() const {
		return true;
	}

	// �������� ���� �������
	const Function &GetFunction() const {
		return pFunction;
	}

	// �������� ������������ ������� ���������	
	const SymbolTable &GetParentSymbolTable() const {
		return parentST;
	}

	// �������� using-������� ���������, ������� ����� ��������������
	// ������������� ��� ������
	void AddUsingNamespace( NameSpace *ns );

	// �������� ������ ��������� �������� ���������
	const SymbolTableList &GetUsingList() const {
		return usingList;
	}

	// ����� ������� � �������������� ������� ���������, ����� � ������ ���������� �������
	// ���� � ����� ������� ��������� � ����� � ���������� ����������
	// ���������� ����� � ��������� �������� ��������� (using). 
	virtual bool FindSymbol( const nrc::CharString &name, IdentifierList &out ) const ;


	// ���������� ����� ��� ����� using-��������, ������ ���������� (��� ���������) ��
	virtual bool FindInScope( const nrc::CharString &name, IdentifierList &out ) const ;


	// ������� ������� �������
	virtual bool InsertSymbol( Identifier *id ) ;


	// ������� ��� �������
	virtual void ClearTable();
};


// ������� �������� ��� ����� � ��� ������ �����������
class LocalSymbolTable : public SymbolTable
{
	// ������ ������� ���������������
	ListOfIdentifierList *table;

	// ������������ ������� ���������
	const SymbolTable &parentST;

public:
	// ������ ������������ ��
	LocalSymbolTable( const SymbolTable &pst )
		: table(NULL), parentST(pst) {
	}

	// ����������� ������ ������� �������
	~LocalSymbolTable() {
		delete table;
	}

	// ������� ������������ ��
	const SymbolTable &GetParentSymbolTable() const {
		return parentST;
	}

	// ����� �������	
	bool FindSymbol( const nrc::CharString &name, IdentifierList &out ) const;

	// ������� ������� �������
	bool InsertSymbol( Identifier *id );

	// ������� ��� �������
	void ClearTable() {
		// delete headId;
	}

	// ���������� true, ���� ������� �������� ���������
	bool IsLocalSymbolTable() const {
		return true;
	}
};


// ����������� ������� ���������, ������� ����� �������� � ������� ��������
// ������ ������� ���������
class NameSpace : public Identifier, public GeneralSymbolTable
{	
	// ���� ����������
	bool unnamed;

public:

	// ������ ��������� ����������� ������� ���������
	NameSpace( const nrc::CharString &name, SymbolTable *entry, bool u ) 
		: Identifier(name, entry), 
		GeneralSymbolTable(DEFAULT_NAMESPACE_HASHTAB_SIZE, entry), unnamed(u) {
		 c_name = name.c_str();
	}

	// ���� ����������� ������� ���������, ��� �����
	bool IsUnnamed() const {
		return unnamed;
	}

	// ���� ����������� ������� ���������, ����������� ����
	virtual bool IsNamespaceSymbolTable() const {
		return true;
	}
};


// ������� ������� ���������
class NameSpaceAlias : public Identifier
{
	// ����������� ������� ���������, ��������� ������� ��������
	// ������ �����
	const NameSpace &ns;

public:
	// ����������� ������ ���������
	NameSpaceAlias( const nrc::CharString &name, SymbolTable *entry, const NameSpace &n )
		: Identifier(name, entry), ns(n) {
	}

	// �������� ������ �� ������� ���������
	const NameSpace &GetNameSpace() const {
		return ns;
	}
};

 
// ������� ���������� ��������� ���������
class Scope
{
	// ������ �� ������ ��������, ��������� ��������� �������
	list<SymbolTable *> symbolTableStack;

public:
	// ������� ������� ���������� ��������� ���������, � ���������
	// ��� ��������� ���������� ������� 
	Scope( GeneralSymbolTable *gst ) {
		INTERNAL_IF( !gst || !gst->IsGlobalSymbolTable() );
		symbolTableStack.push_back(gst);
	}

	// �������� ������� ������� ���������
	const SymbolTable *GetCurrentSymbolTable() const {
		return symbolTableStack.back();
	}

	// ������� ������ ������� ���������, �.�. ����������
	const SymbolTable *GetFirstSymbolTable() const {
		return symbolTableStack.front();
	}	

	// �������� �������������� ������� ��������� ���� ��� ��������,
	// ����� ������� NULL
	const FunctionSymbolTable *GetFunctionSymbolTable() const {		
		if( GetCurrentSymbolTable()->IsLocalSymbolTable() ||
			GetCurrentSymbolTable()->IsFunctionSymbolTable() )
			return static_cast<const FunctionSymbolTable*>(&GetFunctionalSymbolTable());
		return NULL;
	}

	// �������� ��������� ���������� ������� ���������
	const SymbolTable &GetGlobalSymbolTable() const ;

	// �������� ��������� �������������� ������� ���������. 
	// ������� ������� ��������� ����������� ������ ���� ���������
	const SymbolTable &GetFunctionalSymbolTable() const;

	// ������� ����� ������� ��������� � ��������� �� � ����
	void MakeNewSymbolTable( SymbolTable *st ) {
		symbolTableStack.push_back(st);
	}

	// ���������� ������� �������� �� �����, ������ ���������
	// �������� ������ ������� ������ ���������� ���������� ������
	void DestroySymbolTable() {
		symbolTableStack.pop_back();
	}

	// �������� ��������� �������� ���������
	void PushSymbolTableList( SymbolTableList &stl ) {
		for( int i = 0; i<stl.GetSymbolTableCount(); i++ )
			symbolTableStack.push_back( (SymbolTable *)stl[i]);
	}

	// �������������
	void PushSymbolTableList( const list<SymbolTable *> &stl ) {
		for( list<SymbolTable *>::const_iterator p = stl.begin(); 
			 p != stl.end(); p++ )
			symbolTableStack.push_back( (SymbolTable *)(*p) );	
	}

	// �������� ����� �� ���� �������� ���������, 
	// ����� ���������� � �����, �.�. � ������� �� � ������������
	// ������ ������������ - ������ ��������������� �������
	// �������� ���. ���� �����. ��� - false
	bool DeepSearch( const CharString &name, IdentifierList &out ) const;
};


// �������� ��� ������� ������ � ��������� ���������
inline Scope &GetScopeSystem() 
{
	return (Scope &)theApp.GetTranslationUnit().GetScopeSystem();
}


// �������� ������� ������� ���������
inline SymbolTable &GetCurrentSymbolTable() 
{
	return 
		*(SymbolTable *)theApp.GetTranslationUnit().GetScopeSystem().GetCurrentSymbolTable();
}

#endif // end  _SCOPE_H_INCLUDE
