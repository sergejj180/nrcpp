// ���������� ����������� � �-��� - Translator.cpp

#pragma warning(disable: 4786)
#include <nrc.h>

using namespace nrc;

#include "Limits.h"
#include "Application.h"
#include "LexicalAnalyzer.h"
#include "Object.h"
#include "Scope.h"
#include "Class.h"
#include "Manager.h"
#include "Parser.h"
#include "Body.h"
#include "Checker.h"
#include "ExpressionMaker.h"
#include "Translator.h"



// ������� ��������� ��������
int TemporaryObject::temporaryCounter = 0;

// ���������� ��� ������� ��������� ��� ���������� ��������. ���������
// ����. ������� ���������: ����������, �����������, ���������, �������������� (���������)
const string &TranslatorUtils::GenerateScopeName( const SymbolTable &scope )
{
	static string rbuf;	
	const SymbolTable *psc = &scope;
	
	// ��������� �� ��������� ������� ��������� � ��������������
	while( psc->IsLocalSymbolTable() )
	{
		psc = &static_cast<const LocalSymbolTable *>(psc)->GetParentSymbolTable();
		INTERNAL_IF( psc->IsGlobalSymbolTable() );
	}

	for( rbuf = "";; )
	{		
		// ���������, ���� ��������������, ����������� ��� ������� � ������ �����
		if( psc->IsFunctionSymbolTable() )
		{
			rbuf = static_cast<const FunctionSymbolTable *>(psc)->
				GetFunction().GetName() + '_' + rbuf;
			psc = &static_cast<const FunctionSymbolTable *>(psc)->GetParentSymbolTable();
		}

		// ���� ���������
		else if( psc->IsClassSymbolTable() )
		{
			rbuf = static_cast<const ClassType *>(psc)->GetName() + '_' + rbuf;
			psc = &static_cast<const ClassType *>(psc)->GetSymbolTableEntry();
		}

		// ���� �����������
		else if( psc->IsNamespaceSymbolTable() )
		{
			rbuf = static_cast<const NameSpace *>(psc)->GetName() + '_' + rbuf;
			psc = &static_cast<const NameSpace *>(psc)->GetSymbolTableEntry();
		}

		// ���� ����������, ������� ������ �� ��������
		else if( psc->IsGlobalSymbolTable() )
			return rbuf;
		
		// ����� �����������
		else
			INTERNAL("'TranslatorUtils::GenerateScopeName' - ����������� ������� ���������");
	}

	return rbuf;	// kill warning
}


// ������������� ��� ��� ����������� ��������������
string TranslatorUtils::GenerateUnnamed( )
{
	static int unnamedCount = 1;
	CharString dig( unnamedCount++ );
	return string("unnamed") + dig.c_str();
}


// ������� ��������� ���������� ���������
PCSTR TranslatorUtils::GenerateOperatorName( int op )
{	
	PCSTR opBuf;
	switch( op )
	{
	case KWNEW:				opBuf = "new";		  break;
	case -KWDELETE:		
	case KWDELETE:			opBuf = "delete";	  break;
	case OC_NEW_ARRAY:		opBuf = "newar";	  break;
	case -OC_DELETE_ARRAY:  
	case OC_DELETE_ARRAY:	opBuf = "deletear";	  break;
	case OC_FUNCTION:		opBuf = "fn";		  break;
	case OC_ARRAY:			opBuf = "ar";		  break;
	case PLUS_ASSIGN:		opBuf = "plusasgn";   break;
	case MINUS_ASSIGN:		opBuf = "minasgn";       break;
	case MUL_ASSIGN:		opBuf = "mulasgn";       break;
	case DIV_ASSIGN:		opBuf = "divasgn";       break;
	case PERCENT_ASSIGN:	opBuf = "percasgn";      break;
	case LEFT_SHIFT_ASSIGN:	opBuf = "leftshftasgn";  break;
	case RIGHT_SHIFT_ASSIGN:opBuf = "rightshftasgn"; break;
	case AND_ASSIGN:		opBuf = "andasgn";       break;
	case XOR_ASSIGN:		opBuf = "xorasgn";       break;
	case OR_ASSIGN:			opBuf = "orasgn";		 break;
	case LEFT_SHIFT:		opBuf = "lshft";	     break;
	case RIGHT_SHIFT:		opBuf = "rshft";		 break;
	case LESS_EQU:			opBuf = "lessequ";		 break;
	case GREATER_EQU:		opBuf = "grequ";		 break;
	case EQUAL:				opBuf = "equ";			 break;
	case NOT_EQUAL:			opBuf = "notequ";		 break;
	case LOGIC_AND:			opBuf = "logand";		 break;
	case LOGIC_OR:			opBuf = "logor";		 break;
	case -INCREMENT:
	case INCREMENT:			opBuf = "inc";      break;
	case -DECREMENT:
	case DECREMENT:			opBuf = "dec";      break;
	case ARROW:				opBuf = "arrow";    break;
	case ARROW_POINT:		opBuf = "arpoint";  break;
	case DOT_POINT:			opBuf = "dotpoint"; break;
	case '+':				opBuf = "plus";		break;
	case '-':				opBuf = "minus";	break;
	case '*':				opBuf = "mul";		break;
	case '/':				opBuf = "div";		break;
	case '%':				opBuf = "perc";		break;
	case '^':				opBuf = "xor";		break;
	case '!':				opBuf = "not";		break;
	case '=':				opBuf = "asgn";		break;
	case '<':				opBuf = "less";		break;
	case '>':				opBuf = "greater";  break;	
	case '&':				opBuf = "and";		break;
	case '|':				opBuf = "or";		break;
	case '~':				opBuf = "inv";		break;
	case ',':				opBuf = "coma";		break;
	default:
		INTERNAL("'TranslatorUtils::GenerateOverloadOperatorName' - ����������� ��������");
		break;
	}
	
	return opBuf;
}


// ������������� � ������� ����������, ������ ���� ��� �� typedef � ��
// ����� ���������� �� �������������
void TranslatorUtils::TranslateDeclaration( const TypyziedEntity &declarator, 
		const PObjectInitializator &iator, bool global )
{
	// ���� �� ���������, �� ����������
	if( theApp.IsDiagnostic()				||
		(declarator.IsObject() &&
		 static_cast<const ::Object &>(declarator).GetStorageSpecifier() ==
		 ::Object::SS_TYPEDEF) )
		return;

	DeclarationGenerator dgen(declarator, iator, global);
	dgen.Generate();
	theApp.GetGenerator().GenerateToCurrentBuffer(dgen.GetOutBuffer());
	theApp.GetGenerator().FlushCurrentBuffer();
}


