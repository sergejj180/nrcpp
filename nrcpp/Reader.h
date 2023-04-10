// ��������� ��� ��������������� ������� ������� - Reader.h


// �������� � ExpressionMaker.h
class ListInitComponent;


// ���� ����������
enum DeclarationVariant
{
	DV_GLOBAL_DECLARATION,
	DV_PARAMETR ,
	DV_CAST_OPERATION,
	DV_LOCAL_DECLARATION,
	DV_CLASS_MEMBER,
	DV_CAST_OPERATOR_DECLARATION,
	DV_NEW_EXPRESSION = DV_CAST_OPERATOR_DECLARATION,
	DV_CATCH_DECLARATION = DV_PARAMETR,
};


// ���������� �����������, ������� � ���� ������� ����� �������� �����������.
// ������������ ��� ���������� ���������� ����������, ���������� �������,
// ���������� �������, �������� ���������� ����, ��������� ����������, ������ ������.
class DeclaratorReader
{
	// ��� ����������
	DeclarationVariant	declType;

	// ��������� ������, � ������� ����������� ��� ��������� �������
	LexemContainer	lexemContainer;

	// ��������� ������, ������� ������ ��� ���������� ������ � ��������
	// ���������� � ������������ ����������� ������ ������ �� ��������� ������
	LexemContainer	undoContainer;

	// �������� ��������� �������, ������� ������ �� ���������� ��� �� �����
	Lexem lastLxm;

	// �������������� ����� �� ��������������� ���� � � ������������,
	// ������ ���������� ����� ���������, ���� ���������� ��������
	// ��������� ������������
	PNodePackage tslPkg, declPkg;

	// ����������� ����������
	LexicalAnalyzer &lexicalAnalyzer;

	// ������ ���������������, � ������ ���� �� ����, �� ��������� NULL
	PExpressionList initializatorList;

	// ����, ������� ���������, ������� �� ��� ���������� �������
	// �� ����������
	bool canUndo;

	// ��������� ���������� ���������, � ������ ���� ������ �������
	// ���������� '::', � ����� ��� ����
	bool canBeExpression;

	// ��������������� � true, ���� ���� ������� ���
	bool nameRead;

	// ������� ������� ������, ��������� ��� �������������,
	// ���� ������� �������, ����� �������� ���������� �������������� �����
	// ����������� ������� � ������� �������������
	int crampLevel;

public:
	// ����������� ��������� ������ ��� ����������
	DeclaratorReader( DeclarationVariant dt, LexicalAnalyzer &l, bool cbe = false ) 
		: declType(dt), lexicalAnalyzer(l), tslPkg(new NodePackage(PC_TYPE_SPECIFIER_LIST)), 
		canBeExpression(cbe), declPkg(NULL), crampLevel(0), 
		nameRead(false), initializatorList(NULL) {

		canUndo = false;
	}

	// ��������� ��������� ��� ��������� �� ������ ������, ������������
	// ��� �������� ������ ��������� ������ �������
	void LoadToUndoContainer( const LexemContainer &lc ) {
		undoContainer.insert(undoContainer.end(), lc.begin(), lc.end());
		canUndo = true;		
	}

	// �������� ��������� ��������� ������ ������� ������ � ���
	// ���������
	void LoadToLexemContainer( const LexemContainer &lc ) {
		lexemContainer.insert(lexemContainer.end(), lc.begin(), lc.end());
	}

	// ������� ������������� ����
	void ReadTypeSpecifierList();


	// ������� ��������� ����������, ������������ � ������, ����� �
	// ���������� ����� ���� ��������� ������������. ����� ����������
	// �������������� �������� ���� ���� �������������� ������
	PNodePackage ReadNextDeclarator();

	
	// ������� ����� � �����
	PNodePackage GetTypeSpecPackage() const {
		return tslPkg;
	}

	// �������� ��������� ������
	const LexemContainer &GetLexemContainer() const {
		return lexemContainer;
	}

	// �������� ��������� ������
	const LexemContainer &GetUndoContainer() const {
		return undoContainer;
	}

