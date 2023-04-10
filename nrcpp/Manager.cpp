// ����������� ����������� �������-���������� - Manager.cpp

#pragma warning(disable: 4786)
#include <nrc.h>

using namespace nrc;

#include "Limits.h"
#include "Application.h"
#include "LexicalAnalyzer.h"
#include "Object.h"
#include "Scope.h"
#include "Class.h"
#include "Parser.h"
#include "Manager.h"
#include "Maker.h"
#include "Checker.h"
#include "Overload.h"  


// ����� using-�������������� �� ��������� �� ������������ �������������
const UsingIdentifier *SynonymList::find_using_identifier( const Identifier *id ) const
{
	for( register const_iterator p = begin(); p != end(); p ++ )
	{
		if( (*p).second != R_USING_IDENTIFIER )
			continue;

		const UsingIdentifier *ui = static_cast<const UsingIdentifier *>((*p).first);
		if( &ui->GetUsingIdentifier() == id )
			return ui;
	}

	return NULL;
}


// ����������� ��������� ������, � ��������� ������ �����
// �������� �������. 
// qn - ��� (������), bt - ���� ������, ������� ��������� � ������� �������
// ������ ���, watchFriend - ����������� ����� ����� � �� ��������� ��������,
// ��� ������� - ������� ������, ��� ������ - using �������
NameManager::NameManager( const CharString &qn, const SymbolTable *bt, bool watchFriend )
		: queryName(qn), bindTable(bt)
{
	// �������������� ������
	IdentifierList foundList;

	// ���� ������ ������� ��������� � ������� ������� ����������� �����
	if( bindTable != NULL )
	{
		// ����������� ����� � ������ ��������� �������� ���������
		if( watchFriend )
			bindTable->FindSymbol( queryName, foundList );

		// ���������� ����� ������ � ������ �������, �� �������� ���������
		// ��		
		else
			bindTable->FindInScope( queryName, foundList );
		
	}
	
	// � ��������� ������ - �������� ����� �� ���� �������� 
	// ��������� �� ���������� ������� ������������, ������� � ��������
	else
		theApp.GetTranslationUnit().GetScopeSystem().DeepSearch(queryName, foundList);

	// ��������� ������� �� �����.
	if( foundList.empty() )
		return;

	// ����� ��������� ������ out, ������ �������������-����
	for( IdentifierList::iterator p = foundList.begin(); p != foundList.end() ; ++p )
	{		
		Identifier *id = const_cast<Identifier *>(*p);
		INTERNAL_IF( id == NULL );

		// ������� ����
		Role role = GetIdentifierRole(id);

		// ���� ����� ���� - using-�������������, ������� ��������
		// ��������� �� �������������� ����������
		if( role == R_USING_IDENTIFIER )
		{
			// �������� ������� � ������ ��������� ��� ����������� �������� �������
			synonymList.push_back( RolePair(id, role) );
			id = const_cast<Identifier *>(
					&static_cast<UsingIdentifier *>(id)->GetUsingIdentifier());
			role = GetIdentifierRole(id);
			INTERNAL_IF( role == R_USING_IDENTIFIER );
		}
		
		// ���� ����� ������� ������� ���������, ����������� ��� � ������� ���������
		else if( role == R_NAMESPACE_ALIAS )
		{
			synonymList.push_back( RolePair(id, role) );
			id = const_cast<NameSpace *>(&static_cast<NameSpaceAlias *>(id)->GetNameSpace()); 
			role = R_NAMESPACE;
		}

		// ��� �������� � �������� ��������� ����
		this->roleList.push_back( RolePair(id, role) );
	}
}

	
// �������� ���� ��������������
Role NameManager::GetIdentifierRole( const Identifier *id ) 
{
	INTERNAL_IF( id == NULL );

	// ����������� � ������ ������ ��������� id
	// ���� ������
	if( const ::Object *obj = dynamic_cast<const ::Object *>(id) )
		return obj->IsClassMember() ? R_DATAMEMBER : R_OBJECT;		// ����� ���� ������-����
		
	// ����� ���� �������
	else if( const Function *fn = dynamic_cast<const Function *>(id) )
	{
		// ���� ��������� �������, ������� - �����������, �����,
		// ��� ������������� ��������
		if( fn->IsTemplate() )
			return R_TEMPLATE_FUNCTION;

		// ������������� ���������� ������, ������������, ������ � �.�.
		else if( fn->IsTemplateSpecialization() )
			return R_TEMPLATE_FUNCTION_SPECIALIZATION;	
		
		// ���� �����
		else if( fn->IsClassMember() )
		{
			// ������������� �������� ������
			if( fn->IsOverloadOperator() )
				return R_CLASS_OVERLOAD_OPERATOR;

			// ���� �����������
			else if( dynamic_cast<const ConstructorMethod *>(fn) != NULL )
				return R_CONSTRUCTOR;

			// ���� �����
			else
				return R_METHOD;
		}

		// ����� �������, ������� ����� ���� ������������� ����������
		else
		{
			if( fn->IsOverloadOperator() )
				return R_OVERLOAD_OPERATOR;
			else
				return R_FUNCTION;
		}
	}

	// ����� ���� �����
	else if( const ClassType *cls = dynamic_cast<const ClassType *>(id) )
	{
		// ��� �����������
		if( cls->GetBaseTypeCode() == BaseType::BT_UNION )
			return R_UNION_CLASS_TYPE;			

		else
			return R_CLASS_TYPE;
	}	

	// ����� ���� ��� ������������
	else if( const EnumType *enumT = dynamic_cast<const EnumType *>(id) )
		return R_ENUM_TYPE;
	
	
	// ����� ���� ��������� �����
	else if( const TemplateClassType *tmptCls = dynamic_cast<const TemplateClassType *>(id) )
		return R_TEMPLATE_CLASS;
	
	// ����� ���� ��������� ��������
	else if( const TemplateParametr *param = dynamic_cast<const TemplateParametr *>(id) )
	{		
		if( param->GetTemplateParametrType() == TemplateParametr::TP_TYPE )
			return R_TEMPLATE_TYPE_PARAMETR;

		else if( param->GetTemplateParametrType() == TemplateParametr::TP_NONTYPE  )
			return R_TEMPLATE_NONTYPE_PARAMETR;

		else
			return R_TEMPLATE_TEMPLATE_PARAMETR;
	}

	// ����� ���� ����������� ������� ���������
	else if( const NameSpace *ns = dynamic_cast<const NameSpace *>(id) )
		return R_NAMESPACE;

	// ����� ���� �������� �������
	else if( const Parametr *param = dynamic_cast<const Parametr *>(id) )
		return R_PARAMETR;


	// ����� ���� ��������� ������������
	else if( const EnumConstant *ecnst = dynamic_cast<const EnumConstant *>(id) )
		return ecnst->IsClassMember() ? R_CLASS_ENUM_CONSTANT : R_ENUM_CONSTANT;

	// ����� ���� using-�������������
	else if( const UsingIdentifier *ui = dynamic_cast<const UsingIdentifier *>(id) )
		return R_USING_IDENTIFIER;

	// ����� ���� ������� ������� ���������
	else if( const NameSpaceAlias *nsa = dynamic_cast<const NameSpaceAlias *>(id) )
		return R_NAMESPACE_ALIAS;

	// ����� ����������
	else
		return R_UNCKNOWN;
}


