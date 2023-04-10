// ���������� ���������� ��������� ��������� - ExpressionMaker.cpp

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
#include "Maker.h"
#include "Body.h"
#include "Checker.h"
#include "ExpressionMaker.h"


// ����������� ����, ������� ������������ ����� 0
// ������������ ��� �������� ������������ ��������� 
POperand UnaryOverloadOperatorCaller::null = new PrimaryOperand(false,
		*new Literal((BaseType *)&ImplicitTypeManager(KWINT).GetImplicitType(), true, false,
					DerivedTypeList(), 0) );


// ��������� �������� ����, ������������ � ���������� ���� 'int ()',
// � ���������� ���� ������� �������	
POperand ExpressionMakerUtils::MakeSimpleTypeOperand( int tokcode )
{
	// ������� ������ ���� �����
	INTERNAL_IF( !IS_SIMPLE_TYPE_SPEC(tokcode) );

	TypyziedEntity *te = new TypyziedEntity(
		&const_cast<BaseType &>(ImplicitTypeManager(tokcode).GetImplicitType()), 
		false, false, DerivedTypeList());

	// ������� ������� �������
	return new TypeOperand( *te );
}


// ������� ������� ������� � ���������, ���. � ���������� ����,
// typeid, new. ������������ ����������� ����� ����������
POperand ExpressionMakerUtils::MakeTypeOperand( const NodePackage &np )
{
	INTERNAL_IF( np.GetPackageID() != PC_DECLARATION || 
		np.GetChildPackageCount() != 2 || np.IsErrorChildPackages() || 
		np.GetChildPackage(0)->GetPackageID() != PC_TYPE_SPECIFIER_LIST ||
		np.GetChildPackage(1)->GetPackageID() != PC_DECLARATOR );	

	// ������� ��������� ���������
	Position ep = ParserUtils::GetPackagePosition(&np);
	TempObjectContainer toc( ep, "<���>");

	// �������� ������ �������������� ����
	MakerUtils::AnalyzeTypeSpecifierPkg( ((NodePackage *)np.GetChildPackage(0)), &toc, false );

	// ��������, ���� ������� ��� �� �����, ����� ���������� errorOperand
	if( toc.baseType == NULL )
	{
		theApp.Error(ep, "�� ����� ������� ���");
		return ErrorOperand::GetInstance();
	}

	// �����, ���� ���� ����������, ����������� � ���
	MakerUtils::AnalyzeDeclaratorPkg( ((NodePackage *)np.GetChildPackage(1)), &toc );
	
	// �������� ������� ���
	MakerUtils::SpecifyBaseType( &toc );

	// ������ ���������, ����� � ��������� �� ���� ������ �������������� � 
	// �������������
	if( toc.ssCode != -1 ||	toc.fnSpecCode != -1 || toc.friendSpec )
		theApp.Error(ep, "'%s' - ����������� � ������ ���������", 
			toc.friendSpec ? "friend" : 
			GetKeywordName(toc.ssCode != -1 ? toc.ssCode : toc.fnSpecCode));

	// ��������� �������������� ���	
	if( !CheckerUtils::CheckDerivedTypeList(toc)				||
		!CheckerUtils::CheckRelationOfBaseTypeToDerived(toc, true, true) )
		return ErrorOperand::GetInstance();

	// ������� �������������� �������� �� ������ ��������� � ���������� ������� ���
	return  new TypeOperand(
		*new TypyziedEntity(toc.finalType, toc.constQual, toc.volatileQual, toc.dtl));
}


// ��������, �������� �� ��� �����
bool ExpressionMakerUtils::IsIntegral( const TypyziedEntity &op )
{
	BaseType::BT bt = op.GetBaseType().GetBaseTypeCode();
	if( !(op.GetDerivedTypeList().IsEmpty() ||
		  (op.GetDerivedTypeList().GetDerivedTypeCount() == 1 &&
		   op.GetDerivedTypeList().IsReference()) ) )
		return false;

	return bt == BaseType::BT_INT || bt == BaseType::BT_CHAR ||
		bt == BaseType::BT_BOOL || bt == BaseType::BT_ENUM || bt == BaseType::BT_WCHAR_T;
}


// ��������, �������� �� ��� ��������������
bool ExpressionMakerUtils::IsArithmetic( const TypyziedEntity &op )
{
	BaseType::BT bt = op.GetBaseType().GetBaseTypeCode();
	if( !(op.GetDerivedTypeList().IsEmpty() ||
		  (op.GetDerivedTypeList().GetDerivedTypeCount() == 1 &&
		   op.GetDerivedTypeList().IsReference()) ) )
		return false;

	// ����� �� ����� �����, ���� ������������, ���� float ��� double
	return bt == BaseType::BT_INT || bt == BaseType::BT_CHAR || bt == BaseType::BT_FLOAT ||
		bt == BaseType::BT_DOUBLE ||
		bt == BaseType::BT_BOOL || bt == BaseType::BT_ENUM || bt == BaseType::BT_WCHAR_T;
}


// ���������, ���� ��� �������� ���������� rvalue
bool ExpressionMakerUtils::IsRvaluePointer( const TypyziedEntity &op )
{
	// ���� ���������, ���� ������, ���� ������ �� ���
	const DerivedTypeList &dtl = op.GetDerivedTypeList();
	if( dtl.IsPointer() || dtl.IsArray() )
		return true;

	else if( dtl.GetDerivedTypeCount() > 1 && dtl.IsReference() &&
		(dtl.GetDerivedType(1)->GetDerivedTypeCode() == DerivedType::DT_POINTER ||
		 dtl.GetDerivedType(1)->GetDerivedTypeCode() == DerivedType::DT_ARRAY) )
		return true;

	else
		return false;
}


// ������� true, ���� ��� �������� ��������, ������� �� �������, ����������
// �� ������� ��� ���������� �� ����-�������
bool ExpressionMakerUtils::IsFunctionType( const TypyziedEntity &type )
{
	const DerivedTypeList &dtl = type.GetDerivedTypeList();
	if( dtl.IsFunction() || 
		(dtl.GetDerivedTypeCount() > 1 &&
		 dtl.GetDerivedType(1)->GetDerivedTypeCode() == DerivedType::DT_FUNCTION_PROTOTYPE) )
		return true;
	return false;
}


// ��������, �������� �� ������� ����������������. � rval ������������ �������� ��������
bool ExpressionMakerUtils::IsInterpretable( const POperand &op, double &rval )
{
	// �������� �������
	if( !op->IsPrimaryOperand() )
		return false;

	const TypyziedEntity &te = static_cast<const PrimaryOperand &>(*op).GetType();
	if( !IsArithmetic(te) )
		return false;

	// ������ ������� ������ ���� ��������� ��� ���������� ������������,
	// ��� ����������� ��������, ������� ��������������� ����������� ���������
	if( te.IsLiteral() )
	{
		rval = atof( static_cast<const Literal &>(te).GetLiteralValue().c_str() );
		return true;
	}

	else if( te.IsEnumConstant() )
	{
		rval = static_cast<const EnumConstant &>(te).GetConstantValue();
		return true;
	}

	// ���� ������, ������� ��������� ����� ��������� �� ���������������
	else if( te.IsObject() && te.IsConst() )
	{
		const double *obi = static_cast<const ::Object &>(te).GetObjectInitializer();
		if( obi == NULL )
			return false;
		rval = *obi;
		return true;
	}

	// ����� ������� false; ������� �� �������� ����������������
	return false;
}


// ��������, �������� �� ������� lvalue
bool ExpressionMakerUtils::IsLValue( const POperand &op )
{
	if( op->IsPrimaryOperand() )
		return static_cast<const PrimaryOperand &>(*op).IsLvalue();

	else if( op->IsExpressionOperand() )
		return static_cast<const Expression &>(*op).IsLvalue();	

	else
		return false;
}


// ���������, �������� �� �������, �������������� lvalue, 
// ���� ��� ������� ������ � ���������� false
bool ExpressionMakerUtils::IsModifiableLValue( 
				const POperand &op, const Position &errPos, PCSTR opName )
{
	try {
		// ������� ���������, �������� �� ������� lvalue �����.
		if( !IsLValue(op) )
			throw op;
		
		// ������ ��������� ��� ���		
		const TypyziedEntity &type = op->GetType();		

		// ��� ������ ����, ���� ����������, ���� �������������� �����
		if( IsArithmetic(type) )
		{
			if( type.IsConst() )
				throw type;
		}
		
		// ����� ���� ������ ������ ��� ������ ��� ����, ��������� �������������
		else if( type.GetDerivedTypeList().IsEmpty()			 ||
			(type.GetDerivedTypeList().GetDerivedTypeCount() == 1 &&
			 type.GetDerivedTypeList().IsReference()) )
		{
			if( type.IsConst() )
				throw type;
		}

		// ����� ��� ������ ���� ����������
		else if( IsRvaluePointer(type) )
		{
			bool ref = type.GetDerivedTypeList().IsReference();
			int dtcnt = type.GetDerivedTypeList().GetDerivedTypeCount();

			// ���� �� ���������, �����
			if( (type.GetDerivedTypeList().GetDerivedType(ref ? 1 : 0))->
				GetDerivedTypeCode() != DerivedType::DT_POINTER )
				throw type;

			// ����� ��������� ������������� ���������
			if( static_cast<const Pointer&>(
				*type.GetDerivedTypeList().GetDerivedType(ref ? 1 : 0)).IsConst() )
				throw type;
		}

		// ���� ��� ��������� �� ����
		else if( type.GetDerivedTypeList().IsPointerToMember() ||
			(type.GetDerivedTypeList().IsReference()				&&
			 type.GetDerivedTypeList().GetDerivedTypeCount() > 1	&&
			 type.GetDerivedTypeList().GetDerivedType(1)->GetDerivedTypeCode() ==
			 DerivedType::DT_POINTER_TO_MEMBER) )
		{
			// ��������� ������������� ��������� �� ����
			if( static_cast<const PointerToMember &>(
			 		*type.GetDerivedTypeList().GetDerivedType(
					type.GetDerivedTypeList().IsReference() ? 1 : 0) ).IsConst() )
				throw type;			
		}

		// ����� ������. �� �� ��������� ��� �� ������, �.�. �� ����� ����
		// ���� ����������, ���� ��������������, � ��� ����������� ��� �������� ����
		else
			throw type;
	
	} catch( const POperand &op ) {
		theApp.Error(errPos, "'%s' - ������� �� �������� lvalue", 
			op->IsPrimaryOperand() ? ExpressionPrinter(op).GetExpressionString().c_str() :
			opName );
		return false;

	} catch( const TypyziedEntity &type ) {
		theApp.Error(errPos, 
			"'%s' - ��� �� �������� �������������� lvalue; '%s' - �������� ������� lvalue", 
			type.GetTypyziedEntityName(false).c_str(), opName );
		return false;
	}

	return true;
}


// ������� ���������� ��� �� ����, ��� ������� ��� ��� �������������� �
// ���������� ��������� � ���� ����� ��������� ��������
TypyziedEntity *ExpressionMakerUtils::DoResultArithmeticOperand( const TypyziedEntity &op1,
		const TypyziedEntity &op2 )
{
		// ����� ��� �������� ������� ������������� � ����������� ����
	const BaseType &bt1 = op1.GetBaseType(), 
				   &bt2 = op2.GetBaseType();
	BaseType::BT btc1 = bt1.GetBaseTypeCode(),
				 btc2 = bt2.GetBaseTypeCode();
	int tbtc = (btc1 == BaseType::BT_FLOAT || btc1 == BaseType::BT_DOUBLE || 
		btc2 == BaseType::BT_FLOAT || btc2 == BaseType::BT_DOUBLE) ? KWDOUBLE : KWINT;
	int sign = -1, size = -1;
	if( tbtc == KWINT && (bt1.IsUnsigned() || bt2.IsUnsigned()) )
		sign = KWUNSIGNED;
		
	// ������������� ������
	if( tbtc == KWINT && (bt1.IsLong() || bt2.IsLong()) )
		size = KWLONG;

	// ����� ���� �������������� ��� double, long ����� ������ � ���
	// ������, ���� �� � double
	else if( tbtc == KWDOUBLE && 
		((btc1 == BaseType::BT_DOUBLE && bt1.IsLong()) ||
		 (btc2 == BaseType::BT_DOUBLE && bt2.IsLong())) )
		size = KWLONG;

	// ������� ��������
	return new TypyziedEntity( 
		(BaseType*)&ImplicitTypeManager(tbtc, sign, size).GetImplicitType(),
		false, false, DerivedTypeList() );
}


// ���������� ����� ����, ���� ��� �������� ���������, ������ ������
TypyziedEntity *ExpressionMakerUtils::DoCopyWithIndirection( const TypyziedEntity &type )
{
	TypyziedEntity *rval = new TypyziedEntity(type);
	if( rval->GetDerivedTypeList().IsReference() )
		const_cast<DerivedTypeList &>(rval->GetDerivedTypeList()).PopHeadDerivedType();
	return rval;
}


// ��������� ����������� ������������ �� ���������, ������������ �����������, 
// ����������� �� ����������. ������������ ��� ��������� ��� �������������
// �������
bool ExpressionMakerUtils::ObjectCreationIsAccessible( const ClassType &cls, 
		const Position &errPos, bool ctor, bool copyCtor, bool dtor )
{
	bool flgs[] = { ctor, copyCtor, dtor };
	for( int i = 0; i<3; i++ )
	{
		const Method *meth = NULL;
		if( !flgs[i] )
			continue;

		// ��������� ����������
		if( i == 2 )
		{		
			if( cls.GetDestructor() == NULL )
			{
				theApp.Error( errPos, 
					"'~%s()' - ���������� �� ��������; �������� ������� ����������",
					cls.GetName().c_str());
				return false;
			}

			meth = cls.GetDestructor();		
		}

		else
		{
			const ConstructorList &ctorLst = cls.GetConstructorList();
		
			// ���� ����������� ���� ��� ����������. ���� � ������������
			// ������ ���� �������� � ����������
			for( ConstructorList::const_iterator p = ctorLst.begin(); 
					 p != ctorLst.end(); p++ )
			{
				const ConstructorMethod &cm = **p;
				if( ctor )
				{
					if( cm.GetFunctionPrototype().GetParametrList().
						GetFunctionParametrCount() > 0 && 
						!cm.GetFunctionPrototype().GetParametrList().
						 GetFunctionParametr(0)->IsHaveDefaultValue() )
						continue;
					
					// �� ����� ���� ���������������
					if( meth )
					{
						theApp.Error( errPos,
							"��������������� ����� '%s' � '%s'; �������� ������� ����������",
							meth->GetTypyziedEntityName().c_str(), 
							cm.GetTypyziedEntityName().c_str() );
						return false;
					}
					else
						meth = &cm;
				}

				else if( copyCtor )
				{
					// �-��� ������ ��������� ���� ��������
					int cnt = cm.GetFunctionPrototype().GetParametrList().
							  GetFunctionParametrCount() ;
					const ConstructorMethod *temp = NULL;
					if( (cnt == 1)												||						
						(cnt > 1 && cm.GetFunctionPrototype().GetParametrList().
									GetFunctionParametr(1)->IsHaveDefaultValue()) )
					{
						const Parametr &prm = *cm.GetFunctionPrototype().GetParametrList().
											  GetFunctionParametr(0);
						if( &prm.GetBaseType() == &cls &&
							prm.GetDerivedTypeList().IsReference() &&
							prm.GetDerivedTypeList().GetDerivedTypeCount() == 1 )
							temp = &cm;
					}
					
					else if(cnt == 0 && cm.GetFunctionPrototype().IsHaveEllipse())
						temp = &cm;
						
					else
						continue;

					// ���������, ���� ����� ����������� �����������, ����� ���������,
					// ����� �� ���� ���������������
					if( temp )
					{
						if( meth )
						{
							theApp.Error( errPos,
								"��������������� ����� '%s' � '%s'; �������� ������� ����������",
								meth->GetTypyziedEntityName().c_str(), 
								temp->GetTypyziedEntityName().c_str() );

							return false;
						}
						else
							meth = temp;
					}					
				}
			}

			// ���� ����������� �� ������, ������� ������
			if( meth == NULL )
			{
				theApp.Error( errPos, 
					"'%s' - %s �� ��������; �������� ������� ����������",
					(string(cls.GetName().c_str()) + 
					(ctor ? "()" : "(const " + string(cls.GetName().c_str()) + "&)")).c_str() , 
					ctor ? "����������� �� ���������" : "����������� �����������");
				return false;
			}		
			
			// �����, ���� �����������, ��������� �������� �� �������� �������
			// ������������ ������
			else if( meth->IsConstructor() && 
				static_cast<const ClassType&>(meth->GetSymbolTableEntry()).IsAbstract() )
			{
				theApp.Error( errPos, 
					"�������� ������� ������ '%s' ����������; ����� �������� �����������",
					static_cast<const ClassType&>(meth->GetSymbolTableEntry()).
					GetQualifiedName().c_str());
				return false;			
			}
		}

		INTERNAL_IF( meth == NULL );
		AccessControlChecker acc( 
			GetCurrentSymbolTable().IsLocalSymbolTable() ? 
			GetScopeSystem().GetFunctionalSymbolTable() : GetCurrentSymbolTable(), cls, *meth);
		if( !acc.IsAccessible() )
		{
			theApp.Error( errPos, (string("\'") + meth->GetTypyziedEntityName().c_str() + 
				"\' - ����������; �������� � �������� ������� ����������").c_str());
			return false;
		}
	}

	return true;
}


// �������������� � rvalue, ����������, ����� ��� ��� ��������� �������� 
// � �� void. ����� ���������, ����� ������� ��� Primary ��� Expression
bool ExpressionMakerUtils::ToRValue( const POperand &val, const Position &errPos )
{
	if( val->IsErrorOperand() )
		return true;

	// ��� �������� �� ��������
	if( val->IsOverloadOperand() || val->IsTypeOperand() )
	{
		theApp.Error(errPos, "���������� �������� 'rvalue' �� '%s'",
			val->IsTypeOperand() ? "����" : "������������� �������" );
		return false;
	}

	// ����� �������� ��� � ���������
	const BaseType &bt = val->GetType().GetBaseType();
	if( bt.GetBaseTypeCode() == BaseType::BT_VOID ||
		(bt.IsClassType() && static_cast<const ClassType&>(bt).IsUncomplete()) )
	{
		theApp.Error(errPos, "���������� �������� 'rvalue' �� '%s'",
			bt.GetBaseTypeCode() == BaseType::BT_VOID ? "void" : 
			"�������������� ������" );
		return false;
	}

	return true;
}

// �������������� � rvalue ��� �����������. ����������� ������������� �����,
// ���� ����������� ��� ������ ��� ���������
bool ExpressionMakerUtils::ToRvalueCopy( const POperand &val, const Position &errPos )
{
	if( val->IsErrorOperand() )
		return true;

	// ��� �������� �� ��������
	if( val->IsOverloadOperand() || val->IsTypeOperand() )
	{
		theApp.Error(errPos, "���������� �������� 'rvalue' �� '%s'",
			val->IsTypeOperand() ? "����" : "������������� �������" );
		return false;
	}

	// ����� �������� ��� � ���������
	const BaseType &bt = val->GetType().GetBaseType();
	if( bt.GetBaseTypeCode() == BaseType::BT_VOID )
	{
		theApp.Error(errPos, "���������� �������� 'rvalue' �� 'void'");
		return false;
	}

	else if( bt.IsClassType() && static_cast<const ClassType&>(bt).IsUncomplete() )
	{
		if( val->GetType().GetDerivedTypeList().IsPointer() ||
			val->GetType().GetDerivedTypeList().IsReference() )
			;
		else
		{
			theApp.Error(errPos, "���������� �������� 'rvalue' �� '�������������� ������'");			
			return false;
		}
	}

	return true;
}


// ����� ������� �������������� � �������� ���� � ������������ ���������
static bool ToCastTypeConverter( POperand &op, const Position &errPos, 
	const string &opName, OperatorCaster::ACC castCategory, PCSTR tname, 
	bool categoryChecker(const TypyziedEntity &) )
{
	// ������� ������ ������ ������������ ��������� ����, ��� �������
	// ����������� ����� ������� ���� ������� (� ������������)
	INTERNAL_IF( !(op->IsExpressionOperand() || op->IsPrimaryOperand()) );
	register const TypyziedEntity &type = op->GetType();

	// ���� ��� ���������, �������� ������������� � ������� ��������� ����������
	if( ExpressionMakerUtils::IsClassType(type) )
	{
		OperatorCaster opCaster( castCategory, type );
		opCaster.ClassifyCast();

		// ���� �������������� ���������� ������� ������
		if( !opCaster.IsConverted() )
		{
			if( !opCaster.GetErrorMessage().empty() )
				theApp.Error( errPos, opCaster.GetErrorMessage().c_str() );
			else
				theApp.Error( errPos, 
					"'%s' - ����� �� �������� ��������� �������������� � '%s ���'",
				static_cast<const ClassType&>(type.GetBaseType()).GetQualifiedName().c_str(),
				tname);
			return false;
		}

		// ����� ��������� ���������� ��������������, 
		opCaster.DoCast(op, op, errPos);	
		return true;		// �������������� ������� ���������
	}

	// ����� ��������� ������, �������� �� ��� ������
	if( !categoryChecker(type) )
	{
		theApp.Error( errPos,
			"'%s' - �� %s ���; '%s' - �������� ������� %s ���",
			type.GetTypyziedEntityName(false).c_str(), tname, opName.c_str(), tname );
		return false;
	}

	// ��� �������� ������, ���������� ���
	return true;
}


// �������������� ���� � ������ ��� ������������� ����. � ������
// ���� ������������� ��������, ���������� �������, ����� ������� ������
// � ���������� NULL. ������������ ��� �������� ���������. �����
// ��������� ����������� �������������� �������� � rvalue
bool ExpressionMakerUtils::ToIntegerType( 
		POperand &op, const Position &errPos, const string &opName )
{
	return ToCastTypeConverter(op, errPos, opName, OperatorCaster::ACC_TO_INTEGER, 
		"�����", ExpressionMakerUtils::IsIntegral );
}


// �������������� ���� � ��������������� ����
bool ExpressionMakerUtils::ToArithmeticType(
		POperand &op, const Position &errPos,  const string &opName )
{
	return ToCastTypeConverter(op, errPos, opName, OperatorCaster::ACC_TO_ARITHMETIC, 
		"��������������", ExpressionMakerUtils::IsArithmetic );
}


// �������������� ���� � ���� ���������
bool ExpressionMakerUtils::ToPointerType( 
		POperand &op, const Position &errPos,  const string &opName )
{
	return ToCastTypeConverter(op, errPos, opName, OperatorCaster::ACC_TO_POINTER, 
		"��������", ExpressionMakerUtils::IsRvaluePointer );
}