	// �������� ������ ���������������, ����� ���� NULL
	const PExpressionList &GetInitializatorList() const {
		return initializatorList;
	}

	// ������ ���������� ������ ����������� ��� ������� ������	
private:

	// ��������� ����������
	void ReadDeclarator( );


	// ������� �������� �������
	PNodePackage ReadFunctionPrototype();

	// ��������� ��������� ����� �����������, ���� �� ���������� ����
	void ReadDeclaratorTailPart();

	// ������� ������������������ �� cv-��������������, � ��������� �� 
	// � ������
	void ReadCVQualifierSequence( PNodePackage &np );

	// ������� ��� � throw-������������ � ������� ���
	PNodePackage ReadThrowSpecType();

	// ������� ������ �������������� ����, ��� 
	// �������� ��������� �������, ��������� �� � ����������
	const Lexem &NextLexem();


	// ������� ���������� ������� � �����
	void BackLexem();
};


// ����� ������������ ��� ���������� �����������, ������� ����� ���� 
// ������������������ � ������� ������ �������� �������� ���������. 
// ��� ����� ���� ������� ���, ������, ������������� ��������, ����������, 
// ��������� �� ����. ����� ����� �������������� ��� ���������� � 
// ������������������� ����
class QualifiedConstructionReader
{
	// ����
	LexicalAnalyzer &lexicalAnalyzer;

	// ���������, ���� ����������� ��� ��������� �����
	LexemContainer lexemContainer;

	// ��������� ������, ������� ������ ��� ���������� ������ � ��������
	// ���������� � ������������ ����������� ������ ������ �� ��������� ������
	PLexemContainer	undoContainer;

	// �������� ��������� �������, ������� ������ �� ���������� ��� �� �����
	Lexem lastLxm;

	// ����� �� ��������� ������������
	PNodePackage resultPackage;

	// ����, ������� ���������, ������� �� ��� ���������� �������
	// �� ����������
	bool canUndo;

	// ���� ���������� ����������������� �����������, ���� true,
	// ���������� ������ ��������� ����������� � ��������� �� ����
	// � ��������� ������ - ������������� ������
	bool noQualified;

	// ���� ���������� ���������� � ������������, ���� true,
	// ����������� ������ ����� � ��������� �� ����
	bool noSpecial;

	// � ������ ���������� �����������, ������� ���������� � '::',
	// ����� ���� ��������� '::new' ��� '::delete'. ����� ���� ����
	// ���������� � ���������� ������� new ��� delete ����� ������� '::',
	// �������������� ������ �� ���������
	bool noErrorOnExp;

	// ���� ���������������, � ������ ���� ���� ������� ���������
	// '::new' ��� '::delete'
	bool readExpression;

public:
	// ������ ��������� ������
	QualifiedConstructionReader( LexicalAnalyzer &la, 
		bool noq = false, bool nos = false,
		const PLexemContainer &puc = PLexemContainer(0), bool neoe = false ) 

		: noQualified(noq), noSpecial(nos), lexicalAnalyzer(la),
		resultPackage(new NodePackage(PC_QUALIFIED_NAME)),
		undoContainer(puc), noErrorOnExp(neoe), readExpression(false)  {

		canUndo = !undoContainer.IsNull();
		if( undoContainer.IsNull() )
			undoContainer = new LexemContainer;
	}

	// �������� ��������� ������
	const LexemContainer &GetLexemContainer() const {
		return lexemContainer;
	}

	// �������� ��������� ������
	const LexemContainer &GetUndoContainer() const {
		return *undoContainer;
	}

	// ������� ����������������� �����������
	PNodePackage ReadQualifiedConstruction();

	// ��������� ��������� ��� ��������� �� ������ ������, ������������
	// ��� �������� ������ ��������� ������ �������
	void LoadToUndoContainer( const LexemContainer &lc ) {
		if( undoContainer.IsNull() )
			undoContainer = new LexemContainer;

		INTERNAL_IF( !undoContainer->empty() );
		undoContainer->insert(undoContainer->end(), lc.begin(), lc.end());
		canUndo = true;		
	}