// ���� ��� �������� �����
bool NameManager::IsTypeName() const
{
	return AmbiguityChecker(GetRoleList()).IsTypeName(false) != NULL;
}


// ���� ��� �������� ����� typedef
bool NameManager::IsTypedef() const
{
	return AmbiguityChecker(GetRoleList()).IsTypedef() != NULL;
}


// ���� ��� �������� �����
bool QualifiedNameManager::IsTypeName() const
{
	return AmbiguityChecker(GetRoleList()).IsTypeName(false) != NULL;
}


// ���� ��� �������� ����� typedef
bool QualifiedNameManager::IsTypedef() const
{
	return AmbiguityChecker(GetRoleList()).IsTypedef() != NULL;
}


// ������� �������� ���������� �����,
// ����� ������ ������� ���������, � ������� ������� ������ ���. 	 
// ��� ������� ������������ ��� ������ ����� ��������� ����, ���� �����
// �������������� � ��� ���������, � ������ ���� np �������� ������
// ���� ��� �����. np - ������ ����� ��������� PC_QUALIFIED_NAME ��� PC_QUALIFIED_TYPENAME
QualifiedNameManager::QualifiedNameManager( const NodePackage *np, const SymbolTable *bt )
	: queryPackage(np), bindTable(bt)
{   
	// �������� ������������ ������� ����������
	INTERNAL_IF( np == NULL || (np->GetPackageID() != PC_QUALIFIED_NAME &&
		np->GetPackageID() != PC_QUALIFIED_TYPENAME) );

	// ���� ������� ���, �� � ������ ������ �� ����
	if( np->GetChildPackageCount() == 0 )
		return;

	// ���� �� ����� ����� ���� �����
	if( np->GetChildPackageCount() == 1 )
	{
		// �������� ��� ������� ����� � �������
		if( np->GetChildPackage(0)->GetPackageID() == PC_ERROR_CHILD_PACKAGE )
			return;

		// � ��������� ������ ����� ������ ��������� ������� � ����� NAME
		register int pid = np->GetChildPackage(0)->GetPackageID() ;
		INTERNAL_IF( pid != NAME && pid != PC_OVERLOAD_OPERATOR && pid != PC_CAST_OPERATOR &&
			 pid != PC_DESTRUCTOR );

		CharString nam = GetPackageName(*np->GetChildPackage(0));			

		// �������� ���� ��� ���������� �����
		NameManager nm( nam, this->bindTable );
		roleList = nm.GetRoleList();
		synonymList = nm.GetSynonymList();
		return;
	}

	// ����� ����� ��������� ���, ������� ������� ������� �� ������
	// ��������� ����� NameManager
	// ---  ��������� ������ � �������� �������� ��������� �� ���������������,
	// ---  �� ��������� ������� ��������, ����� ����� �������� ���� �������
	// ---  ��������� ��������

	const SymbolTable *entry = NULL;	// ��������� ������� �������� � ������� �������� �����
	int i = 0;						// ������ ������ � �������� ���������� ���� ���������

	// ������ ����������� ��� ��������� ���������� ����� ��������� �����
	// ��� �� �������� �������� ���������. ��� ��������� ���� ������ �� � ������� try-����
	try {

	// ���� ��� ������ - '::', ���������� ����� ������� ����� � 
	// ���������� ������� ���������
	if( np->GetChildPackage(0)->GetPackageID() == COLON_COLON )
	{
		// ��������� �������� ������ ���� �������������
		register int pid = np->GetChildPackage(1)->GetPackageID() ;
		INTERNAL_IF( pid != NAME && pid != PC_OVERLOAD_OPERATOR && pid != PC_CAST_OPERATOR &&
			 pid != PC_DESTRUCTOR );

		// �������� ���������� ������� ���������
		const SymbolTable *globalST = GetScopeSystem().GetFirstSymbolTable();

		// ���������� ����� ����� � ���������� ������� ��������� ��� �����
		// ��������� �������� ���������
		NameManager nm( GetPackageName(*np->GetChildPackage(1)), globalST, false );

		// ����� ���� ������ ����� 2, �� ��� ����� � �� ���� �������� ���������,
		// ��� �������� ��������, � ��������� ������ ��� ������ ���� �������� ���������
		if( np->GetChildPackageCount() == 2 )
		{
			roleList = nm.GetRoleList();
			synonymList = nm.GetSynonymList();
			qualifierList.AddSymbolTable(globalST);		// ��������� �� ������������ � ������
			return;
		}

		else
		{
			entry = IsSymbolTable(nm);
			if( !entry )
				throw 1;	// ������, ��� �� �������� �������� ���������
			qualifierList.AddSymbolTable(entry);
			i = 2;			// ���� ��������� �������� �� ������� ������
		}
	}

	// � ��������� ������ ����� ������� ����������� � ������� ������� ���������,
	// �� ������� ������������
	else
	{
		// ������ ������� � ����� ������ ������ ���� ���
		INTERNAL_IF( np->GetChildPackage(0)->GetPackageID() != NAME );
		NameManager nm( GetPackageName(*np->GetChildPackage(0)), bindTable );		

		entry = IsSymbolTable(nm);
		if( !entry )
			throw 0;
		qualifierList.AddSymbolTable(entry);
		i = 1;
	}

		
	// ���� ������������ �����: ��������� ������� ��������� ��
	// ������� � ������ � �������� ������ ����� ���������� �����
	// 'i' �������� ����, ��� ���������� ��������� ������� ��������� 
	for( ;; )
	{
		// ����� ������ ���� '::' � ������ �� ��� ������ ���� ���
		INTERNAL_IF( np->GetChildPackage(i)->GetPackageID() != COLON_COLON );	
		i++;
		register int pid = np->GetChildPackage(i)->GetPackageID();
		INTERNAL_IF( i == np->GetChildPackageCount() || 
			(pid != NAME && pid != PC_OVERLOAD_OPERATOR && pid != PC_CAST_OPERATOR &&
			 pid != PC_DESTRUCTOR) );
		
		// �������� ��� �� ��������� ���������� ������� ���������,
		// ������ ����� ������ ����� ������������ ������ � ���, ��� ����� ���������
		// �������� ���������
		CharString name = GetPackageName(*np->GetChildPackage(i));
		const SymbolTable *lastSt = 
			&qualifierList.GetSymbolTable(qualifierList.GetSymbolTableCount()-1);

		// ���� ���� ������. ���� lastSt - ��������� ������� ��������� � 
		// name - ����� �� ��� ��� ������, ������ ���������� ������ �������������
		// ������ � �������
		if( lastSt->IsClassSymbolTable() && 
			i == np->GetChildPackageCount()-1 &&
			static_cast<const ClassType *>(lastSt)->GetName() == name )
		{
			INTERNAL_IF( !roleList.empty() );

			// ��������� ������ ������������� � ������ �����
			const ConstructorList &cl =
				static_cast<const ClassType *>(lastSt)->GetConstructorList();
			for( ConstructorList::const_iterator p = cl.begin(); p != cl.end(); p++ )
				roleList.push_back( RolePair((ConstructorMethod*)*p, R_CONSTRUCTOR) );

			// ���� ������ ����, ������
			if( cl.empty() )
				theApp.Error(
					ParserUtils::GetPackagePosition((NodePackage*)np->GetChildPackage(i)),
					"'%s' - ����������� ��� �� ��������", name.c_str());
			break;
		}

		NameManager stm( name, lastSt, false );		// �������� ������ ����� �����
	
		// ���� �� ����� ��������� �������������, �.�.
		// ��, ��� ��� ����� � �������� ����� ����� - �������� ������ 
		// ��� ����� � �������. � ������ ���� � ����� ��� ����� (�� �������),
		// ��� ����� ���� ��������� ��������� ��������, �� ��������� ������
		if( i == np->GetChildPackageCount()-1 )
		{
			roleList = stm.GetRoleList();
			synonymList = stm.GetSynonymList();
			if( roleList.empty() )
				theApp.Error( 
					ParserUtils::GetPackagePosition((NodePackage*)np->GetChildPackage(i)),
					"'%s' - ������������� �� ������ � ������� ��������� '%s'",
					name.c_str(),
					dynamic_cast<const Identifier *>(lastSt)->GetQualifiedName().c_str() );
			
			break;	// ���� �������
		}

		// ����� ��� ������ ���� �������� ���������, ��������� �� � 
		// ������ �������������� �����
		else if( const SymbolTable *st = IsSymbolTable(stm) )
			qualifierList.AddSymbolTable(st);				

		// ����� ���������� �������������� �������� ��� ������ ������
		else
			throw i;	
		i++;
	}

	// ������ ���� ��� �� �������� �������� ���������, � ���������
	// ������ ������ � ������� ���������� ���
	} catch( int pkgIx ) {

		// ����������� ������ ������� �������� � ������� ������
		roleList.clear();
		qualifierList.Clear();
		synonymList.clear();

		LexemPackage *lp = (LexemPackage *)np->GetChildPackage(pkgIx);
		theApp.Error( lp->GetLexem().GetPos(), 
			"'%s' �� �������� �������������� ������� ���������",
			lp->GetLexem().GetBuf().c_str() );
	}	
}