// ������������� ���������� ������
void TranslatorUtils::TranslateClass( const ClassType &cls )
{
	if( theApp.IsDiagnostic() )
		return;
	
	ClassGenerator cgen( cls );
	cgen.Generate();
	theApp.GetGenerator().GenerateToCurrentBuffer(cgen.GetClassBuffer());
	theApp.GetGenerator().GenerateToCurrentBuffer(cgen.GetOutSideBuffer());				
	theApp.GetGenerator().FlushCurrentBuffer();	
}


// ���������� true, ���� ���� ����� ��������� ���
bool SMFGenegator::NeedConstructor( const DataMember &dm )
{
	// ���� �� ��������� ���, ���� ������������ �������� 
	// ���������, ��� ��������������� ����� �� ��������� ������ ������
	if( !dm.GetBaseType().IsClassType()					||
		dm.GetStorageSpecifier() == ::Object::SS_STATIC ||
		dm.GetStorageSpecifier() == ::Object::SS_TYPEDEF )
		return false;
	
	for( int i = 0; i<dm.GetDerivedTypeList().GetDerivedTypeCount(); i++ )
		if( dm.GetDerivedTypeList().GetDerivedType(i)->
			GetDerivedTypeCode() != DerivedType::DT_ARRAY )
			return false;

	// ����� ��� ��������� � ����������� ���������
	return true;
}


// ��������� ������ ��������� �������, ����������� � ������ Generate
void SMFGenegator::FillDependClassList()
{
	// ������� ��������� ������� ������	
	int i = 0;
	for( i = 0; i<pClass.GetBaseClassList().GetBaseClassCount(); i++ )
	{
		const BaseClassCharacteristic &pBase = 
			*pClass.GetBaseClassList().GetBaseClassCharacteristic(i);
		
		AddDependClass( &pBase.GetPointerToClass() );
		if( pBase.IsVirtualDerivation() )
			haveVBC = true;
	}

	// ����� ����� ��������� ������ �������������� (� �� ������) ������� �������,
	// ������� ����� ��������� ���
	for( i = 0; i<pClass.GetMemberList().GetClassMemberCount(); i++ )
		if( const DataMember *dm = dynamic_cast<const DataMember *>(
				&*pClass.GetMemberList().GetClassMember(i)) )
		{
			if( NeedConstructor(*dm) )	
				AddDependClass( static_cast<const ClassType *>(&dm->GetBaseType()) );

			// ������������� ���� ���� � ������ ���� ������� ���������
			// ����� �������������
			if( (dm->GetStorageSpecifier() != ::Object::SS_STATIC   &&
				 dm->GetStorageSpecifier() != ::Object::SS_TYPEDEF) && 
				(dm->GetDerivedTypeList().IsReference() || 
				 ExpressionMakerUtils::IsConstant(*dm)) )
				explicitInit = true;		
		}
}


// ����� �������� �� ������ ��������� ������� � ���������,
// �������� �� ���������, ����������� � ������������ �� ������
// ����������� �����. ����������� ����� �������� ����� ��-�� �� �������
// ����
bool SMFGenegator::CanGenerate( const SMFManagerList &sml, 
			const SMFManager::SmfPair &(SMFManager::*GetMethod)() const ) const
{
	// ���������, ������ ����� � ������
	for( SMFManagerList::const_iterator p = sml.begin(); p != sml.end(); p++ )
	{
		const SMFManager::SmfPair &smp = (*p.*GetMethod)();
		
		// ��������� ������� ����� ����� ��� �������� (first != NULL)
		// � ���������� (second == false)
		if( !smp.first || smp.second )
			return false;

		// � ��������� ������� ��������� ����������� ������
		if( !AccessControlChecker( pClass, static_cast<const ClassType&>(
				smp.first->GetSymbolTableEntry()), *smp.first ).IsAccessible() )
			return false;
	}

	return true;
}



// ���������� true, ���� ����� ����� ���� ������������ ��� �����������.
// ����� �������� �� ������ ������ ��������� ������� � ������������� ������������
SMFGenegator::DependInfo SMFGenegator::GetDependInfo( const SMFManagerList &sml, 
			const SMFManager::SmfPair &(SMFManager::*GetMethod)() const ) const
{
	DependInfo di;

	// ���������, ������ ����� � ������
	for( SMFManagerList::const_iterator p = sml.begin(); p != sml.end(); p++ )
	{
		const Method &meth = *(*p.*GetMethod)().first;
		INTERNAL_IF( !&meth );
		if( !meth.IsTrivial() )
			di.trivial = false;
					
		if( !meth.GetFunctionPrototype().GetParametrList().IsEmpty() &&
			!meth.GetFunctionPrototype().GetParametrList().GetFunctionParametr(0)->IsConst() )
			di.paramIsConst = false;
	}

	// ��� ������������� ����� ���������, ����� � ����� �� ��� �����������,
	// �.�. ������������� ����������� �������
	di.trivial = di.trivial && !pClass.IsPolymorphic() && !haveVBC;
	return di;
}


// ������� true, ���� ����� ��������� �������� �����������
bool SMFGenegator::IsDeclareVirtual(const SMFManagerList &sml, 
			const SMFManager::SmfPair &(SMFManager::*GetMethod)() const ) const
{
	const BaseClassList &bcl = pClass.GetBaseClassList();
	int i = 0;
	for( SMFManagerList::const_iterator p = sml.begin(); 
		 i<bcl.GetBaseClassCount(); i++, p++ )
	{
		INTERNAL_IF( p == sml.end() );
		if( (*p.*GetMethod)().first->IsVirtual() )
			return true;
	}

	return false;
}


// ����� ���������� ������ ����������� ����� � ������� ��� ����������
const DerivedTypeList &SMFGenegator::MakeDTL0() const 
{
	static FunctionParametrList fpl;
	static FunctionThrowTypeList fttl;
	static PDerivedType pfn = new FunctionPrototype(false, false, fpl, fttl, true, false);
	static DerivedTypeList dtl;
	if( dtl.IsEmpty() )
		dtl.AddDerivedType( pfn );
	return dtl;
}