// �������������� ���� � ��������� ���� 
bool ExpressionMakerUtils::ToScalarType( 
		POperand &op, const Position &errPos,  const string &opName )
{
	return ToCastTypeConverter(op, errPos, opName, OperatorCaster::ACC_TO_SCALAR, 
		"��������", ExpressionMakerUtils::IsScalarType);
}

// �������������� � ��������������� ���� ��� ���������
bool ExpressionMakerUtils::ToArithmeticOrPointerType( POperand &op, 
		const Position &errPos, const string &opName )
{
	return ToCastTypeConverter(op, errPos, opName, OperatorCaster::ACC_TO_ARITHMETIC_OR_POINTER, 
		"�������������� ��� ��������", ExpressionMakerUtils::IsArithmeticOrPointer);
}


// ���������, ���� ������������� ������-���� ������, 
// ������������ ��� this, ����� ������� ������ � ������� false
int ExpressionMakerUtils::CheckMemberThisVisibility( 
		const POperand &oper, const Position &errPos, bool printError )
{
	// ������� ������ ���� ������-������
	const TypyziedEntity &te = oper->GetType().IsDynamicTypyziedEntity()		 ?
		static_cast<const DynamicTypyziedEntity&>(oper->GetType()).GetOriginal() :
		oper->GetType();
	const Identifier *member = NULL;

	// ���� �������
	if( te.IsFunction() &&
		static_cast<const Function&>(te).IsClassMember() &&
		static_cast<const Function&>(te).GetStorageSpecifier() != Function::SS_STATIC)
		member = &static_cast<const Function &>(te);

	// ���� ������
	else if( te.IsObject() &&
		static_cast<const ::Object &>(te).IsClassMember() &&
		static_cast<const ::Object &>(te).GetStorageSpecifier() != ::Object::SS_STATIC)
		member = &static_cast<const ::Object &>(te);

	// ����� �� ����. �����. �������� this �� �����, �����. 1
	else
		return 1;

	// �������� ����� ����� � ��������� ����� ��������� ����� 
	// ������� ������� ��������� � ����� ������
	const ClassType &mcls = static_cast<const ClassType&>(member->GetSymbolTableEntry());
	const SymbolTable &st = GetCurrentSymbolTable().IsLocalSymbolTable() ?
		GetScopeSystem().GetFunctionalSymbolTable() : GetCurrentSymbolTable();

	// ���������, ���� ��������������, �� ����� ���������� ��������
	if( st.IsFunctionSymbolTable() &&
		static_cast<const FunctionSymbolTable&>(st).GetFunction().IsClassMember() &&
		static_cast<const FunctionSymbolTable&>(st).GetFunction().GetStorageSpecifier() !=
		 Function::SS_STATIC )
	{
		// �������� �����, � �������� ����������� ����� � ������� �� ���������,
		// ����� ������ ���� ��������� � ������� �����, ����
		// ���� ����������� ��� ����
		const Method &curMeth = static_cast<const Method &>(
			static_cast<const FunctionSymbolTable&>(st).GetFunction());
		const ClassType &cc = static_cast<const ClassType &>(curMeth.GetSymbolTableEntry());
		if( &cc == &mcls ||
			DerivationManager( mcls, cc ).IsBase() )
			return 0;
	}

	// ���� ������� �� ����, ������ ������� ������, � ���������� -1,
	// ��� ������� ������
	if( printError )
		theApp.Error(errPos, 
		"'%s' - ���� �� ����� �������������� � ������� ������� ���������; ����������� 'this'",
		member->GetQualifiedName().c_str());
	return -1;
}


// ��������� ������������ ������������� �������. ������������ ������
// ��� � ������ ���������������. �� ���������������� ��������, ����� ���
// ������� �� �����������. 
ExpressionMakerUtils::InitAnswer ExpressionMakerUtils::CorrectObjectInitialization( 
	const TypyziedEntity &obj, const PExpressionList &initList, bool checkDtor, 
	const Position &errPos )
{
	INTERNAL_IF( initList.IsNull() );

	// ������� ��������, ���� ��� ���������, ������ ��������� ����� �-��.
	// ���� ����� ������ ��������� ��������, ������ ��������� �-�� �� ���������
	bool clsType = obj.GetBaseType().IsClassType(), array = false;int i;
	for( int i = 0; i<obj.GetDerivedTypeList().GetDerivedTypeCount(); i++ )
	{
		if( obj.GetDerivedTypeList().GetDerivedType(i)->GetDerivedTypeCode() !=
			DerivedType::DT_ARRAY )
		{
			clsType = false; 
			break;
		}
			
		else if( i == obj.GetDerivedTypeList().GetDerivedTypeCount()-1 )
			array = true;
	}

	// ��������� ������ ���������������, ����� � ��� �� ���� �����, �������������
	// �������, error operand'��, ������ ������ ��� this
	for( i = 0; i<initList->size(); i++ )
	{
		const POperand &op = initList->at(i);
		if( op->IsErrorOperand() )
			return false;
	}

	// ������� ��������, ���� ����� ������, ������ ������ ������������� ������ ���� ������
	if( array && !initList->empty() )
	{
		theApp.Error(errPos, "������ ��������������� ������ ���� ������ ��� �������");
		return false;
	}

	// ����� ���������, ���� ��� ���������, ������ ������� ��������� ������� ������������
	if( clsType )
	{	
		// ��������� ������� �������������
		const ClassType &cls = static_cast<const ClassType&>(obj.GetBaseType());
		const ConstructorList &ctorLst = cls.GetConstructorList();
		if( ctorLst.empty() )
		{
			// ���� ����� ������, � �� ����� ������� ����������,
			// �� ������� ������������� �� �����������
			if( obj.IsObject() && static_cast<const ::Object &>(obj).
				GetStorageSpecifier() == ::Object::SS_EXTERN && initList->empty() )
				return true;

			// ����� ������
			theApp.Error(errPos, 
				"'%s' - � ������ ����������� ������������; ������������� ����������",
				cls.GetQualifiedName().c_str());
			return false;
		}

		OverloadFunctionList ofl(ctorLst.size());			
		copy(ctorLst.begin(), ctorLst.end(), ofl.begin());
				
		// ��������� ������� �����. ������������
		OverloadResolutor or(ofl, *initList, NULL);
		const Function *fn = or.GetCallableFunction();
	
		// ���� ����������� �� ������, ������� ������
		if( fn == NULL )
		{
			theApp.Error(errPos, "%s; ������������� ����������",
				or.GetErrorMessage().c_str());
			return false;
		}
		
		// ��������� �������������� ������� �������� � ������� ���
		or.DoParametrListCast(errPos);			

		// �������� ����������� ����������� ����������,				
		FunctionCallBinaryMaker::CheckParametrInitialization( initList,
			fn->GetFunctionPrototype().GetParametrList(), errPos);

		// ����� ��������� ����������� ���� �������
		AccessControlChecker acc( 
			GetCurrentSymbolTable().IsLocalSymbolTable() ? 
			GetScopeSystem().GetFunctionalSymbolTable() : GetCurrentSymbolTable(),
			cls, *fn);
		if( !acc.IsAccessible() )
			theApp.Error( errPos, (string("\'") + fn->GetTypyziedEntityName().c_str() + 
				"\' - ����������� ����������; ������������� ����������").c_str());		

		// ��������� , ����� ����� �� ��� ����������. �������� �������
		// ������������ ������ ����������
		if( cls.IsAbstract() || cls.IsUncomplete() )
		{
			theApp.Error( errPos,
				"�������� ������� ������ '%s' ����������; ����� �������� %s",
				cls.GetQualifiedName().c_str(), cls.IsAbstract() ? 
				"�����������" : "��������" );			
			return false;
		}
		
		// ���� ��������� ��������� ������� �����������
		if( checkDtor )
			ExpressionMakerUtils::ObjectCreationIsAccessible( cls, errPos, false, false, true);

		// ���� ����������� ������������� �������������, ������� ����� � �������������		
		return InitAnswer( static_cast<const ConstructorMethod &>(*fn) );
	}

	// ����� ��������� ������������� ��������� ���� ��-�������������
	else
	{
		// ���� ��������������� ���������, ������������� ����������
		if( initList->size() > 1 )
		{
			theApp.Error(errPos, 
				"�������� ��� '%s' �� ����� ������������������ ����������� ����������",
				obj.GetTypyziedEntityName(false).c_str());	
			return false;
		}

		// ������������� �� ���������
		if( initList->size() == 0 )
			return true;

		// ����� ������������� ���� � ������� ��������� ����
		PCaster pc = AutoCastManager( obj, initList->at(0)->GetType(), true ).RevealCaster();
		pc->ClassifyCast();
		if( !pc->IsConverted() )
		{
			if( pc->GetErrorMessage().empty() )
				theApp.Error(errPos, 
					"���������� �������� '%s' � '%s'; ������������� ����������",
					obj.GetTypyziedEntityName(false).c_str(), 
					initList->at(0)->GetType().GetTypyziedEntityName(false).c_str());
			else
				theApp.Error(errPos, "'�������������' - %s", pc->GetErrorMessage().c_str());
			return false;
		}

		pc->DoCast(initList->at(0), const_cast<POperand&>(initList->at(0)), errPos);
		return true;
	}
}


// ������� ����������� �����, ���� NULL, ���� ������������� �������� �������
// ����������
POperand UnaryOverloadOperatorCaller::Call() const
{
	// ������� ������ ����� ���
	if( !(right->IsPrimaryOperand() || right->IsExpressionOperand()) ) 
		return NULL;

	// �������� ������ ���� �������������
	if( opCode == KWSIZEOF || opCode == KWTYPEID || opCode == KWTHROW ||
		abs(opCode) == KWDELETE || abs(opCode) == OC_DELETE_ARRAY )
		return NULL;

	const TypyziedEntity &type = right->GetType();
	bool cor = (type.GetBaseType().IsClassType() || type.GetBaseType().IsEnumType()) &&
		(type.GetDerivedTypeList().IsEmpty()  || 
		 (type.GetDerivedTypeList().GetDerivedTypeCount() == 1 && 
		  type.GetDerivedTypeList().IsReference()) );
	// ��� ����� ������ ���� �������������
	if( !cor )
		return NULL;
	const ClassType *cls = type.GetBaseType().IsClassType() ? 
		&static_cast<const ClassType &>(type.GetBaseType()) : NULL;

	// ���������, ��������� �� ����� ��������� � ���������� ������� ���������	
	OverloadOperatorFounder oof(abs(opCode), opCode != ARROW, cls, errPos);

	// ���� �� ������, ������� NULL
	if( !oof.IsFound() )
		return NULL;

	// ���� �������� ������������, ������� ErrorOperand
	else if( oof.IsAmbigous() )
		return ErrorOperand::GetInstance();

	// ����� �������� ����������, ������ �����
	// ������� ��������, ���� ��� ������ �� �����, ������ � ����� �� ��� ���������
	// ����������� �������. ���� � ����� - ���������������
	if( !oof.GetClassOperatorList().empty() && !oof.GetGlobalOperatorList().empty() )
	{
		PExpressionList plist = new ExpressionList;				

		// ���� �������� ������������ -��������, ������ ������ �������� ��� '0'
		if( opCode == INCREMENT || opCode == DECREMENT )
			plist->push_back( null );

		const Function *fn1, *fn2;
		OverloadFunctionList &cofl = const_cast<OverloadFunctionList &>(
			oof.GetClassOperatorList()), &gofl = const_cast<OverloadFunctionList &>(
			oof.GetGlobalOperatorList());

		// ���� � ������ ��� ����������� ���������, �������� ������ ��
		// ����������
		if( (fn1 = OverloadResolutor(cofl, 
					*plist, &right->GetType()).GetCallableFunction() ) == NULL )					
			cofl.clear();

		// ����� ��������� ����������
		else
		{
			plist->clear();
			plist->push_back(right);			
			if( opCode == INCREMENT || opCode == DECREMENT )
				plist->push_back( null );

			// ���� ����� ���������� ��� �����������, �������� ������ ��
			// ���������
			if( (fn2 = OverloadResolutor(gofl, *plist).GetCallableFunction()) == NULL )
			{
				gofl.clear();
				// � ������ ������ ��������� ������ ���� �������
				cofl.clear();
				cofl.push_back(fn1);
			}


			// � ��������� ������ ���������������
			else
			{
				theApp.Error(errPos,
					"��������������� ����� '%s' � '%s'",
					fn1->GetTypyziedEntityName().c_str(), fn2->GetTypyziedEntityName().c_str());
				return ErrorOperand::GetInstance();
			}
		}					
	}

	// ���� ��������� ������ ����, ������ ����� ������� ������� � ����� ����������
	if( oof.GetClassOperatorList().empty() )
	{
		INTERNAL_IF( oof.GetGlobalOperatorList().empty() );
		POperand pol = new OverloadOperand( oof.GetGlobalOperatorList() );

		// �������� ��� ��������� � ������ �������, ��������� �������
		PExpressionList plist = new ExpressionList;		
		plist->push_back(right);
		if( opCode == INCREMENT || opCode == DECREMENT )
			plist->push_back( null );

		// ������ �����, ��������� �������� �������� ��� ����� �������,
		// ���� ������� �� �������� ��� ������, ������ NULL
		return FunctionCallBinaryMaker(pol, plist, OC_FUNCTION, errPos, true).Make();
	}

	// ����� ���� ������ ���������� ���������� ����, ��������� ����� ������
	else if( oof.GetGlobalOperatorList().empty() )
	{
		INTERNAL_IF( oof.GetClassOperatorList().empty() );
		POperand pol = new OverloadOperand( oof.GetClassOperatorList() );
		 
		// ������� ������� ��������� � �����, ������� ��������, ��� 
		// ��� �������� ����������, �.�. ����� ������ �������
		POperand select = new BinaryExpression( '.', false, right, pol, NULL );
		PExpressionList plist = new ExpressionList;		
		if( opCode == INCREMENT || opCode == DECREMENT )
			plist->push_back( null );

		// ������ ����� 
		return FunctionCallBinaryMaker(select, plist, OC_FUNCTION, errPos, true).Make();
	}

	else
	{
		INTERNAL( "'BinaryOverloadOperatorCaller::Call' - ��� ������ �� �����");
		return NULL;	// kill warning
	}
}


// ������� ����������� �����, ���� NULL, ���� ������������� �������� �������
// ����������
POperand BinaryOverloadOperatorCaller::Call() const
{
	// � ������ ������� ��� �������� ������ ���� ���������� ��� ��������
	if( !( (left->IsPrimaryOperand()  || left->IsExpressionOperand()) &&
		   (right->IsPrimaryOperand() || right->IsExpressionOperand()) ) )
		return NULL;

	// ����� ���������, ���� ��� ��������, ������� �� ����� ���� ����������,
	// �������
	if( opCode == '.' || opCode == DOT_POINT || opCode == OC_CAST ||
		opCode == KWDYNAMIC_CAST || opCode == KWSTATIC_CAST || 
		opCode == KWREINTERPRET_CAST || opCode == KWCONST_CAST )
		return NULL;
	
	// ����� ���� �� ��������� ����� ��������� ��� ��� ������������
	const TypyziedEntity &te1 = left->GetType(), &te2 = right->GetType();
	bool cor1 = (te1.GetBaseType().IsClassType() || te1.GetBaseType().IsEnumType()) &&
		(te1.GetDerivedTypeList().IsEmpty() || (te1.GetDerivedTypeList().
		 GetDerivedTypeCount() == 1 && te1.GetDerivedTypeList().IsReference()) ),
		 cor2 = (te2.GetBaseType().IsClassType() || te2.GetBaseType().IsEnumType()) &&
		(te2.GetDerivedTypeList().IsEmpty() || (te2.GetDerivedTypeList().
		 GetDerivedTypeCount() == 1 && te2.GetDerivedTypeList().IsReference()) );

	// ���� ��� ���� ��������, �����
	if( !cor1 && !cor2 )
		return NULL;

	// ����� ���� ������ ��� ���������, ������ �����
	const ClassType *cls = cor1 && te1.GetBaseType().IsClassType() ? 
		&static_cast<const ClassType &>(te1.GetBaseType()) : NULL;
	bool evrywhere = !(opCode == '=' || opCode == OC_FUNCTION || opCode == ARROW || 
					   opCode == OC_ARRAY);

	// ���������, ��������� �� ����� ��������� � ���������� ������� ���������	
	OverloadOperatorFounder oof(opCode, evrywhere, cls, errPos);

	// ���� �� ������, ������� NULL
	if( !oof.IsFound() )
		return NULL;

	// ���� �������� ������������, ������� ErrorOperand
	else if( oof.IsAmbigous() )
		return ErrorOperand::GetInstance();

	// ���� ���� �� ������� ����, ����� ����� ����� ������� �
	// ������� FunctionCallBinaryMaker.

	// ������� ��������, ���� ��� ������ �� �����, ������ � ����� �� ��� ���������
	// ����������� �������. ���� � ����� - ���������������
	if( !oof.GetClassOperatorList().empty() && !oof.GetGlobalOperatorList().empty() )
	{
		PExpressionList plist = new ExpressionList;				
		plist->push_back(right);

		const Function *fn1, *fn2;
		OverloadFunctionList &cofl = const_cast<OverloadFunctionList &>(
			oof.GetClassOperatorList()), &gofl = const_cast<OverloadFunctionList &>(
			oof.GetGlobalOperatorList());

		// ���� � ������ ��� ����������� ���������, �������� ������ ��
		// ����������
		if( (fn1 = OverloadResolutor(cofl, 
					*plist, &left->GetType()).GetCallableFunction() ) == NULL )					
			cofl.clear();

		// ����� ��������� ����������
		else
		{
			plist->front() = left;
			plist->push_back(right);			

			// ���� ����� ���������� ��� �����������, �������� ������ ��
			// ���������
			if( (fn2 = OverloadResolutor(gofl, *plist).GetCallableFunction()) == NULL )
			{
				gofl.clear();
				// � ������ ������ ��������� ������ ���� �������
				cofl.clear();
				cofl.push_back(fn1);
			}


			// � ��������� ������ ���������������
			else
			{
				theApp.Error(errPos,
					"��������������� ����� '%s' � '%s'",
					fn1->GetTypyziedEntityName().c_str(), fn2->GetTypyziedEntityName().c_str());
				return ErrorOperand::GetInstance();
			}
		}					
	}

	// ���� ��������� ������ ����, ������ ����� ������� ������� � ����� �����������
	if( oof.GetClassOperatorList().empty() )
	{
		INTERNAL_IF( oof.GetGlobalOperatorList().empty() );
		POperand pol = new OverloadOperand( oof.GetGlobalOperatorList() );

		// �������� ��� ��������� � ������ �������, ��������� �������
		PExpressionList plist = new ExpressionList;
		plist->push_back(left);
		plist->push_back(right);

		// ������ ����� 
		return FunctionCallBinaryMaker(pol, plist, OC_FUNCTION, errPos, true).Make();
	}

	// ����� ���� ������ ���������� ���������� ����, ��������� ����� ������
	else if( oof.GetGlobalOperatorList().empty() )
	{
		INTERNAL_IF( oof.GetClassOperatorList().empty() );
		POperand pol = new OverloadOperand( oof.GetClassOperatorList() );
		 
		// ������� ������� ��������� � �����, ������� ��������, ��� 
		// ��� �������� ����������, �.�. ����� ������ �������
		POperand select = new BinaryExpression( '.', false, left, pol, NULL );
		PExpressionList plist = new ExpressionList;		
		plist->push_back(right);

		// ������ ����� 
		return FunctionCallBinaryMaker(select, plist, OC_FUNCTION, errPos, true).Make();
	}

	else
	{
		INTERNAL( "'BinaryOverloadOperatorCaller::Call' - ��� ������ �� �����");
		return NULL;	// kill warning
	}
}


// ���������� ������ �������� ����
int SizeofEvaluator::GetBaseTypeSize( const BaseType &bt ) const
{
	BaseType::BT btc = bt.GetBaseTypeCode();
	if( btc == BaseType::BT_CLASS || btc == BaseType::BT_STRUCT ||
		btc == BaseType::BT_UNION )
		return EvalClassSize( static_cast<const ClassType &>(bt) );

	// ��� ������������� ����, ����� ������������ ���������
	else if( btc == BaseType::BT_ENUM )
		return ENUM_TYPE_SIZE;

	// ������ ���� void ����������
	else if( btc == BaseType::BT_VOID )
	{
		errMsg = "������ ���� void ����������";
		return -1;
	}

	// ����� ���������� ������ � ������� ���������
	else
		return ImplicitTypeManager( bt ).GetImplicitTypeSize();
}


// ���������� ������ ���������, ������ ��� ������������
int SizeofEvaluator::EvalClassSize( const ClassType &cls ) const
{
	bool ucls = cls.GetBaseTypeCode() == BaseType::BT_UNION;
	int sz = 0;

	// ���� ����� �� ��������� ��������, ��� ������
	if( cls.IsUncomplete() )
	{
		errMsg = "����� �� ��������� ��������";
		return -1;
	}

	// ��������� ������ ������� �����, � ����� ������� ������� �������
	const ClassMemberList &cml = cls.GetMemberList();
	for( int i = 0; i<cml.GetClassMemberCount(); i++ )
	{
		const ClassMember &cm = *cml.GetClassMember(i);

		// ���������� ������� �������� ������ ���� ����� ������-����
		if( const DataMember *dm = dynamic_cast<const DataMember *>(&cm) )
		{
			// ����������� ����� � ���� �� ����������� ��� �������� �������
			if( dm->IsStaticMember() || dm->GetStorageSpecifier() == ::Object::SS_TYPEDEF )
				continue;

			SizeofEvaluator se(*dm);
			int msz = se.Evaluate();
			if( msz < 0 )
			{
				errMsg = "���� �� ������ ������ ����� ������������ ���"; 
				return -1;
			}

			// ����� ����������� ����� ������ ���� ��� ����� ��� ��������
			// ����������, ���� ��� �����������
			sz = ucls ? (msz > sz ? msz : sz) : (sz + msz);
		}
	}

	// ���� ����� �����������, �� ��������� ������� ������� ������� �� �����
	INTERNAL_IF( sz < 0 );
	if( ucls )
		return sz > 0 ? sz : EMPTY_CLASS_SIZE;	

	// ��������� ������ ������� �������
	const BaseClassList &bcl = cls.GetBaseClassList();
	int i;
	for( i = 0; i<bcl.GetBaseClassCount(); i++ )
	{
		const BaseClassCharacteristic &bcc = *bcl.GetBaseClassCharacteristic(i);

		// ����������� ������ ��������� �� � ����� ������, � �� ��� ���������. 
		// ������ ������ ������ �������� ������ ��������� �� ���� ������. 
		// ������� ��� ������������ ������������ �������� ������, �����������
		// ����� ������������� �� ������ ���������, � �� �� ������ ������ ������.
		if( bcc.IsVirtualDerivation() )
			sz += DEFAULT_POINTER_SIZE;

		// ����� ����������� ������ ����� �������� ������
		else
			sz += EvalClassSize( bcc.GetPointerToClass() );		
	}

	// ���� ����� ������ ����������� vm-�������, ��������. ��� ������ 
	// ���������� �� 4
	if( cls.IsMadeVmTable() )
		sz += DEFAULT_POINTER_SIZE;

	return sz > 0 ? sz : EMPTY_CLASS_SIZE;
}