// ���������, �������� �� ��� �������� ���������. ���� �������� - ���������� 
// ��������� �� ���, � ��������� ������ - NULL
const SymbolTable *QualifiedNameManager::IsSymbolTable( const NameManager &nm ) const 
{
	// ��� ������ ���� �������, ��������������� �������� ���������,
	// ���� typedef - �����, ������� ���������� �����
	const SymbolTable *rval = NULL;
	AmbiguityChecker achk(nm.GetRoleList(), ParserUtils::GetPackagePosition(queryPackage), true);

	if( const ClassType *cls = achk.IsClassType(true) )
		return cls;

	else if( const NameSpace *ns = achk.IsNameSpace() )
		return ns;

	// � typedef'�, ����� ���� �����
	else if( const ::Object *td = achk.IsTypedef() )
	{
		if( const ClassType *cls = CheckerUtils::TypedefIsClass(*td) )
			return cls;
		return NULL;
	}
	
	else
		return NULL;
}


// ���������� ��� ������. ����� ����� ����� ��� NAME, PC_OVERLOAD_OPERATOR,
// PC_CAST_OPERATOR, PC_DESTRUCTOR. � ��������� ���� ������� ���������� 
// �������-��������� ��� ��������� ����������� ����� ��������������
CharString QualifiedNameManager::GetPackageName( const Package &pkg )
{
	register int pid = pkg.GetPackageID();

	if( pid == NAME )	
		return static_cast<const LexemPackage &>(pkg).GetLexem().GetBuf();	

	else if( pid == PC_OVERLOAD_OPERATOR )
	{
		const NodePackage &np = static_cast<const NodePackage &>(pkg);
		TempOverloadOperatorContainer tooc;
		MakerUtils::AnalyzeOverloadOperatorPkg( np, tooc);
		return tooc.opFullName;
	}

	else if( pid == PC_CAST_OPERATOR )
	{
		const NodePackage &np = static_cast<const NodePackage &>(pkg);
		TempCastOperatorContainer tcoc;
		MakerUtils::AnalyzeCastOperatorPkg( np, tcoc);
		return tcoc.opFullName;
	}

	else if( pid == PC_DESTRUCTOR )
	{
		const NodePackage &np = static_cast<const NodePackage &>(pkg);
		INTERNAL_IF( np.GetChildPackageCount() != 2 || 
			np.GetChildPackage(0)->GetPackageID() != '~' || 
			np.GetChildPackage(1)->GetPackageID() != NAME );
		
		string dn =
			static_cast<const LexemPackage &>(*np.GetChildPackage(1)).GetLexem().GetBuf().c_str();	
		return ('~' + dn).c_str();
	}

	else
		INTERNAL( 
			"'QualifiedNameManager::GetPackageName' ��������� ����� � ������������ �����");
	return "";
}