	// ���� ����������� �����������������
	bool IsQualified() const {
		// ���� ������ ��� ���������� ��� ��������, ���� ������ ���, ������!
		return resultPackage->GetChildPackageCount() > 1;	
	}

	// ���� ����������� ���������
	bool IsSimple() const {
		return !IsQualified();
	}

	// ���� ������ ��������� �� ����
	bool IsPointerToMember() const {
		return resultPackage->GetPackageID() == PC_POINTER_TO_MEMBER;
	}

	// ���� ������ ��� - �����, ������������, typedef, ������������������ ������
	bool IsTypeName() const;

	// ���� ������� ������� ���, � �� �������� ��� ����������
	bool IsIdentifier() const {
		return resultPackage->GetChildPackage(
			resultPackage->GetChildPackageCount()-1)->GetPackageID() == NAME;
	}

	// ���� ���� ������� ��������� '::new' ��� '::delete'
	bool IsExpression() const {
		return readExpression;
	}

private:
	// ������� ������������� ��������
	PNodePackage ReadOverloadOperator( );

	// �������� ��������� ������� � ��������� �� � ���������
	const Lexem &NextLexem();


	// ������� ������� � �����
	void BackLexem() ;
};


// ��������� ����������� ������� � '{' � �� ��������������� �� '}'. � ���������
// ����� ������ ���������� ��������� ������ � ����������, ���� ������ ������������� ������.
class CompoundStatementReader
{
	// ���� true, ������� � ���������� �� ����������� � �����
	// ������ ������������ �� { �� }
	bool ignoreStream;

	// ���������, � ������� ����������� ��������� �������, ����
	// ignoreStream == false
	LexemContainer lexemContainer;

	// ����
	LexicalAnalyzer &lexicalAnalyzer;	

public:
	// ������ ���������
	CompoundStatementReader( LexicalAnalyzer &la, bool ignore ) 
		: lexicalAnalyzer(la), ignoreStream(ignore) {

		INTERNAL_IF( lexicalAnalyzer.LastLexem() != '{' );
	}

	// ������� ���������� ����������� �� { �� �����. �� }
	void Read();

	// �������� ���������
	const LexemContainer &GetLexemContainer() const {
		return lexemContainer;
	}
};


// ���������� ���� �������. ������ �� ������ CompoundStatementReader,
// � ��� ���� ��������, ��� �������������� try-����� � ������ 
// ��������������� � ������������
class FunctionBodyReader
{
	// ������������ �����, �� ��������� ������� � ����������
	bool ignoreStream;

	// ��������� � ������� ����� ����������� �������
	PLexemContainer lexemContainer;

	// ����
	LexicalAnalyzer &lexicalAnalyzer;

	
	// �������� �������, ������� ��������� try-����
	void ReadTryBlock();

	// �������� ������� ��������� ������ �������������
	void ReadInitList();

public:

	// ������ ���������
	FunctionBodyReader( LexicalAnalyzer &la, bool ignore ) 
		: lexicalAnalyzer(la), ignoreStream(ignore), 
		lexemContainer( ignore ? NULL : new LexemContainer ) {

	}

	// ����� ���������� ����
	void Read();

	// �������� ���������
	PLexemContainer GetLexemContainer() const {
		return lexemContainer;
	}
};


// ������� ������ �������������
class InitializationListReader
{
	// ����������
	LexicalAnalyzer &lexicalAnalyzer;

	// �������������� ������, ������� ��������
	ListInitComponent *listInitComponent;
	
	// ��������� ������, ���������� ��������� �� ��������������.
	// ����������� �-���
	void ReadList( ListInitComponent *lic );

public:
	// ������ ����������. ��������� ����������� �������� ������ ���� '{'
	InitializationListReader( LexicalAnalyzer &la ) 
		: lexicalAnalyzer(la), listInitComponent(NULL) {
	}

	// ������� 
	void Read();

	// �������� �������������� ������
	const ListInitComponent *GetListInitComponent() const {
		return listInitComponent;
	}
};


