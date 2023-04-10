// ��������� � ��������������� ����������� - Parser.h


#ifndef _PARSER_H_INCLUDE
#define _PARSER_H_INCLUDE

// ���������� ���� �������
#include "PackCode.h"


// ��������� �����
class Package;

// ��������� �����
class NodePackage;

// �������� � ExpressionMaker.h
class CtorInitListValidator;

// �������� � Body.h
class Instruction;

// �������� � BodyMaker.h
class ConstructionController;

// �������� � Body.h
class ForConstruction;

// �������� � Body.h
class BodyComponent;

// �������� � Body.h
class CompoundConstruction;

// �������� � Body.h
typedef list< SmartPtr<Instruction> > InstructionList;


// ��������������� �������, ����������� � ���������� ��������������� �����������
namespace ParserUtils
{
	// ������� �������������� ������ ������������ ����� � �������� 'lxm'
	void SyntaxError( const Lexem &lxm );


	// ������� ��������� ������� �� ��� ��� ���� �� �������� ';'
	void IgnoreStreamWhileNotDelim( LexicalAnalyzer &la );

	// ���������, ���� ������� ����� � ����. ��������� � ���������
	// ���������� ���� (�����, ������������), ������� ���, ���������
	// ���������� ������� ��������� �� �������������� ���, ����� NULL
	bool LocalTypeDeclaration( LexicalAnalyzer &la, NodePackage *pkg, const BaseType *&outbt );

	// �������� ������� ������
	Position GetPackagePosition( const Package *pkg );

	// ����������� ��� ������ �������, ������� �����
	CharString PrintPackageTree( const NodePackage  *pkg );
}


// ��������� ��� ������� �������. 
// ������������� - ��� ������������� �������������� ������ � ������ 
// ��� ����������� �������
class Package
{
public:
	// ����������� ����������, ������������ ��� �������� ��������
	virtual ~Package() {
	}

	// ��������� �����
	virtual bool IsLexemPackage() const {
		return false;
	}

	// ������� �����
	virtual bool IsNodePackage() const {
		return false;
	}

	// �����-���������
	virtual bool IsExpressionPackage() const {
		return false;
	}

	// �������� ������������� ������
	virtual int GetPackageID() const = 0;	

	// ������� ���������� ����������
	virtual void Trace() const = 0;
};


// ���������������� ��������� �� �����
typedef SmartPtr<Package> PPackage;

// ��������� �����
class LexemPackage : public Package
{
	// �������
	Lexem lexem;

public:

	// ������ ������������� ������� � ���� ������� 
	LexemPackage( const Lexem &lxm ) : lexem(lxm) {
	}

	// ���������� ���������� �� Package ��� ��������
	~LexemPackage() {
	}

	// ��������� �����
	virtual bool IsLexemPackage() const {
		return true;
	}

	// �������� �������, ��������� � �������
	const Lexem &GetLexem() const {
		return lexem;
	}

	// �������� ������������� ������
	int GetPackageID() const {
		return lexem.GetCode();
	}

	// ������� ���������� ����������
	void Trace() const {
		cout << lexem.GetBuf() << "  ";
	}
};


// ����� �������� ������. ������� ����� ��� ������� �������� �������� �����,
// � ����� ������������ ���������� ���������������, ������� �� ����� ����
// ��������
class NodePackage : public Package
{
	// ������������� �������� ������. ������ ���� �����
	// ���� �������� ��������, ����� ��� �������� ��� ��������� ���
	int packageID;

	// ��������� ����� ������
	vector<Package *> childPackageList;

	// ���������� ������ ������� ��������� ��������
	void ClearChildPackageList() {
		for( vector<Package *>::iterator p = childPackageList.begin();
			 p != childPackageList.end(); p++ )
			 delete *p, *p = NULL;	

		childPackageList.clear();
	}

	// ������������� ������� �������� ��������� �� ����. �� ��������� �������
	// ��������� ��� ������, ��� ���������� ������ � �������� �����������
	// ���������
	friend inline PointerToMember *MakePointerToMember( NodePackage & );

public:
	// ����������� � �������� ���� ������ 
	NodePackage( int pid ) : packageID(pid) {		
	}

	// ���������� ����������� ������ ������� ������
	~NodePackage() {
		ClearChildPackageList();
	}

	// ������� �����
	virtual bool IsNodePackage() const {
		return true;
	}
	
	// ���� ��� �������� �������
	bool IsNoChildPackages() const {
		return childPackageList.empty();
	}

	// ���� �������� ������ ��������
	bool IsErrorChildPackages() const {
		if( IsNoChildPackages() ) 
			return false;
		return GetChildPackage(0)->GetPackageID() == PC_ERROR_CHILD_PACKAGE;
	}