// �������� ���������� �����. � ���������� ��� ����, ��� �������������,
// ���� ������������ �� ������ ��� ����� -1
ImplicitTypeManager::ImplicitTypeManager( int lcode, int msgn, int msz )
{
	// ��������� ��� �������� �� ����� ������ ��
	// ���������� ���� ������� �����
	struct LxmToBT
	{
		// ���������� ���
		BaseType::BT btCode;
		
		// ��� �������
		int lxmCode;
	} codes[] = {
		BaseType::BT_BOOL,    KWBOOL,
		BaseType::BT_CHAR,    KWCHAR,
		BaseType::BT_WCHAR_T, KWWCHAR_T,
		BaseType::BT_INT,     KWINT,
		BaseType::BT_FLOAT,   KWFLOAT,
		BaseType::BT_DOUBLE,  KWDOUBLE,
		BaseType::BT_VOID,	  KWVOID
	};

	baseTypeCode = BaseType::BT_NONE;
	for( int i = 0; i< sizeof(codes)/ sizeof(LxmToBT) ; i++ )
		if( codes[i].lxmCode == lcode ) 
		{
			baseTypeCode = codes[i].btCode;			
			break;
		}

	// ��� �������� ���� ������ ���� �����
	INTERNAL_IF( baseTypeCode == BaseType::BT_NONE );
	modSign = BaseType::MN_NONE;
	modSize = BaseType::MZ_NONE;

	// ������ ����������� �����
	if( msgn != -1 )
	{
		INTERNAL_IF( msgn != KWSIGNED && msgn != KWUNSIGNED );
		modSign = (msgn == KWUNSIGNED ? BaseType::MN_UNSIGNED : BaseType::MN_SIGNED);
	}

	// ������ ����������� �������
	if( msz != -1 )
	{
		INTERNAL_IF( msz != KWSHORT && msz != KWLONG );
		modSize = (msz == KWSHORT ? BaseType::MZ_SHORT : BaseType::MZ_LONG);
	}
}