// ��������� ����������, ���� ��������������, ��� ���������� �����������,
// ��������� ���������. ������������ � size, typeid, ���������� �������,
// ��������� �����������, ������������ if, while, switch 
class TypeExpressionReader
{
	// ����� ��� ���������
	bool noComa, noGT, ignore;

	// ���� ���� ����������, 
	// ����.
	LexicalAnalyzer &lexicalAnalyzer;

	// ��������� 
	PLexemContainer lexemContainer;

	// �������������� ����� � ����� PC_DECLARATION, ���� ���� �������
	// ���������� ��� � ����� PC_EXPRESSION, ���� ������� ���������
	Package *resultPkg;

	// ������ ���������������, � ������ ���� �� ����, �� ��������� NULL.
	// ���������� �� DeclaratorReader'�
	PExpressionList initializatorList;

	// � ������, ���� ���� ������� ���������� ������, ��������� ���
	const ClassType *redClass;
	
public:

	// ����������� ������ ���� ������
	TypeExpressionReader( LexicalAnalyzer &la, 
		bool nc = false, bool ng = false, bool ign = true )
		: lexicalAnalyzer(la), noComa(nc), noGT(ng), ignore(ign), 
		lexemContainer(0), resultPkg(NULL), initializatorList(NULL), redClass(NULL) {
	}


	// �������� ���������
	PLexemContainer GetLexemContainer() const {
		return lexemContainer;
	}

	// �������� ��������� �����, ����� ���� NULL
	const ClassType *GetRedClass() const {
		return redClass;
	}
	
	// ������� �������������� �����
	const Package *GetResultPackage() const {
		return resultPkg;
	}

	// �������� ������ ���������������, ����� ���� NULL
	const PExpressionList &GetInitializatorList() const {
		return initializatorList;
	}

	// ������� ����������, ���� ���������� ����������� ������������,
	// ������� ���������. �������� ���������� � true, ���� ���������
	// ���� ������ ���������������, ���� �������� ������� � ���������
	void Read( bool fnParam = false, bool readTypeDecl = true );
};


// ����� ���������
class ExpressionReader
{
	// ����������� ����������
	LexicalAnalyzer &lexicalAnalyzer;

	// ����� ���� ����� ��������� �� ���������, �� �������� ������� 
	// ����������� ����������. � ��������� ������ ����� 0
	LexemContainer *undoContainer;

	// �� ������� ������ ��������� ��������� ����� �������. ��������
	// � true, � ���������� �������, �������, ��������������.
	bool noComa;

	// ��������� ������������� '>' �� ������� ������. ������������
	// ��� ���������� ���������� �������
	bool noGT;

	// ���� ���������� � true, ������ ���������� ������� ������ ��
	// ����������. ��� ���� ��������� �� ����� ���� ������ ��� NULL
	bool canUndo;

	// ������� ������� ����������� ���������. ���� ��������� ������� �
	// ������, ������� ������������� �� 1
	int level;

	// ���� ���������� � true, ������ ������� �� ����������� � ����������,
	// ����� ��������� ��������� � ������ ��������� ������� ������������ � ����.
	// ��������������� � false, ������ ��� ���������� ��������� ����������
	bool ignore;

	// ��������� � ������� ����� ����������� �������
	PLexemContainer lexemContainer;

	// �������, � ������� ��������� ��� ���������
	POperand resultOperand;

	// ���� true, ������ undoContainer ������� �������
	bool deleteUndoContainer;

public:

	// � ������������ �������� �����. ���������
	ExpressionReader( LexicalAnalyzer &la, LexemContainer *uc = NULL,
		bool nc = false, bool ngt = false, bool ig = true )
		: lexicalAnalyzer(la), undoContainer(uc), noComa(nc), noGT(ngt), level(0), ignore(ig), 
		canUndo(uc != NULL), lexemContainer( ignore ? NULL : new LexemContainer ),
		resultOperand(NULL), deleteUndoContainer(false) {
		
		if( undoContainer == NULL )
			undoContainer = new LexemContainer, deleteUndoContainer = true;
	}