	// ������ ����� ������������� ������, � ������ ���� 
	// ��������������, ��� ����� ����� ������ ������������� ��������
	void SetPackageID( int pid ) {
		packageID = pid;
	}

	// �������� ������������� ������
	int GetPackageID() const {
		return packageID;
	}

	// �������� �������� �����
	const Package *GetChildPackage( int ix ) const {
		INTERNAL_IF( ix < 0 || ix > childPackageList.size()-1 );
		return childPackageList[ix];
	}

	// �������� ��������� �������� �����
	const Package *GetLastChildPackage( ) const {
		INTERNAL_IF( childPackageList.empty() );
		return childPackageList.back();
	}

	// �������� ���������� �������� �������
	int GetChildPackageCount() const {
		return childPackageList.size();
	}

	// ������������ �������� �����. ���������� ���������� 
	// ������������� � ����� ������
	void AddChildPackage( Package *pkg ) {
		
		// ���������� ��������� ������ � ����� � ������� ���������������
		// ����������
		if( !childPackageList.empty() &&
			childPackageList.front()->GetPackageID() == PC_ERROR_CHILD_PACKAGE )
			return;
		
		childPackageList.push_back(pkg);
	}

	// �������� ����� � ������
	void AddChildPackageToFront( Package *pkg ) {
		if( !childPackageList.empty() &&
			childPackageList.front()->GetPackageID() == PC_ERROR_CHILD_PACKAGE )
			return;
		
		childPackageList.insert(childPackageList.begin(), pkg);
	}
	
	// ����� ������ � ��������� pid, ���������� ��� ������ � ������ ��������
	// ������ � -1 � ������ �������.
	// spos - ������� � ������� ������� �������� �����
	int FindPackage( int pid, int spos = 0 ) const {
		
		for( int i = spos; i< childPackageList.size(); i++ )
			if( childPackageList[i]->GetPackageID() == pid )
				return i;
		return -1;
	}
	

	// ������ ��������������� ������, ������ ��������� � ��������� �������,
	// �������������. ��� ��������������� ������� � ������ �������� �����������
	// ����. ��������: ��������� ��� �������� �����, �� �� ����� ���������
	// ���� � ���������� PC_ERROR_CHILD_PACKAGE 
	void BuildError() {		
		ClearChildPackageList();		// ����������� ������ ������� ������
		AddChildPackage( new NodePackage(PC_ERROR_CHILD_PACKAGE) );
	}


	// ������ ���������� ���������� � ������
	void Trace() const {
		cout << "\"" << GetChildPackageCount() << ", " << hex << packageID << "\""
			 << endl;
		
		if( GetChildPackageCount() == 0 )
			return;

		cout << "---------------------\n";
		for( vector<Package *>::const_iterator p = childPackageList.begin();
			 p != childPackageList.end(); p++ )
				(*p)->Trace();

		cout << "\n-------------------\n";
	}
};


// ����� �������������� ������������ �����, ����� ��������������
// ��� �������� ������ �� ����� ������������
class OverflowController
{
	// ������� ������� �����
	static int counter;

public:
	// � ������������ ������� ������������� � ���� ����������
	// ������ ������� - ��������� ������
	OverflowController( int barrier ) {
		if( counter >= barrier )
			theApp.Fatal("������������ ����� �������");
		counter++;
	}

	// ���������� ��������� �������
	~OverflowController() {
		counter--;
	}
};


// ����� ����������� ������� ��������������� �����������
class Parser
{
public:
	// ������������ ����������
	enum LS { LS_CPP, LS_C };

private:
	// ������� ������������ ����������
	LS linkSpec;

	// ����������� ���������� ��� �������, 
	// �������� �����
	LexicalAnalyzer	&lexicalAnalyzer;

	// ������ �������� ����������� �������� ���������. ����
	// ������� ������� ���������� � 0, ������ ������� ��������
	// ������ ����������� namespace, ���� 1, ������ extern "C",
	// ����� extern "C++"
	vector<int> crampControl;

public:

	// ����������� � �������� ������������ �����������
	Parser( LexicalAnalyzer &la ) : lexicalAnalyzer(la), linkSpec(LS_CPP) {		
	}

	// ����� ������� ��������������� �����������
	void Run();

	// �������� ������������ ��������
	LS GetLinkSpecification() const {
		return linkSpec;
	}
	
	// ������ ������� ������������ �������� 
	void SetLinkSpecification( LS ls ) {
		linkSpec = ls;
		crampControl.push_back( linkSpec == LS_C ? 1 : 2 );
	}
};