// ����� ���������� ������ ����������� ����� � ������� � �������
// � ��������� ����������� ��� �� ���������� ������ �� ���� �����
// � ������������ ����� �������
const DerivedTypeList &SMFGenegator::MakeDTL1( bool isConst ) const
{		
	static FunctionParametrList fpl;
		
	// ������� ��������
	static DerivedTypeList prmDtl;
	static PDerivedType ref = new Reference;
	if( prmDtl.IsEmpty() )
		prmDtl.AddDerivedType(ref);		

	// ��������� �������� � ������
	fpl.ClearFunctionParametrList();
	fpl.AddFunctionParametr( new Parametr(&pClass, isConst, false, prmDtl, "src",
		&pClass, NULL, false) );

	// ������� ����������� ��� �������
	static FunctionThrowTypeList fttl;	
	static DerivedTypeList dtl;
	dtl.ClearDerivedTypeList();
	dtl.AddDerivedType( new FunctionPrototype(false, false, fpl, fttl, true, false) );
	dtl.AddDerivedType( new Reference );
	return dtl;
}


// ��������� ����������� �� ���������
ConstructorMethod *SMFGenegator::MakeDefCtor( bool trivial ) const 
{
	CharString name(".");	// ��� ������������ ���������� ������ � �����
	name += pClass.GetName().c_str();		
		
	DerivedTypeList dtl = MakeDTL0();
	dtl.AddDerivedType( new Reference );
	return new ConstructorMethod( name, &pClass, &pClass,
		false, false, dtl, true,	// inline - true
		Function::SS_NONE, Function::CC_NON, ClassMember::AS_PUBLIC, false, 
		trivial ? ConstructorMethod::DT_TRIVIAL : ConstructorMethod::DT_IMPLICIT );
}


// ��������� ����������� �����������
ConstructorMethod *SMFGenegator::MakeCopyCtor( bool trivial, bool isConst ) const 
{
	CharString name(".");	// ��� ������������ ���������� ������ � �����
	name += pClass.GetName().c_str();		
	
	return new ConstructorMethod( name, &pClass, &pClass,
		false, false, MakeDTL1(isConst), true,	// inline - true
		Function::SS_NONE, Function::CC_NON, ClassMember::AS_PUBLIC, false, 
		trivial ? Method::DT_TRIVIAL : Method::DT_IMPLICIT );
}


// ��������� ����������
Method *SMFGenegator::MakeDtor( bool trivial, bool isVirtual ) const 
{
	CharString name("~");
	name += pClass.GetName().c_str();

	return new Method( name, &pClass, 
		(BaseType *)&ImplicitTypeManager(KWVOID).GetImplicitType(), false, false, MakeDTL0(),
		true, Function::SS_NONE, Function::CC_NON, ClassMember::AS_PUBLIC, false,
		isVirtual, true, trivial ? Method::DT_TRIVIAL : Method::DT_IMPLICIT );
}


// ��������� �������� ������������
ClassOverloadOperator *SMFGenegator::MakeCopyOperator( 
				Method::DT dt, bool isConst, bool isVirtual ) const 
{
	return new ClassOverloadOperator("operator =", &pClass, &pClass, false, false,
		MakeDTL1(isConst), true, Function::SS_NONE, Function::CC_NON, ClassMember::AS_PUBLIC,
		false, isVirtual, '=', "=", dt);
}


// ������� ���������� ���������� �� ������
void SMFGenegator::DebugMethod( const Method &meth )
{
	string outs = meth.GetTypyziedEntityName().c_str();
	if( meth.IsVirtual() )
		outs = "virtual " + outs;
	if( meth.IsTrivial() )
		outs += ": trivial";

	cout << "* " << outs << endl;
}


// ����� ���������� ��� ������ ����������� ������� �����, ����
// ��� ����������
void SMFGenegator::Generate()
{
	// ���������, ���� ��� �� ����� ������ ������������, �������
	if( !pClass.GetConstructorList().empty()  &&
		smfManager.GetCopyConstructor().first &&
		smfManager.GetDestructor().first	  &&
		smfManager.GetCopyOperator().first  )
		return;

	// ����� ��������� ������ ��������� ������� �������� ������
	FillDependClassList();

	// ����� ���������� ������ ��������� �������, ��������� �� ���
	// ������ ������ ���������� ���� �������
	SMFManagerList dependClsManagers;
	for( ClassTypeList::const_iterator p = dependClsList.begin();
		 p != dependClsList.end(); p++ )
		dependClsManagers.push_back( SMFManager(**p) );

	// ���������, ����� �� ��� ����������� �� ���������,
	// � ������ ���� ������������� �� ���������, ���������� ���������
	if( pClass.GetConstructorList().empty() )
	{
		// ���������, ����� �� ������������� �-�� �� ��������� ��� ������ ������
		if( CanGenerate(dependClsManagers, &SMFManager::GetDefaultConstructor) && !explicitInit )
			pClass.InsertSymbol( MakeDefCtor( 
				GetDependInfo(dependClsManagers, &SMFManager::GetDefaultConstructor).trivial) );
	}

	// ���� ���������� ����������� �� ������������, ����������
	if( smfManager.GetCopyConstructor().first == NULL )
	{
		if( CanGenerate(dependClsManagers, &SMFManager::GetCopyConstructor) && !explicitInit )
		{
			DependInfo di = GetDependInfo(dependClsManagers, &SMFManager::GetCopyConstructor);
			pClass.InsertSymbol( MakeCopyCtor(di.trivial, di.paramIsConst) );
		}
	}

	// ���� �����. ������������� ����������
	if( smfManager.GetDestructor().first == NULL )
	{
		if( CanGenerate(dependClsManagers, &SMFManager::GetDestructor) )
		{
			DependInfo di = GetDependInfo(dependClsManagers, &SMFManager::GetDestructor);
			bool isVirtual = IsDeclareVirtual(dependClsManagers, &SMFManager::GetDestructor);
			pClass.InsertSymbol( MakeDtor(di.trivial, isVirtual) );
		}
	}

	// ���� �����. ������������� �������� ������������
	if( smfManager.GetCopyOperator().first == NULL )
	{
		// ������ ����������� ��������, ����� ���������, ����� � ������
		// �� ���� ������ � ����������� ��������. � ���� ������ ��������
		// ����������� �� ������������
		if( CanGenerate(dependClsManagers, &SMFManager::GetCopyOperator) && !explicitInit )
		{						
			DependInfo di = GetDependInfo(dependClsManagers, &SMFManager::GetCopyOperator);			
			bool isVirtual = IsDeclareVirtual(dependClsManagers, &SMFManager::GetCopyOperator);
			pClass.InsertSymbol( MakeCopyOperator( (di.trivial ?
				Method::DT_TRIVIAL : Method::DT_IMPLICIT), di.paramIsConst, isVirtual) );
		}

		// ����� ��� ��� ����� ������� ������������� ��������, �� ��������
		// ��� ��� �����������
		else
			pClass.InsertSymbol( MakeCopyOperator(Method::DT_UNAVAIBLE, false, false) );
	}
}


