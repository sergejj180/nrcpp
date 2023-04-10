// ��������� ����������� � C-��� - Translator.h


#ifndef _TRANSLATOR_H_INCLUDE
#define _TRANSLATOR_H_INCLUDE


// ������� �����������
namespace TranslatorUtils
{
	// ���������� ��� ������� ��������� ��� ���������� ��������. ���������
	// ����. ������� ���������: ����������, �����������, ���������, �������������� (���������)
	const string &GenerateScopeName( const SymbolTable &scope );

	// ������������� ��� ��� ����������� ��������������
	string GenerateUnnamed( );

	// ������� ��������� ���������� ���������
	PCSTR GenerateOperatorName( int op );
	
	// ������������� ����������, ������ ���� ��� �� typedef � 
	// ����� ���������� �� �������������
	void TranslateDeclaration( const TypyziedEntity &declarator, 
		const PObjectInitializator &iator, bool global );

	// ������������� ���������� ������
	void TranslateClass( const ClassType &cls );

	// ������������� ��������� ������
	inline string GenerateClassHeader( const ClassType &cls ) {
		return (cls.GetBaseTypeCode() == BaseType::BT_UNION ? "union " : "struct ") +
			cls.GetC_Name();
	}
}


// ����� ��� ��������� ����������� ������� ������, �
// ������ �� ����������
class SMFGenegator
{
	// ����� ��� �������� ����������� ���������
	ClassType &pClass;

	// ������ ��������� �������. ������ ������ �������� �������� ������
	// ������������� ������-������ ������ ������, � ����� �������� ��������
	// �������� ������ ������. � ������ �� ����� ���� ���� ���������� �������
	typedef list<const ClassType *> ClassTypeList;
	ClassTypeList dependClsList;

	// �������� ����������� ������� ������, ������� �����������
	// ������� ����� ��� ������ ������
	typedef list<SMFManager> SMFManagerList;
	SMFManager smfManager;

	// ���� ���������� � true, ���� ����� ����� ����������� ������� ������
	bool haveVBC;

	// ���������� � true, ���� � ������ ���� �����, ��������� ����� �������������,
	// ��� ������ � ���������. ���� ���� ���� ����������, ������������ � ��������
	// ����������� �� ������������
	bool explicitInit;

	// ������������� ���������, � ������� ������������ ������ ��
	// ��������� ���������. �.�. ���� ��� �������� ����������� �������
	// �����������, �� �������� � ���� ��������� ����� true ��� ���� trivial,
	// ��� � ��� ��������� �����
	struct DependInfo {
		bool trivial, paramIsConst;
		DependInfo() { trivial = paramIsConst = true; }
	};

	// �������� ����� � ������ ��������� �������, 
	// ������ ���� �� ��� �� ���������
	void AddDependClass( const ClassType *cls ) {
		if( find(dependClsList.begin(), dependClsList.end(), cls) == dependClsList.end() )
			dependClsList.push_back(cls);
	}

	// ��������� ������ ��������� �������, ����������� � ������ Generate
	void FillDependClassList();

	// ���������� true, ���� ���� ����� ��������� ���
	bool NeedConstructor( const DataMember &dm );

	// ����� �������� �� ������ ��������� ������� � ���������,
	// �������� �� ���������, ����������� � ������������ �� ������
	// ����������� �����. ����������� ����� �������� ����� ��-�� �� �������
	// ����
	bool CanGenerate( const SMFManagerList &sml, 
			const SMFManager::SmfPair &(SMFManager::*GetMethod)() const ) const;

	// ������ ����������� ��������, ��������� ����� � ����������� ������ �� ����
	// ���������� ������ ������ � ������
	bool CanGenerateCopyOperator( const SMFManagerList &sml, 
			const SMFManager::SmfPair &(SMFManager::*GetMethod)() const ) const;

	// ���������� true, ���� ����� ����� ���� ������������ ��� �����������.
	// ����� �������� �� ������ ������ ��������� ������� � ������������� ������������
	DependInfo GetDependInfo( const SMFManagerList &sml, 
			const SMFManager::SmfPair &(SMFManager::*GetMethod)() const ) const ;

	// ������� true, ���� ����� ��������� �������� �����������
	bool IsDeclareVirtual(const SMFManagerList &sml, 
			const SMFManager::SmfPair &(SMFManager::*GetMethod)() const ) const ;