// ����������� ��� ��� ���������� ����
ImplicitTypeManager::ImplicitTypeManager( const BaseType &bt )
{
	baseTypeCode = bt.GetBaseTypeCode();
	modSign = bt.GetSignModifier();
	modSize = bt.GetSizeModifier();
}


// �� ����, ���������� ��������� �� ��� ��������� ������� ���
const BaseType &ImplicitTypeManager::GetImplicitType()
{
	// ����������������� ������� ������� �����
	static BaseType *btTable[] = {
		new BaseType(BaseType::BT_BOOL),
		new BaseType(BaseType::BT_CHAR),
		new BaseType(BaseType::BT_CHAR, BaseType::MZ_NONE, BaseType::MN_UNSIGNED),
		new BaseType(BaseType::BT_CHAR, BaseType::MZ_NONE, BaseType::MN_SIGNED),
		new BaseType(BaseType::BT_WCHAR_T),
		new BaseType(BaseType::BT_INT),
		new BaseType(BaseType::BT_INT, BaseType::MZ_SHORT),
		new BaseType(BaseType::BT_INT, BaseType::MZ_NONE, BaseType::MN_UNSIGNED),
		new BaseType(BaseType::BT_INT, BaseType::MZ_NONE, BaseType::MN_SIGNED ),
		new BaseType(BaseType::BT_INT, BaseType::MZ_LONG),
		new BaseType(BaseType::BT_INT, BaseType::MZ_SHORT, BaseType::MN_UNSIGNED ),
		new BaseType(BaseType::BT_INT, BaseType::MZ_SHORT, BaseType::MN_SIGNED ),
		new BaseType(BaseType::BT_INT, BaseType::MZ_LONG,  BaseType::MN_UNSIGNED),
		new BaseType(BaseType::BT_INT, BaseType::MZ_LONG,  BaseType::MN_SIGNED ),
		new BaseType(BaseType::BT_FLOAT),
		new BaseType(BaseType::BT_DOUBLE),
		new BaseType(BaseType::BT_DOUBLE, BaseType::MZ_LONG),
		new BaseType(BaseType::BT_VOID),

	};


	if( baseTypeCode != BaseType::BT_INT && baseTypeCode != BaseType::BT_CHAR )	
		modSign = BaseType::MN_NONE;		

	if( baseTypeCode != BaseType::BT_INT && baseTypeCode != BaseType::BT_DOUBLE )
		modSize = BaseType::MZ_NONE;

	for( int i = 0; i<sizeof(btTable)/sizeof(BaseType*); i++ )
		if( btTable[i]->GetBaseTypeCode() == baseTypeCode &&
			btTable[i]->GetSizeModifier() == modSize	  &&
			btTable[i]->GetSignModifier() == modSign	  )
			return *btTable[i];
		
	INTERNAL( "��� �������� ���� �� ��������������� � ������ 'GetImplicitType'" );
	return *new BaseType(BaseType::BT_NONE);	// ����� �������.
}