// ��������� ������ ����, ���� ��� �����������, ������� -1
int SizeofEvaluator::Evaluate() const
{
	// ���� ��� ����������� �����, ������ ������ �������� ���� � ���
	const DerivedTypeList &dtl = type.GetDerivedTypeList();
	if( dtl.IsEmpty() )
		return GetBaseTypeSize(type.GetBaseType());

	// ����� ���������, � ����� ����� ����� ����
	const DerivedType &dt =  *dtl.GetDerivedType(0);
	DerivedType::DT dc = dt.GetDerivedTypeCode();
			
	if( dc == DerivedType::DT_POINTER )
		return DEFAULT_POINTER_SIZE;

	else if( dc == DerivedType::DT_POINTER_TO_MEMBER )
		return DEFAULT_POINTER_TO_MEMBER_SIZE;

	else if( dc == DerivedType::DT_REFERENCE )
		return DEFAULT_REFERENCE_SIZE;

	else if( dc == DerivedType::DT_FUNCTION_PROTOTYPE )
	{
		errMsg = "������� �� ����� �������";
		return -1;
	}

	// ���� ������� ����
	else if( type.IsObject() && 
		static_cast<const ::Object &>(type).GetStorageSpecifier() == ::Object::SS_BITFIELD )
	{
		errMsg = "������� ���� �� ����� �������";
		return -1;		
	}

	// ����� ������
	int sz = 1;
	for( int i = 0; i<dtl.GetDerivedTypeCount(); i++ )
	{
		const DerivedType &dt =  *dtl.GetDerivedType(i);
		DerivedType::DT dc = dt.GetDerivedTypeCode();

		if( dc == DerivedType::DT_ARRAY )
		{	
			int arsz = static_cast<const Array&>(dt).GetArraySize();			
			if( arsz <= 0 )
			{
				errMsg = "������ ������������ �������";
				return -1;
			}

			else
				sz *= arsz;
		}

		else if( dc ==  DerivedType::DT_POINTER || dc == DerivedType::DT_REFERENCE || 
			dc == DerivedType::DT_POINTER_TO_MEMBER )
			return sz * DEFAULT_POINTER_SIZE;

		else
			INTERNAL("'SizeofEvaluator::Evaluate()' - ������������ ����������� ���");
	}

	// ��� ��� ������ �������, ������� ������ ���� * ������ �������
	return sz * GetBaseTypeSize(type.GetBaseType());
}


// ����������������. ���� ������������� ���������� ������� NULL,
// ����� ��������� �� ����� �������
POperand UnaryInterpretator::Interpretate() const
{
	// �������� �������� ��������
	double opval = 0;

	// � ������ ������� ��������� �� sizeof
	if( op == KWSIZEOF )
	{
		const TypyziedEntity &te = static_cast<const PrimaryOperand &>(*cnst).GetType();
		SizeofEvaluator se(te);
		opval = se.Evaluate();
		if( opval < 0 )
			theApp.Error( errPos,
				"'sizeof' �� �������� ������ ���������; %s", se.GetErrorMessage());

		// ��������� ������� ���� 'unsigned int' � �������� ����������
		Literal *res = new Literal( 
			(BaseType *)&ImplicitTypeManager(KWINT, KWUNSIGNED).GetImplicitType(),
			true, false, DerivedTypeList(), CharString( int(opval) ) );
				
		return new PrimaryOperand( false, *res );		
	}


	// ����� ���� �� sizeof, ��������� ������� �� ������������������
	if( !ExpressionMakerUtils::IsInterpretable( cnst, opval ) )
		return NULL;

	// ����� ��������� ��������, ���������. ������ � ������ � ����������
	// �����������, ������� ���������, ����� ��� ��� �����
	if( op == '!' )
		opval = !opval;

	// ��� ����� ��������, �������� ������ ���� �����
	else if( op == '~' )
	{
		if( !ExpressionMakerUtils::IsIntegral( 
			static_cast<const PrimaryOperand &>(*cnst).GetType()) )
		{
			theApp.Error( errPos, "�������� '~' �������� ������ � ����� �����" );
			opval = 0;
		}

		else
			opval = ~(int)opval;
	}

	else if( op ==  '-' )
		opval = -opval;
	
	// ������������� ������� ��������� ����������
	else
		return NULL;

	// ����� ���� ����������, ������� ����� PrimaryOperand, � ����������
	// ��������
	CharString sval;
	const PrimaryOperand &pop = static_cast<const PrimaryOperand &>(*cnst);
	BaseType::BT bt = pop.GetType().GetBaseType().GetBaseTypeCode();
	if( bt == BaseType::BT_FLOAT ||  bt == BaseType::BT_DOUBLE )
		sval = opval;
	else
		sval = (int)opval;

	// ������� ����� �������
	const TypyziedEntity &te = pop.GetType();
	return new PrimaryOperand( false, *new Literal( 
		const_cast<BaseType *>(&te.GetBaseType()), te.IsConst(), te.IsVolatile(), 
			te.GetDerivedTypeList(), sval) );
			
}


// �������� �����, ������� ���������
POperand BinaryInterpretator::MakeResult( const BaseType &bt1, const BaseType & bt2, double res )
{
	// �������� �� ���� ����� ����������
	BaseType::BT btc1 = bt1.GetBaseTypeCode(), btc2 = bt2.GetBaseTypeCode();
	const BaseType *rbt = NULL;

	// ���� ���� �� ����� double, ������ � ������ double
	if( btc1 == BaseType::BT_DOUBLE || btc2 == BaseType::BT_DOUBLE )
	{
		bool lng = bt1.IsLong() || bt2.IsLong();
		rbt = &ImplicitTypeManager(KWDOUBLE, -1, lng ? KWLONG : -1).GetImplicitType();
	}

	// ���� ���� �� ����� float, ������ � ������
	else if( btc1 == BaseType::BT_FLOAT || btc2 == BaseType::BT_FLOAT )
		rbt = &ImplicitTypeManager(KWFLOAT).GetImplicitType();

	// ����� int
	else
	{
		bool uns = bt1.IsUnsigned() || bt2.IsUnsigned(),
			 lng = bt1.IsLong() || bt2.IsLong();
		rbt = &ImplicitTypeManager(KWINT, uns ? KWUNSIGNED : -1, 
			lng ? KWLONG : -1).GetImplicitType();
	}

	// ���������� �������
	CharString sval;
	if( rbt->GetBaseTypeCode() == BaseType::BT_INT )
		sval = (int)res;
	else
		sval = res;

	return new PrimaryOperand( false, *new Literal( 
		const_cast<BaseType *>(rbt), true, false, DerivedTypeList(), sval) );			
}


// ����������������. ���� ������������� ���������� ������� NULL,
// ����� ��������� �� ����� �������
POperand BinaryInterpretator::Interpretate() const
{
	double val1, val2, res;
	val1 = val2 = res = 0;

	// ��������� �������� �� ������������������
	if( !ExpressionMakerUtils::IsInterpretable( cnst1, val1 ) ||
		!ExpressionMakerUtils::IsInterpretable( cnst2, val2 ) )
		return NULL;

	const PrimaryOperand &pop1 = static_cast<const PrimaryOperand &>(*cnst1), 
			&pop2 = static_cast<const PrimaryOperand &>(*cnst2);

	// ��������� ������������� ��������
	if( (op == '%' || op == '/') && val2 == 0 )
	{
		theApp.Error(errPos, op == '/' ? "������� �� ����" : "������� �� ����");
		return cnst1;
	}

	// � ���� ������� �������� ������ ���� ������
	else if( op == '%' || op == '^' || op == '|' || op == '&' || 
		     op == LEFT_SHIFT || op == RIGHT_SHIFT )
	{
		BaseType::BT bt1 = pop1.GetType().GetBaseType().GetBaseTypeCode(),
					 bt2 = pop2.GetType().GetBaseType().GetBaseTypeCode();

		if( bt1 == BaseType::BT_DOUBLE || bt2 == BaseType::BT_DOUBLE || 
			bt1 == BaseType::BT_FLOAT  || bt2 == BaseType::BT_FLOAT )
		{
			theApp.Error(errPos, "�������� ������ ���� ������ ����");
			return cnst1;
		}
	}

	// ������ ���������, ����� ����� �������� � ��������������	
	switch( op )
	{
	case '+': res = val1 + val2; break;
	case '-': res = val1 - val2; break;
	case '*': res = val1 * val2; break;
	case '/': res = val1 / val2; break;
	case '%': res = (int)val1 % (int)val2; break;
	case '<': res = val1 < val2; break;
	case '>': res = val1 > val2; break;
	case EQUAL:			res = val1 == val2; break;
	case NOT_EQUAL:		res = val1 != val2; break;
	case GREATER_EQU:	res = val1 >= val2; break;
	case LESS_EQU:		res = val1 <= val2; break;
	case LOGIC_AND:		res = val1 && val2; break;
	case LOGIC_OR:		res = val1 || val2; break;
	case '^':			res = (int)val1 ^ (int)val2; break;
	case '|':			res = (int)val1 | (int)val2; break;
	case '&':			res = (int)val1 & (int)val2; break;
	case LEFT_SHIFT:	res = (int)val1 << (int)val2; break;
	case RIGHT_SHIFT:	res = (int)val1 >> (int)val2; break;
	case ',':			res = val2; break;
	default:	
		return NULL;
	}

	return MakeResult(pop1.GetType().GetBaseType(), 
		pop2.GetType().GetBaseType(), res);
}


// ����������������. ���� ������������� ���������� ������� NULL,
// ����� ��������� �� ����� �������
POperand TernaryInterpretator::Interpretate() const
{
	double val1, val2, val3, res;
	val1 = val2 = val3 = res = 0;

	// ��������� �������� �� ������������������
	if( !ExpressionMakerUtils::IsInterpretable( cnst1, val1 ) ||
		!ExpressionMakerUtils::IsInterpretable( cnst2, val2 ) ||
		!ExpressionMakerUtils::IsInterpretable( cnst3, val3 ) )
		return NULL;

	// �������� �������� �� ������� ����� �������� �������������� ���
	const PrimaryOperand &pop1 = static_cast<const PrimaryOperand &>(*cnst2), 
			&pop2 = static_cast<const PrimaryOperand &>(*cnst3);

	// ��������� ���������
	res = val1 ? val2 : val3;

	// ���������� ���
	return BinaryInterpretator::MakeResult(pop1.GetType().GetBaseType(),
		pop2.GetType().GetBaseType(), res);
}


// �������� ��������� 'this', ���� ��������.
POperand ThisMaker::Make()
{
	// ���� ������� ������� ��������� �� ��������� � �� ��������������,
	// ������� ������, ������� 'errorOperand'
	if( !GetCurrentSymbolTable().IsFunctionSymbolTable() &&
		!GetCurrentSymbolTable().IsLocalSymbolTable() )
	{
		theApp.Error(errPos, "'this' � '%s'", 
			ManagerUtils::GetSymbolTableName(GetCurrentSymbolTable()).c_str());
		return ErrorOperand::GetInstance();
	}

	const SymbolTable &fst = GetScopeSystem().GetFunctionalSymbolTable();
	const Function &fn = static_cast<const FunctionSymbolTable &>(fst).GetFunction();

	// ���� ������� �� �������� ������� ��� �������� ����������� �������,
	// ������������� 'this' �����������
	if( !fn.IsClassMember() )
	{
		theApp.Error(errPos, "'this' � '�������-�� �����'");
		return ErrorOperand::GetInstance();
	}

	// ����������� �����
	if( fn.GetStorageSpecifier() == Function::SS_STATIC )
	{
		theApp.Error(errPos, "'this' � '����������� ������'");
		return ErrorOperand::GetInstance();
	}
		
	// ������� �������� �������. this �� �������� lvalue, � �������������� ��������
	// ��������
	return new PrimaryOperand(false, 
		*MakeThis( static_cast<const Method &>(fn) ) );
}


// ������� ������ ��� 'this'
const TypyziedEntity *ThisMaker::MakeThis( const Method &meth ) const
{
	// ������� ��������
	const FunctionPrototype &fp = meth.GetFunctionPrototype();
	
	// ������� ����� � �������� ����������� �����
	const ClassType &cls = static_cast<const ClassType &>(meth.GetSymbolTableEntry());

	// �������� ����������� ��������� � ������� ��� � ������ ������. �����
	DerivedTypeList dtl;
	dtl.AddDerivedType( new Pointer(true, false) );

	// ������ �������� cv-������������� ������
	bool c = fp.IsConst(), v = fp.IsVolatile();

	// �������
	return new TypyziedEntity( const_cast<ClassType *>(&cls), c, v, dtl );
}


// ��������� ���������� ������� � ����� �����
int LiteralMaker::CharToInt( PCSTR chr, bool wide ) const
{
	register PCSTR p;
	extern int isdigit8( int c );
	int r;
	PCSTR end;	// ����� ���������, ������ ��������� �� '\''

	p = wide ? chr+2 : chr + 1;	// ����� '\''
	end = p + 1;
	r = *p;

	if(*p == '\\')
	{
		end = p+2;
		if( *(p+1) == 'x' && *(p+2) == '\'')
		{
			theApp.Error(  literalLxm.GetPos(),
					"����������� 16-������ ������������������ ����� '\\x'");
			return 'x';
		}

		if( *(p+1) == 'x' || isdigit8(*(p+1)) )
		{
			int base = *(p+1) == 'x' ? 16 : 8;
			char *stop;
			PCSTR start = base == 16 ? p+2 : p+1;

			r = strtol( start, &stop, base );

			// ��������� ������������
			if( (errno == ERANGE) || 
				(wide ? r > MAX_WCHAR_T_VALUE : r > MAX_CHAR_VALUE) )
			{
				theApp.Warning( literalLxm.GetPos(),
					"'0x%x' - �������� ������� ������ ��� ���� '%s'",
					r, wide ? "wchar_t" : "char");
				return r;
			}

			if( *stop != '\'' )
			{
				theApp.Error(  literalLxm.GetPos(),
					"'%x' - ����������� ������ � %d-������ ������������������", 
					*stop, base );
				
				return *(p+1);
			}

			else
				return r;
		}

		else
		switch( *(p + 1) )
		{
		case 'n':  r = '\n'; break;
		case 't' : r = '\t'; break;
		case 'v' : r = '\v'; break;
		case 'b' : r = '\b'; break;
		case 'r' : r = '\r'; break;
		case 'f' : r = '\f'; break;
		case 'a' : r = '\a'; break;
		case '\\': r = '\\'; break;
		case '?' : r = '\?'; break;
		case '\'': r = '\''; break;
		case '\"': r = '\"'; break;
		default:
			theApp.Error(  literalLxm.GetPos(),
				"'\\%c' - ������������ ��������� ������������������", *(p+1));
			return *(p+1);
		}
	}

	if( *end == '\'' )
		return r;

	else
	{
		if( wide )
		{
			theApp.Warning( literalLxm.GetPos(),
				"������� ����� ������� ������������ � ��������� 'wchar_t'" ); 
			return r;
		}

		else
		{
			theApp.Error( literalLxm.GetPos(),
				"%s - ������������ ��������� ������������������", chr);		
			return 0;
		}
	}	
}


// ������� PrimaryOperand � �������������� ��������� ���������
// �� ������ �������
POperand LiteralMaker::Make()
{
	// ����������� ��� �������
	register int lc = literalLxm;
				  			   
	// �������������� �������
	const Literal *literal = NULL;
	DerivedTypeList dtl;

	// ������� int, float, double ��������, ������� �� ������� ��������������
	if( lc == INTEGER10 || lc == UINTEGER10 || lc == LFLOAT || lc == LDOUBLE )
	{
		int btype = lc == LFLOAT ? KWFLOAT : (lc == LDOUBLE ? KWDOUBLE : KWINT) ;
		literal = new Literal( 
			const_cast<BaseType*>(
			&ImplicitTypeManager(btype, lc == UINTEGER10 ? KWUNSIGNED : -1).GetImplicitType()),
			true, false, dtl, literalLxm.GetBuf());
	}

	// ������� ������, � ����� const char [N]
	else if( lc == STRING )
	{		
		dtl.AddDerivedType( new Array( literalLxm.GetBuf().length()-1 ) );
		literal = new Literal( 
			const_cast<BaseType*>(&ImplicitTypeManager(KWCHAR).GetImplicitType()),
			false, false, dtl, literalLxm.GetBuf());
	}

	// ������� ������ � ����� const wchar_t [N]
	else if( lc == WSTRING )
	{
		INTERNAL_IF( literalLxm.GetBuf()[0] != 'L' );
		
		dtl.AddDerivedType( new Array( literalLxm.GetBuf().length()-2 ) );
		literal = new Literal( 
			const_cast<BaseType*>(&ImplicitTypeManager(KWWCHAR_T).GetImplicitType()),
			false, false, dtl, literalLxm.GetBuf());
	}
	
	// 16-������ � ������������ ���������
	else if( lc == INTEGER16 || lc == UINTEGER16 ||	lc == INTEGER8 || lc == UINTEGER8 )
	{
		int unsign = lc == INTEGER16 || lc == UINTEGER8 ? KWUNSIGNED : -1;
		int base = lc == INTEGER16 || lc == UINTEGER16 ? 16 : 8;

		CharString val( strtol(literalLxm.GetBuf().c_str(), NULL, base) );
		literal = new Literal( 
			const_cast<BaseType*>(
			&ImplicitTypeManager(KWINT, unsign).GetImplicitType()), true, false, dtl, val );
	}

	// ���������� ��������
	else if( lc == CHARACTER || lc == WCHARACTER )
	{
		CharString val( CharToInt(literalLxm.GetBuf().c_str(), lc == WCHARACTER) );
		literal = new Literal( 
			const_cast<BaseType*>(
			&ImplicitTypeManager(lc == CHARACTER ? KWCHAR : KWWCHAR_T).GetImplicitType()), 
			true, false, dtl, val );

	}

	// ������� ���������
	else if( lc == KWTRUE || lc == KWFALSE )
	{
		CharString val( lc == KWTRUE ? "1" : "0" );
		literal = new Literal( 
			const_cast<BaseType*>(&ImplicitTypeManager(KWBOOL).GetImplicitType()), 
			true, false, dtl, val );
	}

	// ����� ������
	else
		INTERNAL( "'LiteralMaker::Make' ��������� ������� � ����������� �����");
	
	INTERNAL_IF( literal == NULL );		
	return new PrimaryOperand( false, *literal );
}


// �������� ����� � ���������������
IdentifierOperandMaker::IdentifierOperandMaker( const NodePackage &ip,const TypyziedEntity *obj ) 
	: idPkg(ip), errPos( ParserUtils::GetPackagePosition(&ip) ), 
		object(obj), noErrorIfNotFound(false), notFound(false)
{

	INTERNAL_IF( idPkg.GetPackageID() != PC_QUALIFIED_NAME || obj == NULL );

	// ���� ����� ������, ��� ��� ������ ���� ���������. � ��������� ������ �� ����������
	INTERNAL_IF( !obj->GetBaseType().IsClassType() );		
	
	// ������ ������� ������� ���������, ������ ���� ��� ���������,
	// ����� ���������� ����� ������� �� ������� ������� ��������� �
	// ����� ��������� �������������� ����� � �������	
	curST = idPkg.GetChildPackageCount() == 1 ?
			&static_cast<const ClassType&>(obj->GetBaseType()) : NULL;		

	name = ParserUtils::PrintPackageTree( &idPkg );	
}


// �������� ������ ������� ���������, ��� �������
IdentifierOperandMaker::IdentifierOperandMaker( const NodePackage &ip, 
		const SymbolTable *cst, bool neinf ) 
	: idPkg(ip), errPos( ParserUtils::GetPackagePosition(&ip) ), 
		object(NULL), curST(cst), noErrorIfNotFound(neinf), notFound(false)
{
	INTERNAL_IF( idPkg.GetPackageID() != PC_QUALIFIED_NAME );
	name = ParserUtils::PrintPackageTree( &idPkg );
}


// ������� ����������, ��� �������������� ������ ������ � ����������.
// � ������ ���� ��� ����������������� ��� �������� ����������,
// ��� �� ���������
void IdentifierOperandMaker::MakeTempName( const CharString &nam ) const
{
	// ��� ������� �� ������������ �� �������� ���������� ����������,
	// �� ��� �� ����������
	if( !isalpha(nam[nam.size()-1]) && !isdigit(nam[nam.size()-1]) &&
		(nam[nam.size()-1]) != '_' )
		return;

	// ���� ������� ������� ��������� ���������, �� �������
	// ����, �.�. ��� �������� ����� ������
	if( GetCurrentSymbolTable().IsClassSymbolTable() )
		return;

	// ������� �������������
	::Object *obj = new ::Object(nam, &GetCurrentSymbolTable(),
		(BaseType *)&ImplicitTypeManager(KWINT).GetImplicitType(), 
		false, false, DerivedTypeList(), ::Object::SS_NONE);

	// ��������� � �������
	INTERNAL_IF( !GetCurrentSymbolTable().InsertSymbol( obj ) ); 

}


// ���������, ����������� �� 'srcId' ������������� 'destId'
// �� ������� ����� ��������. ���� destId ����������� ������
// 'V', ������� �������� ����������� �� ��������� � curST,
// � srcId ����������� � ������ 'B' �������� ������ 'curST',
// � 'B' ��������� 'V', �� ������������ true
bool IdentifierOperandMaker::HideVirtual( const Identifier *destId, 
										 const Identifier *srcId ) const
{
	INTERNAL_IF( destId == NULL || srcId == NULL || curST == NULL );
	INTERNAL_IF( !curST->IsClassSymbolTable() || 
		!destId->GetSymbolTableEntry().IsClassSymbolTable() ||
		!srcId->GetSymbolTableEntry().IsClassSymbolTable() );

	const ClassType &clsD = *static_cast<const ClassType *>(curST),
					&clsV = static_cast<const ClassType &>(destId->GetSymbolTableEntry()),
					&clsB = static_cast<const ClassType &>(srcId->GetSymbolTableEntry());

	// ���������, ����� 'V' ����������� �� ��������� � 'D'
	if( DerivationManager(clsV, clsD).IsVirtual() )
	{
		// ���������, ����� 'B' ��� ������� �� ��������� � 'D' �
		// 'V' ��� ����������� �� ��������� � 'B'
		DerivationManager dm(clsB, clsD);

		if( !dm.IsBase() || !dm.IsUnambigous() )
			return false;

		// ����� 'V' �������� ����������� �� ��������� � 'B'
		if( DerivationManager(clsV, clsB).IsVirtual() )
			return true;
	}

	return false;
}