// ������������� ����������� ������� �����, ������� �� ������
// ���� �������������
void ClassParserImpl::GenerateSMF()
{
	SMFGenegator(*clsType).Generate();
}


// ������������� �-��� � �������� �����
void CTypePrinter::Generate( )
{	
	BaseType::BT bt = type.GetBaseType().GetBaseTypeCode();
	
	// const - ����������, volatile ���������
	if( type.IsVolatile() )
		baseType = "volatile ";

	// ���� ������� ��� ����������, �������� ���. bool � ������ ��������� 
	// ������������ ����� ���� stdbool.h
	if( bt == BaseType::BT_BOOL    || bt == BaseType::BT_CHAR   ||
		bt == BaseType::BT_WCHAR_T || bt == BaseType::BT_INT    || 
		bt == BaseType::BT_FLOAT   || bt == BaseType::BT_DOUBLE ||
		bt == BaseType::BT_VOID )
		baseType += ImplicitTypeManager(type.GetBaseType()).GetImplicitTypeName().c_str();

	// ����� �������� ��� ������
	else
	{
		INTERNAL_IF( bt != BaseType::BT_CLASS	 &&
					 bt != BaseType::BT_STRUCT	 &&
					 bt != BaseType::BT_ENUM	 &&
					 bt != BaseType::BT_UNION );
		
		// ���� ������������, �������� �� int
		if( bt == BaseType::BT_ENUM )
			baseType += "int";

		// ����� ��������� ��� �����������
		else
		{
			baseType += (bt == BaseType::BT_UNION ? "union " : "struct ");
		
			// ���������� �-���. ���������, ��� �-��� ������������� � ����� ������
			baseType += static_cast<const ClassType &>(type.GetBaseType()).GetC_Name();
		}
	}
	
	// ���� ����� �����������, ����������� ���������� ��������� �� ������������
	// ������. �����. � ������ ������������ ������� �������� "return this;" 
	// � ����� ���� � ��� ��� �������� return
	if( type.IsFunction() && static_cast<const Function &>(type).IsClassMember() &&
		static_cast<const Method &>(type).IsConstructor() )
		baseType += " *";

	if( type.GetDerivedTypeList().GetDerivedTypeCount() != 0 )
	{
		string dtlBuf;
		int ix = 0;
		bool namePrint = id == NULL;

		PrintPointer(dtlBuf, ix, namePrint);		
		outBuffer = baseType + ' ' + dtlBuf;
	}

	else if( id != NULL )		
		outBuffer = baseType + ' ' + id->GetC_Name();

	// ������� �������
	if( outBuffer[outBuffer.length()-1] == ' ' )
		outBuffer.erase(outBuffer.end()-1);
	if( outBuffer[0] == ' ' )
		outBuffer.erase(outBuffer.begin());	
}


// �������� ���������� ����������� ���� � ��������� �� � �����
void CTypePrinter::PrintPointer( string &buf, int &ix, bool &namePrint )
{
	bool isPrint = false;
	for( ; ix < type.GetDerivedTypeList().GetDerivedTypeCount(); ix++, isPrint++ )
	{
		const DerivedType &dt = *type.GetDerivedTypeList().GetDerivedType(ix);
		DerivedType::DT dtc = dt.GetDerivedTypeCode();

		// ������ � ��������� ������������ ����� ��������
		if( dtc == DerivedType::DT_REFERENCE || dtc == DerivedType::DT_POINTER )
			buf = '*' + buf;

		// ������ ������� ��� �� int � ������� �� �����
		else if( dtc == DerivedType::DT_POINTER_TO_MEMBER )
		{	
			// ���� ����� ��������� �� ����-�������, ����������� ��� � ���������
			// �� �������, 
			if( ix != type.GetDerivedTypeList().GetDerivedTypeCount()-1 &&
				type.GetDerivedTypeList().GetDerivedType(ix+1)->GetDerivedTypeCode() ==
				DerivedType::DT_FUNCTION_PROTOTYPE )
			{
				buf = '*' + buf;
				INTERNAL_IF( !thisBuf.empty() );
				PrintThis( static_cast<const PointerToMember&>(dt).GetMemberClassType(),thisBuf);
			}
			
			// ����� ����������� ��� � int � �������
			else
			{
				baseType = "int";
				ix = type.GetDerivedTypeList().GetDerivedTypeCount();
				break;
			}
		}

		else
		{
			// ���� ��� ��� �� ���� �����������, �������� ���
			if( !namePrint )
			{				
				buf = buf + id->GetC_Name();
				namePrint = true;
			}

			if( isPrint )			
				buf = '(' + buf + ')';			

			PrintPostfix( buf, ix );			
		}
	}

	// ���� ��� ��� � �� ���� ����������, ��������	
	if( !namePrint )
	{				
		buf = buf + id->GetC_Name();
		namePrint = true;
	}
}