// �������� ������ ����
int ImplicitTypeManager::GetImplicitTypeSize() const 
{
	switch( baseTypeCode )
	{
	case BaseType::BT_BOOL:	
		return BOOL_TYPE_SIZE;

	case BaseType::BT_CHAR:
		return CHAR_TYPE_SIZE;

	case BaseType::BT_WCHAR_T:
		return WCHAR_T_TYPE_SIZE;

	case BaseType::BT_INT:	
		if( modSize == BaseType::MZ_SHORT )
			return SHORT_INT_TYPE_SIZE;

		else if( modSize == BaseType::MZ_LONG )
			return LONG_INT_TYPE_SIZE;

		else
			return INT_TYPE_SIZE;

	case BaseType::BT_FLOAT:
		return FLOAT_TYPE_SIZE;

	case BaseType::BT_DOUBLE:
		return modSize == BaseType::MZ_LONG ? LONG_DOUBLE_TYPE_SIZE : DOUBLE_TYPE_SIZE;

		// ������ ���� void �� ������ ������, �� ������� ���������� �������
	case BaseType::BT_VOID:		
		return VOID_TYPE_SIZE;
	
		// � ��������� ������, ���������� ������
	default:
		INTERNAL( "��� �������� ���� �� ��������������� � ������ 'GetImplicitTypeSize'" );
	}

	return -1;		// ����� warning
}

// �������� ��������� ������������� �����
CharString ImplicitTypeManager::GetImplicitTypeName() const 
{
	switch( baseTypeCode )
	{
	case BaseType::BT_BOOL:	
		return "bool";

	case BaseType::BT_CHAR:
		if( modSign == BaseType::MN_SIGNED )
			return "signed char";
		
		else if( modSign == BaseType::MN_UNSIGNED )
			return "unsigned char";

		else
			return "char";

	case BaseType::BT_WCHAR_T:
		return "wchar_t";

	case BaseType::BT_INT:	
		{
			CharString intnam;
			
			if( modSign == BaseType::MN_SIGNED )
				intnam += "signed ";
		
			else if( modSign == BaseType::MN_UNSIGNED )
				intnam += "unsigned ";

			if( modSize == BaseType::MZ_SHORT )
				intnam += "short ";

			else if( modSize == BaseType::MZ_LONG )
				intnam += "long "; 
			
			intnam += "int";
			return intnam;
		}

	case BaseType::BT_FLOAT:
		return "float";

	case BaseType::BT_DOUBLE:
		return modSize == BaseType::MZ_LONG ? "long double" : "double";

		// ������ ���� void �� ������ ������, �� ������� ���������� �������
	case BaseType::BT_VOID:		
		return "void";
	
		// � ��������� ������, ���������� ������
	default:
		INTERNAL( "��� �������� ���� �� ��������������� � ������ 'GetImplicitTypeName'" );
	}

	return CharString();		// ����� warning
}


// ����������� ���������� � ����� ������ ��������� ���
TypeSpecifierManager::TypeSpecifierManager( int c ) : code(c), group(TSG_UNCKNOWN)
{
	if( c == KWBOOL || c == KWCHAR  || c == KWWCHAR_T ||
		c == KWINT  || c == KWFLOAT || c == KWDOUBLE  || c == KWVOID )
		group = TSG_BASETYPE;

	else if( c == KWCLASS || c == KWSTRUCT || c == KWUNION || c == KWENUM )
		group = TSG_CLASSSPEC;

	else if( c == KWCONST ||  c == KWVOLATILE )
		group = TSG_CVQUALIFIER;

	else if( c == KWUNSIGNED || c == KWSIGNED )
		group = TSG_SIGNMODIFIER;

	else if( c == KWSHORT || c == KWLONG )
		group = TSG_SIZEMODIFIER;

	else if( c == KWAUTO || c == KWEXTERN || c == KWSTATIC || 
		c == KWREGISTER || c == KWTYPEDEF || c == KWMUTABLE )
		group = TSG_STORAGESPEC;

	else if( c == KWFRIEND )
		group = TSG_FRIEND;

	else if( c == KWINLINE || c == KWVIRTUAL || c == KWEXPLICIT )
		group = TSG_FUNCTIONSPEC;
}