	// ����� ���������� ������ ����������� ����� � ������� ��� ����������
	const DerivedTypeList &MakeDTL0() const;

	// ����� ���������� ������ ����������� ����� � ������� � �������
	// � ��������� ����������� ��� �� ���������� ������ �� ���� �����
	const DerivedTypeList &MakeDTL1( bool isConst ) const;

	// ��������� ����������� �� ���������
	ConstructorMethod *MakeDefCtor( bool trivial ) const;

	// ��������� ����������� �����������
	ConstructorMethod *MakeCopyCtor( bool trivial, bool isConst ) const;

	// ��������� ����������
	Method *MakeDtor( bool trivial, bool isVirtual ) const;

	// ��������� �������� ������������
	ClassOverloadOperator *MakeCopyOperator( Method::DT dt, bool isConst, bool isVirtual ) const;

	// ������� ���������� ���������� �� ������
	void DebugMethod( const Method &meth );

public:
	// ������ �����
	SMFGenegator( ClassType &pcls )
		: pClass(pcls), smfManager(pClass), haveVBC(false), explicitInit(false) {
	}

	// ����� ���������� ��� ������ ����������� ������� �����, ����
	// ��� ����������
	void Generate() ;
};


// ������ �-���� �� ��������� �++-����. ���� ������ ���, ������ �������������
// � ������. const-�������������, ������������� ��������, ������������� ������� ������������
class CTypePrinter
{
	// �++-���
	const TypyziedEntity &type;

	// �������� �����
	string outBuffer;

	// �������������� ������� ���
	string baseType;

	// �������������, ���� �����, ������ �������� ��� ���
	const Identifier *id;

	// ����� ����������� � ������ ����� ��������� ������ 'this'
	string thisBuf;

	// ���� ���������������, ���� ������������� �������� ��������
	// �������� ���������� ����������� ���� � ��������� �� � �����
	void PrintPointer( string &buf, int &ix, bool &namePrint );

	// �������� ����������� ����������� ���� � ��������� � �����
	void PrintPostfix( string &buf, int &ix );

	// ������� ����� � this, ��� ��������� ������
	void PrintThis( const ClassType &cls, string &buf, bool addThis = true ) {	
		buf = (cls.GetBaseTypeCode() == BaseType::BT_UNION ? "union " : "struct ")
			+ cls.GetC_Name() + (addThis ? " *this" : "*");
	}

public:
	// ����������� ��� ����, ���� ��� �������� �������������� �� �������
	CTypePrinter( const TypyziedEntity &type, const Identifier *id = NULL )
		: type(type), id(id) {
		
	}

	// ����������� ��� ������� ��� ������
	CTypePrinter( const TypyziedEntity &type, const Function &fn, bool printThis )
		: type(type), id(&fn) {

		// ���� ������������� �����, ����������� this. ���� ����� ���������,
		// ����������� ����� ��������� ������
		if( printThis )
		{
			const ClassType *cls = &static_cast<const ClassType &>(fn.GetSymbolTableEntry());
			if( fn.IsClassMember() && static_cast<const Method &>(fn).IsVirtual() )
				cls = &static_cast<const Method &>(fn).GetVFData().GetRootVfClass();
			PrintThis( *cls, thisBuf );
		}
	}
	
	// ����������� ��� ���������� �-���� ������������ ������, ��� �����
	CTypePrinter( const TypyziedEntity &type, const Method &vmethod )
		: type(type), id(NULL) {
		INTERNAL_IF( !vmethod.IsVirtual() );
		PrintThis( static_cast<const ClassType &>(vmethod.GetSymbolTableEntry()),
			thisBuf, false );
	}

	// ������������
	void Generate();

	// ������� ��������������� �����
	const string &GetOutBuffer() const {
		return outBuffer;
	}
};


// ������������� ����������� ������. ���� ����� �� ��������� ��������,
// ������������� ������ ��������� ������.
class ClassGenerator
{
	// �������� ����� ������. ����������� ������ 
	string clsBuffer;

	// �������������� ����� ������, �������� ���������� ������������ 
	// �� ��������� ������: ������, ����������� ������-�����
	string outSideBuffer;

	// ������ �� ������������ �����
	const ClassType &pClass;

	// ������� ���������� ���������� ������, ��� ���������� ��������� ����.
	// ��������� ������������� ������ ������������ ������������ ���������� �� � ������
	int castOperatorCounter;