// �������� ����������� ����������� ���� � ��������� � �����
void CTypePrinter::PrintPostfix( string &buf, int &ix )
{
	int i;
	for( ; ix < type.GetDerivedTypeList().GetDerivedTypeCount(); ix++)
	{
		const DerivedType &dt = *type.GetDerivedTypeList().GetDerivedType(ix);
		DerivedType ::DT dtc = dt.GetDerivedTypeCode();

		if( dtc == DerivedType::DT_FUNCTION_PROTOTYPE )
		{
			const FunctionPrototype &fp =  static_cast<const FunctionPrototype &>(dt);
			buf += '(';

			// ���� ����� this-�����, ������ ������� ������� �������� this ���
			// ������ � �������� �����
			if( !thisBuf.empty() )
			{
				buf += thisBuf;
				thisBuf = "";
				if( !fp.GetParametrList().IsEmpty() || fp.IsHaveEllipse() )
					buf += ", ";
			}

			for( i = 0;	i<fp.GetParametrList().GetFunctionParametrCount(); i++ )
			{
				const Parametr &prm = *fp.GetParametrList().GetFunctionParametr(i);

				// ���� �������� ���������� (������ ���), ���������, ����� � ���������
				// ����� ���� ������ ���				
				CTypePrinter prmPrinter(prm, id ? &prm : NULL);
				prmPrinter.Generate();

				// ���������� ������������� ��������
				if( prm.IsHaveRegister() )
					buf += "register ";
				buf += prmPrinter.GetOutBuffer();
							
				if( i < fp.GetParametrList().GetFunctionParametrCount()-1 )
					buf += ", ";
			}						
			
			if( fp.IsHaveEllipse() )			
				buf += i == 0 ? "..." : ", ...";
			buf += ')';		
		}

		else if( dtc == DerivedType::DT_ARRAY )		
		{
			if( dt.GetDerivedTypeSize() > 0 )
				buf = buf + '[' + CharString( dt.GetDerivedTypeSize() ).c_str() + ']';
			else
				buf += "[]";
		}

		// ����� ����� ������� �������� ���������, ������ �������
		else
		{
			INTERNAL_IF( dtc != DerivedType::DT_REFERENCE &&
						 dtc != DerivedType::DT_POINTER &&
						 dtc != DerivedType::DT_POINTER_TO_MEMBER );

			bool nm = true;
			PrintPointer(buf, ix, nm);						 
		}
	}
}


// ������ ���������� ��������� ��� ������������ ������
Method::VFData::VFData( int vtIndex, const Method &thisMeth )
	: vtIndex(vtIndex), 
	rootVfCls(static_cast<const ClassType &>(thisMeth.GetSymbolTableEntry())) 
{
	INTERNAL_IF( !thisMeth.IsVirtual() );
	PTypyziedEntity ent = new TypyziedEntity(thisMeth);
	INTERNAL_IF( !ent->GetDerivedTypeList().IsFunction() );
	const_cast<DerivedTypeList &>(ent->GetDerivedTypeList()).PushHeadDerivedType(
		new Pointer(false, false));
	
	CTypePrinter ctp(*ent, thisMeth);
	castBuf = '(' + (ctp.Generate(), ctp.GetOutBuffer()) + ')';
}


// ������������� ��������� ������
string ClassGenerator::GenerateHeader( const ClassType &cls )
{
	string header;
	header = cls.GetBaseTypeCode() == BaseType::BT_UNION ? "union " : "struct ";
	header += cls.GetC_Name();

	// �������� ��������� � ��������� ������
	return header;
}


// ������������� ������� ���������� (������)
void ClassGenerator::GenerateBaseClassList( const BaseClassList &bcl )
{
	for( int i = 0; i<bcl.GetBaseClassCount(); i++ )
	{
		const BaseClassCharacteristic &bcc = *bcl.GetBaseClassCharacteristic(i);
		const ClassType &pbcls = bcc.GetPointerToClass();
		string subObjBuf = '\t' + GenerateHeader(pbcls);

		// ���� ����������� ������������, ���������� ���������
		if( bcc.IsVirtualDerivation() )
			subObjBuf += " *";

		// ���������� ��� ����������
		subObjBuf += ' ' + pbcls.GetC_Name() + "so;\n";
		clsBuffer += subObjBuf;
	}
}


// ������������� ������ ������-������ � �������
void ClassGenerator::GenerateMemberList( const ClassMemberList &cml )
{
	for( int i = 0; i<cml.GetClassMemberCount(); i++ )
	{
		const ClassMember &member = *cml.GetClassMember(i);		

		// ���� ������-����, ���������� ����
		if( const DataMember *pobj = dynamic_cast<const DataMember *>(&member) )
		{
			// typedef-�� ����������
			if( pobj->GetStorageSpecifier() == ::Object::SS_TYPEDEF )
				continue;

			// ����� ����������� �����������			
			CTypePrinter tp( *pobj, pobj );
			tp.Generate();

			// ���� static, ���������� �� ��������� ������, ��� extern-����
			if( pobj->GetStorageSpecifier() == ::Object::SS_STATIC )			
			{
				string mbuf = tp.GetOutBuffer();
				// ���� ���� �������������, ������
				if( pobj->IsConst() && pobj->GetObjectInitializer() )
					mbuf = "const " + mbuf + " = " + 
						CharString( (int)*pobj->GetObjectInitializer() );

				// ����� ��������� ��� extern
				else
					mbuf = "extern " + mbuf;

				outSideBuffer += mbuf + ";\n";
			}

			// ���� ������� ����
			else if( pobj->IsBitField() )
			{
				const double *v = pobj->GetObjectInitializer();
				INTERNAL_IF( !v );
				string mbuf = '\t' + tp.GetOutBuffer() + " : " + 
					CharString( (int)*v ).c_str() + ";\n";
				clsBuffer += mbuf;
			}

			// ����� ���������� ��� ������� ����
			else
				clsBuffer += '\t' + tp.GetOutBuffer() + ";\n";				
		}

		// ���� �����, ���������� ��� �� ��������� ������
		else if( const Method *pmeth = dynamic_cast<const Method *>(&member) )
		{
			// ���� ����� ����������� ��� ����������� ��� �����������,
			// �� ���������� ���
			if( pmeth->IsTrivial() || pmeth->IsAbstract() || pmeth->IsUnavaible() )
				continue;

			CTypePrinter tp( *pmeth, *pmeth, 
				pmeth->GetStorageSpecifier() != Function::SS_STATIC );
			tp.Generate();
			const string &mbuf = tp.GetOutBuffer();
			if( pmeth->IsInline() )
				const_cast<string &>(mbuf) = "inline " + mbuf;
			outSideBuffer += tp.GetOutBuffer() + ";\n";
		}
	}
}


// ���������� ����� ���������� �� GenerateVTable
void ClassGenerator::GenerateVTable( const ClassType &cls, string *fnArray, int fnArraySize )
{
	const VirtualFunctionList &vfl = cls.GetVirtualFunctionList();
	for( VirtualFunctionList::const_iterator p = vfl.begin(); p != vfl.end(); p++ )
	{
		INTERNAL_IF( *p == NULL );
		const Method::VFData &vd = (*p)->GetVFData();
		INTERNAL_IF( vd.GetVTableIndex() >= fnArraySize );

	}
}