// �������� ��� ������ �� ������� �����
CharString TypeSpecifierManager::GetGroupNameRU() const
{
	switch( group )
	{	
	case TSG_BASETYPE:		return "������� ���";
	case TSG_CLASSSPEC:		return "������������ ������";
	case TSG_CVQUALIFIER:	return "cv-������������";
	case TSG_SIGNMODIFIER:	return "����������� �����";
	case TSG_SIZEMODIFIER:	return "����������� �������";
	case TSG_STORAGESPEC:	return "������������ ��������";
	case TSG_FRIEND:		return "������������ ������";
	case TSG_FUNCTIONSPEC:	return "������������ �������";
	}

	return "<����������� ������>";
}


// �������� ��� ������ �� ����������
CharString TypeSpecifierManager::GetGroupNameENG() const
{
	switch( group )
	{	
	case TSG_BASETYPE:		return "base type";
	case TSG_CLASSSPEC:		return "class specifier";
	case TSG_CVQUALIFIER:	return "cv-qualifier";
	case TSG_SIGNMODIFIER:	return "sign modifier";
	case TSG_SIZEMODIFIER:	return "size modifier";
	case TSG_STORAGESPEC:	return "storage specifier";
	case TSG_FRIEND:		return "friend specifier";
	case TSG_FUNCTIONSPEC:	return "function specifier";
	}

	return "<uncknown group>";
}


// ������� ������� ���, � ������ ���� ������������ �������� ������� �����	
BaseType::BT TypeSpecifierManager::CodeToBaseType() const 
{
	INTERNAL_IF( !IsBaseType() );
	return ImplicitTypeManager(code).GetImplicitTypeCode();
}


// ������� ������������ ������
BaseType::BT TypeSpecifierManager::CodeToClassSpec() const
{
	INTERNAL_IF( !IsClassSpec() );		
	if( code == KWCLASS )
		return BaseType::BT_CLASS;

	else if( code == KWSTRUCT )
		return BaseType::BT_STRUCT;

	else if( code == KWUNION )
		return BaseType::BT_UNION;

	else 
		return BaseType::BT_ENUM;

}

// ������� ����������� �����
BaseType::MSIGN TypeSpecifierManager::CodeToSignModifier() const
{
	INTERNAL_IF( !IsSignModifier() );
	return code == KWSIGNED ? BaseType::MN_SIGNED : BaseType::MN_UNSIGNED;			
}

// ������� ����������� �������
BaseType::MSIZE TypeSpecifierManager::CodeToSizeModifier() const 
{
	INTERNAL_IF( !IsSizeModifier() );	
	return code == KWLONG ? BaseType::MZ_LONG : BaseType::MZ_SHORT;			
}


// ������� ������������ ��������
::Object::SS TypeSpecifierManager::CodeToStorageSpecifierObj() const
{
	INTERNAL_IF( !IsStorageSpecifier() );	
	if( code == KWAUTO )
		return ::Object::SS_AUTO;

	else if( code == KWEXTERN )
		return ::Object::SS_EXTERN;

	else if( code == KWSTATIC )
		return ::Object::SS_STATIC;
	
	else if( code == KWREGISTER )
		return ::Object::SS_REGISTER;

	else  if( code == KWTYPEDEF )
		return ::Object::SS_TYPEDEF;

	else if( code == KWMUTABLE )
		return ::Object::SS_MUTABLE;

	INTERNAL( "'CodeToStorageSpecifierObj' ������� �� ���������� ���" );
	return ::Object::SS_NONE;	// ����� �� ������ ����
}

	
// ������� ������������ �������� �������
Function::SS TypeSpecifierManager::CodeToStorageSpecifierFn() const
{
	INTERNAL_IF( !IsStorageSpecifier() );
	if( code == KWSTATIC )
		return Function::SS_STATIC;
	
	else if( code ==  KWEXTERN )
		return Function::SS_EXTERN;
	
	else if( code == KWTYPEDEF )
		return Function::SS_TYPEDEF;
	
	INTERNAL( "'CodeToStorageSpecifierFn' ������� �� ���������� ���" ); 
	return Function::SS_NONE;
}


// �������� ��������� ������������� ������������� �������� �������
CharString ManagerUtils::GetObjectStorageSpecifierName( ::Object::SS ss )
{
	switch( ss )
	{
	case ::Object::SS_AUTO:			return "auto";
	case ::Object::SS_REGISTER:		return "register";
	case ::Object::SS_EXTERN:		return "extern";
	case ::Object::SS_STATIC:		return "static";
	case ::Object::SS_TYPEDEF:		return "typedef";
	case ::Object::SS_BITFIELD:		return "<bitfield>";
	case ::Object::SS_MUTABLE:		return "mutable";
	case ::Object::SS_NONE:			return "<none>";
	}

	INTERNAL("'GetObjectStorageSpecifierName' ������� �������� ����������� ���");
	return "";
}