// ��� - ���������������� ��������� �� �����
typedef SmartPtr<NodePackage> PNodePackage;


// ����� �������������� ����� ��������� ��������������� �����������,
// ������� ������������ ��� ������� ������� ���������� �� �������
// ������ (����� ��� �������)
class DeclareParserImpl
{
	// ����������� ���������� ��� �������, 
	// �������� �����
	LexicalAnalyzer	&lexicalAnalyzer;

public:
	
	// ����������� � �������� ������������ �����������
	DeclareParserImpl( LexicalAnalyzer &la ) : lexicalAnalyzer(la) {		
	}

	// ����� ������� ����������, ���������� �����, �� ��������� ������� 
	// ����� ���������� ��� ������ ������: ��������� ������ �����, 
	// ��������� ������ �������, ���� ����������, �.�. ���������� ����������������. 
	// � ������ ���� ����������, ���������������� ��������� ��� ��������� ������
	void Parse();
};


// ��������� �� ���������
typedef SmartPtr<LexemContainer> PLexemContainer;


// �����, ����� ��� � Parser �������� �����������,
// ��� �� ���� �� ��������� ������ ������������� �������, � �����
// ��������� inline-������� � �����������. �������� ������ ������,
// ��������� ������������, ��� ������� ������ ������
class ClassParserImpl
{
	// ��� - ���� �� ��������� �� ������� � ����������
	typedef pair<Function *, PLexemContainer> FnContainerPair;

	// ����
	LexicalAnalyzer &lexicalAnalyzer;

	// ������ ����������� ��� inline-�������
	list<FnContainerPair> methodBodyList;

	// �����, ��������� ������� �������� ������������ ����� ��� ������ � 
	// ���� ������, ���� ������ ���� ������
	NodePackage &typePkg;

	// ������� ������������ �������
	ClassMember::AS  curAccessSpec;

	// ��������� �� �����, ������� �� ���������
	ClassType *clsType;

	// ������ �������������� ������� ��������� ������, ����
	// ����� ��� �������� � ������ ������� ���������, � ������������ 
	// � �������. ������ ����� ���� ������
	SymbolTableList qualifierList;

	// ���������� � true, ���� ��������� �����������
	bool isUnion;

	// ��������������� � true, ���� ���� ������ ��� ��������������� ������
	bool wasRead;

public:
	// � ����������� �������� ����� �� ������� �����, ���������
	// � ������� ������ ���� �����, � ����� ����������� ����������
	// ������ ���������� �������� ������������ �������, ���� ����� �����������
	// ��� ���� ������� ������
	ClassParserImpl( LexicalAnalyzer &la, NodePackage *np, ClassMember::AS as );

	// ���������� ������ ������ ������
	void Parse();

	// ������� true, ���� ����� ������ � ��������
	bool IsBuilt() const {
		return clsType != NULL;
	}

	// ������� ����������� �����
	const ClassType &GetClassType() const {
		INTERNAL_IF( clsType == NULL );
		return *clsType;
	}

private:

	// ������� ������ ������� ����� � ��������� �� � ������,
	// ����� ��������� �� ������������
	void ReadBaseClassList();	

	// ��������� ����
	void ParseMember( );

	// ������������� ����������� ������� �����, ������� �� ������
	// ���� �������������
	void GenerateSMF();

	// �� ���������� ����������� ������, ��������� inline-�������, �������
	// ��������� � ����������
	void LoadInlineFunctions();

	// ��������� ������������� �������, ��������� � �������� ��������� ������
	void LoadFriendFunctions();
};


// ������ ������������, ��������� ��� ��������� �� '}', ���������
// �� � �������, ��������� ���� ������������
class EnumParserImpl
{
	// ����������
	LexicalAnalyzer &lexicalAnalyzer;

	// �����, ��������� ������� �������� ������������ ����� ��� ������������	
	NodePackage &typePkg;

	// ������� ������������ �������
	ClassMember::AS  curAccessSpec;

	// ��������� �� �����, ������� �� ���������
	EnumType *enumType;

	// ������� ��� ������ ������
	Position errPos;

public:
	// ����������, ����� � �������������, ������������ ������� ���� ��������� ������ ������
	EnumParserImpl( LexicalAnalyzer &la, NodePackage *np, ClassMember::AS as )
		: lexicalAnalyzer(la), typePkg(*np), curAccessSpec(as), enumType(NULL) {
		errPos = ParserUtils::GetPackagePosition(np);
	}

	// ��������� �������� ������ �������
	void Parse();

	// ������� ����������� ������������
	const EnumType &GetEnumType() const {
		INTERNAL_IF( enumType == NULL );
		return *enumType;
	}
};