// ������������� ������� ����������� ������� ��� ������� ������,
// �������� ������� ������
void ClassGenerator::GenerateVTable( )
{
	const int vfcnt = pClass.GetVirtualFunctionCount();
	// ���� ����������� ������� � ������ ���, ������� ����������� 
	// ������� �� ���������
	if( vfcnt == 0 )
		return;
	string *fnBuf = new string[vfcnt];
	GenerateVTable( pClass, fnBuf, vfcnt);

	// �����������, ����� ��� �������� ������ ���� ���������
	for( int i = 0; i<vfcnt; i++ )
		INTERNAL_IF( fnBuf[i].empty() );
	delete [] fnBuf;
}


// ������������� ����������� ������ � ��������� ����������
void ClassGenerator::Generate()
{
	// ���������� ���������
	clsBuffer = GenerateHeader(pClass);

	// ���� ����� �� ��������� ��������, ���������� ������ ��������� � �������
	if( pClass.IsUncomplete() )
	{
		clsBuffer += ';';
		return;
	}

	// ��������� '{'
	clsBuffer += "\n{\n";
	int prevBufSize = clsBuffer.size();

	// ���������� ������� ����������
	GenerateBaseClassList( pClass.GetBaseClassList() );
	// ���������� �����
	GenerateMemberList( pClass.GetMemberList() );

	// �����, ���������, ���� ����� �������� ������ "\n{\n",
	// ������ � ���� ������ �� ������������� � ������� �������������
	// ����, ����� ����� �� ��� ������
	if( clsBuffer.size() == prevBufSize )
		clsBuffer += "\tint _;";

	// ��������� ������
	clsBuffer += "\n};\n\n";
}


// �������������, ������������� �������������. ���������� ����� � �������
// ������������. �� ������������� ����������� ������������
void DeclarationGenerator::GenerateConstructorCall( const ::Object &obj,
		const ConstructorInitializator &ci, string &out )
{
	INTERNAL("!!! ����������� ����� ������������ � ����������� �������");
	INTERNAL_IF( !ci.GetConstructor() || ci.GetConstructor()->IsTrivial() );
}


// ������������� ��������� �� ��������������. � ������ ��������� ������ ���� ����.
// ���� ��������� �������� ���������������� � ����� �������������� ��� ������
// ������������� ����������� �������, ������� true, ����� false
bool DeclarationGenerator::GenerateExpressionInit(  const ::Object &obj,
		const ConstructorInitializator &ci, string &out )
{
	INTERNAL_IF( ci.GetExpressionList().IsNull() || ci.GetExpressionList()->size() > 1 );
	if( ci.GetExpressionList()->empty() )
		return true;

	// ����� ����� ���� �������������, ����������� ���
	const POperand &exp = ci.GetExpressionList()->front();
	INTERNAL_IF( !(exp->IsExpressionOperand() || exp->IsPrimaryOperand()) );

	// ���� ������������� ����� �������������� ��� ������
	// ��� ����������� ������� ���������� � true
	bool directIator = false;

	// ���� ����� �������� �������, ��������������, �������� �� �� ����������������
	if( exp->IsPrimaryOperand() )
	{
		// ��������, �������� �� ������� ����������������
		double rval;
		if( ExpressionMakerUtils::IsInterpretable(exp, rval) ||
			exp->GetType().IsLiteral() )
			directIator = true;		
	}

	INTERNAL(" !!!! ����������� ��� ��������� � �������� �����");

	return directIator;
}


// ������������� ������������� ��� �������. ���� ������������� ��������
// ������������� ��� ������, ����������. ����� ���������� � ����� ���������
// �������������
void DeclarationGenerator::GenerateInitialization( const ::Object &obj )
{
	INTERNAL_IF( iator.IsNull() );

	// ���� ������������� ������������� (���� ���������)
	if( iator->IsConstructorInitializator() )
	{
		const ConstructorInitializator &oci = 
			static_cast<const ConstructorInitializator &>(*iator);
		string out;

		// ���� ������������� ����������
		if( !oci.GetConstructor() || oci.GetConstructor()->IsTrivial() )
		{			
			bool directIator = GenerateExpressionInit(obj, oci, out);
			if( !out.empty() )
			{
				// ���� ������������ ������ ������������� ��� ����� ��������� ������,
				// ���������� ������������� ����� �����
				if( directIator || !global )
					outBuffer += " = " + out;			

				// ����� ���������� � ����� ��������� �������������
				else
					indirectInitBuf = obj.GetC_Name() + " = " + out;
			}
		}

		// �����, �������������� �������������
		else
			GenerateConstructorCall(obj, oci, out);		
	}

	// ����� ������������� �������� ���������
	else
	{
		INTERNAL("�� �����������");
	}
}


// ������������� ����������
void DeclarationGenerator::Generate()
{
	// ����������� ���������� �������
	if( declarator.IsObject() )
	{
		const ::Object &obj = static_cast<const ::Object &>(declarator);
		CTypePrinter ctp( declarator, &obj );
		ctp.Generate();
		outBuffer += ctp.GetOutBuffer();

		// ������� � ���������� ������������ ��������, ���� ����.
		// ��� ��������� ���� ����� �������� ������ extern, static � register
		if( (obj.GetStorageSpecifier() == ::Object::SS_STATIC &&
			 !obj.GetSymbolTableEntry().IsClassSymbolTable())  || 
			obj.GetStorageSpecifier() == ::Object::SS_EXTERN   ||
			(!global && obj.GetStorageSpecifier() == ::Object::SS_REGISTER) )
			outBuffer = ManagerUtils::GetObjectStorageSpecifierName( 
				obj.GetStorageSpecifier() ) + ' ' + outBuffer;

		// ���� ������������� ������������, ����������� ������������� ��� �������
		if( !iator.IsNull() )
			GenerateInitialization(obj);
	}
		
	// ����������� ���������� �������
	else
	{
		// � ������� �� ������ ���� ��������������
		INTERNAL_IF( !iator.IsNull() );
		const Function &fn = static_cast<const Function &>(declarator);
		CTypePrinter ctp( declarator, fn, false );
		ctp.Generate();
		outBuffer += ctp.GetOutBuffer();

		if( fn.GetStorageSpecifier() == Function::SS_STATIC )
			outBuffer = "static " + outBuffer;
		else if( fn.GetStorageSpecifier() == Function::SS_STATIC )
			outBuffer = "extern " + outBuffer;

		// ������� ������������ inline
		if( fn.IsInline() )
			outBuffer = "inline " + outBuffer;
	}

	// � ����� ��������� ';'
	outBuffer += ";\n";
}