// �������� ��������� �� ������, �������� �������� �����. � ������ ����
// � ������ ��������� ������, ��������������� ��� ������������ �������������,
// ���������� false
bool IdentifierOperandMaker::ExcludeDuplicates( RoleList &idList ) const
{	
	// ��������, ����� ������ ������������� �� ��� �������� ����������
	if( idList.front().second == R_NAMESPACE || 
		idList.front().second == R_NAMESPACE_ALIAS )
	{
		theApp.Error( errPos, 
			"'%s' - ������� ��������� �� ����� ������������� � ���������",
			name.c_str() );
		return false;
	}

	// ���� ������������� ����, �������
	if( idList.size() == 1 )
		return true;	
	

	// �������� �� ������, ���������, �������������� ������� ����� ���������
	// ������ ������ �������������, �� ������ �������� ����� ����������� 
	// ��������� ��������
	Identifier *first = idList.front().first;
	register RoleList::iterator p = idList.begin();
	// ���� ��������������
	register Role fr = idList.front().second;

	// ������������� ���������� � ����� 'Identifier *'. ���
	// ��������� ��� ���������������
	try {

	// �������� ����� ������
	p++;
	while( p != idList.end() )	
	{
		// �������� ��������� ������������� � ���������� ��� � ������
		Identifier *next = (*p).first;
		
		// ���� ��������� �����, ������� ���������, �������� �� ��������
		// 'p' �� ������ �������� �������� �����
		if( first == next )
		{
			// ���� ��� �������������� ��������� � �� ���������
			// ������� ���������, �������� next �� ������ � ���������
			if( !first->GetSymbolTableEntry().IsClassSymbolTable() )
			{
				p = idList.erase(p);
				continue;
			}

			// ����� ��� �������������� �������� �������. ����
			// ������������� 'first' �������� �����������, ����������
			// ������������ ��� �����, �� next ����� ��������� ��
			// ������
			if( fr == R_DATAMEMBER )
			{				
				// ������������� �������� ����� ���� 'typedef' ��� 'static'				
				const ::Object &dm = static_cast<const ::Object &>(*first);
				if( dm.GetStorageSpecifier() == ::Object::SS_TYPEDEF ||
					dm.GetStorageSpecifier() == ::Object::SS_STATIC )
					p = idList.erase(p);

				// ����� ���������, ����������� �� ���� ������ 'V',
				// ������� �������� ����������� ������� �������, ������������ 
				// ������� ������� ���������, ������� ������ ���� �������
				else if( curST && 
					DerivationManager( 
						static_cast<const ClassType &>(first->GetSymbolTableEntry()),
						static_cast<const ClassType &>(*curST)).IsVirtual() )
					p = idList.erase(p);

				// ����� ����� ������
				else
					throw next;
			}

			// ���� ����� �����
			else if( fr == R_METHOD || fr == R_CLASS_OVERLOAD_OPERATOR )
			{
				const Method &mt = static_cast<const Method &>(*first);
				if( mt.GetStorageSpecifier() == Function::SS_STATIC )
					p = idList.erase(p);

				// ���� ����� ����������� ������������ ������
				else if( curST && 
					DerivationManager( 
						static_cast<const ClassType &>(first->GetSymbolTableEntry()),
						static_cast<const ClassType &>(*curST)).IsVirtual() )
					p = idList.erase(p);

				// ����� ������
				else
					throw next;

			}

			// ��������� ������������ �� �������� ���������������
			// ��� ����� �� �������� ���������������
			else if( fr == R_CLASS_TYPE || fr == R_ENUM_TYPE ||
					 fr == R_UNION_CLASS_TYPE || fr == R_CLASS_ENUM_CONSTANT )
				p = idList.erase(p);

	
			// ����� ��� �������, ���� ��� ������ ����������� �����������
			// ������� �� ���������� �������
			else 
				throw next;			
		}

		// ����� ��������� �� �����, � ������� ��������� ����� ��
		// ���� �� ��������������� ����������� ������
		else
		{
			// ���� ������������� �������� ����������� ��, ����� � �������
			if( idList.front().second == R_NAMESPACE || 
				idList.front().second == R_NAMESPACE_ALIAS )
			{
				theApp.Error( errPos, 
					"'%s' - ������� ��������� �� ����� ������������� � ���������",
					next->GetQualifiedName().c_str() );
				return false;
			}


			// ���� ������. ��������� � ��������� ������� ���������,
			// �������� �� �� ����������
			const SymbolTable &st1 = first->GetSymbolTableEntry(),
							  &st2 = next->GetSymbolTableEntry();

			// �������� ���� �������������� 'next'
			register Role nr = (*p).second;

			// ���� �������������� 'next' � 'first' �� ����� ������� ���������
			bool stEq = &st1 == &st2;


			// ��� ��������� � ������, �������� ����������� ����������
			if( st1.IsClassSymbolTable() && st2.IsClassSymbolTable() && !stEq )
			{
				// ���� 'next' ����������� 'first', ������� ����������� ������������
				// ������, �� erase(first), first=next, 
				if( HideVirtual( first, next ) )
				{
					idList.erase(idList.begin());
					first = next;
					p++;
					fr = nr;
					continue;
				}

				// ����� ��������, ���� first, ����������� 'next', ����� next
				// ���������
				else if( HideVirtual( next, first ) )				
				{
					p = idList.erase(p);				
					continue;
				}
			}

			// ���������� �� ���� ���������, ���� next �������� �����
			if( nr == R_CLASS_TYPE || nr == R_ENUM_TYPE || nr == R_UNION_CLASS_TYPE )
			{
				// ���� ������ ������� ���������, ��� ���������������
				if( !stEq )
					throw next;

				// ����� fr, ������ ���� �������������� ���������
				INTERNAL_IF( !dynamic_cast<const TypyziedEntity *>(first) );
				p = idList.erase(p);
			}

			// ���� 'next' �������� �-�� (�������, ����������)
			else if( nr == R_FUNCTION		   || nr == R_METHOD ||
					 nr == R_OVERLOAD_OPERATOR || nr == R_CLASS_OVERLOAD_OPERATOR ||
					 nr == R_CONSTRUCTOR )
			{
				// ���� ������ ������� ���������, �� ��������� ������ ����
				// fr ����� ������������� �������� � ������� ��������� �� �����
				if( !stEq )
				{
					if( nr != fr || next->GetSymbolTableEntry().IsClassSymbolTable() )
						throw next;					
				}

				else
				{
					// ������� ��������� ����������, ���� 'fr' �������� �����,
					// ����������� ��� ��������
					if( fr == R_CLASS_TYPE || fr == R_ENUM_TYPE || fr == R_UNION_CLASS_TYPE )
						idList.erase(idList.begin()), first = next, fr = nr;

					// ����� �������� ������ ������������� �������
					else if( fr != nr )
						throw next;
				}

				p++;
			}

			// ���� 'next' �������� �������� (����������)
			else if( nr == R_OBJECT    || nr == R_DATAMEMBER    ||
					 nr == R_PARAMETR  || nr == R_ENUM_CONSTANT ||
					 nr == R_CLASS_ENUM_CONSTANT )
			{
				// ���� ������� ��������� ����������, �� ��������
				// ���������� ������ ����
				if( stEq )
				{
					if( fr == R_CLASS_TYPE || fr == R_ENUM_TYPE || fr == R_UNION_CLASS_TYPE )
						idList.erase(idList.begin()), first = next, fr = nr, p++;
					else
						throw next;
				}

				// ����� ���������������
				else
					throw next;
			}

			// ����� ���������� ������, �.�. ������������� �������� ����������
			else
				INTERNAL( "'IdentifierOperandMaker::ExcludeDuplicates' ���������"
					" ������������� ������������ ����" );

		}	// else

	}	// for 
	
	// ������������� ���������� ��������������� ��� ���������������
	} catch( const Identifier *id ) {
		
		// ������� ������ � �������
		theApp.Error( errPos, "��������������� ����� '%s' � '%s'",
			first->GetQualifiedName().c_str(), id->GetQualifiedName().c_str());
		return false;
	}

	return true;
}


// ������� ������� �� ��������� ���������������� ������
POperand IdentifierOperandMaker::MakeOperand( const QualifiedNameManager &qnm ) const
{
	const RoleList &idList = qnm.GetRoleList();
	
	INTERNAL_IF( idList.empty() );
	Role fr = idList.front().second;
	POperand rval = NULL;

	// � ���� ������ ������� ������ ��� ���
	if( idList.size() == 1 )
	{
		Identifier *id = idList.front().first;

		// ���� ������������� �������� �����, ������� TypeOperand,
		// ����� PrimaryOperand
		if( fr == R_CLASS_TYPE || fr == R_ENUM_TYPE || fr == R_UNION_CLASS_TYPE )
		{
			TypyziedEntity *te = new TypyziedEntity(dynamic_cast<BaseType *>(id),
				false, false, DerivedTypeList() );

			rval = new TypeOperand(*te);
		}

		// ����� ������������� �������� �������������� ���������
		else
		{
			// ���� ������������� �� �������� �������������� ���������,
			// ��� �� ��������������� ��������� (�����, ���)
			TypyziedEntity *te = dynamic_cast<TypyziedEntity *>(id);
			INTERNAL_IF( te == NULL );

			// ���� ������������� �������� �������� � � ������� ������������
			// �������� - typedef, ������� ������� ���. 
			if( te->IsObject() )
			{
				const Object &member = static_cast<const ::Object&>(*te);
				
				// ���� ����� typedef, ������� ������� �������
				if( member.GetStorageSpecifier() == ::Object::SS_TYPEDEF )
					rval = new TypeOperand(*new TypyziedEntity(member));
				
				// ����� ���� �� ����������� � �� mutable, ����������� ������������
				else if( !( member.GetStorageSpecifier() == ::Object::SS_STATIC ||
					member.GetStorageSpecifier() == ::Object::SS_MUTABLE) )
				{
					bool c = false, v = false;

					// ���� ����� ������, �������� ������������ ������� �����
					if( object != NULL )
						c = object->IsConst(), v = object->IsVolatile();

					// ����� ���� ����� ���� � ���� ��������� ����� this,
					// �������� ������������ this
					else if( member.IsClassMember() &&
						(GetScopeSystem().GetCurrentSymbolTable()->IsFunctionSymbolTable() ||
						 GetScopeSystem().GetCurrentSymbolTable()->IsLocalSymbolTable()) )
					{
						const Function &curFn = 
							GetScopeSystem().GetFunctionSymbolTable()->GetFunction();
							;
						if( curFn.IsClassMember() &&
							curFn.GetStorageSpecifier() != Function::SS_STATIC )
							c = curFn.GetFunctionPrototype().IsConst(),
							v = curFn.GetFunctionPrototype().IsVolatile();
					}

					// ������� ����� ����������������� ������
					if( c > member.IsConst() || v > member.IsVolatile() )
						rval = new PrimaryOperand(true, 
							*new DynamicTypyziedEntity(te->GetBaseType(),
								c, v, te->GetDerivedTypeList(), *te));					
				}
			}
			
			// ����� ������� ������� �������������			
			if( rval.IsNull() )	
			{
				// ���� ����� �����, �� ������ ���� ��������
				if( te->IsFunction() && static_cast<const Function *>(te)->IsClassMember() &&
					static_cast<const Method *>(te)->IsUnavaible() )
					theApp.Error(errPos, 
						"'%s' - ����� ������������ �����������", 
						te->GetTypyziedEntityName().c_str());
				rval = new PrimaryOperand(true, *te);
			}
		}

		// ��������� �����������
		if( object )
		{		
			const ClassMember *cm = dynamic_cast<const ClassMember *>(id);
			if( cm && cm->GetAccessSpecifier() != ClassMember::NOT_CLASS_MEMBER )
			{
				AccessControlChecker acc( 
					GetCurrentSymbolTable().IsLocalSymbolTable() ? 
					GetScopeSystem().GetFunctionalSymbolTable() : GetCurrentSymbolTable(),
					static_cast<const ClassType&>(object->GetBaseType()), *cm);

				if( !acc.IsAccessible() )
					theApp.Error( errPos, "'%s' - ���� ���������� � '%s'",
						id->GetQualifiedName().c_str(), 
						ManagerUtils::GetSymbolTableName(GetCurrentSymbolTable()).c_str());			
			}
		}

		else
			CheckerUtils::CheckAccess( qnm, *id, errPos, 
				curST  ? *curST : GetCurrentSymbolTable());
	}

	// ����� ��������������� ���������, ��� ������ ���� ������
	// ������������� �������, �������� ��� ��� ���
	else
	{
		//
		Identifier *id = idList.front().first;

		// ���������, ����� ���� ���� ��������
		if( fr == R_FUNCTION		  || fr == R_METHOD ||
			fr == R_OVERLOAD_OPERATOR || fr == R_CLASS_OVERLOAD_OPERATOR ||
			fr == R_CONSTRUCTOR )
		{
			OverloadFunctionList ofl;

			// ��������� � ������ �������, �������������� ��������
			// ��� ���� ���������
			for( RoleList::const_iterator p = idList.begin(); p != idList.end(); p++ )
			{
				// ���� ������ ���������
				if( (*p).second != fr )
					goto err;
				ofl.push_back( static_cast<Function *>((*p).first) );
			}

			// ������� ������� - ������ ������������� �������
			rval = new OverloadOperand( ofl );
		}

		// ����� ���������������
		else
		{
		err:
			theApp.Error( errPos, "��������������� ����� '%s' � '%s'",
				id->GetQualifiedName().c_str(),
				idList.front().first->GetQualifiedName().c_str());					

			return ErrorOperand::GetInstance();
		}		
	}

	return rval;
}


// ������� �������. �������������� �������� �������������
// �� �������������, ����������� � �������������.
POperand IdentifierOperandMaker::Make()
{
	// � ������ ������� ���� ���, ������ �������� ������ ���� ��-����������
	QualifiedNameManager qnm( &idPkg, curST );

	// ���� ������ �� �������, ������� ������ � ���������� ErrorOperand.
	// ���� ����� �������� �������������� ��� ��������������
	// ������ ������ � ����������� 
	if( qnm.GetRoleCount() == 0 )
	{
		// ��� ������ �������������� ���������, ������ �������� �� �������
		if( !noErrorIfNotFound )
		{
			if( object != NULL )
				theApp.Error(errPos, 
					"'%s' - �� �������� ������ ������ '%s'",
					name.c_str(),
				static_cast<const ClassType&>(object->GetBaseType()).GetQualifiedName().c_str());
			else
				theApp.Error( errPos, "'%s' - �� ��������", name.c_str());

			// ���� ��� �� �������� �����������������, �������
			// ��������� ���������� ���� int, ��� ��������������
			// ����� ������ � ����������
			if( name.find(':') == string::npos )
				MakeTempName(name);
		}

		// ������ ����, ��� ������ �� �������
		notFound = true;
		return ErrorOperand::GetInstance();
	}


	// ����� ��������������� ������, ������� ������ ���������
	// ������� ���������, �.�. ��� ����� ����� ��� ��������������.
	// ���� ������� ������� ��������� �� ����� � �� ���������,
	// curST ��� � �������� ����� ����
	const SymbolTable *st = curST;
	if( curST == NULL )
	{		
		// ��������� ����� ��������� �����, ������� �������� �����
		const SymbolTable &ct = GetCurrentSymbolTable();
		if( ct.IsLocalSymbolTable() )
		{
			const Function *fn = &static_cast<const FunctionSymbolTable &>(
				GetScopeSystem().GetFunctionalSymbolTable()).GetFunction();
			if( fn->IsClassMember() )
				curST = &static_cast<const ClassType &>(fn->GetSymbolTableEntry());
		}

		// ���� �������, ����� ���� �����
		else if( ct.IsFunctionSymbolTable() )
		{
			const Function *fn = &static_cast<FunctionSymbolTable &>(
				GetCurrentSymbolTable()).GetFunction();

			if( fn->IsClassMember() )
				curST = &static_cast<const ClassType &>(fn->GetSymbolTableEntry());	
		}

		// ���� �����
		else if( ct.IsClassSymbolTable() )
			curST = &GetCurrentSymbolTable();
	}


	// ����������� ������ �������� �� ���� ������������� ��������,
	// ������� �������� �������� ����� �� ����� ���� ������������
	if( !ExcludeDuplicates( const_cast<RoleList &>(qnm.GetRoleList()) ) )
		return ErrorOperand::GetInstance();

	// ��������������� �������� ������� ������� ���������
	curST = st;

	// ����� ���� ��� ������ ������������ � �� ���� ��������� ���������,
	// ������� ������� �������. 
	return MakeOperand( qnm );
}


// ��������� ���������� ������������ � ��������� � ���������� ����� ���
PTypyziedEntity TypeCastBinaryMaker::UnsetQualification()
{
	const TypyziedEntity &et = right->GetType();
	DerivedTypeList rdtl;

	// �������� ������ ����������� �����
	for( int i = 0; i<et.GetDerivedTypeList().GetDerivedTypeCount(); i++ )
	{
		const DerivedType &dt = *et.GetDerivedTypeList().GetDerivedType(i);

		// ���� ���������, �������� ��� ������.
		if( dt.GetDerivedTypeCode() == DerivedType::DT_POINTER )
			rdtl.AddDerivedType( new Pointer(false,false) );

		// ����� � ��-��� �� ����
		else if( dt.GetDerivedTypeCode() == DerivedType::DT_POINTER_TO_MEMBER )
			rdtl.AddDerivedType( new PointerToMember( 
				&static_cast<const PointerToMember&>(dt).GetMemberClassType(),
				false,false) );

		// ����� �������� ��� ����
		else
			rdtl.AddDerivedType(et.GetDerivedTypeList().GetDerivedType(i));
	}

	// ������� ���
	return new TypyziedEntity( (BaseType*)&et.GetBaseType(), false, false, rdtl );
}


// ��������� ���������� � ������� static_cast. ������� 0 � ������ ������,
// ������� 1 � ������ ������ � ���������� �������������� �� ���������, 
// -1 - � ������ �������, -2 � ������ ���� ���������� ����� ����������,
// ������� 
int TypeCastBinaryMaker::CheckStaticCastConversion( const TypyziedEntity &expType )
{
	// � ������ ������ ���������, ���� ��� �������� ��������� �� ����� 
	// ��������� ��������������, �.�. �������� ���������,
	// ����� ����� ��� ���������, � ��� ��� �� �����
	const TypyziedEntity &toType = left->GetType(), &fromType = expType;

	// ������ ��������� � ���� ������� ��� �������
	if( toType.GetDerivedTypeList().IsArray() || toType.GetDerivedTypeList().IsFunction() ) 
		return -2;
	
	if( toType.GetBaseType().IsClassType()						&&
		toType.GetDerivedTypeList().IsPointer()					&& 
		toType.GetDerivedTypeList().GetDerivedTypeCount() == 1 )
	{
		// ���������, ����� � ������ ��� ��� ���������� �� �����
		if( fromType.GetBaseType().IsClassType() &&
			((fromType.GetDerivedTypeList().IsPointer()					&& 
			  fromType.GetDerivedTypeList().GetDerivedTypeCount() == 1)     ||
			 (fromType.GetDerivedTypeList().GetDerivedTypeCount() == 2	&&
			  fromType.GetDerivedTypeList().IsReference()				&&
			  fromType.GetDerivedTypeList().GetDerivedType(1)->GetDerivedTypeCode() ==
			  DerivedType::DT_POINTER))
		   )
		{
			// ���������, �������� �� ���� �� ������� ������� ��� �����������
			const ClassType &toCls = static_cast<const ClassType&>(toType.GetBaseType()),
				&fromCls = static_cast<const ClassType&>(fromType.GetBaseType());

			// ������� ������� ������������� �� ������������ � �������
			DerivationManager dtbm(toCls, fromCls);			
			if( dtbm.IsBase() )
			{
				// �������� ������, ����� ����� ��� �����������
				if( !dtbm.IsUnambigous() )
				{
					theApp.Error(errPos, "'(���)' - ������� ����� '%s' ������������",
						toCls.GetQualifiedName().c_str());
					const_cast<POperand&>(right) = ErrorOperand::GetInstance();
					return 1;		// ���������� 1, ����� �� �������� ���. ������
				}

				// ����� ������ ��������������
				const_cast<POperand&>(right) = 
					new BinaryExpression( GOC_DERIVED_TO_BASE_CONVERSION, 
						false, left, right, new TypyziedEntity(left->GetType()) );
				return 1;

			}

			// ����� �������� �� �������� � �����������
			DerivationManager btdm(fromCls, toCls);
			if( btdm.IsBase() )
			{
				// �������� ������, ����� ����� ��� �����������
				if( !btdm.IsUnambigous() || btdm.IsVirtual() )
				{
					theApp.Error(errPos, 
						"'(���)' - ������� ����� '%s' %s",
						fromCls.GetQualifiedName().c_str(), 
						btdm.IsVirtual() ? "�������� �����������" : "������������" );
					const_cast<POperand&>(right) = ErrorOperand::GetInstance();
					return 1;		// ���������� 1, ����� �� �������� ���. ������
				}

				// ����� ������ ��������������
				const_cast<POperand&>(right) = 
					new BinaryExpression( GOC_BASE_TO_DERIVED_CONVERSION, 
						false, left, right, new TypyziedEntity(left->GetType()) );
				return 1;
			}
		}		
	}

	// ������� �������, ������������� � ������� ��������������� ��������������,
	// ���� "T t(e)"
	PCaster pCaster= 
		AutoCastManager(left->GetType(), expType, true, true).RevealCaster();
	Caster &caster = *pCaster;
	caster.ClassifyCast();

	// ���� �������������� ��������, ����������� �������� ��������� �
	// ���������� 1
	if( caster.IsConverted() )
	{
		caster.DoCast(left, const_cast<POperand&>(right), errPos);
		return 1;
	}

	// ����� �������� ������������� �������� ������ ��������
	// �������� �������������� � void	
	if( toType.GetBaseType().GetBaseTypeCode() == BaseType::BT_VOID &&
		toType.GetDerivedTypeList().IsEmpty() )
		return 0;

	// �� lvalue 'B', � 'cv D &'
	if( ExpressionMakerUtils::IsClassType(toType) && 
		ExpressionMakerUtils::IsClassType(fromType) &&
		toType.GetDerivedTypeList().IsReference()   &&
		toType.GetDerivedTypeList().GetDerivedTypeCount() == 1 )
	{		
		if( !ExpressionMakerUtils::IsLValue(right) )
			return -2;

		// ���������, ����� 'B' ��� ����������� �� ����������� ������� ������� 'D'
		DerivationManager dm( static_cast<const ClassType &>(fromType.GetBaseType()),
			static_cast<const ClassType &>(toType.GetBaseType()) );
		
		// ���������, ����� ��� �������, �� �����������, �����������,
		if( !dm.IsBase() || dm.IsVirtual() || !dm.IsUnambigous() )
			return -2;

		// ����� ������ ���������, �������������� � ����������� �����
		const_cast<POperand&>(right) = 
			new BinaryExpression( GOC_BASE_TO_DERIVED_CONVERSION, 
				false, left, right, new TypyziedEntity(left->GetType()) );
			
		// ����� ���������� 1, �.�. ��������� ��� ���������
		return 1;
	}

	// ���� ��� ���� �����, ����� ��� ����� ������������� � ������������
	if( ExpressionMakerUtils::IsIntegral(toType) && ExpressionMakerUtils::IsIntegral(fromType) )
		return 0;

	// ��������� �� void, ����� ������������� � ��������� �� ��. ���
	if( fromType.GetBaseType().GetBaseTypeCode() == BaseType::BT_VOID &&
		fromType.GetDerivedTypeList().GetDerivedTypeCount() == 1 &&
		fromType.GetDerivedTypeList().IsPointer() ) 
	{
		if( toType.GetDerivedTypeList().IsPointer() &&
			toType.GetDerivedTypeList().GetDerivedTypeCount() == 1 )
			return 0;
	}

	// ����� �������, �� ����� �������� ����� reinterpret_cast
	return -1;
}

 
// ��������� ���������� � ������� reinterpret_cast. ������� true � ������ ������	
bool TypeCastBinaryMaker::CheckReinterpretCastConversion( const TypyziedEntity &expType )
{
	const TypyziedEntity &fromType = expType, &toType = left->GetType();
	const DerivedTypeList &fromDtl = fromType.GetDerivedTypeList(), 
						  &toDtl = toType.GetDerivedTypeList();
	
	// ������ ��������� � ���� ������� ��� �������
	if( toDtl.IsArray() || toDtl.IsFunction() ) 
		return false;
	
	// ���� ���� ��� �����, � ������ ��������� ��� ��������, ����
	// ��� ���������
	if( (ExpressionMakerUtils::IsIntegral(fromType) &&
		 ExpressionMakerUtils::IsRvaluePointer(toType))     ||
		(ExpressionMakerUtils::IsIntegral(toType) &&
		 ExpressionMakerUtils::IsRvaluePointer(fromType))   ||
		(ExpressionMakerUtils::IsRvaluePointer(fromType) &&
		 ExpressionMakerUtils::IsRvaluePointer(toType))	)
		 ;

	// ����� ���� ��� ��������� �� ����, ���������, ����� ��� ����
	// ������� ��� �������
	else if( toDtl.IsPointerToMember() && fromDtl.IsPointerToMember() &&
			 ((toDtl.GetDerivedTypeCount() > 1 && toDtl.GetDerivedType(1)->
			   GetDerivedTypeCode() == DerivedType::DT_FUNCTION_PROTOTYPE) +
			  (fromDtl.GetDerivedTypeCount() > 1 && fromDtl.GetDerivedType(1)->
			   GetDerivedTypeCode() == DerivedType::DT_FUNCTION_PROTOTYPE)) != 1 )
		;

	// ���� ������ ������� lvalue, � �������� ��� - ������,
	// �������������� ��������.
	else if( toType.GetDerivedTypeList().IsReference() &&
		((right->IsExpressionOperand() && static_cast<const Expression&>(*right).IsLvalue()) ||
		 (right->IsPrimaryOperand() && static_cast<const PrimaryOperand&>(*right).IsLvalue())) )
		;

	else
		return false;

	return true;
}