	// � ����������� ����� ���������� ���������
	~ExpressionReader() {
		if( deleteUndoContainer )
			delete undoContainer;
	}



	// ������� ���������
	void Read();

	// �������� ���������
	PLexemContainer GetLexemContainer() const {
		return lexemContainer;
	}

	// ������� ��������� ���������� ���������
	const POperand &GetResultOperand() const {
		return resultOperand;
	}
		
private:

	// ��������� ��������� �������
	Lexem lastLxm;

	// ���������� ��������� ������� �� ������, ���� �� ����������, ���� ��� �� ����
	Lexem NextLexem();

	// ���������� � ����� ��� � ��������� ��������� ��������� �������
	void BackLexem();

	// ������� � undoContainer �������. ��� ���� ���� ����� ��������������
	// �� �������, �� ����������
	void UndoLexem( const Lexem &lxm );

	// ������������ ��������� ������� � undoContainer � ������������� � ����� 
	// ���������� �� ����������
	void UndoLexemS( const LexemContainer &lc ) {
		INTERNAL_IF( !undoContainer->empty() );
		undoContainer->insert( undoContainer->end(), lc.begin(), lc.end() );
		canUndo = true;
	}

	// ������� ����������� ����������, ������� ����� � �����. ���� 
	// noError - true, ������ ��������� ������� ����������� � undoContainer
	PNodePackage ReadAbstractDeclarator( bool noError, 
		DeclarationVariant dv = DV_CAST_OPERATION );

	// ������� ������ ���������, ������� ������ � ����������� ����,
	// �������� ���������� ���������� �������� ������� ')'. 
	// �������������� ������ ����� ���� ������
	PExpressionList ReadExpressionList( );

	// ��������� �������������� �������, �� �� ������ ���� �����, 
	// ������������� ��������, � ����� �������� ���������, �������
	// �������� ������������� ������ ������
	void CheckResultOperand( POperand &pop );

	// ������� ���������� ��������� � ������ ����������� ��������
private:	

	// �������� ','
	void EvalExpr1( POperand &result );

	// ��������� ���������� '=', '+=','-=','*=','/=','%=','>>=','<<=','|=','&=','^=',
	// throw
	void EvalExpr2( POperand &result );

	// �������� '?:'
	void EvalExpr3( POperand &result );

	// �������� ||
	void EvalExpr4( POperand &result );

	// �������� &&
	void EvalExpr5( POperand &result );

	// �������� |
	void EvalExpr6( POperand &result );

	// �������� '^'
	void EvalExpr7( POperand &result );

	// �������� '&'
	void EvalExpr8( POperand &result );
	
	// ��������� ==, !=
	void EvalExpr9( POperand &result );

	// ��������� <=, <, >, >=
	void EvalExpr10( POperand &result );

	// ��������� <<, >>
	void EvalExpr11( POperand &result );

	// ��������� +, -
	void EvalExpr12( POperand &result );

	// ��������� *, /, %
	void EvalExpr13( POperand &result );

	// ��������� .*, ->*
	void EvalExpr14( POperand &result );

	// ��������� ���������� ���� '(���)'
	void EvalExpr15( POperand &result );

	// ������� ��������� !, ~, +, -, *, &. size, ++, --, new, delete
	void EvalExpr16( POperand &result );

	// ��������� '()', '[]', '->', '.', ����������� ++, --,
	// dynamic_cast, static_cast, const_cast, reinterpret_cast, typeid
	void EvalExpr17( POperand &result );

	// �������, true, false, this,
	// ������������� (������������� ��������, ����������, �������� ����������)
	// ��� ��������� � �������
	void EvalExpr18( POperand &result );

private:

	// ����� �������������� ������������ �����
	class StackOverflowCatcher
	{
		// ������� �����
		static int stackDeep;

	public:
		StackOverflowCatcher( ) {
			if( stackDeep == MAX_PARSER_STACK_DEEP )
				theApp.Fatal( "���� ����������; ������� ������� ���������" );
			stackDeep++;
		}

		~StackOverflowCatcher() {
			stackDeep--;
		}
	};
};