// ������� ����� ��������� ������
const TemporaryObject &TemporaryManager::CreateTemporaryObject( const TypyziedEntity &type )
{
	// ���� ����� �������������� �������, ������ ��
	if( unUsed > 0 )
	{
		for( TemporaryList::iterator p = temporaryList.begin();
			 p != temporaryList.end(); p++ )
			if( !(*p).IsUsed() )
				return (*p).SetUsed(), unUsed--, (*p);
		INTERNAL("������ � �������� �������������� ��������� ��������");
	}

	// ����� ������� ��������� ������ ������� � ���������� ����������
	INTERNAL_IF( !genBuffer.empty() );
	TemporaryObject temporary(type);
	static LocalSymbolTable lst(GetScopeSystem().GetGlobalSymbolTable());
	Identifier id( temporary.GetName().c_str(), &lst );

	// ���������� ����������
	CTypePrinter ctp( temporary.GetType(), &id );
	genBuffer = (ctp.Generate(), ctp.GetOutBuffer());

	// ��������� ��������� ������ � ������
	temporaryList.push_front(temporary);
	return temporaryList.front();
}


// ���������� ��������� ������
void TemporaryManager::FreeTemporaryObject( TemporaryObject &tobj ) 
{
	tobj.SetUnUsed();
	unUsed++;

	// ���� ������ ����� ��������� ��� � �� ����������� ����������,
	// ������� ������� ����������� �������
	const TypyziedEntity &type = tobj.GetType();
	if( type.GetBaseType().IsClassType() && type.GetDerivedTypeList().IsEmpty() &&
		static_cast<const ClassType &>(type.GetBaseType()).GetDestructor() &&
		!static_cast<const ClassType &>(type.GetBaseType()).GetDestructor()->IsTrivial() )
	{
		// ����� ������ ���� ������ ��� ������������ �������
		INTERNAL_IF( !genBuffer.empty() );
		genBuffer = "__destroy_last_registered_object();";
	}
}


// ����������� ����� ��������� ����
bool ExpressionGenerator::PrintPathToBase( 
		const ClassType &cur, const ClassType &base, string &out )
{
	const BaseClassList &bcl = cur.GetBaseClassList();
	for( int i = 0; i<bcl.GetBaseClassCount(); i++ )
	{
		const BaseClassCharacteristic &bcc = *bcl.GetBaseClassCharacteristic(i);

		// ���� ������ �������, �������� � �������
		if( &bcc.GetPointerToClass() == &base )
		{			
			out += base.GetC_Name() + "so" + (bcc.IsVirtualDerivation() ? "->" : ".");
			return true;
		}

		// ����� ���������, ���� ����� ��������� ���� � ��������, ����� ��������� ���		
		else if( PrintPathToBase(bcc.GetPointerToClass(), base, out) )
		{
			// ��������� ��������� �������
			out = bcc.GetPointerToClass().GetC_Name() + "so" + 
				(bcc.IsVirtualDerivation() ? "->" : ".") + out;			
			return true;
		}
	}

	return false;
}


// ������������� ���� �� �������� ������ � ��������. ���� ������ ���������,
// ������ �� ������������. ���� ������� ����� ����������� � �������� - ���������� ������
const string &ExpressionGenerator::PrintPathToBase( const ClassType &cur, const ClassType &base )
{
	static string out;
	out = "";
	
	// ���� ���������� �������� ������ �������, ������ �� ��������
	if( &cur == &base )	
		return out;	

	INTERNAL_IF( !PrintPathToBase(cur, base, out) );
	return out;
}


// ����������� �������� �������. ���� printThis ��������� �� ��������������
// ����������� ��������� this ��� ������������� � �� typedef ������ ������.
// ���������� �������������� �������������� �������� ��� ����������� �������
const TypyziedEntity &ExpressionGenerator::PrintPrimaryOperand( 
		const PrimaryOperand &po, bool printThis, string &out )
{
	const TypyziedEntity &entity = po.GetType().IsDynamicTypyziedEntity() ?
		static_cast<const DynamicTypyziedEntity &>(po.GetType()).GetOriginal() : po.GetType();

	// ���� ������
	if( entity.IsObject() )
	{
		const ::Object &obj = static_cast<const ::Object &>(entity);

		// � ����� ������ ����������� ������ ���� � ����� � ���� ���������
		// ����������� this
		if( obj.IsClassMember()										&& 
			!static_cast<const DataMember &>(obj).IsStaticMember()  )
		{
			INTERNAL_IF( thisCls == NULL );
			out = (printThis ? "this->" : "") + PrintPathToBase(*thisCls, 
				static_cast<const ClassType &>(obj.GetSymbolTableEntry()) ) + obj.GetC_Name();
		}

		// ����� ������������� ������ ���
		else
			out = obj.GetC_Name();
	}

	// ���� �������, ������ ����������� ���
	else if( entity.IsFunction() )
		out = static_cast<const Function &>(entity).GetC_Name();
	
	// ���� �������� ����������� ���
	else if( entity.IsParametr() )
		out = static_cast<const Parametr &>(entity).GetC_Name();

	// ���� �������, ����������� ����� � ���������
	else if( entity.IsLiteral() )	
		out = static_cast<const Literal &>(entity).GetLiteralValue().c_str();		

	// ���� ��������� ������������, ����������� ����� ��������
	else if( entity.IsEnumConstant() )	
		out = CharString(static_cast<const EnumConstant &>(entity).GetConstantValue()).c_str();	

	// � ��������� ������ �������� ������ this. ��������, ����� this-����� 
	// ������������ � ���� ���������
	else
	{
		INTERNAL_IF( thisCls == NULL || &entity.GetBaseType() != thisCls ||
			!entity.GetDerivedTypeList().IsPointer() || 
			entity.GetDerivedTypeList().GetDerivedTypeCount() != 1 );
		out = "this";
	}

	return entity;
}