// �������� ��������� ������������� ������������� �������
CharString ManagerUtils::GetFunctionStorageSpecifierName( Function::SS ss )
{
	switch( ss )
	{	
	case Function::SS_EXTERN:		return "extern";
	case Function::SS_STATIC:		return "static";
	case Function::SS_TYPEDEF:		return "typedef";
	case ::Object::SS_NONE:			return "<none>";
	}

	INTERNAL("'GetFunctionStorageSpecifierName' ������� �������� ����������� ���");
	return "";
}


// �������� ������������ ������� � ���� �����
PCSTR ManagerUtils::GetAccessSpecifierName( ClassMember::AS as )
{
	switch( as )
	{
	case ClassMember::AS_PUBLIC:	return "��������";
	case ClassMember::AS_PRIVATE:	return "��������";
	case ClassMember::AS_PROTECTED:	return "����������";
	case ClassMember::NOT_CLASS_MEMBER: return "<�� ����>";
	}

	INTERNAL( "'GetAccessSpecifierName' �������� ����������� ��� ������������� �������" );
	return "";
}


// �������� �������� ������� ��������� � ���� �����, ����
// ��� �������� ���������������, ������� ���
CharString ManagerUtils::GetSymbolTableName( const SymbolTable &st )
{
	if( st.IsGlobalSymbolTable() )
		return "���������� ������� ���������";
	
	else if( st.IsNamespaceSymbolTable() )
		return static_cast<const NameSpace &>(st).GetQualifiedName();
	
	else if( st.IsLocalSymbolTable() )
		return 
			(GetScopeSystem().GetFunctionSymbolTable())->GetFunction().GetQualifiedName();
	
	else if( st.IsFunctionSymbolTable() )
		return static_cast<const FunctionSymbolTable &>(st).GetFunction().GetQualifiedName();
	
	else if( st.IsClassSymbolTable() )
		return static_cast<const ClassType &>(st).GetQualifiedName();
	
	else
		INTERNAL( "'GetAccessSpecifierName' �������� ����������� ������� ���������" );
	return "";
}


// ����� ������������� ����� 'base' �� ��������� � 'derived'
void DerivationManager::Characterize( const ClassType &base, 
							const ClassType &curCls, bool ac )
{
	register const BaseClassList &bcl = curCls.GetBaseClassList();
	for( int i = 0; i<bcl.GetBaseClassCount(); i++ )
	{
		const BaseClassCharacteristic &clh = *bcl.GetBaseClassCharacteristic(i);
		const ClassType &bcls = clh.GetPointerToClass();

		// ac ��������� ����������� ��� ���������� ������� ��� � �������
		ac = ac && clh.GetAccessSpecifier() == ClassMember::AS_PUBLIC;

		// ���� ���������� ����� �������� 'base', �������� �������� �
		// ������ �� �����, �.�. ����� �� ����� ����������� ��� ����
		if( &base == &bcls )
		{
			baseCount++;

			// ���� ������ �����, ��������� ��� �� ������������� � �� �����������
			if( baseCount == 1 )
			{
				virtualDerivation = clh.IsVirtualDerivation();
				accessible = ac;
			}

			else
			{
				virtualDerivation = virtualDerivation && clh.IsVirtualDerivation();

				// �� ���������� ������� ������� ����� ������� ����� ���������,
				// ���� �� ���������
				accessible = accessible || ac;
			}				   			
		}


		// ����� �������� �������� �������� ����� �� ��������
		else
			Characterize( base, bcls, ac );
	}		
}


// ������� ������ ���� ������� ������� ������
void SMFManager::FoundSMF()
{
	// ������� �������� �� ������ �������������
	for( ConstructorList::const_iterator p = pClass.GetConstructorList().begin();
		 p != pClass.GetConstructorList().end(); p++ )
	{
		if( IsDefaultConstructor(**p, pClass) )		
			ctorDef.first ? (ctorDef.second = true) : (void)(ctorDef.first = *p);
		
		else if( IsCopyConstructor(**p, pClass) )
			ctorCopy.first ? (ctorCopy.second = true) : (void)(ctorCopy.first = *p);
	}

	// ��������� ����������
	dtor.first = pClass.GetDestructor();

	// ������� ���� �������� �����������
	NameManager nm("operator =", &pClass, false);	
	if( nm.GetRoleCount() != 0 )
	{
		for( RoleList::const_iterator p = nm.GetRoleList().begin();
			 p != nm.GetRoleList().end(); p++ )
		{
			INTERNAL_IF( (*p).second != R_CLASS_OVERLOAD_OPERATOR );
			const ClassOverloadOperator &coo = 
				static_cast<const ClassOverloadOperator &>( * (*p).first );

			// ���������, ���� �������� �����������, ��������� ���
			if( IsCopyOperator(coo, pClass) )		
				copyOperator.first ? 
					(void)(copyOperator.second = true) : (copyOperator.first = &coo);
		}
	}
}