// ����������� �����, ����� ��� Parser � ClassParserImpl
// ��������� ��������� ���������� ��� ������� ���������� ��������� �
// ���� �������.
class FunctionParserImpl
{
	// ����
	LexicalAnalyzer &lexicalAnalyzer;

	// ���� �������, ������� �������� � �������� �������
	FunctionBody *fnBody;

	// ������� ������ ������������� ������������ � ��������� �����.
	// ��������
	void ReadContructorInitList( CtorInitListValidator &civl );

public:

	// ����������� ��������� ������� � ����������� ���������� � 
	// ������ �������� ��������� ��� ��������������. �������� �������� �����
	// ���� ����� 0
	FunctionParserImpl( LexicalAnalyzer &la, Function &fn );

	// ������� �������������� ������� ���������
	~FunctionParserImpl() {
		// ������� ������� ��������� ���� ���������, ��� ����� ������������� 
		// � ��������� ������
		while( GetCurrentSymbolTable().IsLocalSymbolTable() )
			GetScopeSystem().DestroySymbolTable();

		// ��������� ������ ���� ��������������
		INTERNAL_IF( !GetCurrentSymbolTable().IsFunctionSymbolTable() );
		GetScopeSystem().DestroySymbolTable();
	}

	// ������ ���� �������
	void Parse();
};


// ������ ���� �������
class StatementParserImpl
{
	// ����
	LexicalAnalyzer &la;

	// ���������� �����������
	ConstructionController &controller;

	// ���� �������
	FunctionBody &fnBody;


	// ������� ����
	CompoundConstruction *ParseBlock( );

	// ������� ��������� �������, ������� ���
	BodyComponent *ParseComponent( );

	// ������� � ������� using �����������
	void ReadUsingStatement( );

	// ������� for �����������
	ForConstruction *ReadForConstruction( ConstructionController &cntCtrl );

	// ������� catch-����������
	PTypyziedEntity ReadCatchDeclaration( );

	// ������� ��������� ��� �����������, ������� ���������
	SmartPtr<Operand> ReadExpression( bool noComa = false );
	
	// ������� �������. ����������� �������� ���������� � ��������������
	// ��� ���������. ������������ � if, switch, while, for
	SmartPtr<Instruction> ReadCondition(  );

	// ������� ������ catch-������������
	void ReadCatchList();

	// ������� ��� ����������� ������� ��������
	void MakeLST() {
		GetScopeSystem().MakeNewSymbolTable( new LocalSymbolTable(GetCurrentSymbolTable()) );
	}


	// ������������ ������������� �����
	class OverflowStackController {
		// ������� �����������
		static int deep;
	public:
		// ����������� �������
		OverflowStackController( LexicalAnalyzer &la ) {
			deep ++;
			if( deep == MAX_CONSTRUCTION_DEEP )
				theApp.Fatal(la.LastLexem().GetPos(), 
					"������� ��������� �����������; ������������ �����");
		}

		// ���������
		~OverflowStackController() {
			deep--;
		}
	};

public:
	// ������ �����. ����������
	StatementParserImpl( LexicalAnalyzer &la, 
		ConstructionController &cntrl, FunctionBody &fnBody ) 

		: la(la), controller(cntrl), fnBody(fnBody) {
	}

	// ������� ���� ��� try-����, � ����������� �� �������� �����������
	void Parse();
};


// ������� �����, ���� ���� ������� ����� � TypeExpressionReader,
// ������������ �������������� �������� � ���� �����
class LabelLexem : public Lexem 
{
public:
	LabelLexem( const Lexem &lxm ) : Lexem(lxm) {
	}
};


// ������ ��������� ��������� ����������
class InstructionParserImpl
{
	// ����������� ���������� ��� �������, 
	// �������� �����
	LexicalAnalyzer	&lexicalAnalyzer;

	// ������ ����������� ����������. ���� ���� ���������,
	// ���� ������ ����������, � ������ ������ ������
	InstructionList &insList;

	// ���� ��������� �� ��, ��� ���������� ���������� ������ �����
	bool inBlock;

public:
	
	// ����������� � �������� ������������ �����������. ������ �����
	// ������� ������ ����������, � ������� ����������� ����������
	InstructionParserImpl( LexicalAnalyzer &la, InstructionList &il, bool inBlock = false ) 
		: lexicalAnalyzer(la), insList(il), inBlock(inBlock) {		
	}

	// ����� ������� ����������
	void Parse();

	// �������� ������ �����������
	const InstructionList &GetInstructionList() const {
		return insList;
	}
};


#endif // end  _PARSER_H_INCLUDE