// ����������� ������� ���������
void ExpressionGenerator::PrintUnary( const UnaryExpression &ue, string &out )
{
	int op = ue.GetOperatorCode();
	const POperand &opr = ue.GetOperand();
	switch( op )
	{
	case '!':  
	case '~':  
	case '+':  
	case '-': 
	case '*':
		out = (char)op + PrintExpression(opr);
		break;
	case INCREMENT:
		out = PrintExpression(opr) + "++";
		break;
	case DECREMENT:
		out = PrintExpression(opr) + "--";
		break;
	case -INCREMENT:
		out = "++" + PrintExpression(opr);
		break;
	case -DECREMENT:
		out = "--" + PrintExpression(opr);
		break;	
	case GOC_REFERENCE_CONVERSION:		
		// ���������� �������������, �.�. ��������� ������� ��� �������������
		out = PrintExpression(opr);
		break;
	case '&':
		// ���� ����� ������ ��������� �� ����, ���������� offsetof,		
		if( opr->IsPrimaryOperand() && 
			ue.GetType().GetDerivedTypeList().IsPointerToMember() )
		{
			const DataMember *dm = dynamic_cast<const DataMember *>(&opr->GetType());
			INTERNAL_IF( dm == NULL );
			out = "offsetof(" + TranslatorUtils::GenerateClassHeader(
				static_cast<const ClassType &>(dm->GetSymbolTableEntry()) ) + ", ";

			string dmBuf;
			PrintPrimaryOperand( static_cast<const PrimaryOperand&>(*opr), false, dmBuf);
			out += dmBuf + ')';
		}

		// ����� ��������� ��� ���������
		else
			out = "&" + PrintExpression(opr);
		break;
	case KWTYPEID:
	case KWTHROW:
	default:
		INTERNAL( "'ExpressionGenerator::PrintUnary': ����������� ������� ��������");
	}
}


// ����������� ������� ���������
void ExpressionGenerator::PrintBinary( const BinaryExpression &be, string &out )
{
	int op = be.GetOperatorCode();	
	switch( op )
	{
	case '.':
	case ARROW:
		{
		INTERNAL_IF( !be.GetOperand2()->IsPrimaryOperand() );
		out = PrintExpression(be.GetOperand1()) + (op == '.' ? "." : "->");
		string idBuf;
		PrintPrimaryOperand( static_cast<const PrimaryOperand &>(*be.GetOperand2()), 
			false, idBuf);
		out += idBuf;
		break;
		}
	case OC_CAST:
	case GOC_BASE_TO_DERIVED_CONVERSION:
	case GOC_DERIVED_TO_BASE_CONVERSION:
	case KWREINTERPRET_CAST:
	case KWSTATIC_CAST:
	case KWCONST_CAST:
	case KWDYNAMIC_CAST:
	case ARROW_POINT:
	case DOT_POINT:
		INTERNAL("!! �� �����������");
	case OC_ARRAY:
		out = '(' + PrintExpression(be.GetOperand1()) + ")[" +
			PrintExpression(be.GetOperand2() ) + ']';
		break;
	default:
		{
		string opbuf = ExpressionPrinter::GetOperatorName(op);
		out = PrintExpression(be.GetOperand1() ) + ' ' + 
			opbuf + ' ' + PrintExpression(be.GetOperand2());
		}
		break;
	}
}


// ����������� ������� ���������
void ExpressionGenerator::PrintTernary( const TernaryExpression &te, string &out )
{
	INTERNAL_IF( te.GetOperatorCode() != '?' );
	out = PrintExpression(te.GetOperand1()) + " ? " + 
		PrintExpression(te.GetOperand2()) + " : " + PrintExpression(te.GetOperand3());
}


// ����������� ������� ���������
void ExpressionGenerator::PrintFunctionCall( const FunctionCallExpression &fce, string &out )
{
	const POperand &fn = fce.GetFunctionOperand();
	string objBuf;

	// ���� ��������� - ��������� � �����, ��������� ������ ������ ����������
	if( fn->IsExpressionOperand() )
	{
		int op = static_cast<const Expression &>(*fn).GetOperatorCode();
		if( op == '.' || op == ARROW || op == DOT_POINT || op == ARROW_POINT )
			objBuf = PrintExpression(static_cast<const BinaryExpression &>(*fn).GetOperand1());
	}
}


// ����������� ������� ���������
void ExpressionGenerator::PrintNewExpression( const NewExpression &ne, string &out )
{
}


// ����������� ���������� ���������, �������� ����� ��������������� ���������.
// prvOp - ��� ����������� ���������, ���� ��������� �� ���� -1
string ExpressionGenerator::PrintExpression( const POperand &opr )
{
	INTERNAL_IF( opr.IsNull() || !(opr->IsExpressionOperand() || opr->IsPrimaryOperand()) );
	string out;

	// ���� ��� �������� ��������� � ����� ��� ������ ������ �� ����,
	// ���� �������� ����������� ��� ���������� ���� ����������
	if( opr->IsPrimaryOperand() )	
		PrintPrimaryOperand( static_cast<const PrimaryOperand &>(*opr), true, out );		

	// ����� ������������� �� ��� ���������
	else 
	{
		const Expression &exp = static_cast<const Expression &>(*opr);
		if( exp.IsUnary() )
			PrintUnary( static_cast<const UnaryExpression &>(exp), out );
		else if( exp.IsBinary() )
			PrintBinary( static_cast<const BinaryExpression &>(exp), out );
		else if( exp.IsTernary() )
			PrintTernary( static_cast<const TernaryExpression &>(exp), out );
		else if( exp.IsFunctionCall() )
			PrintFunctionCall( static_cast<const FunctionCallExpression&>(exp), out );
		else if( exp.IsNewExpression() )
			PrintNewExpression( static_cast<const NewExpression &>(exp), out );
		else
			INTERNAL( "����������� ��� ���������" );

		// ���� ��������� � �������, ��������� ���
		if( exp.IsInCramps() )
			out = '(' + out + ')';
	}

	// ���� ���� ������, ��������������
	if( opr->GetType().GetDerivedTypeList().IsReference() )
		out = "(*" + out + ')';
	return out;
}


// ������������� ���������
void ExpressionGenerator::Generate()
{
	// ���� ��������� �������� �������� ���������, ����� ����������
	if( exp->IsPrimaryOperand() )
		PrintPrimaryOperand( static_cast<const PrimaryOperand &>(*exp), true, outBuffer );
	else
		outBuffer = PrintExpression(exp);
}