// ������� ��������� (���)
POperand TypeCastBinaryMaker::Make()
{
	// ������� ������� ������������ � ����
	PTypyziedEntity unqual = UnsetQualification();
	
	// ����� �������� ������������� � ������� static_cast
	int r = CheckStaticCastConversion(*unqual);
	if( r == 1 )
		return right;

	// ���� 0, ������ ���������
	else if( r == 0 || ( r == -1 && CheckReinterpretCastConversion(*unqual)) )		
		return new BinaryExpression( OC_CAST, left->GetType().GetDerivedTypeList().IsReference(),
			left, right, new TypyziedEntity(left->GetType()));
	
	// � ������, ���� ��� �������������� ����������, ������� ������
	theApp.Error(errPos, "'(���)' - ���������� ������������� '%s' � '%s'",
		right->GetType().GetTypyziedEntityName(false).c_str(),
		left->GetType().GetTypyziedEntityName(false).c_str());
	return ErrorOperand::GetInstance();
}


// �����, �������� ��������� ������� �� ������ � ��������� �� �����������.
// ���� ������� ������������ ��� �� �������, ������� NULL
const Function *FunctionCallBinaryMaker::CheckBestViableFunction( 
	const OverloadFunctionList &ofl, const ExpressionList &pl, const TypyziedEntity *obj ) const
{
	OverloadResolutor or(ofl, pl, obj);
	const Function *fn = or.GetCallableFunction();
	
	// ���� ������� �� �������, ������� ������, ���� ����� �������.
	if( fn == NULL )
	{		
		// ���� ������� �� ������������, ������ �� �� ����������
		if( !or.IsAmbigous() )
			noFunction = true;

		// ���� ����� ����� ��� ���������������, ������� ������
		if( !implicit || !noFunction )
			theApp.Error(errPos, or.GetErrorMessage().c_str());

		// �������
		return NULL;
	}

	// ����� ��������� ����������� ���� �������
	if( fn->IsClassMember() )
	{
		AccessControlChecker acc( 
			GetCurrentSymbolTable().IsLocalSymbolTable() ? 
			GetScopeSystem().GetFunctionalSymbolTable() : GetCurrentSymbolTable(),
			obj ? static_cast<const ClassType&>(obj->GetBaseType())
				: static_cast<const ClassType&>(fn->GetSymbolTableEntry()), *fn);

		if( !acc.IsAccessible() )
			theApp.Error( errPos, 
				"'%s' - ����� ����������; ����� ����������", 
				fn->GetTypyziedEntityName().c_str() );		
	}

	// � ����� ��������� �������������� ������� �������� � ������� ���
	or.DoParametrListCast(errPos); 
	return fn;
}


// ���� 'fn' - ��� ������, ������� ��� ���� ������������� ��������� '()',
// ���� �������� ���������� � ���� ��������� �� �������. ���� ����������
// �������, ������ ����� �������, ����� NULL
POperand FunctionCallBinaryMaker::FoundFunctor( const ClassType &cls ) const
{
	// ���� ��������
	OverloadOperatorFounder oof(OC_FUNCTION, false, &cls, errPos);
	// ���� ������������� �������� ������, ���������� �������� ��������� �������. 
	if( oof.IsFound() )
	{
		// ���� ������������, ������� ������
		if( oof.IsAmbigous() )
			return ErrorOperand::GetInstance();

		// ����� ���������� �������� ��������� ������� ��� ������
		// ���������� � ����������
		return FunctionCallBinaryMaker(
			new BinaryExpression('.', false, fn, new OverloadOperand(oof.GetClassOperatorList()),
			  new TypyziedEntity(fn->GetType()) ),
			  parametrList, OC_FUNCTION, errPos, false).Make();
	}
	
	// ����� ���������� NULL, ��� ����������, ��� �������� �� ������
	else
		return NULL;
}

// ��������� ����������� ����������� � ����������� ������� ���������.
// ��������� ����� ����������� ����������� � ���������� ���� ��������
void FunctionCallBinaryMaker::CheckParametrInitialization( const PExpressionList &parametrList, 
	const FunctionParametrList &formalList, const Position &errPos ) 
{
	// ��������� ������������� ��������� ���������, ��� �������� ����������
	// ������� ��������� ����� ������������. ���������, ����� ��� ���
	// ��������� ����������� ����� � �������� ����������� ����������� � ����������.
	// �� ���������, ������� ��� ��������������� � ������� 
	// ����������� (���� ��� ������), ��������� �� �����
	int pcnt = parametrList->size() < formalList.GetFunctionParametrCount() ?
		parametrList->size() : formalList.GetFunctionParametrCount();

	for( int i = 0; i<pcnt; i++ )
	{
		const Parametr &prm = *formalList[i];
		if( !(prm.GetBaseType().IsClassType() && prm.GetDerivedTypeList().IsEmpty()) )
			continue;

		// ����� ��������� ��������, 
		const ClassType &cls = static_cast<const ClassType&>(prm.GetBaseType());
		
		// ���������, ����� ����� ��� ��������� ��������
		if( cls.IsUncomplete() )
		{
			theApp.Error( errPos, 
				"'%i-��' �������� ����� ��� '%s', ������� �� ���������",
				i+1, cls.GetName().c_str());
			continue;
		}

		// ��������� �� ���� �� ��� ��������������
		const POperand &exp = (*parametrList)[i];
		if( exp->IsExpressionOperand() &&
			static_cast<const Expression&>(*exp).IsFunctionCall() )
		{
			const FunctionCallExpression &fnc = static_cast<const FunctionCallExpression&>(*exp);
			if( &fnc.GetFunctionOperand()->GetType().GetBaseType() == &cls	&&
				dynamic_cast<const ConstructorMethod *>(
					&fnc.GetFunctionOperand()->GetType()) != NULL )
				continue;
		}

		// � ��������� ������, ���������, ����� � ������� ��� �������� �-��� �����������
		// � ����������
		ExpressionMakerUtils::ObjectCreationIsAccessible(cls, errPos, false, true, true);
	}
}


// ��������� �������� �� ������� ������ �������� ������, ���� ���
// ���������� �������� ��� this, � ��������� �� �� ������������ � ��������
void FunctionCallBinaryMaker::CheckMemberCall( const Method &m ) const
{
	// ����������� ����� ��� ����������� ����� �������� ��� 'this'
	if( m.GetStorageSpecifier() == Function::SS_STATIC ||
		m.IsConstructor() )
		return;

	// �������� ����� ������ � ��������� ����� ��������� ����� 
	// ������� ������� ��������� � ����� ������
	const ClassType &mcls = static_cast<const ClassType &>(m.GetSymbolTableEntry());
	const SymbolTable &st = GetCurrentSymbolTable().IsLocalSymbolTable() ?
		GetScopeSystem().GetFunctionalSymbolTable() : GetCurrentSymbolTable();

	// ���������, ���� ��������������, �� ����� ���������� ��������
	if( st.IsFunctionSymbolTable() &&
		static_cast<const FunctionSymbolTable&>(st).GetFunction().IsClassMember() )
	{
		// �������� �����, � �������� ����������� ����� � ������� �� ���������,
		// ����� ������ ���� ��������� � ������� ����������� ������, ����
		// ���� ����������� ��� ����
		const Method &curMeth = static_cast<const Method &>(
			static_cast<const FunctionSymbolTable&>(st).GetFunction());
		const ClassType &cc = static_cast<const ClassType &>(curMeth.GetSymbolTableEntry());
		if( &cc == &mcls ||
			DerivationManager( mcls, cc ).IsBase() )
		{
			// ��������� ������������, ����� ����� ��������� � ����� 
			// ����������������� ������
			if( curMeth.GetFunctionPrototype().IsConst() > m.GetFunctionPrototype().IsConst() ||
				curMeth.GetFunctionPrototype().IsVolatile() > 
						m.GetFunctionPrototype().IsVolatile() )
				theApp.Error( errPos,
					"'%s' - ����� ����� ��������������, ��� this; ����� ����������",
					m.GetQualifiedName().c_str() );
			return;
		}		
	}

	// ���� ������� �� ����, ������ ������� ������
	theApp.Error(errPos, 
		"'%s' - ����� �� ����� ���������� � ������� ������� ���������; ����������� 'this'",
		m.GetQualifiedName().c_str());
}


// ������� ���������. ����� ������� ErrorOperand, ���� ���������� ����������
POperand FunctionCallBinaryMaker::Make()
{
	// ��������� ����������� �������� ����� ������� �������. ����
	// �� ��������� ��������� ��� �������, ����� ���������� ������ �������,
	// ���� ����� �� �������, � ������
	if( fn->IsErrorOperand() )
		return fn;

	// ��������, ���� �� ��������� ��������� ��� ��������� ����,
	// � ����� ���� ���� ��������� - ������������� �������, ���������
	// ���������������
	for( int i = 0; i<parametrList->size(); i++ )	
		if( (*parametrList)[i]->IsErrorOperand() )
			return ErrorOperand::GetInstance();

	// ���� ������� �������� �������� ��������� ��� ����������
	if( fn->IsPrimaryOperand() || fn->IsExpressionOperand() )
	{
		// ����� ����, ���� � ��������, ���� � �������������� �����,
		// �� ��� ������ ��������, ���� ��������� � �������� ������,
		// �� ������ ������� ����� ���� ������� ������������� �������
		// ������� ���� ���������
		if( fn->IsExpressionOperand() )
		{
			const Expression &exp = static_cast<const Expression &>(*fn);
			if( (exp.GetOperatorCode() == '.' || exp.GetOperatorCode() == ARROW) &&
				 static_cast<const BinaryExpression &>(exp).GetOperand2()->IsOverloadOperand() )
			{
				const Function *vf = CheckBestViableFunction(
				   static_cast<const OverloadOperand &>(
					*static_cast<const BinaryExpression &>(exp).GetOperand2()).GetOverloadList(),
					*parametrList, 
					&static_cast<const BinaryExpression &>(exp).GetOperand1()->GetType()
				);

				// ���� ������� �� �������, �������
				if( vf == NULL )
				{
					if( implicit && noFunction )
						return NULL;
					return ErrorOperand::GetInstance();
				}

				// ������� ����������� �������, �������� ����������� ����������� ����������,				
				CheckParametrInitialization(vf->GetFunctionPrototype().GetParametrList());
					
				// ����� ������ ����� ��������� � ���������� ��������				
				const_cast<POperand&>(fn) = new BinaryExpression(exp.GetOperatorCode(), false,
					static_cast<const BinaryExpression &>(exp).GetOperand1(), 
					new PrimaryOperand(false, *vf), new TypyziedEntity(*vf));

				// ������ ����� �������
				PTypyziedEntity rtype = new TypyziedEntity(*vf);
				const_cast<DerivedTypeList&>(rtype->GetDerivedTypeList()).PopHeadDerivedType();
				return new FunctionCallExpression( vf->GetDerivedTypeList().IsReference(),
					fn, parametrList, rtype);
			}			
		}
		
		// � ��������� ������ ������� ����������. ������� ���������, ����
		// ����� ��������� ���, �������� ��������� ����� �������������� ��������� '()'
		if( ExpressionMakerUtils::IsClassType(fn->GetType()) )
		{
			// ���� ������������� �������� ()
			POperand p = 
				FoundFunctor(static_cast<const ClassType&>(fn->GetType().GetBaseType()));

			// ���� �� ������, ������� ������
			if( p.IsNull() )
			{
				if( fn->IsPrimaryOperand() )
					theApp.Error(errPos, "'%s' - ������ �� �������� ��������",
						ExpressionPrinter(fn).GetExpressionString().c_str());
				else
					theApp.Error( errPos, 
						"'%s' - ����� �� �������� �������������� ��������� '()'",
						static_cast<const ClassType&>(fn->GetType().GetBaseType()).
							GetQualifiedName().c_str());
				return ErrorOperand::GetInstance();
			}

			// ����� ������ �������
			return p;
		}

		// ����� ������� ������ ���� ��������, ���������� �� ������� ��� 
		// ���������� �� ����-�������		
		const DerivedTypeList &dtl = fn->GetType().GetDerivedTypeList();
		if( dtl.IsFunction() || 
			(dtl.GetDerivedTypeCount() > 1 && dtl.IsPointer() &&
			 dtl.GetDerivedType(1)->GetDerivedTypeCode() == DerivedType::DT_FUNCTION_PROTOTYPE ))
		{			
			bool isFunction = dynamic_cast<const Function *>(&fn->GetType()) != NULL;
			const FunctionPrototype &fpt = dtl.IsFunction() ?
				static_cast<const FunctionPrototype &>(*dtl.GetDerivedType(0)) :
				static_cast<const FunctionPrototype &>(*dtl.GetDerivedType(1)) ;
		
			// ����� �������������� ���, ��������� ���� ����������,
			OverloadFunctionList ofl;			
			const SmartPtr<Function> tempf = isFunction ? NULL :
			(	!dtl.IsFunction() ? ((DerivedTypeList&)dtl).PopHeadDerivedType() : (void)0,
				new Function( "", &GetCurrentSymbolTable(), 
					(BaseType *)&fn->GetType().GetBaseType(), false, false,
					fn->GetType().GetDerivedTypeList(), false, Function::SS_NONE,
					Function::CC_NON )
			 );
			ofl.push_back( tempf.IsNull() ?
				static_cast<const Function*>(&fn->GetType()) : &*tempf );
					
			// ��������� �������� �����. ���������� �������
			const TypyziedEntity *obj = NULL;
			int opc = ((Expression&)*fn).GetOperatorCode();
			if( fn->IsExpressionOperand() && 
				(opc == '.' || opc == ARROW || opc == DOT_POINT || opc == ARROW_POINT) )
				obj = &((BinaryExpression&)*fn).GetOperand1()->GetType();
			
			OverloadResolutor or( ofl, *parametrList, obj );
			if( or.GetCallableFunction() == NULL )
			{
				if( implicit && noFunction )
					return NULL;

				theApp.Error(errPos, or.GetErrorMessage().c_str());
				return ErrorOperand::GetInstance();
			}

			// �������� ��������� ���������
			or.DoParametrListCast(errPos);
			
			// ����� ���� ��� ���� ���������� �������, ��������� ����������� ��
			// �����������			
			CheckParametrInitialization(fpt.GetParametrList());
		
			// ���� ����� �������-����, ������� ���������, ������ �� ��
			// �������� �������� ��� 'this' � ������� ������� ���������
			if( fn->IsPrimaryOperand()						 &&	
				tempf.IsNull()								 && 
				static_cast<const Function &>(fn->GetType()).IsClassMember() )
				CheckMemberCall( static_cast<const Method &>(fn->GetType()) );
			
			// ������� �������������� ���
			PTypyziedEntity rtype = new TypyziedEntity(fn->GetType());
			const_cast<DerivedTypeList&>(rtype->GetDerivedTypeList()).PopHeadDerivedType();

			// ���������� �����
			return new FunctionCallExpression( rtype->GetDerivedTypeList().IsReference(),
					fn, parametrList, rtype );
		}

		// ����� ��� �� �������� ��������, ����� ����������
		else
		{
			if( fn->IsPrimaryOperand() )
				theApp.Error(errPos, "'%s' - ������� �� �������� ��������",
					ExpressionPrinter(fn).GetExpressionString().c_str());
			else
				theApp.Error(errPos, "'%s' - �� �������������� ���",
					fn->GetType().GetTypyziedEntityName().c_str());
			return ErrorOperand::GetInstance();
		}
			
	}

	// ���� ������� �������� ������������� ���������
	else if( fn->IsOverloadOperand() )
	{
		// ������� ��������, ���� ����� ����� this, ������ ��������� ��������� 
		// � ���������� �������� ���������
		const SymbolTable &st = GetCurrentSymbolTable().IsLocalSymbolTable() ?
			GetScopeSystem().GetFunctionalSymbolTable() : GetCurrentSymbolTable();
	
		// �������� ���������
		const Function *vf = CheckBestViableFunction(
			static_cast<const OverloadOperand &>(*fn).GetOverloadList(),*parametrList, NULL);
					
		// ���� ������� �� �������, �������
		if( vf == NULL )
		{
			if( implicit && noFunction )
				return NULL;
			return ErrorOperand::GetInstance();
		}

		// ������� ����������� �������, �������� ����������� ����������� ����������,				
		CheckParametrInitialization(vf->GetFunctionPrototype().GetParametrList());

		// ���� ����� �������-����, ������� ���������, ������ �� ��
		// �������� �������� ��� 'this' � ������� ������� ���������
		if(	vf->IsClassMember() )
			CheckMemberCall( static_cast<const Method&>(*vf) );

		// ������ ����� ��������� � ���������� ��������										
		PTypyziedEntity rtype = new TypyziedEntity(*vf);
		const_cast<DerivedTypeList&>(rtype->GetDerivedTypeList()).PopHeadDerivedType();
		return new FunctionCallExpression( vf->GetDerivedTypeList().IsReference(),
			new PrimaryOperand(false, *vf), parametrList, rtype);
	}
	
	// ���� �������������� ���������� ����
	else if( fn->IsTypeOperand() ) 
	{
		// ���� ����� ��������� ���, ������ ������ ����� ������������
		if( fn->GetType().GetBaseType().IsClassType() && 
			fn->GetType().GetDerivedTypeList().IsEmpty() )
		{
			const ClassType &cls = static_cast<const ClassType&>(fn->GetType().GetBaseType());
			const ConstructorList &ctorLst = cls.GetConstructorList();

			// ������ ������������� ����� ���� ������
			if( ctorLst.empty() )
			{
				theApp.Error(errPos, "'%s' - ����� �� ����� �������������", 
					cls.GetQualifiedName().c_str());
				return ErrorOperand::GetInstance();
			}

			OverloadFunctionList ofl(ctorLst.size());			
			copy(ctorLst.begin(), ctorLst.end(), ofl.begin());

			// �������� ��������� �� ������ �������������, ���� ���� �����,
			// ������ �����, ����� �������
			if( const Function *ctor = CheckBestViableFunction(ofl, *parametrList, NULL) )
			{
				// �������� ����������� ����������� ����������,				
				CheckParametrInitialization(ctor->GetFunctionPrototype().GetParametrList());
				const ClassType &ctorCls = 
					static_cast<const ClassType&>(ctor->GetSymbolTableEntry());
				ExpressionMakerUtils::ObjectCreationIsAccessible(
					ctorCls, errPos, false, false, true);

				// ��������� , ����� ����� �� ��� ����������. �������� �������
				// ������������ ������ ����������
				if( ctorCls.IsAbstract() )
					theApp.Error( errPos,
						"�������� ������� ������ '%s' ����������; ����� �������� �����������",
						ctorCls.GetQualifiedName().c_str() );
			
				// ������ ����� ��������� � ���������� ��������										
				PTypyziedEntity rtype = new TypyziedEntity(*ctor);
				const_cast<DerivedTypeList&>(rtype->GetDerivedTypeList()).PopHeadDerivedType();
				return new FunctionCallExpression( false,
					new PrimaryOperand(false, *ctor), parametrList, rtype);
			}

			else
				return ErrorOperand::GetInstance();			
		}

		// ����� ��� �� �������� �������, ������ ������������ 
		// �������������� CastMaker'�
		else
		{
			// ������� ��������, ����� �������� ��� 1
			if( parametrList->size() > 1)
			{
				theApp.Error(errPos, 
					"'%s' - ����������� ���� �� ����� ��������� '%i' ��������(�)",
					ExpressionPrinter(fn).GetExpressionString().c_str(),
					parametrList->size());
				return ErrorOperand::GetInstance();
			}

			// ���� ��� ����������, ������ �������� ����� �� null
			else if( parametrList->empty() )
			{
				return new PrimaryOperand(false, *new Literal(
					(BaseType*)&fn->GetType().GetBaseType(), true, false, 
					fn->GetType().GetDerivedTypeList(), "0") );
			}

			return TypeCastBinaryMaker(fn, (*parametrList)[0], OC_CAST, errPos).Make();
		}
	}

	// ����� ����������� �������
	else
		INTERNAL( "'FunctionCallBinaryMaker::Make' - ����������� �������");
	return NULL;
}