	// ������������� ��������� ������
	string GenerateHeader( const ClassType &cls );

	// ������������� ������� ���������� (������)
	void GenerateBaseClassList( const BaseClassList &bcl );

	// ������������� ������ ������-������ � �������
	void GenerateMemberList( const ClassMemberList &cml );

	// ������������� ������� ����������� ������� ��� ������� ������,
	// �������� ������� ������
	void GenerateVTable( );

	// ���������� ����� ���������� �� GenerateVTable
	void GenerateVTable( const ClassType &cls, string *fnArray, int fnArraySize );

public:
	// ������ �����
	ClassGenerator( const ClassType &pClass )
		: pClass(pClass) {
		INTERNAL_IF( theApp.IsDiagnostic() );
	}

	// ������������� ����������� ������ � ��������� ����������
	void Generate();

	// ������� �������������� �����
	const string &GetOutSideBuffer() const {
		return outSideBuffer;
	}

	// ������� ����� �� ��������������� �������
	const string &GetClassBuffer() const {
		return clsBuffer;
	}
};


// ��������� ���������� ����������. ���������� ���������� �������, ���� �������
class DeclarationGenerator
{
	// ������ �� ����������
	const TypyziedEntity &declarator;

	// ��������� �������������, ����� ���� NULL
	const PObjectInitializator &iator;

	// ���� ���������� �������� ���������� ���������� � true
	bool global;

	// �������� ����� � �����������
	string outBuffer;

	// ��������������� ����� � ������� ������������ ��������������
	// ���������� � ��������� ������������� �����������. ���������
	// ������������� ������������ ��� ��������� �������� � �������������
	// �������������, ���� ���������� �������� � ������������������
	// ���������������
	string indirectInitBuf;

	// �������������, ������������� �������������. ���������� ����� � �������
	// ������������. �� ������������� ����������� ������������
	void GenerateConstructorCall( const ::Object &obj,
		const ConstructorInitializator &ci, string &out );

	// ������������� ��������� �� ��������������. � ������ ��������� ������ ���� ����.
	// ���� ��������� �������� ���������������� � ����� �������������� ��� ������
	// ������������� ����������� �������, ������� true, ����� false
	bool GenerateExpressionInit(  const ::Object &obj,
		const ConstructorInitializator &ci, string &out );

	// ������������� ������������� ��� �������. ���� ������������� ��������
	// ������������� ��� ������, ����������. ����� ���������� � ����� ���������
	// �������������
	void GenerateInitialization( const ::Object &obj );

public:
	// ������ ���������� � �������������
	DeclarationGenerator( const TypyziedEntity &d, const PObjectInitializator &ir, bool global )
		: declarator(d), iator(ir), global(global) {
		INTERNAL_IF( !(declarator.IsObject() || declarator.IsFunction()) );
	}

	// ������������� ����������
	void Generate();

	// ������� ����� � �����������
	const string &GetOutBuffer() const {
		return outBuffer;
	}

	// ������� ����� � ��������� �����������
	const string &GetIndirectInitBuf() const {
		return indirectInitBuf;
	}

};


// �������� ���������� �������
class TemporaryObject
{
	// C++ ��� �������
	TypyziedEntity type;

	// ����� ������������� �������
	bool isUsed;

	// ��� �������
	string name;

	// ������� ��������� ��������
	static int temporaryCounter;

public:
	// ������ ���, ��� �������� ������ ������������ �� ���������
	TemporaryObject( const TypyziedEntity &type ) 
		: type(type), isUsed(true) {
		const static string t = "__temporary";
		name = t + CharString(temporaryCounter++).c_str();
	}

	// � ����������� ��������� ������� ��������� ��������
	~TemporaryObject() {
		temporaryCounter--;
	}

	// ������� ���
	const string &GetName() const {
		return name;
	}

	// ������� ��� �������
	const TypyziedEntity &GetType() const {
		return type;
	}

	// ���� ������ ������������, ������� true
	bool IsUsed() const {
		return isUsed;
	}

	// �������� ������ ��� ��������������
	void SetUnUsed() {
		isUsed = false;
	}

	// �������� ������ ��� ������������
	void SetUsed() {
		isUsed = true;
	}
};


// �������� ��������� ��������
class TemporaryManager
{
	// ��� - ������ ��������� ��������
	typedef list<TemporaryObject> TemporaryList;