// ������� ��������� []
POperand ArrayBinaryMaker::Make()
{
	// ���������, ���� ������ ��� - ���������, ������ ������� ��������
	if( ExpressionMakerUtils::IsRvaluePointer(right->GetType()) )
		swap(const_cast<POperand&>(left), const_cast<POperand&>(right));

	// ���� �� ����� ������ ���� ��������, ������ �����
	if( ExpressionMakerUtils::ToPointerType(const_cast<POperand&>(left), errPos, "[]") )
	{
		// ���������, ����� ������ ��� �����
		if( !ExpressionMakerUtils::ToIntegerType(const_cast<POperand&>(right), errPos, "[]") )	
			return ErrorOperand::GetInstance();		

		// ����� ������
		else
		{
			ExpressionMakerUtils::ToRValue(left, errPos);
			TypyziedEntity *rtype = 
				ExpressionMakerUtils::DoCopyWithIndirection(left->GetType());

			// ������� ������ ���
			INTERNAL_IF( rtype->GetDerivedTypeList().IsEmpty() );
			const_cast<DerivedTypeList&>(rtype->GetDerivedTypeList()).PopHeadDerivedType();
			return new BinaryExpression(OC_ARRAY, true, left, right, rtype);
		}
	}

	else
		return ErrorOperand::GetInstance();
}


// ��������, ����� ���� ����������� ������� ������, ��������
// �������������� ������ ��� ��������� � ������������������ �����
bool SelectorBinaryMaker::CheckMemberVisibility( const Identifier &id, const ClassType &objCls )
{
	if( !id.GetSymbolTableEntry().IsClassSymbolTable() )
	{
		theApp.Error(errPos, "'%s' - �� �������� ������ ������ '%s'",
			id.GetQualifiedName().c_str(), objCls.GetQualifiedName().c_str());
		return false;
	}

	const ClassType &memCls = static_cast<const ClassType&>(id.GetSymbolTableEntry());
	DerivationManager dm(memCls, objCls);
	if( dm.IsBase() || &memCls == &objCls )
		return true;
	else
	{
		theApp.Error(errPos, "'%s' - �� �������� ������ ������ '%s'",
			id.GetQualifiedName().c_str(), objCls.GetQualifiedName().c_str());
		return false;
	}
}


// ������� ���������. ����� ������� ErrorOperand, ���� ���������� ����������
POperand SelectorBinaryMaker::Make()
{
	PCSTR opName = op == '.' ? "." : "->";

	// ��� ������ ��������, ����� ������� ����� ��� ���������� ��� ���������������
	if( left->IsErrorOperand() )
		return ErrorOperand::GetInstance();
	if( !(left->IsExpressionOperand() || left->IsPrimaryOperand()) )
	{
		theApp.Error(errPos, 
			"'%s' - �������� �������, ����� ����� ��� ������", opName );			
		return ErrorOperand::GetInstance();
	}

	// �����, ��������� ��� ��������
	if( op == '.' )
	{
		if( !ExpressionMakerUtils::IsClassType(left->GetType()) )
		{
			theApp.Error(errPos, 
				"'.' - ������� ����� ������ ����� ��������� ���");				
			return ErrorOperand::GetInstance();
		}
	}

	// ����� ������ ���� ��������� �� �����
	else
	{
		// ������� ��������, �������� ������� ������������� �������� '->',
		// ����� ������ ������� ���
		POperand rval = UnaryOverloadOperatorCaller(left, op, errPos).Call();
		if( !rval.IsNull() )
			const_cast<POperand&>(left) = rval;

		const DerivedTypeList &dtl = left->GetType().GetDerivedTypeList();
		bool ptr = (dtl.IsPointer() && dtl.GetDerivedTypeCount() == 1) ||
			(dtl.GetDerivedTypeCount() == 2 && dtl.IsReference() && 
			dtl.GetDerivedType(1)->GetDerivedTypeCode() == DerivedType::DT_POINTER);
		if( !ptr || !left->GetType().GetBaseType().IsClassType() )
		{
			theApp.Error(errPos, 
				"'->' - ������� ����� ������ ����� ��� '��������� �� �����'");				
			return ErrorOperand::GetInstance();
		}		
	}

	// ����� �������� ������ ��������, ������� �������������
	bool qualified = idPkg.GetChildPackageCount() > 1;
	POperand right = IdentifierOperandMaker( idPkg, &left->GetType() ).Make();

	if( right->IsErrorOperand() )
		return ErrorOperand::GetInstance();

	// ����� ���� ������������� �������
	else if( right->IsOverloadOperand() )
	{
		if( qualified )
		{
			const OverloadFunctionList &ofl =
				static_cast<OverloadOperand &>(*right).GetOverloadList();
			for( int i = 0; i<ofl.size(); i++ )
			if( !CheckMemberVisibility( *ofl[i],				
				static_cast<const ClassType&>(left->GetType().GetBaseType()) ) )
				return ErrorOperand::GetInstance();
		}

		// ������� ���������
		return new BinaryExpression( op, true, left, right, NULL );
	}

	// ����� ���� ������� ����
	else if( right->IsPrimaryOperand() )
	{
		if( qualified )
		{
			const Identifier &id = 
				dynamic_cast<const Identifier &>(right->GetType().IsDynamicTypyziedEntity() ?
					static_cast<const DynamicTypyziedEntity &>(right->GetType()).GetOriginal() :
					right->GetType());
			if( !CheckMemberVisibility( id,
				static_cast<const ClassType&>(left->GetType().GetBaseType()) ) )
				return ErrorOperand::GetInstance();
		}

		// ���������� ��������� ���������
		return new BinaryExpression( op, !right->GetType().IsEnumConstant(),
			left, right, new TypyziedEntity(right->GetType()) );
	}

	// ����� �� �� ����� ���������� � ����� ����� ��������
	else
	{
		theApp.Error(errPos,
			"'%s' - ��������� � ����� ���������� ����� '%s'",
			ParserUtils::PrintPackageTree(&idPkg).c_str(), opName );
		return ErrorOperand::GetInstance();
	}
}


// ������� ��������� '!'
POperand LogicalUnaryMaker::Make()
{
	// ��������� �������� �������
	if( !ExpressionMakerUtils::ToScalarType(const_cast<POperand&>(right), errPos, "!") )
		return ErrorOperand::GetInstance();

	// ����� ������ ������� ���������, ����������� ����� bool
	return new UnaryExpression('!', false, false, right, 
		new TypyziedEntity((BaseType*)&ImplicitTypeManager(KWBOOL).GetImplicitType(),
			false, false, DerivedTypeList()) );
}


// ������� ��������� '~'. ����� ������� ErrorOperand, ���� ���������� ����������
POperand BitReverseUnaryMaker::Make()
{
	// ��������� ����� �������
	if( !ExpressionMakerUtils::ToIntegerType(const_cast<POperand&>(right), errPos, "~") )	
		return ErrorOperand::GetInstance();

	// ����� ������ ������� ���������, ����������� ����� int
	return new UnaryExpression('~', false, false, right, 
		new TypyziedEntity((BaseType*)&ImplicitTypeManager(KWINT).GetImplicitType(),
			false, false, DerivedTypeList()) );
}


// ������� ��������� % << >> ^ | &
POperand IntegralBinaryMaker::Make()
{
	string opName = ExpressionPrinter::GetOperatorName(op);
	
	// ��� �������� ������ ���� ������	
	if( !ExpressionMakerUtils::ToIntegerType(const_cast<POperand&>(left), errPos, opName) ||
		!ExpressionMakerUtils::ToIntegerType(const_cast<POperand&>(right), errPos, opName) )	
		return ErrorOperand::GetInstance();

	// ����� ������ �������� ���������, ����������� ����� int
	return new BinaryExpression(op, false, left, right, 
		new TypyziedEntity((BaseType*)&ImplicitTypeManager(KWINT).GetImplicitType(),
			false, false, DerivedTypeList()) );	
}


// ������� ��������� *  /
POperand MulDivBinaryMaker::Make()
{
	PCSTR opName = op == '*' ? "*" : "/";
	// ��� �������� ������ ���� ���������������
	if( !ExpressionMakerUtils::ToArithmeticType(const_cast<POperand&>(left), errPos, opName) ||
		!ExpressionMakerUtils::ToArithmeticType(const_cast<POperand&>(right), errPos, opName) )	
		return ErrorOperand::GetInstance();

	// ����� ������ �������� ���������
	return new BinaryExpression(op, false, left, right, 
		ExpressionMakerUtils::DoResultArithmeticOperand(
			left->GetType(), right->GetType()) );
}


// ������� ��������� +  - 
POperand ArithmeticUnaryMaker::Make()
{
	PCSTR opName = op == '+' ? "+" : "-";
	// � ������, ���� ������� '+', ����� �������� ���. � '-', ������ ��������������
	if( !(op == '+' ? ExpressionMakerUtils::ToArithmeticOrPointerType(
			const_cast<POperand&>(right), errPos, opName) :
		 ExpressionMakerUtils::ToIntegerType(const_cast<POperand&>(right), errPos, opName)) )	
		return ErrorOperand::GetInstance();

	// ����� ������ ������� ��������� 
	return new UnaryExpression(op, false, false, right, 
		ExpressionMakerUtils::DoCopyWithIndirection(right->GetType()) );
}


// ������� ��������� '*'
POperand IndirectionUnaryMaker::Make()
{	
	if( !ExpressionMakerUtils::ToPointerType(const_cast<POperand&>(right), errPos, "*") )	
		return ErrorOperand::GetInstance();

	// ��� ����� ����� ��, ������ ������� ���������
	TypyziedEntity *rtype = new TypyziedEntity(right->GetType());

	// ���� ������ ��� ������, ������� ��
	if( rtype->GetDerivedTypeList().IsReference() )
		const_cast<DerivedTypeList&>(rtype->GetDerivedTypeList()).PopHeadDerivedType();
	INTERNAL_IF( !rtype->GetDerivedTypeList().IsPointer() );
	const_cast<DerivedTypeList&>(rtype->GetDerivedTypeList()).PopHeadDerivedType();

	// ����� ������ ������� ���������, ������� �������� lvalue
	return new UnaryExpression('*', true, false, right, rtype);
}


// ������� ��������� ����������� � ���������� �������
POperand IncDecUnaryMaker::Make()
{
	PCSTR opName = abs(op) == INCREMENT ? "++" : "--";
	// ������� ��������, ����� ��� ��� ��������
	if( !ExpressionMakerUtils::ToArithmeticOrPointerType(
				const_cast<POperand&>(right), errPos, opName) )	
		return ErrorOperand::GetInstance();

	// ����� �������� ����, ���������, ����� ������� ��� �������������� lvalue,
	// �.�. ���������� ��� �������������� �� ����������� �����
	if( !ExpressionMakerUtils::IsModifiableLValue(right, errPos, opName) )
		return ErrorOperand::GetInstance();

	// ���������, ���� ��� �������� ��������, ��� ������
	if( ExpressionMakerUtils::IsFunctionType(right->GetType()) )
	{
		theApp.Error(errPos, "'%s' - �� �������� � ���� '%s'", opName,
			right->GetType().GetTypyziedEntityName(false).c_str());
		return ErrorOperand::GetInstance();
	}

	// ����� ���� �������� �� ��������� ����������� ���
	ExpressionMakerUtils::ToRValue(right, errPos);

	// ���������, ���� �������� ����������, ��� �� ����� ���� bool
	if( abs(op) == DECREMENT && ExpressionMakerUtils::IsIntegral(right->GetType()) &&
		right->GetType().GetBaseType().GetBaseTypeCode() == BaseType::BT_BOOL )
	{
		theApp.Error(errPos, "'--' - �� �������� � ���� 'bool'");
		return ErrorOperand::GetInstance();
	}

	// ������� ���������, ���� ��������� ���� ���������� (������ ����), ��������� lvalue	
	return new UnaryExpression(op, op < 0, op > 0, right, 
		ExpressionMakerUtils::DoCopyWithIndirection(right->GetType()) );
}


// ������� ��������� &
POperand AddressUnaryMaker::Make()
{
	// ������ ������ - ����������� ��������. ��� ����� ���������� �� ������ �
	// ���������� ��� �������� ���������, �� � � ������������� ��������.
	// ����� ������� ���������, ��� ��� ������������ ��������, ������� ���������
	// � ������ ������, ��� ������������� 'this'.
	// ������������ �������� - �������� �� lvalue, � ��� ����� � ������ � �������
	try {

		// ���� ������� �������������, ������������ ��������, ����� �� ��� lvalue,
		// � ������ ��� rvalue
		if( right->IsOverloadOperand() )
		{
			OverloadOperand &ovop = const_cast<OverloadOperand &>(
				static_cast<const OverloadOperand &>(*right));
			if( !ovop.IsLvalue() ) 
				throw 0;
		
			// ����� ������ rvalue � �������
			ovop.SetRValue();
			return right;
		}

		// ���� ���������, �� ��� ����� ������ ���� rvalue � ��������� ���������,
		// ���� ��� �� �������� ��������
		else if( right->IsExpressionOperand() )
		{
			if( !static_cast<const Expression &>(*right).IsLvalue() )
				throw 1;
		}

		// ���� �������� �������
		else if( right->IsPrimaryOperand() )
		{
			if( !static_cast<const PrimaryOperand &>(*right).IsLvalue() )
				throw 2;				
		}

		// ����� ����������� �������
		else
			INTERNAL( "'AddressUnaryMaker::Make()' - �������� ����������� �������");

	} catch( int ) {
		// �������� ������, ������� �� �������� lvalue
		theApp.Error(errPos, "'&' - ������� �� �������� lvalue");
		return ErrorOperand::GetInstance(); 
	}

	// ��� ����� ����� ��, ������ ��������� ���������
	TypyziedEntity *rtype = new TypyziedEntity(right->GetType());

	// ���� ������ ��� ������, ������� ��
	if( rtype->GetDerivedTypeList().IsReference() )
		const_cast<DerivedTypeList&>(rtype->GetDerivedTypeList()).PopHeadDerivedType();
	
	// ���������, ���� ����� ���� ������ ��� this, �������� �� ���� ���������
	// �� ����, ����� ������� ���������	
	if( right->IsPrimaryOperand() && 		
		ExpressionMakerUtils::CheckMemberThisVisibility(right, errPos, false) < 0 )
	{
		const ClassType &mcls = static_cast<const ClassType &>(
			dynamic_cast<const Identifier &>(right->GetType()).GetSymbolTableEntry());

		// ������� ��������� ��������� �� ����
		const_cast<DerivedTypeList&>(rtype->GetDerivedTypeList()).PushHeadDerivedType(
			new PointerToMember(&mcls, false, false));
	}

	// ����� ��������� ������� ���������
	else	
	{
		// ����� ���� ��������, ����� �� ��� ���� ����� �� �������� ����
		// ��������, ������ ������ �������� �� �������� ����
		const TypyziedEntity *te = NULL;
		if( right->IsExpressionOperand() )
		{
			if( static_cast<const Expression&>(*right).GetOperatorCode() == '.' ||
				static_cast<const Expression&>(*right).GetOperatorCode() == ARROW )
				te = &static_cast<const BinaryExpression&>(*right).GetOperand2()->GetType();
		}

		else if( right->IsPrimaryOperand() )
			te = &right->GetType();

		if( te && te->IsObject() && static_cast<const ::Object*>(te)->
			GetStorageSpecifier() == ::Object::SS_BITFIELD )	
			theApp.Error(errPos, "'&' - �������� ���������� � �������� ����");	

		// ������� ��������� ���������
		const_cast<DerivedTypeList&>(rtype->GetDerivedTypeList()).PushHeadDerivedType(
			new Pointer(false, false));	
	}

	// ����� ������ ������� ���������, ������� �������� rvalue
	return new UnaryExpression('&', false, false, right, rtype);	
}


// ������� ��������� +
POperand PlusBinaryMaker::Make()
{
	// �������� ��� ������� ��������
	if( !ExpressionMakerUtils::ToArithmeticOrPointerType(
					const_cast<POperand&>(left), errPos, "+") ||
	   !ExpressionMakerUtils::ToArithmeticOrPointerType(
				const_cast<POperand&>(right), errPos, "+") )	
		return ErrorOperand::GetInstance();

	// ����� ���������, ����� ���� ��������. ������ ����, ���� ���
	// ��������������, ���� ���� �����, � ������ ���������
	if( ExpressionMakerUtils::IsArithmetic(left->GetType()) &&
		ExpressionMakerUtils::IsArithmetic(right->GetType()) )
	{
		// ������ ���������
		return new BinaryExpression('+', false, left, right, 
			ExpressionMakerUtils::DoResultArithmeticOperand(
				left->GetType(), right->GetType()) );
	}

	// ����� ���������, ����� ���� ��� �����, � ������ ����������
	else
	{
		// ���� ������ - ���������
		if( ExpressionMakerUtils::IsRvaluePointer(left->GetType()) )
		{
			// ���������, ����� ������ ��� ����� � ��������� ���
			// ��������� ����������� ���
			if( !ExpressionMakerUtils::IsIntegral(right->GetType()) )
			{
				theApp.Error(errPos, "'+' - ������ ������� ������ ����� ����� ���");
				return ErrorOperand::GetInstance(); 
			}

			// ��������� �����, ����� �� ���� ��������� �� �������
			if( ExpressionMakerUtils::IsFunctionType(left->GetType()) )
			{
				theApp.Error(errPos, "'+' - ���������� � ��������� �� �������");
				return ErrorOperand::GetInstance(); 
			}

			// ���������, ����� ��������� ��� ��������� ����������� �����			
			ExpressionMakerUtils::ToRValue(left, errPos);

			// ������ ���������
			return new BinaryExpression('+', false, left, right, 
				ExpressionMakerUtils::DoCopyWithIndirection(left->GetType()) );
		}

		// ���� ������ ���������, ������ ������ ������ ���� �����
		else if( ExpressionMakerUtils::IsRvaluePointer(right->GetType()) )
		{
			// ���������, ����� ������ ��� ����� � ��������� ���
			// ��������� ����������� ���
			if( !ExpressionMakerUtils::IsIntegral(left->GetType()) )
			{
				theApp.Error(errPos, "'+' - ������ ������� ������ ����� ����� ���");
				return ErrorOperand::GetInstance(); 
			}
			
			// ��������� �����, ����� �� ���� ��������� �� �������
			if( ExpressionMakerUtils::IsFunctionType(right->GetType()) )
			{
				theApp.Error(errPos, "'+' - ���������� � ��������� �� �������");
				return ErrorOperand::GetInstance(); 
			}

			// ���������, ����� ��������� ��� ��������� ����������� �����			
			ExpressionMakerUtils::ToRValue(right, errPos);

			// ������ ���������
			return new BinaryExpression('+', false, left, right, 
				ExpressionMakerUtils::DoCopyWithIndirection(right->GetType()) );
		}

		// �����, ������
		else
		{
			theApp.Error(errPos, "'+' - ���������� � ����� '%s' � '%s'",
				left->GetType().GetTypyziedEntityName(false).c_str(),
				right->GetType().GetTypyziedEntityName(false).c_str());
			return ErrorOperand::GetInstance(); 
		}
	}		
}


// ������� ��������� -
POperand MinusBinaryMaker::Make()
{	
	// �������� ��� ������� ��������
	if( !ExpressionMakerUtils::ToArithmeticOrPointerType(
			const_cast<POperand&>(left), errPos, "-") ||
		!ExpressionMakerUtils::ToArithmeticOrPointerType(
			const_cast<POperand&>(right), errPos, "-") )
		return ErrorOperand::GetInstance();

	// ����� ���������, ����� ���� ��������. ������ ����, ���� ���
	// ��������������, ���� ������ ���������, � ������ �����, ����
	// ��� ���������
	if( ExpressionMakerUtils::IsArithmetic(left->GetType()) &&
		ExpressionMakerUtils::IsArithmetic(right->GetType()) )
	{
		// ������ ���������
		return new BinaryExpression('-', false, left, right, 
			ExpressionMakerUtils::DoResultArithmeticOperand(
				left->GetType(), right->GetType()) );
	}

	// ����� ���������, ���� ������ ���������, �� ������ ������ ����
	// ���� �����, ���� ����������
	else if( ExpressionMakerUtils::IsRvaluePointer(left->GetType()) )
	{
		// ���� ������ - ���������
		if( ExpressionMakerUtils::IsRvaluePointer(right->GetType()) )
		{		
			// ��������� �����, ����� �� ���� ��������� �� �������
			if( ExpressionMakerUtils::IsFunctionType(left->GetType()) ||
				ExpressionMakerUtils::IsFunctionType(right->GetType()) )
			{
				theApp.Error(errPos, "'-' - ���������� � ��������� �� �������");
				return ErrorOperand::GetInstance(); 
			}

			// ���������, ����� ���� ���� ����������
			ScalarToScalarCaster stsc(left->GetType(), right->GetType(), false);
			stsc.ClassifyCast();
			if( !stsc.IsConverted() )
			{
				theApp.Error(errPos, "'-' - %s", stsc.GetErrorMessage().c_str());
				return ErrorOperand::GetInstance(); 
			}

			// ��������� ���������� ��������������, �������� ��������� ��
			// ������������ � �������
			stsc.DoCast(left, const_cast<POperand&>(right), errPos);

			// ���������, ����� ��������� ��� ��������� ����������� �����			
			ExpressionMakerUtils::ToRValue(left, errPos);
			ExpressionMakerUtils::ToRValue(right, errPos);

			// ������ ���������, �������������� ��� - unsigned int
			return new BinaryExpression('-', false, left, right, 
				new TypyziedEntity(
					(BaseType*)&ImplicitTypeManager(KWINT, KWUNSIGNED).GetImplicitType(),
					false, false, DerivedTypeList()) );
		}

		// �����, ���� ������ �����
		else if( ExpressionMakerUtils::IsIntegral(right->GetType()) )
		{				
			// ��������� �����, ����� �� ���� ��������� �� �������
			if( ExpressionMakerUtils::IsFunctionType(left->GetType()) )
			{
				theApp.Error(errPos, "'-' - ���������� � ��������� �� �������");
				return ErrorOperand::GetInstance(); 
			}

			// ���������, ����� ��������� ��� ��������� ����������� �����			
			ExpressionMakerUtils::ToRValue(left, errPos);

			// ������ ���������, ����������� ����� ���������
			return new BinaryExpression('-', false, left, right, 
				ExpressionMakerUtils::DoCopyWithIndirection(left->GetType()) );
		}

		// �����, ������
		else
		{
			theApp.Error(errPos, "'-' - ���������� � ����� '%s' � '%s'",
				left->GetType().GetTypyziedEntityName(false).c_str(),
				right->GetType().GetTypyziedEntityName(false).c_str());
			return ErrorOperand::GetInstance(); 
		}
	}		

	// ���� ��� ���� �� �������������� � ������ �� ���������,
	// ������ ��������� - ������ ���, ������� ������
	else
	{
		theApp.Error(errPos, "'-' - ������ ������� ������ ����� �������� ���");
		return ErrorOperand::GetInstance();
	}
}


// ������� ���������  <   <=   >=  >  ==  !=
POperand ConditionBinaryMaker::Make()
{
	string opName = ExpressionPrinter::GetOperatorName(op);
	
	// ���������, ����� ��� �����. ��������
	if( !(op == EQUAL || op == NOT_EQUAL ?
		ExpressionMakerUtils::ToScalarType(const_cast<POperand&>(left), errPos, opName) :
		ExpressionMakerUtils::ToArithmeticOrPointerType(
			const_cast<POperand&>(left), errPos, opName)) )
		return ErrorOperand::GetInstance();
		
	if( !(op == EQUAL || op == NOT_EQUAL ?
		ExpressionMakerUtils::ToScalarType(const_cast<POperand&>(right), errPos, opName) :
		ExpressionMakerUtils::ToArithmeticOrPointerType(
			const_cast<POperand&>(right), errPos, opName)) )
		return ErrorOperand::GetInstance();

	// ������ ���������, ����� �� ������������� ���� ��� � �������
	ScalarToScalarCaster stsc(left->GetType(), right->GetType(), false);
	stsc.ClassifyCast();

	// ���� �������������� ����������, �����
	if( !stsc.IsConverted() )
	{
		theApp.Error(errPos, "'%s' - %s", opName.c_str(), stsc.GetErrorMessage().c_str());
		return ErrorOperand::GetInstance();
	}

	// �������������� ��� - bool
	stsc.DoCast(left, const_cast<POperand&>(right), errPos);
	return new BinaryExpression(op, false, left, right, 
		new TypyziedEntity(
			(BaseType*)&ImplicitTypeManager(KWBOOL).GetImplicitType(),
			false, false, DerivedTypeList()) );	
}


// ������� ���������. ����� ������� ErrorOperand, ���� ���������� ����������
POperand LogicalBinaryMaker::Make()
{
	string opName = op == LOGIC_AND ? "&&" : "||";
	if( !ExpressionMakerUtils::ToScalarType(const_cast<POperand&>(left), errPos, opName) ||
		!ExpressionMakerUtils::ToScalarType(const_cast<POperand&>(right), errPos, opName) )
		return ErrorOperand::GetInstance();
	return new BinaryExpression(op, false, left, right, 
		new TypyziedEntity(
			(BaseType*)&ImplicitTypeManager(KWBOOL).GetImplicitType(),
			false, false, DerivedTypeList()) );	
}


// ������� ��������� ?:
POperand IfTernaryMaker::Make()
{
	string opName = "?:";

	// ������ ��������� ����� ����� �������� ���
	if( !ExpressionMakerUtils::ToScalarType(const_cast<POperand&>(cond), errPos, opName) )
		return ErrorOperand::GetInstance();

	// ���� ������ ��� void, ������ ��������� ����� ������
	if( left->GetType().GetDerivedTypeList().IsEmpty() &&
		left->GetType().GetBaseType().GetBaseTypeCode() == BaseType::BT_VOID )
		return new TernaryExpression('?', false, cond, left, right, 
			ExpressionMakerUtils::DoCopyWithIndirection(right->GetType()) );

	// ���� ������ void, ������ ��������� ������
	else if( right->GetType().GetDerivedTypeList().IsEmpty() &&
		right->GetType().GetBaseType().GetBaseTypeCode() == BaseType::BT_VOID )
		return new TernaryExpression('?', false, cond, left, right, 
			ExpressionMakerUtils::DoCopyWithIndirection(left->GetType()) );

	// ����� �������� ������������� ��� ����
	PCaster caster1 = AutoCastManager(left->GetType(), right->GetType(), true).RevealCaster(),
			caster2 = AutoCastManager(right->GetType(), left->GetType(), true).RevealCaster();
	caster1->ClassifyCast();
	caster2->ClassifyCast();

	// ���� ��� �������������� �������, ��� �� �����
	if( caster1->IsConverted() && caster2->IsConverted() &&
		caster1->GetCastCategory() > Caster::CC_EQUAL )
	{
		theApp.Error(errPos,
			"'?:' - �������������� �� '%s' � '%s' ������������",
			left->GetType().GetTypyziedEntityName().c_str(),
			right->GetType().GetTypyziedEntityName().c_str());
		return ErrorOperand::GetInstance();
	}

	// ���� ���� ���������, ���� ���� �������� lvalue
	if( caster1->GetCastCategory() == Caster::CC_EQUAL &&
		caster2->GetCastCategory() == Caster::CC_EQUAL )
	{
		bool lv = ExpressionMakerUtils::IsLValue(left) && ExpressionMakerUtils::IsLValue(right);
		return new TernaryExpression('?', lv, cond, left, right, 
			ExpressionMakerUtils::DoCopyWithIndirection(left->GetType()) );
	}

	// ���� ������ �������, ����������� ����� ������ ���
	if( caster1->IsConverted() )
	{
		caster1->DoCast(left, const_cast<POperand&>(right), errPos);
		return new TernaryExpression('?', false, cond, left, right, 
			ExpressionMakerUtils::DoCopyWithIndirection(left->GetType()) );
	}

	else if( caster2->IsConverted() )
	{
		caster2->DoCast(right, const_cast<POperand&>(left), errPos);
		return new TernaryExpression('?', false, cond, left, right, 
			ExpressionMakerUtils::DoCopyWithIndirection(right->GetType()) );
	}

	// ����� �������������� ����������
	else
	{
		theApp.Error(errPos,
			"'?:' - ���������� ������������� '%s' � '%s'",
			left->GetType().GetTypyziedEntityName(false).c_str(),
			right->GetType().GetTypyziedEntityName(false).c_str());
		return ErrorOperand::GetInstance();
	}	
}


// �����, ��������� ��������� ���� �������� ���������;
// -1 - ������, 0 - ��������� ��������, 1 - ������� ��������� � �����
int AssignBinaryMaker::CheckOperation( const string &opName )
{
	INTERNAL_IF( op == '=' );

	// ���������, ���� ����� �������, �������� ��������� �����,
	// �� �������� �� ����� ����������
	if( ExpressionMakerUtils::IsClassType(left->GetType()) )
	{
		theApp.Error(errPos, "'%s::operator %s(%s)' - �� ��������",
			static_cast<const ClassType&>(
				left->GetType().GetBaseType()).GetQualifiedName().c_str(),
			opName.c_str(), 
			right->GetType().GetTypyziedEntityName(false).c_str() );
			
		return -1;
	}

	// ����� ���� += ��� -=, ��� ����� ���� ���������� ��� ��������������,
	// ����� ������ ��������������
	if( op == PLUS_ASSIGN || op == MINUS_ASSIGN )
	{
		// ���� ���������
		if( ExpressionMakerUtils::IsRvaluePointer(left->GetType()) )
		{
			// ���������, ����� ������ ��� ����� � ��������� ���
			// ��������� ����������� ���
			if( !ExpressionMakerUtils::IsIntegral(right->GetType()) )
			{
				theApp.Error(errPos, 
					"'%s' - ������ ������� ������ ����� ����� ���", opName.c_str());
				return -1; 
			}

			// ��������� �����, ����� �� ���� ��������� �� �������
			if( ExpressionMakerUtils::IsFunctionType(left->GetType()) )
			{
				theApp.Error(errPos, 
					"'%s' - ���������� � ��������� �� �������", opName.c_str());
				return -1; 
			}

			// ���������, ����� ��������� ��� ��������� ����������� �����			
			ExpressionMakerUtils::ToRValue(left, errPos);
			return 1;
		}
		
		// ����� ���� �� ��������������, ������
		else if( !ExpressionMakerUtils::IsArithmetic(left->GetType()) ||
			left->GetType().GetBaseType().GetBaseTypeCode() == BaseType::BT_ENUM )
			goto err;		
	}

	// ��������, ����� ��� ��� ��������������
	else
	{
		// ������ �����, ����� ��� ��� ������������
		if( !ExpressionMakerUtils::IsArithmetic(left->GetType()) ||
			left->GetType().GetBaseType().GetBaseTypeCode() == BaseType::BT_ENUM )
		{
		err:
			theApp.Error(errPos, 
				"'%s' - �� �������������� ���; '%s' - �������� ������� �������������� ���",
				left->GetType().GetTypyziedEntityName().c_str(), opName.c_str());
			return -1;
		}
	}

	return 0;
}


// ������� ��������� -  =   +=   -=   *=   /=   %=   >>=   <<=  |=  &=  ^=
POperand AssignBinaryMaker::Make()
{
	// ���� �������� ���������� ���������, ������� ������������� ��������,
	// ������� � ����������� ������������ ��� ������ �������	
	string opName = ExpressionPrinter::GetOperatorName(op);

	// ���������, ����� ������� ��� �������������� lvalue � ������ �������
	if( !ExpressionMakerUtils::IsModifiableLValue(left, errPos, opName.c_str()) )
		return ErrorOperand::GetInstance();

	// �������� �� ��������� ��������
	if( op != '=' )
	{		
		if( int r = CheckOperation(opName) )
			return r < 0 ? ErrorOperand::GetInstance() :
				new BinaryExpression(op, true, left, right, 
					ExpressionMakerUtils::DoCopyWithIndirection(left->GetType()) );
	
	}

	// ���� left, ����� ��������� ���, ���������, ����� ����� ��� ��������� ��������
	if( ExpressionMakerUtils::IsClassType(left->GetType()) &&
		static_cast<const ClassType&>(left->GetType().GetBaseType()).IsUncomplete() )
	{
		theApp.Error(errPos, "'%s' - ������������� �����; ���������� ����������",
			static_cast<const ClassType&>(left->GetType().GetBaseType()).
			GetQualifiedName().c_str());
		return ErrorOperand::GetInstance();
	}

	// ����� ���������������, ������ ������� �������������� �� ������ � �� ������,
	// �.�. ��������������� ������� ������ �� ������ ������� �������������������
	if( left->GetType().GetDerivedTypeList().IsReference() )	
		const_cast<POperand&>(left) = 
			new UnaryExpression(GOC_REFERENCE_CONVERSION, true, false,
				left, ExpressionMakerUtils::DoCopyWithIndirection(left->GetType()) );	

	// ��������� �������������� ��������������
	PCaster caster = AutoCastManager(left->GetType(), right->GetType(), true).RevealCaster();
	INTERNAL_IF( caster.IsNull() );
	caster->ClassifyCast();

	// ���� �������������� �� �������, �����
	if( !caster->IsConverted() )
	{
		if( caster->GetErrorMessage().empty() )
			theApp.Error(errPos, 
				"'%s' - ���������� ������������� '%s' � '%s'",
				opName.c_str(), right->GetType().GetTypyziedEntityName(false).c_str(),
				left->GetType().GetTypyziedEntityName(false).c_str() );
		else
			theApp.Error(errPos, 
				"'%s' - %s",
				opName.c_str(), caster->GetErrorMessage().c_str());
		return ErrorOperand::GetInstance();
	}

	// ����� ��������� ��������������
	caster->DoCast(left, const_cast<POperand&>(right), errPos);

	// ������ ���������. �������� '=', �.�. ��������� �������� �������� �� �����������,
	// ��������� lvalue
	return new BinaryExpression(op, true, left, right, 
		ExpressionMakerUtils::DoCopyWithIndirection(left->GetType()));
}


// ������� ���������. ����� ������� ErrorOperand, ���� ���������� ����������
POperand ComaBinaryMaker::Make()
{
	// ��� �������� ������ ���� ���������� ��� �������� ���������
	INTERNAL_IF( !( (left->IsPrimaryOperand()  || left->IsExpressionOperand()) &&
					(right->IsPrimaryOperand() || right->IsExpressionOperand()) ) );

	bool lv = right->IsPrimaryOperand()
		? static_cast<const PrimaryOperand &>(*right).IsLvalue()
		: static_cast<const Expression &>(*right).IsLvalue();

	return new BinaryExpression(',', lv, left, right, new TypyziedEntity(right->GetType()));
}


// ����� ��� ��� ������ ������������� ���������, ������� �������������
// ����� � ����� � ��������� sizeof, ������� �������� �� ��������� ������
void NewTernaryMaker::MakeSizeofExpression( const NodePackage &typePkg )
{
	INTERNAL_IF( typePkg.GetPackageID() != PC_DECLARATION || 
		typePkg.GetChildPackageCount() != 2 || typePkg.IsErrorChildPackages() || 
		typePkg.GetChildPackage(1)->GetPackageID() != PC_DECLARATOR );
	INTERNAL_IF( !allocType->IsTypeOperand() );

	// ����� �������� �������������� ��� ���������
	PTypyziedEntity sizeofExpType = new TypyziedEntity( 
		(BaseType*)&ImplicitTypeManager(KWINT, KWUNSIGNED).GetImplicitType(),
		false, false, DerivedTypeList() );

	// ����� ���������, ���� ���� ������ ����������� ����� � �����������
	const NodePackage &dtr = static_cast<const NodePackage &>(*typePkg.GetChildPackage(1));
	if( dtr.GetChildPackageCount() > 0	&&
		dtr.GetChildPackage(0)->GetPackageID() == PC_ARRAY )
	{
		const NodePackage &ar = *static_cast<const NodePackage *>(dtr.GetChildPackage(0));
		// �������� ���������, ������ ���� ���������� �������� ������� ����� 3
		if( ar.GetChildPackageCount() == 3 )
		{
			const POperand &arraySz = static_cast<const ExpressionPackage *>(
					ar.GetChildPackage(1))->GetExpression();

			// ���������, ����� ��������� ���� �����
			if( !ExpressionMakerUtils::IsIntegral(arraySz->GetType()) )
				theApp.Error(errPos, "'new[]' - ������ ������� �� �����");

			// ���� ��������� ����������������
			double asz;
			if( ExpressionMakerUtils::IsInterpretable(arraySz, asz) )			
				if( asz < 1 )
					theApp.Error(errPos, "'new[]' - ������������ ������ ���������� ������");			

			// ������� � ����������� ���� ������� ������
			TypyziedEntity *destType = new TypyziedEntity(allocType->GetType());
			INTERNAL_IF( !destType->GetDerivedTypeList().IsArray() );
			const_cast<DerivedTypeList &>(destType->GetDerivedTypeList()).PopHeadDerivedType();

			// ������� ��������� �������
			POperand szofExp = new UnaryExpression( KWSIZEOF, false, false, 
					new TypeOperand(*destType), sizeofExpType );

			// �������� ������ �� ���������
			allocSize = new BinaryExpression('*', false, arraySz, szofExp, sizeofExpType );
			return ;
		}
	}

	// � ��������� ������ ������ ����������� ������ ����
	allocSize = new UnaryExpression( KWSIZEOF, false, false, allocType, sizeofExpType );
}


// ����� ���������� ����� ��������� new � ������ �����
POperand NewTernaryMaker::MakeNewCall( bool array, bool clsType )
{
	PExpressionList paramList = placementList;

	// ��������� � ������ ����������, ������ ���������� ��������� sizeof
	paramList->insert(paramList->begin(), allocSize);

	// ����� ��������� ����� ���������. �������, ���� ��� ���������,
	// ���� ������ ������
	if( clsType && !globalOp )
	{
		const ClassType &cls = 
			static_cast<const ClassType &>(allocType->GetType().GetBaseType());
		OverloadOperatorFounder oof( array ? OC_NEW_ARRAY : KWNEW, false, &cls, errPos);

		// ���� �������� ������, ������ ����� �������
		if( oof.IsFound() )
		{
			INTERNAL_IF( oof.GetClassOperatorList().empty() || oof.IsAmbigous() );
			POperand pol = new OverloadOperand( oof.GetClassOperatorList() );
			return FunctionCallBinaryMaker(pol, paramList, OC_FUNCTION, errPos).Make();
		}

		// ���� ������������, ����� ���������� errorOperand
		else if( oof.IsAmbigous() )
			return ErrorOperand::GetInstance();
	}

	// � ��������� ������ ��������� ����� ����������� ���������
	OverloadOperatorFounder oof( array ? OC_NEW_ARRAY : KWNEW, true, NULL, errPos);

	// ���� �������� ������, ������ ����� �������
	if( oof.IsFound() )
	{
		INTERNAL_IF( oof.GetGlobalOperatorList().empty() || oof.IsAmbigous() );
		POperand pol = new OverloadOperand( oof.GetGlobalOperatorList() );				 
		return FunctionCallBinaryMaker(pol, paramList, OC_FUNCTION, errPos).Make();
	}

	// � ���� ����� ������� �������� ������
	theApp.Fatal(errPos, "'void *operator new(unsigned)' - �� ��������");
	return ErrorOperand::GetInstance();
}


// ������� ���������. ����� ������� ErrorOperand, ���� ���������� ����������
POperand NewTernaryMaker::Make()
{
	// ���������, ���� ���� ���� �� ��������� ��������, �������
	// errorOperand
	int i;
	for( i = 0; i<placementList->size(); i++ )
		if( placementList->at(i)->IsErrorOperand() )
			return ErrorOperand::GetInstance();

	for( i = 0; i<initializatorList->size(); i++ )
		if( initializatorList->at(i)->IsErrorOperand() )
			return ErrorOperand::GetInstance();

	// ���� ��� �������� ���������
	if( allocType->IsErrorOperand() )
		return ErrorOperand::GetInstance();
	INTERNAL_IF( !allocType->IsTypeOperand() );

	// ��� �������� �������� �����������. ��������� ���������� ���.
	// �� ����� ���� �������, void, ������������ ��� ����������� �����,
	// ��������
	const TypyziedEntity &at = allocType->GetType();
	if( at.GetDerivedTypeList().IsReference() || at.GetDerivedTypeList().IsFunction() ||
		(at.GetDerivedTypeList().IsEmpty() && 
		 at.GetBaseType().GetBaseTypeCode() == BaseType::BT_VOID) )
	{
		theApp.Error(errPos, "'new' - ��� '%s' �� �������� ���������",
			at.GetTypyziedEntityName().c_str());
		return ErrorOperand::GetInstance();
	}

	// ���������, ����� ����� �� ��� ����������� ��� �������������
	PTypyziedEntity rtype = new TypyziedEntity(at);
	bool array = at.GetDerivedTypeList().IsArray();
	while( rtype->GetDerivedTypeList().IsArray() )
		const_cast<DerivedTypeList&>(rtype->GetDerivedTypeList()).PopHeadDerivedType();
	
	// ������ ���������, ���� �������������� ��� - ����������� ��� �������������
	// �����, ��� ������
	if( rtype->GetDerivedTypeList().IsEmpty() &&
		(at.GetBaseType().IsClassType() &&  
		 (static_cast<const ClassType&>(at.GetBaseType()).IsUncomplete() ||
		  static_cast<const ClassType&>(at.GetBaseType()).IsAbstract()) ) )
	{
		theApp.Error(errPos, "'new%s' - �������� ������� %s ������ ����������",
			array ? "[]" : "", static_cast<const ClassType&>(at.GetBaseType()).
				IsUncomplete() ? "��������������" : "������������");
		return ErrorOperand::GetInstance();
	}
	
	// ��������� �������-�������, ������� ������������� ���������� ��� ��������� ������
	bool cls = rtype->GetDerivedTypeList().IsEmpty() && rtype->GetBaseType().IsClassType();
	POperand call = MakeNewCall(array, cls);
	if( call->IsErrorOperand() )
		return ErrorOperand::GetInstance();

	// ����� ��������� ��������������
	ExpressionMakerUtils::CorrectObjectInitialization( at, 
		const_cast<PExpressionList&>(initializatorList), false, errPos);

	const_cast<DerivedTypeList&>(rtype->GetDerivedTypeList()).
		PushHeadDerivedType(new Pointer(false, false) );
	// ���������� ���������
	return new NewExpression( array ? OC_NEW_ARRAY : KWNEW, call, initializatorList, rtype );
}


// ����� ������ ������ ��������� delete
POperand DeleteUnaryMaker::MakeDeleteCall( bool clsType )
{
	// ��������� ������ ����������
	PExpressionList paramList = new ExpressionList;
	paramList->push_back( right );

	// ����� ��������� ����� ���������. �������, ���� ��� ���������,
	// ���� ������ ������
	if( clsType && op > 0 )
	{
		const ClassType &cls = 
			static_cast<const ClassType &>(right->GetType().GetBaseType());
		OverloadOperatorFounder oof( op, false, &cls, errPos);

		// ���� �������� ������, ������ ����� �������
		if( oof.IsFound() )
		{
			INTERNAL_IF( oof.GetClassOperatorList().empty() || oof.IsAmbigous() );
			POperand pol = new OverloadOperand( oof.GetClassOperatorList() );
			return FunctionCallBinaryMaker(pol, paramList, OC_FUNCTION, errPos).Make();
		}

		// ���� ������������, ����� ���������� errorOperand
		else if( oof.IsAmbigous() )
			return ErrorOperand::GetInstance();
	}

	// � ��������� ������ ��������� ����� ����������� ���������
	OverloadOperatorFounder oof( op, true, NULL, errPos);

	// ���� �������� ������, ������ ����� �������
	if( oof.IsFound() )
	{
		INTERNAL_IF( oof.GetGlobalOperatorList().empty() || oof.IsAmbigous() );
		POperand pol = new OverloadOperand( oof.GetGlobalOperatorList() );				 
		return FunctionCallBinaryMaker(pol, paramList, OC_FUNCTION, errPos).Make();
	}

	// � ���� ����� ������� �������� ������
	theApp.Fatal(errPos, "'void operator delete(void *)' - �� ��������");
	return ErrorOperand::GetInstance();
}


// ������� ��������� delete, delete[], ::delete, ::delete[]
POperand DeleteUnaryMaker::Make()
{
	string opName = ExpressionPrinter::GetOperatorName(op);
	if( !ExpressionMakerUtils::ToPointerType(const_cast<POperand&>(right), errPos, opName) )
		return ErrorOperand::GetInstance();
		
	// ����� ���������, ����� ��������� �� ��� ��������
	if( ExpressionMakerUtils::IsFunctionType(right->GetType()) )
	{
		theApp.Error(errPos, 
			"'%s' - ���������� ������� ��������� �� �������",
			opName.c_str());
		return ErrorOperand::GetInstance();
	}

	// ���������, ��������� ����������� �����������, ���� ��� ���������
	const DerivedTypeList &dtl = right->GetType().GetDerivedTypeList();
	bool clsType = right->GetType().GetBaseType().IsClassType() &&
		(dtl.GetDerivedTypeCount() == 1 || 
		 (dtl.IsReference() && dtl.GetDerivedTypeCount() == 2) );
	if( clsType )
		ExpressionMakerUtils::ObjectCreationIsAccessible(static_cast<const ClassType&>(
			right->GetType().GetBaseType()), errPos, false, false, true);
	
	return MakeDeleteCall(clsType);
}