	// ������ ��������� ��������
	TemporaryList temporaryList;

	// ����� ���������. ����������� ��� �������� ������ ������� �-�����������
	// � ��� ����������� ���������� ������� � ������������� ������������
	string genBuffer;

	// ���������� �������������� �������� �� ������ ������
	int unUsed;

public:
	//
	TemporaryManager()
		: unUsed(0) {
	}

	// ������� ����� ��������� ������
	const TemporaryObject &CreateTemporaryObject( const TypyziedEntity &type );

	// ���������� ��������� ������
	void FreeTemporaryObject( TemporaryObject &tobj );

	// ��������� ����� ���������
	const string &FlushOutBuffer() {
		static string rbuf;
		rbuf = genBuffer;
		genBuffer = "";
		return rbuf;
	}
};


// ��������� ���������
class ExpressionGenerator
{
	// ��������� ������� ������� �����������
	const POperand &exp;

	// ��������� �� ����� � �������� ����������� this, ���� 
	// ���������� ��������� ������ �������������� ������
	const ClassType *thisCls;

	// �������� ����� ���������
	string outBuffer;

	// ����� � ������������ � ���������������� ��������� ��������,
	// ���� ��������� ������� �� ���������, ����� ������
	string temporaryBuf;

	// ����� � ���������� ������������ ��������� ��������. ����� ����
	// ������, ���� ��������� ������� �� ������� ������ ������������, ����
	// �� ������ ���
	string destroyBuf;

	// ���� ��������� �� ����� ����� (�� ������� �������������� �������)
	// ���������� � true
	bool oneLineExpression;

	// ����������� �����
	static bool PrintPathToBase( const ClassType &cur, const ClassType &base, string &out );

	// ������������� ���� �� �������� ������ � ��������. ���� ������ ���������,
	// ������ �� ������������. ���� ������� ����� ����������� � �������� - ���������� ������
	static const string &PrintPathToBase( const ClassType &cur, const ClassType &base );

	// ����������� �������� �������. ���� printThis ��������� �� ��������������
	// ����������� ��������� this ��� ������������� � �� typedef ������ ������
	const TypyziedEntity &PrintPrimaryOperand( 
		const PrimaryOperand &po, bool printThis, string &out );

	// ����������� ���������� ���������, �������� ����� ��������������� ���������.
	// prvOp - ��� ����������� ���������, ���� ��������� �� ���� -1
	string PrintExpression( const POperand &exp );

	// ����������� ������� ���������
	void PrintUnary( const UnaryExpression &ue,  string &out );

	// ����������� �������� ���������
	void PrintBinary( const BinaryExpression &be, string &out );

	// ����������� ��������� ���������
	void PrintTernary( const TernaryExpression &te, string &out );

	// ����������� ����� �������
	void PrintFunctionCall( const FunctionCallExpression &fce, string &out );

	// ����������� new-��������
	void PrintNewExpression( const NewExpression &ne, string &out );

public:
	// ������ ��������� � thisCls, ���� ���������
	ExpressionGenerator( const POperand &exp, const ClassType *thisCls )
		: exp(exp), thisCls(thisCls), oneLineExpression(true) {
		INTERNAL_IF( theApp.IsDiagnostic() );
		INTERNAL_IF( exp.IsNull() || !(exp->IsExpressionOperand() || exp->IsPrimaryOperand()) );
	}

	// ������������� �-���������
	void Generate();

	// ���� ��������� �� ���� ��������� �� ���������, � ������ 
	// ������ ������������, ��������� true
	bool IsOneLineExpression() const {
		return oneLineExpression;
	}
	
	// ���� ��������� �������� ��������� ��������, ������ true
	bool IsNeedTemporary() const {
		return !temporaryBuf.empty();
	}

	// ���� ��������� ����� ������������
	bool IsNeedDestructorCall() const {
		return !destroyBuf.empty();
	}

	// ������� ��������������� �����
	const string &GetOutBuffer() const {
		return outBuffer;
	}

	// ������� ����� �������� � ��������������� ��������� ��������
	const string &GetTemporaryBuffer() const {
		return temporaryBuf;
	}

	// ������� ����� ������ ������������ ��� ��������� ��������
	const string &GetDestroyBuffer() const {
		return destroyBuf;
	}
};


#endif