// ������� ���������. ����� ������� ErrorOperand, ���� ���������� ����������
POperand PointerToMemberBinaryMaker::Make()
{
	const ClassType *lcls = NULL;	// �����, � ����� �������� ����������
	string opName = op == DOT_POINT ? ".*" : "->*";

	// ��������� ��� ������ ��������,
	if( op == DOT_POINT )
	{
		if( !ExpressionMakerUtils::IsClassType(left->GetType()) )
		{
			theApp.Error(errPos, "'%s' - �� ��������� ���; '%s' - ������� ��������� ���",
				left->GetType().GetTypyziedEntityName().c_str(), opName.c_str());
			return ErrorOperand::GetInstance();
		}

		// ����� �������� �����
		lcls = &static_cast<const ClassType&>(left->GetType().GetBaseType());
	}

	// ����� ������ ���� ��������� �� �����
	else
	{
		// ������� ��� ������ ���� ���������
		if( !left->GetType().GetBaseType().IsClassType() )
		{
			theApp.Error(errPos, "'%s' - �� ��������� ���; '%s' - ������� ��������� ���",
				left->GetType().GetTypyziedEntityName().c_str(), opName.c_str());
			return ErrorOperand::GetInstance();
		}

		// ��������� ���������
		const DerivedTypeList &dtl = left->GetType().GetDerivedTypeList();
		int cnt = dtl.GetDerivedTypeCount();
		if( (dtl.IsPointer() && cnt == 1) || 
			( cnt == 2 && dtl.IsReference() && dtl.GetDerivedType(1)->
			   GetDerivedTypeCode() == DerivedType::DT_POINTER ) )
			lcls = &static_cast<const ClassType&>(left->GetType().GetBaseType());

		// ����� ������
		else
		{
			theApp.Error(errPos, "'%s' - ����� ������� ������ ����� ��� '%s'", opName.c_str(),
				(static_cast<const ClassType&>(left->GetType().GetBaseType()).
				 GetQualifiedName().c_str() + string(" *")).c_str());
			return ErrorOperand::GetInstance();
		}
	}

	// ����� ���� ���������, ����� ������ �������, ��� ���������� �� ����
	const DerivedTypeList &dtl = right->GetType().GetDerivedTypeList();
	int cnt = dtl.GetDerivedTypeCount();
	const ClassType *mcls = NULL;
	if( dtl.IsPointerToMember() )
		mcls = &static_cast<const PointerToMember &>(*dtl.GetDerivedType(0)).GetMemberClassType();

	else if( cnt > 1 && dtl.IsReference() && dtl.GetDerivedType(1)->
		   GetDerivedTypeCode() == DerivedType::DT_POINTER_TO_MEMBER )
		mcls = &static_cast<const PointerToMember &>(*dtl.GetDerivedType(1)).GetMemberClassType();

	// ����� ������
	else
	{
		theApp.Error(errPos, 
			"'%s' - ������ ������� ������ ����� ��� '��������� �� ����'", opName.c_str());				
		return ErrorOperand::GetInstance();
	}

	// ����� ���� ���������, ����� ��������� �� ����, ���� ����� ����� �� ��� � �������,
	// ���� ����������� ��������� �������
	INTERNAL_IF( mcls == NULL || lcls == NULL );
	if( mcls != lcls )
	{
		DerivationManager dm(*mcls, *lcls);
		if( dm.IsBase() && dm.IsUnambigous() && dm.IsAccessible() )
			;

		// ����� ������
		else
		{
			theApp.Error(errPos,
				"'%s' - �� �������� ��������� ����������� ������� ������� ��� '%s'",
				mcls->GetQualifiedName().c_str(), lcls->GetQualifiedName().c_str());
			return ErrorOperand::GetInstance();
		}
	}

	// ������� �������� �������������, �� ������� ����� ������������ � ��������������� ����
	bool rc = right->GetType().IsConst() || left->GetType().IsConst(),
		 rv = right->GetType().IsVolatile() || left->GetType().IsVolatile();

	// ������ �������� ���������
	TypyziedEntity *rtype = new TypyziedEntity((BaseType*)&right->GetType().GetBaseType(),
		rc, rv, right->GetType().GetDerivedTypeList());

	// ���� ���� ������, ������� ������
	if( rtype->GetDerivedTypeList().IsReference() )
		const_cast<DerivedTypeList&>(rtype->GetDerivedTypeList()).PopHeadDerivedType();

	// ������� ��������� �� ����
	const_cast<DerivedTypeList&>(rtype->GetDerivedTypeList()).PopHeadDerivedType();

	// ���������� ���������
	return new BinaryExpression(op, true, left, right, rtype);
}

// ������� ���������. ����� ������� ErrorOperand, ���� ���������� ����������
POperand TypeidUnaryMaker::Make()
{
	// ���� ���������������� ��������� std::type_info
	const ClassType *clsTI = NULL;
	try {
		NameManager nm("std", &GetScopeSystem().GetGlobalSymbolTable(), false);
		if( nm.GetRoleCount() == 0 )
			throw 0;
		INTERNAL_IF( nm.GetRoleCount() != 1 );
		if( nm.GetRoleList().front().second != R_NAMESPACE )
			throw 1;

		// ����� � ���� ������� ��������� ���� ��������� type_info
		NameManager tnm("type_info", 
			static_cast<const NameSpace *>(nm.GetRoleList().front().first), false);
		if( tnm.GetRoleCount() != 1 ||
			tnm.GetRoleList().front().second != R_CLASS_TYPE )
			throw 2;

		// ����� �������� ����� � ��������� � ��������� ���������
		clsTI = dynamic_cast<const ClassType *>(tnm.GetRoleList().front().first);
		INTERNAL_IF( clsTI == NULL );

	} catch( int ) {
		theApp.Error(errPos, 
			"'typeid' - ��������� std::type_info �� ���������; ���������� ���� 'typeinfo'");
		return ErrorOperand::GetInstance();
	}

	return new UnaryExpression(KWTYPEID, false, false, right,
		new TypyziedEntity((BaseType*)clsTI, false, false, DerivedTypeList()) );
}


// ������� ���������. ����� ������� ErrorOperand, ���� ���������� ����������
POperand DynamicCastBinaryMaker::Make()
{
	// dynamic_cast<T>(v);
	// T - ������ ���� ������� ��� ���������� �� ��������� ����������� �����.
	// ���� ���������� �� void.
	// v - ������ ���� lvalue  ��� ���������� �� ��� ��������������
	const TypyziedEntity &toType = left->GetType(),
						 &fromType = right->GetType();
	
	// ���������, ����� ������������ ���� �� ������
	if( toType.IsConst() < fromType.IsConst() ||
		toType.IsVolatile() < fromType.IsVolatile() )
		theApp.Error(errPos, 
		"'dynamic_cast' - '%s' ����� ��������������, ��� '%s'; �������������� ����������",
			toType.GetTypyziedEntityName(false).c_str(),
			fromType.GetTypyziedEntityName(false).c_str());	

	// ��������, ���� ����� ������� 'void *', ����� �������� ��������������
	// ��� �������������� ��������
	if( toType.GetBaseType().GetBaseTypeCode() == BaseType::BT_VOID )
	{
		// ���������, ����� ��� ��������� �� void, � ������ ������� ��� ���������
		// �� ��������� ����������� ����������� �����
		const DerivedTypeList &dtl = fromType.GetDerivedTypeList();
		if( toType.GetDerivedTypeList().IsPointer()				&&
			toType.GetDerivedTypeList().GetDerivedTypeCount() == 1 &&
			fromType.GetBaseType().IsClassType()					&&
			( (dtl.GetDerivedTypeCount() == 1 && dtl.IsPointer())	||
			  (dtl.GetDerivedTypeCount() == 2 && dtl.IsReference() &&
			   dtl.GetDerivedType(1)->GetDerivedTypeCode() == DerivedType::DT_POINTER) ) )
		{
			// ����� ������ ���� ����������� � ��������� ��������
			const ClassType &cls = static_cast<const ClassType&>(fromType.GetBaseType());
			if( cls.IsUncomplete() || !cls.IsPolymorphic() )
			{
				theApp.Error(errPos, 
					"'dynamic_cast' - ����� '%s' �� �������� %s",
					cls.GetQualifiedName().c_str(), 
					cls.IsUncomplete() ? "�����������" : "�����������");				
				return ErrorOperand::GetInstance();
			}

			// ����� ������ ���������
			return new BinaryExpression(KWDYNAMIC_CAST, false, left, right,
				new TypyziedEntity(toType) );	
		}

		// ����� ������
		else
		{
			theApp.Error(errPos, 
				"'dynamic_cast' - ����� ������� ������ ���� 'void *'; "
				"������ ������� ������ ���� ���������� �� �����");
			return ErrorOperand::GetInstance();
		}
	}

	// ����� ��������� ��� ������
	if( !toType.GetBaseType().IsClassType()  ||
		!fromType.GetBaseType().IsClassType() )
	{
		theApp.Error(errPos, "'dynamic_cast' - �������� ������ ����� ��������� ���");
		return ErrorOperand::GetInstance();
	}

	// ���������, ���� ��-��
	if( toType.GetDerivedTypeList().IsPointer() &&
		toType.GetDerivedTypeList().GetDerivedTypeCount() == 1 )
	{
		const DerivedTypeList &dtl = fromType.GetDerivedTypeList();
		if( (dtl.GetDerivedTypeCount() == 1 && dtl.IsPointer())	||
			(dtl.GetDerivedTypeCount() == 2 && dtl.IsReference() &&
			 dtl.GetDerivedType(1)->GetDerivedTypeCode() == DerivedType::DT_POINTER) )
			 ;
		else
		{
			theApp.Error(errPos, "'dynamic_cast' - ������ ������� ������ ���� ����������");
			return ErrorOperand::GetInstance();
		}
	}

	// ����� ����� ������ ���� �������, � ������ lvalue
	else if( toType.GetDerivedTypeList().IsReference() &&
			  toType.GetDerivedTypeList().GetDerivedTypeCount() == 1 &&
		((right->IsExpressionOperand() && static_cast<const Expression&>(*right).IsLvalue()) ||
		 (right->IsPrimaryOperand() && static_cast<const PrimaryOperand&>(*right).IsLvalue()))
		&&  
		fromType.GetDerivedTypeList().GetDerivedTypeCount() <= 1 )
		;

	// ����� ������
	else
	{
		theApp.Error(errPos, "'dynamic_cast' - ���������� ������������� '%s' � '%s'",
			fromType.GetTypyziedEntityName(false).c_str(),
			toType.GetTypyziedEntityName(false).c_str());		
		return ErrorOperand::GetInstance();
	}

	// ����� ��������� ������
	const ClassType &fromCls = static_cast<const ClassType&>(fromType.GetBaseType()),
					&toCls = static_cast<const ClassType&>(toType.GetBaseType());

	// ��������, ����� ����� ������� �������� ��� ��������� ��������
	if( fromCls.IsUncomplete() )
	{
		theApp.Error(errPos, 
			"'dynamic_cast' - ����� '%s' �� ��������� ��������; ���������� ����������",
			fromCls.GetQualifiedName().c_str());
		return ErrorOperand::GetInstance();
	}

	// ������ ��������� �������� ����������� ��������
	if( toType.GetBaseType().IsClassType() )
	{
		// �������� � ������ �������, ����� �������������� ����� ��� ��������
		if( toCls.IsUncomplete() )
		{
			theApp.Error(errPos, 
				"'dynamic_cast' - ����� '%s' �� ��������� ��������; ���������� ����������",
				toCls.GetQualifiedName().c_str());
			return ErrorOperand::GetInstance();
		}

		// ���� toCls, �������� ��� fromCls ����������� ��������� �������, �� 
		// ������ ����� ����������, ����� static_cast, ����� ���������� ��������
		DerivationManager dm(toCls, fromCls);
		if( dm.IsBase() )
		{
			if( !dm.IsAccessible() || !dm.IsUnambigous() )
				theApp.Warning(errPos, 
				"'dynamic_cast' - �������������� � ����������� ��� ������������� ������� �����");

			// ������ ����������� ����������, �.�. ������������ �� ���������
			else
				return new BinaryExpression(KWSTATIC_CAST, 
					toType.GetDerivedTypeList().IsReference(), left, right,
					new TypyziedEntity(toType) );				
		}
	}

	// ����� ���� ������������ ��������������, ���������, ����� �������
	// ���� ��� ������������ ������
	if( !fromCls.IsPolymorphic() )
	{
		theApp.Error(errPos,
			"'dynamic_cast' - ����� '%s' �� �������� �����������",
			fromCls.GetQualifiedName().c_str());
		return ErrorOperand::GetInstance(); 
	}

	return new BinaryExpression(KWDYNAMIC_CAST, 
		toType.GetDerivedTypeList().IsReference(), 
		left, right, new TypyziedEntity(toType) );	
}


// ���������� true, ���� �������� �������������� �� lvalue B � cv D&,
// ���� �� B * � D *
bool StaticCastBinaryMaker::BaseToDerivedExist( 
					const TypyziedEntity &toType, const TypyziedEntity &fromType )
{
	if( !(toType.GetBaseType().IsClassType() && fromType.GetBaseType().IsClassType()) )
		return false;

	// ���� ������, ����� ��������� ������ ���� lvalue
	if( toType.GetDerivedTypeList().IsReference() && 
		toType.GetDerivedTypeList().GetDerivedTypeCount() == 1 )
		return ExpressionMakerUtils::IsLValue(right);

	// ����� ������ ���� ���� ���������
	else if( toType.GetDerivedTypeList().IsPointer() &&
		toType.GetDerivedTypeList().GetDerivedTypeCount() == 1 )
	{
		return (fromType.GetDerivedTypeList().GetDerivedTypeCount() == 1 &&
			   fromType.GetDerivedTypeList().IsPointer()) ||
			   (fromType.GetDerivedTypeList().GetDerivedTypeCount() == 2 &&
			   fromType.GetDerivedTypeList().IsReference() &&
			   fromType.GetDerivedTypeList().GetDerivedType(1)->GetDerivedTypeCode() ==
			   DerivedType::DT_POINTER) ;
	}

	// ����� ����������
	else
		return false;
}


// ��������� ��������������
int StaticCastBinaryMaker::CheckConversion( )
{
	// ������ ��������� � ���� ������� ��� �������
	if( left->GetType().GetDerivedTypeList().IsArray() || 
		left->GetType().GetDerivedTypeList().IsFunction() ) 
		return -1;
	
	// ������� �������, ������������� � ������� ��������������� ��������������,
	// ���� "T t(e)"
	PCaster pCaster= 
		AutoCastManager(left->GetType(), right->GetType(), true, true).RevealCaster();
	Caster &caster = *pCaster;
	caster.ClassifyCast();

	// ���� �������������� ��������, ����������� �������� ��������� �
	// ���������� 1
	if( caster.IsConverted() )
	{
		caster.DoCast(left, const_cast<POperand&>(right), errPos);
		return 1;
	}

	// ����� �������� ������������� �������� ������ ��������
	// �������� �������������� � void
	const TypyziedEntity &toType = left->GetType(), &fromType = right->GetType();
	if( toType.GetBaseType().GetBaseTypeCode() == BaseType::BT_VOID &&
		toType.GetDerivedTypeList().IsEmpty() )
		return 0;

	// �� lvalue 'B', � 'cv D &', ���� �� B * � D *
	if( BaseToDerivedExist(toType, fromType) )
	{	
		// ���������, ����� ������������ ��������� ���� ������
		if( fromType.IsConst() > toType.IsConst() ||
			fromType.IsVolatile() > toType.IsVolatile() )
			return -1;

		// ���������, ����� 'B' ��� ����������� ���������, �� ����������� ������� ������� 'D'
		DerivationManager dm( static_cast<const ClassType &>(fromType.GetBaseType()),
			static_cast<const ClassType &>(toType.GetBaseType()) );
		
		// ���������, ����� ��� �������, �� �����������, �����������, ���������
		if( !dm.IsBase() || dm.IsVirtual() || !dm.IsUnambigous() || !dm.IsAccessible() )
			return -1;

		// ����� ������ ��������� � ���������� 1
		const_cast<POperand&>(right) = 
			new BinaryExpression( GOC_BASE_TO_DERIVED_CONVERSION, 
				false, left, right, new TypyziedEntity(left->GetType()) );
		return 1;	
	}

	// ���� ��� ���� �����, ����� ��� ����� ������������� � ������������
	if( ExpressionMakerUtils::IsIntegral(toType) && ExpressionMakerUtils::IsIntegral(fromType) )
	{
		if( toType.GetBaseType().GetBaseTypeCode() == BaseType::BT_ENUM )
			return 0;
		return -1;
	}

	// ��������� �� void, ����� ������������� � ��������� �� ��. ���
	if( fromType.GetBaseType().GetBaseTypeCode() == BaseType::BT_VOID &&
		fromType.GetDerivedTypeList().GetDerivedTypeCount() == 1 &&
		fromType.GetDerivedTypeList().IsPointer() ) 
	{
		if( toType.GetDerivedTypeList().IsPointer() &&
			toType.GetDerivedTypeList().GetDerivedTypeCount() == 1 )
			return 0;
	}

	return -1;
}

	
// ������� ��������� static_cast
POperand StaticCastBinaryMaker::Make()
{
	if( int r = CheckConversion() )
	{
		// ���� r==1, ���������� �������������� �� ���������,
		// ���������� right
		if( r == 1 )
			return right;

		theApp.Error(errPos, "'static_cast' - ���������� ������������� '%s' � '%s'",
			right->GetType().GetTypyziedEntityName(false).c_str(),
			left->GetType().GetTypyziedEntityName(false).c_str());
		return ErrorOperand::GetInstance();
	}

	bool llv = 
		static_cast<const TypeOperand&>(*left).GetType().GetDerivedTypeList().IsReference();
	return new BinaryExpression
		(KWSTATIC_CAST, llv, left, right, new TypyziedEntity(left->GetType()) );
}


// ������� ��������� reinterpret_cast
POperand ReinterpretCastBinaryMaker::Make()
{
	const TypyziedEntity &fromType = right->GetType(), &toType = left->GetType();
	const DerivedTypeList &fromDtl = fromType.GetDerivedTypeList(), 
						  &toDtl = toType.GetDerivedTypeList();

	// ������ ��������� � ���� ������� ��� �������
	if( toDtl.IsArray() || toDtl.IsFunction() ) 
	{
		theApp.Error(errPos, 
			"'reinterpret_cast' - �������������� ��� �� ����� ���� �������� ��� ��������");
		return ErrorOperand::GetInstance();
	}

	// ���� ���� ��� �����, � ������ ��������� ��� ��������, ����
	// ��� ���������
	if( (ExpressionMakerUtils::IsIntegral(fromType) &&
		 ExpressionMakerUtils::IsRvaluePointer(toType))     ||
		(ExpressionMakerUtils::IsIntegral(toType) &&
		 ExpressionMakerUtils::IsRvaluePointer(fromType))   ||
		(ExpressionMakerUtils::IsRvaluePointer(fromType) &&
		 ExpressionMakerUtils::IsRvaluePointer(toType))	)
		 ;

	// ����� ���� ��� ��������� �� ����, ���������, ����� ��� ����
	// ������� ��� �������
	else if( toDtl.IsPointerToMember() && fromDtl.IsPointerToMember() &&
			 ((toDtl.GetDerivedTypeCount() > 1 && toDtl.GetDerivedType(1)->
			   GetDerivedTypeCode() == DerivedType::DT_FUNCTION_PROTOTYPE) +
			  (fromDtl.GetDerivedTypeCount() > 1 && fromDtl.GetDerivedType(1)->
			   GetDerivedTypeCode() == DerivedType::DT_FUNCTION_PROTOTYPE)) != 1 )
		;

	// ���� ������ ������� lvalue, � �������� ��� - ������,
	// �������������� ��������.
	else if( toType.GetDerivedTypeList().IsReference() &&
		((right->IsExpressionOperand() && static_cast<const Expression&>(*right).IsLvalue()) ||
		 (right->IsPrimaryOperand() && static_cast<const PrimaryOperand&>(*right).IsLvalue())) )
		;

	// ����� ���������� ����������
	else
	{
		theApp.Error(errPos, "'reinterpret_cast' - ���������� ������������� '%s' � '%s'",
			right->GetType().GetTypyziedEntityName(false).c_str(),
			left->GetType().GetTypyziedEntityName(false).c_str());
		return ErrorOperand::GetInstance();
	}

	// � �����, ���� �������������� ��� ������, ��������� lvalue
	return new BinaryExpression(KWREINTERPRET_CAST, toType.GetDerivedTypeList().IsReference(),
		left, right, new TypyziedEntity(left->GetType()));
}


// ������� ��������� const_cast. ����� ������� ErrorOperand, ���� ���������� ����������
POperand ConstCastBinaryMaker::Make()
{ 
	// ���������� ����
	ScalarToScalarCaster stsc(left->GetType(), right->GetType(), false);
	stsc.ClassifyCast();
	if( !stsc.IsConverted() || stsc.GetCastCategory() != Caster::CC_EQUAL )
	{
		theApp.Error(errPos, "'const_cast' - ���������� ������������� '%s' � '%s'",
			right->GetType().GetTypyziedEntityName(false).c_str(),
			left->GetType().GetTypyziedEntityName(false).c_str());
		return ErrorOperand::GetInstance();
	}

	// ���������� rvalue, ������������� � lvalue
	bool llv = 
		static_cast<const TypeOperand&>(*left).GetType().GetDerivedTypeList().IsReference(),
		rlv = ExpressionMakerUtils::IsLValue(right);

	// ���������� �������������, ���� ����� lvalue, � ������ rvalue
	if( llv && !rlv )
	{
		theApp.Error(errPos, "'const_cast' - ���������� ������������� rvalue � lvalue");
		return ErrorOperand::GetInstance();
	}

	// �������������� ��� - �����
	return new BinaryExpression(KWCONST_CAST, llv, left, right, 
		new TypyziedEntity(left->GetType()) );
}


// ������� ��������� throw
POperand ThrowUnaryMaker::Make()
{	
	// ������ ��� ��������. ������� ��������, ��� throw ����� ����
	// ��� ���������, ����� right - ��������� ��� ������� � ����� void
	return new UnaryExpression(KWTHROW, false, false,
		right, new TypyziedEntity(
		(BaseType*)&ImplicitTypeManager(KWVOID).GetImplicitType(), false, false,
		DerivedTypeList()) );	
}

