// �������� ������������ ��������� - ExpressionChecker.cpp

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
#include "Maker.h"
#include "MemberMaker.h"
#include "Parser.h"
#include "Body.h"
#include "Checker.h"
#include "ExpressionMaker.h"


// ����������� ������ ������������
list<AgregatController *> ListInitializationValidator::allocatedControllers;


// ����� ��������� �������� �� ��� ���������
bool ExpressionMakerUtils::IsClassType( const TypyziedEntity &type )
{
	if( !type.GetBaseType().IsClassType() ||
		type.GetDerivedTypeList().GetDerivedTypeCount() > 1 )
		return false;

	return type.GetDerivedTypeList().IsEmpty() || type.GetDerivedTypeList().IsReference();
}


// ��������, ������ �� ��������� � �� ������� � �� void
bool ExpressionMakerUtils::IsScalarType( const TypyziedEntity &type )
{
	if( IsClassType(type) )
		return false;

	// ��� �������
	if( type.GetDerivedTypeList().IsFunction() ||
		(type.GetDerivedTypeList().GetDerivedTypeCount() > 1 &&
		type.GetDerivedTypeList().IsReference() && type.GetDerivedTypeList().GetDerivedType(1)->
		GetDerivedTypeCode() == DerivedType::DT_FUNCTION_PROTOTYPE) )
		return false;

	// ��� void
	if( type.GetBaseType().GetBaseTypeCode() == BaseType::BT_VOID &&
		type.GetDerivedTypeList().IsEmpty() )
		return false;

	// ����� ��� ��������
	return true;
}


// ��������, �������� �� ������� �����������. �.�. ����������
// �� ����� ������� ������, ���� ���������, ���� ������� ���
bool ExpressionMakerUtils::IsConstant( const TypyziedEntity &op )
{	
	// �������� �� ������ ����������� ����� �������
	for( int i = 0; i < op.GetDerivedTypeList().GetDerivedTypeCount(); i++ )
	{
		const DerivedType &dt = *op.GetDerivedTypeList().GetDerivedType(i);
		if( dt.GetDerivedTypeCode() == DerivedType::DT_POINTER )
			return static_cast<const Pointer &>(dt).IsConst();

		else if( dt.GetDerivedTypeCode() == DerivedType::DT_POINTER_TO_MEMBER )
			return static_cast<const PointerToMember &>(dt).IsConst();
	}

	// ����� ������ ���� ������, ��������� ������� ���
	return op.IsConst();
}


// ����� ��������� ��������� ��������������� ����� � ��������� ���������.
// �������� ������ ���������� ����� � ����������� �����, ��� ����� ��������������
bool ExpressionMakerUtils::CompareTypes( 
		const TypyziedEntity &type1, const TypyziedEntity &type2 )
{
	// 1. ��������� ������� ����
	const BaseType &bt1 = type1.GetBaseType(),
				   &bt2 = type2.GetBaseType();

	// ��������� ������� ����, ��� ���� ������ ���������
	if( &bt1 != &bt2								   ||
		bt1.GetSignModifier() != bt2.GetSignModifier() ||
		bt1.GetSizeModifier() != bt2.GetSizeModifier() )
		return false;

	// 3. ��������� ������ ����������� �����
	const DerivedTypeList &dtl1 = type1.GetDerivedTypeList(),
						  &dtl2 = type2.GetDerivedTypeList();

	// 3�. ������� ������� ������ ���������
	if( dtl1.GetDerivedTypeCount() != dtl2.GetDerivedTypeCount() )
		return false;

	// �������� �� ����� ������ �������� ������ ����������� ��� �� �����������
	for( int i = 0; i<dtl1.GetDerivedTypeCount(); i++ )
	{
		const DerivedType &dt1 = *dtl1.GetDerivedType(i),
						  &dt2 = *dtl2.GetDerivedType(i);

		// ���� ���� ����������� ����� �� ����� �������
		if( dt1.GetDerivedTypeCode() != dt2.GetDerivedTypeCode() )
			return false;

		// ����� ��������� ������������� �������� ������������ ����
		DerivedType::DT dc1 = dt1.GetDerivedTypeCode(), 
						dc2 = dt2.GetDerivedTypeCode();

		// ���� ������ ��� ���������, �� ��������� ������
		if( dc1 == DerivedType::DT_REFERENCE || dc2 == DerivedType::DT_POINTER )
			;

		// ���� ��������� �� ���� �������� ������������� � ����� � ��������
		// ����������� ���������
		else if( dc1 == DerivedType::DT_POINTER_TO_MEMBER )
		{
			const PointerToMember &ptm1 = static_cast<const PointerToMember &>(dt1),
								  &ptm2 = static_cast<const PointerToMember &>(dt2);

			// ��������, ����� ������ ���������
			if( &ptm1.GetMemberClassType() != &ptm2.GetMemberClassType() )
				return false;
		}

		// ���� ������, �������� ������� ���� ��������
		else if( dc1 == DerivedType::DT_ARRAY )
		{
			if( dt1.GetDerivedTypeSize() != dt2.GetDerivedTypeSize() )
				return false;
		}

		// ���� �������� �������, �� �������� ������ ������� ��� ������� ���������,
		// � ����� �������� cv-������������� � throw-������������
		else if( dc1 == DerivedType::DT_FUNCTION_PROTOTYPE )
		{
			const FunctionPrototype &fp1 = static_cast<const FunctionPrototype &>(dt1),
								    &fp2 = static_cast<const FunctionPrototype &>(dt2);	
			
			// ���������� ���������� ������ ���������
			const FunctionParametrList &fpl1 = fp1.GetParametrList(),
									   &fpl2 = fp2.GetParametrList();

			if( fpl1.GetFunctionParametrCount() != fpl2.GetFunctionParametrCount() )
				return false;

			// ��������� ������ �������� � ������ �� ������������
			for( int i = 0; i<fpl1.GetFunctionParametrCount(); i++ )
				if( !CompareTypes(*fpl1[i], *fpl2[i]) )
					return false;					
		}

		// ����� ����������� ���
		else
			INTERNAL("'ExpressionMakerUtils::CompareTypes' ����������� ��� ������������ ����");
	}

	return true;	
}


// ������� ����� �������
POperand ExpressionMakerUtils::MakeFunctionCall( POperand &fn, PExpressionList &params )
{
	// ������� ��������������� ��� ������ �������
	PTypyziedEntity rt = new TypyziedEntity(fn->GetType());	
	const_cast<DerivedTypeList &>(rt->GetDerivedTypeList()).PopHeadDerivedType();
	
	// ���������� ����� �������
	return new FunctionCallExpression( rt->GetDerivedTypeList().IsReference(),
		fn, params, rt);
}


// �������� ��������� ������������� ��������� �� ����
PCSTR ExpressionPrinter::GetOperatorName( int opCode )
{	
	// ���� �������� ������� ���������� []
	switch( opCode )
	{
	case KWNEW:				return "new";		
	case -KWDELETE:		
	case KWDELETE:			return "delete";	
	case OC_NEW_ARRAY:		return "new[]";	
	case -OC_DELETE_ARRAY:  
	case OC_DELETE_ARRAY:	return "delete[]";	
	case OC_FUNCTION:		return "()";		
	case OC_ARRAY:			return "[]";		
	case PLUS_ASSIGN:		return "+=";		
	case MINUS_ASSIGN:		return "-=";		
	case MUL_ASSIGN:		return "*=";		
	case DIV_ASSIGN:		return "/=";		
	case PERCENT_ASSIGN:	return "%=";		
	case LEFT_SHIFT_ASSIGN:	return "<<=";		
	case RIGHT_SHIFT_ASSIGN:return ">>=";		
	case AND_ASSIGN:		return "&=";		
	case XOR_ASSIGN:		return "^=";		
	case OR_ASSIGN:			return "|=";		
	case LEFT_SHIFT:		return "<<";		
	case RIGHT_SHIFT:		return ">>";		
	case LESS_EQU:			return "<=";		
	case GREATER_EQU:		return ">=";		
	case EQUAL:				return "==";		
	case NOT_EQUAL:			return "!=";		
	case LOGIC_AND:			return "&&";		
	case LOGIC_OR:			return "||";		
	case -INCREMENT:
	case INCREMENT:			return "++";		
	case -DECREMENT:
	case DECREMENT:			return "--";		
	case ARROW:				return "->";		
	case ARROW_POINT:		return "->*";		
	case DOT_POINT:			return ".*";		
	case KWTHROW:			return "throw";
	case KWTYPEID:			return "typeid";
	case KWSIZEOF:			return "sizeof";
	case KWREINTERPRET_CAST: return "reinterpret_cast";
	case KWSTATIC_CAST:		 return "static_cast";
	case KWCONST_CAST:	 	 return "const_cast";
	case KWDYNAMIC_CAST:     return "dynamic_cast";	
	case GOC_REFERENCE_CONVERSION: return "*";	
	default:
		{
			static char buf[2] = " ";
			buf[0] = opCode;
			return buf;
		}		
	}	
}


// ����������� �������� ���������
string ExpressionPrinter::PrintBinaryExpression( const BinaryExpression &expr )
{
	string op = GetOperatorName(expr.GetOperatorCode());

	// ��������� ����� ��������
	switch( expr.GetOperatorCode() ) 
	{
	case OC_CAST:
	case GOC_BASE_TO_DERIVED_CONVERSION:
	case GOC_DERIVED_TO_BASE_CONVERSION:
		return " ((" + PrintExpression(expr.GetOperand1()) + ')' +
				PrintExpression(expr.GetOperand2()) + ") ";

	case KWREINTERPRET_CAST:
	case KWSTATIC_CAST:
	case KWCONST_CAST:
	case KWDYNAMIC_CAST:
		return op + '<' + PrintExpression(expr.GetOperand1()) + ">(" +
				PrintExpression(expr.GetOperand2()) + ')';
	case OC_ARRAY:
		return PrintExpression(expr.GetOperand1()) + '[' +
			   PrintExpression(expr.GetOperand2()) + ']';
	default:
		return PrintExpression(expr.GetOperand1()) + ' ' + op + ' ' +
			   PrintExpression(expr.GetOperand2());
	}
}


// ����������� ���������
string ExpressionPrinter::PrintExpression( const POperand &expr )
{
	// ���� ���������, ����������� �������� � �������� ����������
	if( expr->IsExpressionOperand() )
	{
		const Expression &expop = static_cast<const Expression&>(*expr);
		
		// ���� ��������� �������� �������
		if( expop.IsUnary() )
		{
			string op = GetOperatorName(expop.GetOperatorCode()),
				   temp = PrintExpression( 
							static_cast<const UnaryExpression&>(expop).GetOperand() );

			int opC = expop.GetOperatorCode();
			if( opC == INCREMENT || opC == DECREMENT )
				return temp + op;

			else if( opC == KWSIZEOF || opC == KWTYPEID )
				return op + '(' + temp + ')';

			else if( abs(opC) == KWDELETE ||  opC == KWTHROW )
				return op + ' ' + temp;
			else
				return op + temp;
		}

		else if( expop.IsBinary() )
			return PrintBinaryExpression( static_cast<const BinaryExpression&>(expop) );

		else if( expop.IsTernary() )
		{
			const TernaryExpression &tern = static_cast<const TernaryExpression&>(expop);
			return PrintExpression(tern.GetOperand1()) + " ? " +
				   PrintExpression(tern.GetOperand2()) + " : " +
				   PrintExpression(tern.GetOperand3());
		}

		else if( expop.IsFunctionCall() || expop.IsNewExpression() )
		{
			const Operand *pfce = &expop;
			if( expop.IsNewExpression() )
				pfce = &*static_cast<const NewExpression &>(expop).GetNewOperatorCall();
			const FunctionCallExpression &fce = 
				static_cast<const FunctionCallExpression&>(*pfce);
			string temp = PrintExpression(fce.GetFunctionOperand()) + '(';
			for( int i = 0; i<fce.GetParametrList()->size(); i++ )
			{
				temp += PrintExpression(fce.GetParametrList()->at(i));
				if( i != fce.GetParametrList()->size()-1 )
					temp += ", ";
			}

			temp += ')';
			return temp;
		}

		else
			return "<����������� ���������>";
	}

	else if( expr->IsPrimaryOperand() )
	{
		const TypyziedEntity &te = expr->GetType().IsDynamicTypyziedEntity()		 ?
			static_cast<const DynamicTypyziedEntity&>(expr->GetType()).GetOriginal() : 
			expr->GetType();
		if( te.IsLiteral() )
			return static_cast<const Literal &>(te).GetLiteralValue().c_str();

		else if( const Identifier *id = dynamic_cast<const Identifier *>(&te) )
			return id->GetQualifiedName().c_str();

		else if( &te == &expr->GetType() )
			return " this ";

		else
			return te.GetTypyziedEntityName(false).c_str();
	}

	else if( expr->IsTypeOperand() )
		return expr->GetType().GetTypyziedEntityName(false).c_str();

	else if( expr->IsErrorOperand() )
		return "<error operand>";

	else if( expr->IsOverloadOperand() )
		return "<overload operand>";

	else
	{
		INTERNAL( "'ExpressionMakerUtils::PrintExpression' - ����������� �������" );
		return "";
	}
}


// ������� ��������������� �� ������ ���� �����
PCaster &AutoCastManager::RevealCaster()
{
	register bool cls1 = ExpressionMakerUtils::IsClassType(destType), 
				  cls2 = ExpressionMakerUtils::IsClassType(srcType);

	// ��� ������, ���� ����� �����������, �� �������� �����������
	// �������������� �� ��������� � ���������
	if( isCopy && cls1 && !cls2 )
		return caster = new ConstructorCaster(destType, srcType, explicitCast);	

	// �� ������ ��������� ������, �������� 
	if( cls1 )
		return caster = (cls2 ? (Caster *)new ClassToClassCaster(destType, srcType, explicitCast) 
					          : (Caster *)new OperatorCaster(destType, srcType, isCopy));
	else if( cls2 )
		return caster = new OperatorCaster(destType, srcType, isCopy);

	// ����� �������� ���������� � ���������
	else
		return caster = new ScalarToScalarCaster(destType, srcType, isCopy);
}


// ����� �������� ���������� ��� �� ���� �� �������� �����
const TypyziedEntity *ScalarToScalarCaster::GetBiggerType()
{
	// ������� ���������, ���� ��� ���� ���������
	if( &destType.GetBaseType() == &srcType.GetBaseType() )
	{		
		castCategory = CC_EQUAL;
		return &destType;
	}

	const BaseType &bt1 = destType.GetBaseType(), &bt2 = srcType.GetBaseType();
	BaseType::BT btc1 = bt1.GetBaseTypeCode(), 
			     btc2 = bt2.GetBaseTypeCode();

	// ���� ���� �� ����� double, ������ � ������ double
	if( btc1 == BaseType::BT_DOUBLE )
	{
		if( btc2 == BaseType::BT_DOUBLE )
			return bt1.IsLong() ? &destType : &srcType;
		else
			return &destType;
	}

	else if( btc2 == BaseType::BT_DOUBLE )
		return &srcType;

	// ���� ���� �� ����� float, ������ � ������
	else if( btc1 == BaseType::BT_FLOAT )
		return &destType;
		
	else if( btc2 == BaseType::BT_FLOAT )
		return &srcType;
	
	// ����� int
	else
	{
		bool uns = bt1.IsUnsigned() || bt2.IsUnsigned(),
			 lng = bt1.IsLong() || bt2.IsLong();
		BaseType *rbt = (BaseType *)&ImplicitTypeManager(KWINT, uns ? KWUNSIGNED : -1, 
							lng ? KWLONG : -1).GetImplicitType();
		resultType = new TypyziedEntity(rbt, false, false, DerivedTypeList());
		return resultType ;
	}
}

// ��������� ����������� �������������� �� ������� � ���������,
// �� ������� � ��������� �� �������, � �� ������ � ��������� �� ���� �������,
// ���� ���������
bool ScalarToScalarCaster::StandardPointerConversion( TypyziedEntity &type,
									const TypyziedEntity &orig )
{
	DerivedTypeList &dtl = const_cast<DerivedTypeList &>(type.GetDerivedTypeList());

	// ���� ����� ������, ����������� ��� � ���������
	if( dtl.IsArray() )
		dtl.PopHeadDerivedType(),
		dtl.PushHeadDerivedType( new Pointer(false,false) );
	
	// ����������� � ��������� �� ������� ��� � ��������� �� �������-����
	else if( dtl.IsFunction() )		
	{
		const Function &fn = *dynamic_cast<const Function *>(&orig);

		// ���� ������������ �������-����, ����������� � ��������� �� �-�
		if( &fn && fn.IsClassMember() && fn.GetStorageSpecifier() != Function::SS_STATIC )
			dtl.PushHeadDerivedType( new PointerToMember(
				&static_cast<const ClassType&>(fn.GetSymbolTableEntry()), false, false) );
		else
			dtl.PushHeadDerivedType( new Pointer(false,false) );
	}	

	return true;
}


// �������������� ������������ � ��������, ������ ������ ��� ��������
int ScalarToScalarCaster::DerivedToBase( 
					const ClassType &base, const ClassType &derived, bool ve )
{
	DerivationManager dm( base, derived );
	if( dm.IsBase() )
	{
		// ���� ������� ����� ������������
		if( !dm.IsUnambigous() )
		{
			errMsg = ('\'' + string(base.GetQualifiedName().c_str()) +
				"\' ������������� ������� �����; ���������� ���� ����������").c_str();
			return -1;
		}
		
		// ���� ����������� �������� � ����� ���������
		else if( ve && dm.IsVirtual() )
		{
			errMsg = ('\'' + string(base.GetQualifiedName().c_str()) +
				"\' ����������� ������� �����; ���������� ���� ����������").c_str();
			return -1;
		}


		// ���� ����� ����������, �������� ��������� �� ����
		else if( !dm.IsAccessible() )
		{
			errMsg = ('\'' + string(base.GetQualifiedName().c_str()) +
				"\' ����������� ������� �����; ���������� ���� ����������").c_str();
			return -1;
		}

		// ����� ���������� �������
		else
		{
			isConverted = true;	
			castCategory = CC_STANDARD;
			derivedToBase = true;
			return 0;
		}
	}

	return 1;
}


// �������������� ������������ � ��������, ���� ��������,
// ������������� � ������� 0. ���������� -1, ���� ��������������
// ������ � �������, ����� > 0.
int ScalarToScalarCaster::DerivedToBase( 
		const TypyziedEntity &base, const TypyziedEntity &derived, bool ve )
{
	// ��� ���� ������ ���� ����������
	if( !base.GetBaseType().IsClassType() || !derived.GetBaseType().IsClassType() )
		return 1;

	const ClassType &cls1 = static_cast<const ClassType&>(base.GetBaseType()),
					&cls2 = static_cast<const ClassType&>(derived.GetBaseType());

	int r = DerivedToBase(cls1, cls2, ve);
	if( !r )
		resultType = &base;
	return r;
}


// ���������� ��������� �������������� ��� �������������� �����,
// ����������� ������� ����� � ������ �����������
void ScalarToScalarCaster::SetCastCategory( const BaseType &dbt, const BaseType &sbt )
{
	INTERNAL_IF( resultType == NULL );
	if( castCategory != CC_NOCAST )
		return;

	// ���� ��� ���� ���������, ������ ���
	if( &dbt == &sbt )
	{
		castCategory = CC_EQUAL;
		return;
	}

	// ��������� ����� �������� ������ ��� ���������� ���������������,
	// ������� ���� �� �����������, ������ ����������� �������������� � ���
	if( !isCopy )
	{
		castCategory = CC_STANDARD;
		return;
	}

	// sbt ���������� � dbt, �������� ����� ��� ��������������. 
	// ���� ������ ������� ���� �� �����������, ���� �� ���� ������
	// �������������� �����������. ����������� ����� � ������ ���� ��� ����
	// ������������ � sbt ������ dbt, ���� ���� sbt - float, � dbt - double
	register BaseType::BT dc = dbt.GetBaseTypeCode(), sc = sbt.GetBaseTypeCode();
	if( ExpressionMakerUtils::IsIntegral(destType) && 
		ExpressionMakerUtils::IsIntegral(srcType) )
	{
		if( dc > sc )
			castCategory = CC_INCREASE;

		else if( dc == sc )
		{
		    if( dbt.GetSizeModifier() == sbt.GetSizeModifier() )
				castCategory = dbt.GetSignModifier() >= sbt.GetSignModifier() ?
					CC_INCREASE : CC_STANDARD;
			else
		       castCategory = 
				(sbt.IsShort() && dbt.GetSizeModifier() == BaseType::MZ_NONE &&
				 dbt.GetSignModifier() == sbt.GetSignModifier()) ?
					CC_INCREASE : CC_STANDARD;			
		}
		    
		else
			castCategory = CC_STANDARD;
	}

	// ����� ���� sbt-float, dbt-double
	else if( sc == BaseType::BT_FLOAT && dc == BaseType::BT_DOUBLE )
		castCategory = CC_INCREASE;

	// ����� ����������� ��������������
	else
		castCategory = CC_STANDARD;
}


// �������� ������������, � ������ �������� ��������������. �����
// �������� ������ ��� �����������. ������ ������� ������ ���� �����
// ��������������, ��� ������. � ������, ���� ������������ 'src',
// ������ ��� 'dest' ������ false
bool ScalarToScalarCaster::QualifiersCheck( TypyziedEntity &dest, TypyziedEntity &src )
{
	DerivedTypeList &dtl1 = const_cast<DerivedTypeList &>(dest.GetDerivedTypeList()),
					&dtl2 = const_cast<DerivedTypeList &>(src.GetDerivedTypeList());
	
	if( !origDestType.GetDerivedTypeList().IsEmpty() &&
		(origDestType.IsConst() < src.IsConst() || 
		origDestType.IsVolatile() < src.IsVolatile())  )
		return false;

	// ������� ���������, ����������� ����� ���
	bool notEq = dest.IsConst() != src.IsConst() || dest.IsVolatile() != src.IsVolatile();
	for( int i = 1; i<dtl1.GetDerivedTypeCount(); i++ )
	{
		const DerivedType &d1 = *dtl1.GetDerivedType(i), &d2 = *dtl2.GetDerivedType(i);
		if( d1.GetDerivedTypeCode() != d2.GetDerivedTypeCode() )
			return true;

		bool c1, c2, v1, v2;
		if( d1.GetDerivedTypeCode() == DerivedType::DT_POINTER )
		{
			c1 = static_cast<const Pointer &>(d1).IsConst();
			c2 = static_cast<const Pointer &>(d2).IsConst();
			v1 = static_cast<const Pointer &>(d1).IsVolatile();
			v2 = static_cast<const Pointer &>(d2).IsVolatile();
		}

		else if( d1.GetDerivedTypeCode() == DerivedType::DT_POINTER_TO_MEMBER )
		{
			c1 = static_cast<const PointerToMember &>(d1).IsConst();
			c2 = static_cast<const PointerToMember &>(d2).IsConst();
			v1 = static_cast<const PointerToMember &>(d1).IsVolatile();
			v2 = static_cast<const PointerToMember &>(d2).IsVolatile();
		}

		else
			return true;

		// ���� �� �����, const ������ ��������������
		if( notEq )
			if( !c1 )
				return false;
			else
				continue;

		// ����� ��������� ����� ������������ ���������
		else
			notEq = c1 != c2 || v1 != v2;
	}

	// ���� ������������ �� �����, ���� ��������������,
	// ������������� ����� �������� ���������
	if( notEq && castCategory == CC_EQUAL )		
		castCategory = CC_QUALIFIER;

	return true;
}


// ���������������� ��������������. ��������� ��������� �����������
// �������������� ������ ���� � ������, � ����� ������� �����������
// ���������� ��� ����������� ��������������
void ScalarToScalarCaster::ClassifyCast()
{
	// �������� ������ ����������� �����
	DerivedTypeList &ddtl = const_cast<DerivedTypeList &>(destType.GetDerivedTypeList()),
				    &sdtl = const_cast<DerivedTypeList &>(srcType.GetDerivedTypeList());

	// ������� ����������� ��������� ��� � ��-���������, ���� ���������
	if( isRefConv1 )
		const_cast<DerivedTypeList &>(ddtl).PopHeadDerivedType();

	if( isRefConv2 )
		const_cast<DerivedTypeList &>(sdtl).PopHeadDerivedType();

	// ��������� ����������� �������������� ����������. �� �������, �������, ������
	if( (isCopy && !StandardPointerConversion(destType, origDestType)) ||
		 !StandardPointerConversion(srcType, origSrcType) )
		return;

	// ���� ��� ���� ��������������
	if( ExpressionMakerUtils::IsArithmetic(destType) &&
		ExpressionMakerUtils::IsArithmetic(srcType) )
	{
		// ���� �����������, �� ��������� ������ ��� � �������� ���������������.
		// �� ���� ���� ������. ���� ����� ��������� ������������ ���, �� � ������
		// ������ ���� ����� �� ������������ ���
		if( isCopy )
		{
			// ������, ������������ ���� �� ���������
			if( destType.GetBaseType().IsEnumType() && 
				&destType.GetBaseType() != &srcType.GetBaseType() )
				;
			else			
				resultType = &destType,
				isConverted = true,
				SetCastCategory( destType.GetBaseType(), srcType.GetBaseType() );			
		}

		else
		{
			resultType = GetBiggerType();
			isConverted = true;
			SetCastCategory( destType.GetBaseType(), srcType.GetBaseType() );
		}
	}

	// ���� ���� �� ����� ��������� ��� ��������� �� ����, � ������
	// �����
	else if( ( (ddtl.IsPointer() || ddtl.IsPointerToMember()) && 
				ExpressionMakerUtils::IsIntegral(srcType) ) 			||
			 ( (sdtl.IsPointer() || sdtl.IsPointerToMember()) && 
			    ExpressionMakerUtils::IsIntegral(destType) ) )
	{
		const TypyziedEntity &lit = destType.GetDerivedTypeList().IsPointer() || 
			destType.GetDerivedTypeList().IsPointerToMember() ? origSrcType : origDestType;

		// ������� ������������� � ������� ���������
		if( lit.IsLiteral() && 
			atoi(static_cast<const Literal&>(lit).GetLiteralValue().c_str()) == 0 )
		{
			resultType = &lit == &origSrcType ? &destType : &srcType;
			castCategory = CC_STANDARD;
			isConverted = true;

			// ��� �������������� � ������� ���������, �������� �� �������������������
			// �� ����������� � ����� ����� �����
			return;
		}
	}

	// ���� ��� ���� ���������, �������� ������������� ������� ��� � ������������,
	// ���� ���� ��������� � ���� void *, ���� ��� ��������� ����� ���� ���
	else if( ddtl.IsPointer() && sdtl.IsPointer()  )
	{
		// ���������, ���� ���� �� ����������, ��������� �� 'void',
		// �� �������������� �������
		if( (ddtl.GetDerivedTypeCount() == 1 && 
			 destType.GetBaseType().GetBaseTypeCode() == BaseType::BT_VOID) )			
			isConverted = true, resultType = &destType,
			castCategory = CC_STANDARD;

		// ���� �� �����������, �� � � ������ �� ������ ��������� ������. �������
		else if( !isCopy							&&
			sdtl.GetDerivedTypeCount() == 1			&&
			srcType.GetBaseType().GetBaseTypeCode() == BaseType::BT_VOID )			
			isConverted = true, resultType = &srcType,
			castCategory = CC_STANDARD;				


		// ���� ���������� ����������� ����� ������, ���������� ��������
		// �� ����� ������
		else if( ddtl.GetDerivedTypeCount() != sdtl.GetDerivedTypeCount() )
			;

		// ���� ���������� ����������� ����� ������ ������,
		// �� �������� ���� �� ��������
		else if( ddtl.GetDerivedTypeCount() > 1 )
		{
			isConverted = ExpressionMakerUtils::CompareTypes(destType, srcType);
			if( isConverted )			
				resultType = &destType,
				castCategory = CC_EQUAL;				
		}

		// ���� ������� ���� �����, �� ���������� ������� � ����� ������
		else if( &destType.GetBaseType() == &srcType.GetBaseType() )
			isConverted = true,	resultType = &destType,
			castCategory = CC_EQUAL;

		// ����� ����� ��� ���������. ���� �������� �����������, ������
		// ��� ����� ������������� � ������ ���� �� 'void *'
		else if( isCopy )
		{					
			// ���� ����� � ������ ������� ����� ��� ������, �������� �������������
			// ������ � ������, ���� �� ����������� �����
			if( !DerivedToBase(destType, srcType, false) )
				castCategory = CC_STANDARD;		

		}	// ���������� ����������, ������ � ������ �����������

		// ����� �������� ����� ����� �� �������-����������� � �� void
		else
		{
			// ������� �������� ������������� ���� ����� � �������,
			// ����� �������			
			if( DerivedToBase(destType, srcType, false) > 0 )
				DerivedToBase(srcType, destType, false);			
		}
	}

	// ���� ��� ���� - ��������� �� ����, �������� �� �����.
	else if( ddtl.IsPointerToMember() && sdtl.IsPointerToMember() )
	{
		// ��������, �����. ���� ����� ��� ���������� �� ����
		PDerivedType ptm1( ddtl[0] ), ptm2( sdtl[0] );
		ddtl.PopHeadDerivedType();
		sdtl.PopHeadDerivedType();

		if( ExpressionMakerUtils::CompareTypes(destType, srcType) )
		{	
			// ���� ���������� �� ���� �������, �������� ������	
			const ClassType &cls1 = static_cast<PointerToMember &>(*ptm1).GetMemberClassType(), 
							&cls2 = static_cast<PointerToMember &>(*ptm2).GetMemberClassType();

			// ���� ������ ���������, ������ ���� ���������
			if( &cls1 == &cls2 )
				isConverted = true,
				resultType = &destType,
				castCategory = CC_EQUAL;

			// ����� ���������, �������� �� ���� �� ������� �����������
			// �������
			else if( int r = DerivedToBase(cls1, cls2, true) )
			{
				if( r > 0 && !isCopy && !DerivedToBase(cls2, cls1, true) )
					resultType = &srcType;
			}

			else
				resultType = &destType;
		}

		ddtl.PushHeadDerivedType(ptm1);
		sdtl.PushHeadDerivedType(ptm2);

		// �������������� ��������� �� ����, �� ������� ������ ��������������
		// ��� ��������� ����, ������� �� ����� �������� ������ ���������
		derivedToBase = false;		
	}

	// � ��������� ������� ���������, ��������� �� ����
	else
	{
		// � ������ �����������, ����� ��� ���� ��������� ������� ���������
		// �������� �� ������ ��� ����������� ������� ������
		if( isCopy && ExpressionMakerUtils::IsClassType(destType) &&
			ExpressionMakerUtils::IsClassType(srcType) )
		{
			const ClassType &cls1 = static_cast<const ClassType &>(destType.GetBaseType()), 
							&cls2 = static_cast<const ClassType &>(srcType.GetBaseType());

			// ���� ��� ���� ���������, ���������� �������
			if( &cls1 == &cls2 )
				isConverted = true,
				resultType = &destType, castCategory = CC_EQUAL;

			// ����� ���� ������ �������� ����������� ������� ������
			else if( !DerivedToBase(cls1, cls2, false) )
				resultType = &destType;
		}

		else if( ExpressionMakerUtils::CompareTypes(destType, srcType) )
		{
			isConverted = true; 
			resultType = &destType;
			castCategory = CC_EQUAL;
		}
	}

	// ���� �������������� ����������, � ��������� �� ������, ������
	if( !isConverted )
	{
		if( errMsg.empty() )
			errMsg = ("���������� ������������� '" + string(
				srcType.GetTypyziedEntityName().c_str()) + "' � '" +
				destType.GetTypyziedEntityName().c_str() + '\'').c_str();
	}

	// �������������� ��� ������ ���������� � ����� ������
	else
	{
		INTERNAL_IF( resultType == NULL || castCategory == CC_NOCAST );

		// � ��������� ������� ��������� ������������
		if( isCopy && !QualifiersCheck(destType, srcType) )
		{
			errMsg = ("'" + string(
				srcType.GetTypyziedEntityName().c_str()) + "' ����� �������������� ��� '" +
				destType.GetTypyziedEntityName().c_str() + '\'').c_str();
			isConverted = false;
			resultType = NULL;
			castCategory = CC_NOCAST;
		}
	}	
}


// ��������� �������������� �� ������������ ������ � �������
// ����� ��������� � ������ ���������. �������� ������ ������� (derived),
// � ���� ������ (base)
void ScalarToScalarCaster::MakeDerivedToBaseConversion( 
  const TypyziedEntity &baseClsType, const POperand &base, POperand &derived )
{
	// ������� ����� ���, �� ������ �����������
	TypyziedEntity *rt = new TypyziedEntity(baseClsType);
	POperand pop = new TypeOperand( *new TypyziedEntity(baseClsType) );
	derived = new BinaryExpression( GOC_DERIVED_TO_BASE_CONVERSION, 
		baseClsType.GetDerivedTypeList().IsReference(), pop, derived, rt );		
}


// ��������� ���������� ��������������, ������� 
// ���������� ������ ���������
void ScalarToScalarCaster::DoCast( const POperand &destOp, POperand &srcOp, const Position &ep )
{
	INTERNAL_IF( resultType == NULL || castCategory == CC_NOCAST || !isConverted );

	// ���������� ������������� rvalue � lvalue ��� �����������
	if( isCopy && isRefConv1 &&
		!ExpressionMakerUtils::IsConstant(destOp->GetType())	&&
		!ExpressionMakerUtils::IsLValue(srcOp) && errMsg.empty() )
		theApp.Error(ep, "���������� ������������� 'rvalue' � 'lvalue'");

	// ��������� ��������������, ������ � ������ ���� ���� ��������������
	// �� ������������ ������ � �������
	if( derivedToBase )
	{
		if( resultType == &destType )
			MakeDerivedToBaseConversion( origDestType, destOp, srcOp );
		else
		{
			// ��� ����������� �� ����� ���������� destOp
			INTERNAL_IF( isCopy );	
			MakeDerivedToBaseConversion( origSrcType, srcOp, const_cast<POperand &>(destOp) );
		}		
	}
}


// ��������� �����. ���� ��������� � ���������� �����, ������� �� �����
// ��������� ����� � ������� ScalarToScalarCaster
int OperatorCaster::COSCast( const ClassCastOverloadOperator &ccoo )
{
	// ����� ���������, ����� ������������� �������� ��� ����� ��������������
	// ��� ��� (������). � ����� ����� ��� ��� ��������
	if( ccoo.GetFunctionPrototype().IsConst() < clsType.IsConst() ||
		ccoo.GetFunctionPrototype().IsVolatile() < clsType.IsVolatile() )
		return 1;

	// �������� �������� �������� ��� ��������� � ������������ ��������� ����	
	PScalarToScalarCaster stsc =
		 new ScalarToScalarCaster(scalarType, ccoo.GetCastType(), true);
	stsc->ClassifyCast();

	// ���� �������������� ���������� - �������
	if( !stsc->IsConverted() )
		return 1;

	// ���� ��� ������ ������� ��������������, ��������� ���
	// ��� �������������� ��������
	if( castOperator == NULL )
	{
		castOperator = &ccoo;
		scalarCaster = stsc;
		return 0;
	}

	// ����� ���������, ����� �����. ���������. ����� ���� ���������������
	else
	{
		// ���� � �������� �������������� ������� ������, ������ ������ �� ������
		if( stsc->GetCastCategory() > scalarCaster->GetCastCategory() )
			return 1;

		// ����� ���� � �������� �������������� ������� ������, ������ ������ ���
		else if( stsc->GetCastCategory() < scalarCaster->GetCastCategory() )
		{	
			castOperator = &ccoo;			
			scalarCaster = stsc;
			return 0;
		}

		// ����� ���������������, ��� �������������� ����� ���� �������.
		// ���� ��������� � ����� ������, ����������� ������ ��������,
		// ����� ����������� ���� �����, �������� ���
		else
		{						
			if( ccoo.GetName() == castOperator->GetName() )
			{
				// ������� ��������, ���� ������������� ������, �� ����� �����������������
				// ����������, � ������, ���� ������ ����� ����� ����������������� ���
				bool c1 = castOperator->GetFunctionPrototype().IsConst(),
					 v1 = castOperator->GetFunctionPrototype().IsVolatile(),
					 c2 = ccoo.GetFunctionPrototype().IsConst(),
					 v2 = ccoo.GetFunctionPrototype().IsVolatile();

				if( c1 != c2 || v1 != v2 )					
				{
					int cv1 = (c1 == scalarType.IsConst()) + (v1 == scalarType.IsVolatile());
					int cv2 = (c2 == scalarType.IsConst()) + (v2 == scalarType.IsVolatile());

					// ���� ������ �������� ����� �������������� ��� ������,
					// �������
					if( cv1 >= cv2 )
						return 1;

					// ����� ������ ��������
					else
					{
						castOperator = &ccoo;			
						scalarCaster = stsc;
						return 0;
					}
				}

				const SymbolTable *st = dynamic_cast<const SymbolTable *>(
					static_cast<const ClassType *>(&clsType.GetBaseType()) );
				INTERNAL_IF( st == NULL );
				NameManager nm(ccoo.GetName(), st);
				INTERNAL_IF( nm.GetRoleCount() == 0 );

				// ������ ���� ��� ��������� � ��������� �� ���� ����� ������ ���������
				if( nm.IsUnique() )
				{
					// ���� �������� ������, ������� ��� ��������
					if( nm.GetRoleList().front().first == (Identifier *)&ccoo )
					{
						castOperator = &ccoo;			
						scalarCaster = stsc;
						return 0;
					}
		

					// ����� �����. �������� ��������� ���� � ��������
					else
						return 1;
				}				
			}

			// ����� ���������������
			return -1;
		}
	}

	return 1;
}


// ��������� ������������ � ��������� � �������� ���������������� ���������,
// �� ������������ ���������� ������������ ��������
int OperatorCaster::CVcmp( const ClassCastOverloadOperator &ccoo )
{
	bool curC = ccoo.GetFunctionPrototype().IsConst(),
		 curV = ccoo.GetFunctionPrototype().IsVolatile();

	// ���� ������������ ������, ��� � ������� - �������� �� ��������
	if( curC < clsType.IsConst() || curV < clsType.IsVolatile() )
		return 1;

	// ���� ������������ ������ �����. ������, ��� ������� ��������
	if( castOperator != NULL )
	{
		int candCV = (castOperator->GetFunctionPrototype().IsConst() == clsType.IsConst()) +
				(castOperator->GetFunctionPrototype().IsVolatile() == clsType.IsVolatile()),
			curCV = (curC == clsType.IsConst()) + (curV == clsType.IsVolatile());
				 
		// ���� � �������� ������������ �����, ������� 0
		if( curCV > candCV )
			return 0;

		// ���� �����, ������ ���������������, �� �� ��� ��������,
		// �������� ���� �������� ����������� ������
		else if( curCV == candCV )
		{
			// ���� ����� ���������� �� �����, ����� ������
			if( castOperator->GetName() != ccoo.GetName() )
				return -1;

			// ����� ���� �������� ������� � ��� ������� ���������
			const SymbolTable *st = dynamic_cast<const SymbolTable *>(
					static_cast<const ClassType *>(&clsType.GetBaseType()) );
			INTERNAL_IF( st == NULL );
			NameManager nm(ccoo.GetName(), st);
			INTERNAL_IF( nm.GetRoleCount() == 0 );

			// ������ ���� ��� ��������� � ��������� �� ���� ����� ������ ���������
			if( nm.IsUnique() )
			{
				// ���� �������� ������, ������� ��� ��������
				if( nm.GetRoleList().front().first == (Identifier *)&ccoo )				
					return 0;

				// ����� �����. �������� ��������� ���� � ��������
				else
					return 1;
			}				
			
			// ����� ���������������
			return -1;
		}

		// ����� ��������� ��� ��� ����
		else
			return 1;
	}

	else
		return 0;
}


// ���������, �������� �� ��� ��������� ��������������
int OperatorCaster::COSArith( const ClassCastOverloadOperator &ccoo )
{
	// ���� ��� ��������������, ��������� ���������� ��������
	if( ExpressionMakerUtils::IsArithmetic(ccoo.GetCastType()) )
		return CVcmp(ccoo);
	else
		return 1;
}


// ���������, �������� �� ��� ��������� ����������
int OperatorCaster::COSPointer( const ClassCastOverloadOperator &ccoo )
{
	// ���� ��� ���������, ��������� ���������� ��������
	if( ccoo.GetCastType().GetDerivedTypeList().IsPointer() )
		return CVcmp(ccoo);
	else
		return 1;
}


// ���������, �������� �� ��� ��������� �����
int OperatorCaster::COSIntegral( const ClassCastOverloadOperator &ccoo )
{
	// ���� ��� �����, ��������� ���������� ��������
	if( ExpressionMakerUtils::IsIntegral(ccoo.GetCastType()) )
		return CVcmp(ccoo);
	else
		return 1;
}

// ���������, �������� �� ������� �������������� ��� ����������
int OperatorCaster::COSArithmeticOrPointer( const ClassCastOverloadOperator &ccoo )
{
		// ���� ��� �����, ��������� ���������� ��������
	if( ExpressionMakerUtils::IsArithmeticOrPointer(ccoo.GetCastType()) )
		return CVcmp(ccoo);
	else
		return 1;
}


// ���������, �������� �� ������� �������� �����. �������������� ��� ����������
int OperatorCaster::COSScalar( const ClassCastOverloadOperator &ccoo )
{
	// ���� ��� �����, ��������� ���������� ��������
	if( ExpressionMakerUtils::IsScalarType(ccoo.GetCastType()) )
		return CVcmp(ccoo);	
	else
		return 1;
}


// ������� �������� ������ ������� ���������� �������� ��� ��������������
// � �������� ��� ��� ���������
void OperatorCaster::ClassifyCast()
{
	const ClassType &cls = static_cast<const ClassType &>(clsType.GetBaseType());
	
	// ���� � ������ ��� ���������� ��������������, ����� ����� ��������
	if( cls.GetCastOperatorList() == NULL || cls.GetCastOperatorList()->empty() )
		return;

	// �������� �� ������ �������� ������ ���������� ����������, ������ ���������
	int (OperatorCaster::*cos)( const ClassCastOverloadOperator &ccoo );
	switch( category ) 
	{
	case ACC_NONE:			cos = &OperatorCaster::COSCast;		break;
	case ACC_TO_ARITHMETIC: cos = &OperatorCaster::COSArith;		break;
	case ACC_TO_INTEGER:	cos = &OperatorCaster::COSIntegral;	break;
	case ACC_TO_POINTER:	cos = &OperatorCaster::COSPointer;	break;
	case ACC_TO_SCALAR:		cos = &OperatorCaster::COSScalar;    break;
	case ACC_TO_ARITHMETIC_OR_POINTER: cos = &OperatorCaster::COSArithmeticOrPointer; break;
	default: INTERNAL( "'OperatorCaster::ClassifyCast' - ����������� ���������");
	}

	for( CastOperatorList::const_iterator p = cls.GetCastOperatorList()->begin();
		 p != cls.GetCastOperatorList()->end(); p++ )
	{		
		int r = (this->*cos)(**p);
		if( r == -1 )
		{
			errMsg = ("��������������� ����� '" + string((*p)->GetTypyziedEntityName().c_str()) +
				"' � '" + castOperator->GetTypyziedEntityName().c_str() + 
				"'; ���������� ����������").c_str();
			castOperator = NULL;
			break;
		}

		else if( r == 0 )
			castOperator = *p;
	}

	// ���� �������� �����, ������, ��� �������������� ��������, 
	// � ��������� ����������� ���������
	if( castOperator != NULL )	
		isConverted = true;	

	// ����� �������� �� �����, �� ��������� ����� ����� �� ����������.
	// ��������� �������� ������ � ������ ���������������, ��� �������������,
	// ����� ���������� ������� ������ �������, ��� �������� �� ����������
}

// ��������� ���������� ��������������, ������� 
// ���������� ������ ���������. SrcOp - ��������� ���, �������,
// �������� �������� ����������, destOp - ��� � �������� �������� ���������.
// � srcOp ����� ����������� � ���� destOp ���������
void OperatorCaster::DoCast( const POperand &destOp, POperand &srcOp, const Position &ep )
{
	INTERNAL_IF( castOperator == NULL || !isConverted || 
		!ExpressionMakerUtils::IsClassType(srcOp->GetType()) );

	// ��������� �������� �� �����������
	AccessControlChecker acc( 
		GetCurrentSymbolTable().IsLocalSymbolTable() ? 
		GetScopeSystem().GetFunctionalSymbolTable() : GetCurrentSymbolTable(),
		static_cast<const ClassType&>(srcOp->GetType().GetBaseType()), *castOperator );
	if( !acc.IsAccessible() )
		theApp.Error( ep, (string("\'") + castOperator->GetTypyziedEntityName().c_str() + 
			"\' - �������� ����������; ���������� ����������").c_str());

	// ������� ������� ��������� � ����� ���������
	POperand cop = new PrimaryOperand(false, *castOperator);
	POperand select = new BinaryExpression( '.', false, srcOp, cop, 
		new TypyziedEntity(*castOperator));
	
	// ����� ������� ����� �������
	POperand call = ExpressionMakerUtils::MakeFunctionCall(select, 
		PExpressionList(new ExpressionList));

	// ������� ����������� �������
	if( !scalarCaster.IsNull() )
		scalarCaster->DoCast(destOp, call, ep);
	srcOp = call;
}


// ����� ���������� false, ���� ��������� ���������� ����
// ������ ����������� ������������. � ������ ���� �����������
// �������� ��� ��������������, �������� �����. ���� ������
bool ConstructorCaster::AnalyzeConstructor( const ConstructorMethod &srcCtor )
{
	// ������� ����������� ���������� ����������, ����������� ������
	// ��������� ���� ��������, ���� ������, �� ��������� ������ ���� �� ���������
	int pcnt = srcCtor.GetFunctionPrototype().GetParametrList().GetFunctionParametrCount();

	// ���� �� ������, ������ ��������� ���� �� '...'
	if( pcnt == 0 )
	{
		// ���� ��� '...' ��� ����������� ��� �����, ������ �������
		if( !srcCtor.GetFunctionPrototype().IsHaveEllipse() ||
			constructor != NULL )
			return true;

		// ����������� ��� �� �����, ������. ��������������� �� ��������,
		// �� ���� ������� �������, ����� ����� �������������� ����������
		constructor = &srcCtor;
		return true;
	}

	// ����� ���� ������ ������, ���������, ����� ��������� ���� �� ���������
	else if( pcnt > 1 )
	{
		const Parametr &prm = 
			*srcCtor.GetFunctionPrototype().GetParametrList().GetFunctionParametr(1);

		// ��������� ����� ������ �������� ��� �� ���������, ���� �� �� ���������,
		// �������
		if( !prm.IsHaveDefaultValue() )
			return true;
	}

	// ����� ���� ��������, �������� ��� ��� � �������� ��������
	const TypyziedEntity &ptype = 
		*srcCtor.GetFunctionPrototype().GetParametrList().GetFunctionParametr(0);

	// �������� ������������� 'rvalue', � ���� ������� ��������� ������������.
	PScalarToScalarCaster stsc = new ScalarToScalarCaster(ptype, rvalue, true);
	stsc->ClassifyCast();

	// ���� �������������� ���������� - �������
	if( !stsc->IsConverted() )
		return true;

	// ���� ��� ������ ������� ��������������, ��������� ���
	// ��� �������������� ��������
	if( constructor == NULL )
	{
		constructor = &srcCtor;
		scalarCaster = stsc;
		return true;
	}

	// ����� ��������� ��������������. ���� ��� ������� ���� ��������,
	// ������ ��������� ���, ����� ���� ������� ���� �������, �����
	// ������ ����� - ����� ���������������
	else
	{
		// ���� � �������� �������������� ������� ������, ������ ������ ���
		// ��� ���������� ��������������� ��� '...'
		if( scalarCaster.IsNull() ||
			stsc->GetCastCategory() < scalarCaster->GetCastCategory() )
		{		
			constructor = &srcCtor;
			scalarCaster = stsc;
			return true;
		}

		// ����� ���� � �������� �������������� ������� ������, ������ ������ �� ������
		else if( stsc->GetCastCategory() > scalarCaster->GetCastCategory() )
			return true;
		
		// ����� ������ ���������, ��������� ������������, ���� ������������
		// ������, �������� ���������, ����� ���������������
		else
		{		
			// ������� ��������, ���� ������������� ������, �� ����� �����������������
			// ����������, � ������, ���� ������ ����� ����� ����������������� ���
			bool c1 = constructor->GetFunctionPrototype().IsConst(),
				 v1 = constructor->GetFunctionPrototype().IsVolatile(), 
				 c2 = ptype.IsConst(), v2 = ptype.IsVolatile();
				 
			if( c1 != c2 || v1 != v2 )					
			{
				int cv1 = (c1 == rvalue.IsConst()) + (v1 == rvalue.IsVolatile());
				int cv2 = (c2 == rvalue.IsConst()) + (v2 == rvalue.IsVolatile());

				// ���� ��������� ����������� ����� �������������� ���,
				// ������� ����������� �������
				if( cv1 >= cv2 )
					return true;

				// ����� ������ �����������
				else
				{
					constructor = &srcCtor;
					scalarCaster = stsc;
					return true;				
				}
			}

			// ����� ���������������
			errMsg = ("��������������� ����� '" + string(
				srcCtor.GetTypyziedEntityName().c_str()) +
				"' � '" + constructor->GetTypyziedEntityName().c_str() + 
				"'; ���������� ����������").c_str();
			constructor = NULL;
			return false;
		}
	}		
}


// ���������������� ��������������. ��������� ��������� �����������
// �������������� ������ ���� � ������, � ����� ������� �����������
// ���������� ��� ����������� ��������������
void ConstructorCaster::ClassifyCast()
{
	INTERNAL_IF( !lvalue.GetBaseType().IsClassType() );
	const ClassType &cls = static_cast<const ClassType &>(lvalue.GetBaseType());

	// � ����� ������ ���������, ���� ����� �������, ��������
	// �� ���������� �������, ����� ���������� ����������
	if( lvalue.GetDerivedTypeList().IsReference() && !lvalue.IsConst() )
	{
		errMsg = (string("���������� ������� ��������� ������ ������ '") + 
			cls.GetQualifiedName().c_str() + "'; ��������� ����������� ������").c_str();
		return;			
	}
	
	// ����� �������� �� ����� ������ ������������� ������� ���������
	for( ConstructorList::const_iterator p = cls.GetConstructorList().begin();
		 p != cls.GetConstructorList().end(); p++ )
		if( !AnalyzeConstructor(**p) )
			break;

	// ���� ����� �����������, ������ �������������� ��������
	if( constructor != NULL )
		isConverted = true;

	// ����� ������ �����. �� ������, ���� ��� ��� �� ������
	else if( errMsg.empty() )
		errMsg = string("����������� '" + string(cls.GetQualifiedName().c_str()) + "( " +
			rvalue.GetTypyziedEntityName(false).c_str() + 
			" )' �� ��������; ���������� ����������").c_str();
}


// �������� ����������� ������������ ��� �����������
bool ConstructorCaster::CheckCtorAccess( const POperand &destOp, const Method &meth )
{
	const SymbolTable *st = &GetCurrentSymbolTable();
	if( st->IsLocalSymbolTable() )
		st = &GetScopeSystem().GetFunctionalSymbolTable();

	// ��������� ����������� �� �����������
	AccessControlChecker acc( *st, 
		static_cast<const ClassType&>(destOp->GetType().GetBaseType()), meth );
	return acc.IsAccessible();
}


// ��������� ���������� ��������������, ������� 
// ���������� ������ ���������. SrcOp - ����� ���, �������,
// ���������� ������������, destOp - ��������� ��� � �������� �������� ���������.
// � srcOp ����� ����������� � ���� destOp ���������
void ConstructorCaster::DoCast( const POperand &destOp, POperand &srcOp, const Position &ep )
{
	INTERNAL_IF( constructor == NULL || !isConverted || 
		!ExpressionMakerUtils::IsClassType(destOp->GetType()) );

	// ��������� ����������� �� explicit
	if( constructor->IsExplicit() && !explicitCast )
		theApp.Error( ep, (string("\'") + constructor->GetTypyziedEntityName().c_str() + 
			"\' - ����������� �������� ��� explicit; ������� ���������� ����������").c_str());

	// ��������� ����������� ������������
	if( !CheckCtorAccess(destOp, *constructor) )
		theApp.Error( ep,  
			"'%s' - ����������� ����������; ���������� ����������",
			constructor->GetTypyziedEntityName().c_str());

	// ���������, �������� �� ���������� � �������� �� ��
	const Method *dtor = 
		static_cast<const ClassType &>(constructor->GetSymbolTableEntry()).GetDestructor();
	if( dtor == NULL )
		theApp.Error( ep,  
			"'~%s()' - ���������� �� ��������; ���������� � ������� ������������ ����������",
			static_cast<const ClassType &>(constructor->GetSymbolTableEntry()).
			GetQualifiedName().c_str());

	else if( !CheckCtorAccess(destOp, *dtor) )
		theApp.Error( ep,  
			"'%s' - ���������� ����������; ���������� � ������� ������������ ����������",
			dtor->GetTypyziedEntityName().c_str());

	// ��������� �����, ����� ����� �� ��� ����������. �������� �������
	// ������������ ������ ����������
	if( static_cast<const ClassType&>(constructor->GetSymbolTableEntry()).IsAbstract() )
		theApp.Error( ep,
			"�������� ������� ������ '%s' ����������; ����� �������� �����������",
			static_cast<const ClassType&>(constructor->GetSymbolTableEntry()).
			GetQualifiedName().c_str());

	// �������� srcOp, � ������������ ����, ���� ���������
	scalarCaster->DoCast(destOp, srcOp, ep);

	// ������� ����������� �������
	POperand cop = new PrimaryOperand(false, *constructor);
	PExpressionList pl = new ExpressionList;
	pl->push_back(srcOp);

	// ����� ������� ����� ������������ � ���������� ��� �� ����� srcOp
	srcOp = ExpressionMakerUtils::MakeFunctionCall(cop, pl);
}


// ������������� ���� ��������� ��� � �������. �������������� �������� ����� ������:
// ������ �� ����������� ����� ������������� � �������,
// � ������� ��������� ����������, � ������� ������������, 
void ClassToClassCaster::ClassifyCast()
{
	INTERNAL_IF( !ExpressionMakerUtils::IsClassType(cls1) ||
		!ExpressionMakerUtils::IsClassType(cls2) );
	const ClassType &ct1 = static_cast<const ClassType &>(cls1.GetBaseType()),
			&ct2 = static_cast<const ClassType &>(cls2.GetBaseType());

	// ���������, ����� �� ������������� ������ ������� � ������ �
	// ������� �������������� ������������ ������ � ��������,
	// ���� ���� ������ �����.
	PScalarToScalarCaster stsc = new ScalarToScalarCaster(cls1, cls2, true);
	stsc->ClassifyCast();

	// �������������� �� ������������ ������ � ������� ��������
	if( stsc->IsConverted() )
	{
		// ���������, ����� ��������� ������
		category = ( &ct1 == &ct2 ) ? CC_EQUAL : CC_STANDARD;			
		isConverted = true;
		caster = stsc.Release();
		return;
	}

	// �������� ��������� �������������� � ������� ������������ ������ 'cls1'
	PConstructorCaster cc = new ConstructorCaster(cls1, cls2, explicitCast);
	cc->ClassifyCast();
	
	// �������� ��������� �������������� � ������� ��������� �������������� 'cls2'
	POperatorCaster oc = new OperatorCaster(cls1, cls2, true);
	oc->ClassifyCast();

	// ���� ��� �������������� �������, ����� ���������������
	if( cc->IsConverted() && oc->IsConverted() )
	{
		INTERNAL_IF( cc->GetConstructor() == NULL || oc->GetCastOperator() == NULL );
		errMsg = ("��������������� ����� '" + string(
				cc->GetConstructor()->GetTypyziedEntityName().c_str()) +
				"' � '" + oc->GetCastOperator()->GetTypyziedEntityName().c_str() + 
				"'; ���������� ����������").c_str();
		return;
	}

	// ����� ���� �������������� �������, ��������� ���
	else if( cc->IsConverted() )
	{		
		isConverted = true;
		conversionMethod = cc->GetConstructor();
		conversionClass = &static_cast<const ClassType &>(cls1.GetBaseType());
		caster = cc.Release();
		category = CC_USERDEFINED;
		return;
	}

	// ����� ���� ����������� �������, ��������� ���
	else if( oc->IsConverted() )
	{
		isConverted = true;
		conversionMethod = oc->GetCastOperator();
		conversionClass = &static_cast<const ClassType &>(cls2.GetBaseType());
		caster = oc.Release();
		category = CC_USERDEFINED;
		return;
	}

	// ����� ������
	else
	{
		if( stsc->IsDerivedToBase() && !errMsg.empty() )
			;
		else
			errMsg = 
				!oc->GetErrorMessage().empty() ? oc->GetErrorMessage() : cc->GetErrorMessage();
		INTERNAL_IF( errMsg.empty() );
	}
}


// ��������� ���������� ��������������, �� srcOp � destOp,
// ��� ���� ���������� srcOp
void ClassToClassCaster::DoCast( const POperand &destOp, POperand &srcOp, const Position &ep )
{
	// conversionMethod ����� ���� ����� ����, ���� ���� �������������� ��
	// ������������ ������ � �������
	INTERNAL_IF( !isConverted || caster.IsNull() );

	// �������������� �������� ���� �� ���� ����������������
	caster->DoCast(destOp, srcOp, ep);
}


// ������� true, ���� ������� 'fn' ����� ��������� 'pcnt' ����������
bool OverloadResolutor::CompareParametrCount( const Function &fn, int pcnt )
{
	const FunctionParametrList &pl = fn.GetFunctionPrototype().GetParametrList();

	// ���� ���������� ����� �� ����������, ������� true
	if( pl.GetFunctionParametrCount() == pcnt )
		return true;

	// ����� ���� ���������� ������, ��������� ���� �� '...'
	else if( pl.GetFunctionParametrCount() < pcnt )
		return fn.GetFunctionPrototype().IsHaveEllipse();

	// ����� ���� ������, ��������� ����� 'pcnt+1-��' �������� ��� 
	// �� ���������
	else
		return pl.GetFunctionParametr(pcnt)->IsHaveDefaultValue();
}


// ���������, �������� �� ������� ���������� (����������) ��� 
// ������, �� ������ ���������� ������ ����������.
bool OverloadResolutor::ViableFunction( const Function &fn )
{
	viableCasterList.clear();

	const FunctionParametrList &pl = fn.GetFunctionPrototype().GetParametrList();
	
	// �������� ���������� ����������, ������� ��� ���������� ����������
	int pcnt = pl.GetFunctionParametrCount();

	// ���� ���������� ������, ������������ ������ ������ N
	if( pcnt > apl.size() )
		pcnt = apl.size();

	// ��������� ���� ��� ������� ���������
	ExpressionList::const_iterator p = apl.begin();
	for( int i = 0; i<pcnt; i++, p++ )
	{
		PCaster caster = 
			AutoCastManager( *pl.GetFunctionParametr(i), (*p)->GetType(), true).RevealCaster();
		caster->ClassifyCast();

		// ���� �������������� ����������, ������� �� ����� � ������
		// ��������� �� ������ ���� ���������
		if( !caster->IsConverted() )
		{
			if( candidate == NULL && errMsg.empty() )
				errMsg = (string("�������������� ����� '") + CharString(i+1).c_str() +
					"-�� ���������'; '" + (*p)->GetType().GetTypyziedEntityName(false).c_str() +
						"' ���������� �������� � '" + 
				pl.GetFunctionParametr(i)->GetTypyziedEntityName(false).c_str() + '\'').c_str();
			return false;
		}

		// �����, ��������� ��������������� � ������
		viableCasterList.push_back(caster);
	}

	return true;
}


// ���������� 'fn' � ������� ���������� � ���������, ����� 
// �� ������� �������� �������� ��� ������ ����������, ����
// �������� ���, �������� ��������������� � ���������� false
bool OverloadResolutor::SetBestViableFunction( const Function &fn )
{
	// ���� �������� ��� �� �����, ������ ��� � �����
	if( candidate == NULL )
	{
		candidate = &fn;
		candidateCasterList = viableCasterList;		// ������ ������ ����������������
		return true;
	}

	// ����� ���������� ������ ����������������.
	int bt = 0, bc = 0, i = 0, 		
		fnpcnt = fn.GetFunctionPrototype().GetParametrList().GetFunctionParametrCount(),
		cpcnt = candidate->GetFunctionPrototype().GetParametrList().GetFunctionParametrCount(),
		// �������������, ������ �� ���������, ������� ������� ����������
		mcnt = fnpcnt < cpcnt ? fnpcnt : cpcnt;		

	mcnt = apl.size() < mcnt ? apl.size() : mcnt;
	try {

	// �������� �� ������ ����������, ������� ���������������
	for( CasterList::iterator pc = candidateCasterList.begin(), pfn = viableCasterList.begin();
		 i<mcnt; i++, pc++, pfn++ )
	{
		INTERNAL_IF( pc == candidateCasterList.end() || pfn == viableCasterList.end() );

		// �������� �������� ����� ����������� �������������� ��������� � 
		// ������� �������. ���� ��������� ����� - 0, ���� ������� ����� ��������� - >0,
		// ���� ������� ���� ��������� - <0
		bc = (*pc)->GetCastCategory() - (*pfn)->GetCastCategory();

		// ������ ���������, ����� �� ���� ���������������. ���� bt == 0, �� bc 
		// ����� ��������, ���� bt < 0 � bc < 0, ������ �� �������� ���� ���������,
		// ���� bt > 0 � bc > 0, �� ������� �� �������� ����� ���������, ����� ����������
		if( bt == 0 )
			bt = bc;
		else if( (bt < 0 && bc <= 0) || (bt > 0 && bc >= 0) )
			;
		// ����� ��� ������ � ��� ���������������
		else
			throw i;
	}

	// ��������� '...', ���� ���������� ���������� ������. � ���� ����
	// '...', ��� � ������
	if( fnpcnt != cpcnt )
	{
		if( i == fnpcnt && fn.GetFunctionPrototype().IsHaveEllipse() )
			bc = -1;
		else if( i == cpcnt && candidate->GetFunctionPrototype().IsHaveEllipse() )
			bc = 1;
		else
			bc = 0;

		// ��������� ��������� ��������, ������� �� ���������������
		if( bt == 0 || (bt < 0 && bc <= 0) || (bt > 0 && bc >= 0) )
			bt = bc;

		// ����� ��� ������ � ��� ���������������
		else
			throw i;
	}

	// ���������, ���� bt, ����� 0, ������ ���������������
	if( bt == 0 )
		throw i;

	// � ���� bt > 0, ������ ��������� ������� ��������
	else if( bt > 0 )
	{
		candidate = &fn;
		candidateCasterList = viableCasterList;		// ������ ������ ����������������	
	}

	// ������������� �������������� �������� ��������� ����������������
	} catch( int ) {
		
		errMsg = (string("��������������� ����� '") + fn.GetTypyziedEntityName().c_str() +
					"' � '" + candidate->GetTypyziedEntityName().c_str() + '\'').c_str();
		candidate = NULL;
		candidateCasterList.clear();
		ambigous = true;
		return false;
	}
	
	return true;
}


// ��������� ���������� ������� ��������� � ������ � ������������ ����
void OverloadResolutor::DoParametrListCast( const Position &errPos )
{
	if( candidate == NULL )
		return;

	int i = 0;
	for( CasterList::iterator p = candidateCasterList.begin(); 
		 p != candidateCasterList.end(); p++, i++ )
	{
		POperand pop = new PrimaryOperand( true, *candidate->GetFunctionPrototype().
			GetParametrList().GetFunctionParametr(i) );	
		(*p)->DoCast( pop, const_cast<POperand &>(apl[i]), errPos );
	}
}


// ����� �������� ������������ �������, ������� �������� ��� ������ ����������
void OverloadResolutor::PermitUnambigousFunction()
{
	// ��������� ������ ������� � ������
	for( OverloadFunctionList::const_iterator p = ofl.begin(); p != ofl.end(); p++ )
	{
		const Function &fn = *(*p);

		// ������� ���������, ����� �� ������� ��������� 
		// �������� ���������� ����������
		if( !CompareParametrCount(fn, apl.size()) )
			continue;

		// ����� ���� ����� ������, ���������, ����� �������������� �������
		// ���� �� ������, ��� ������������ �������		
		if( object != NULL && fn.GetStorageSpecifier() != Function::SS_STATIC )
		{
			if( object->IsConst() > fn.GetFunctionPrototype().IsConst() ||
				object->IsVolatile() > fn.GetFunctionPrototype().IsVolatile() )
			{
				if( errMsg.empty() )
					errMsg = (string("����� '") + fn.GetTypyziedEntityName().c_str() +
						"' ����� �������������� ��� ������; ����� ����������").c_str();
				continue;
			}
		}

		// ������� ������������ ������ ���������� �� ������� ����������
		// ���������� �������. ���� ������ �������� - ������� true.
		if( !ViableFunction( fn ) )
			continue;

		// ����� ��� �������, 'fn' � 'candidate'. �������� ����� �������
		// ����� ��������, ����� ���� ���������������, ����� ������������ false.
		// ���� candidate ��� ���, fn ���������� �� ��� �������������� ��������
		if( !SetBestViableFunction( fn ) )
			break;
	}

	// ���� �������� �� ����� � ��������� �� ������, ������ ���������
	if( candidate == NULL && errMsg.empty() )
	{
		const Function &fn = *(*ofl.begin());

		// ���� ��� ���������� �������
		if( ofl.size() > 1 )
			errMsg = (string("\'") + fn.GetQualifiedName().c_str()  + 
				"' - ��� ���������� �������, ������� ��������� '" + 
				CharString((int)apl.size()).c_str() + "' ��������(�)").c_str();

		// ���� ������� ��������� �� �� ���-�� ����������
		else
			errMsg = (string("\'") + fn.GetTypyziedEntityName().c_str()  + 
				"' - ������� ��������� '" + CharString(
				fn.GetFunctionPrototype().GetParametrList().GetFunctionParametrCount()).c_str() +
				"' ��������(�) ������ '" + CharString((int)apl.size()).c_str() + '\'').c_str();
	}
}


// �������� �����, ������� ���������� ������������� ���
// ������ ����������
void OverloadOperatorFounder::Found( int opCode, bool evrywhere, 
				const ClassType *cls, const Position &ep  )
{
	// ������� ���������, ��� ��������� ������� ������ �� ������
	// � ������, �� � � ��������� �������� ���������
	const CharString &opName = 
		(string("operator ") + ExpressionPrinter::GetOperatorName(opCode)).c_str();

	// ������ �� ������� � ����� ��������� �����
	Lexem lxm( opName, NAME, ep );
	NodePackage np( PC_QUALIFIED_NAME );
	np.AddChildPackage( new LexemPackage(lxm) );

	// ������ ���� ������� � ������, ���� �� �����
	if( cls != NULL )
	{
		IdentifierOperandMaker iom( np, cls, true );
		POperand op = iom.Make();

		// ���� ������, �� �������� ������, ����� �.�. ���������������
		if( op->IsErrorOperand() )
		{
			if( !iom.IsNotFound() )
			{
				ambigous = true;
				return;
			}
		}

		// ����� ���� ���� ��������, ��������� ��� � ������
		else if( op->IsPrimaryOperand() )
			clsOperatorList.push_back( &static_cast<const Function &>(op->GetType()) );

		// ����� ���� ��������� ����������, ��������� ��
		else if( op->IsOverloadOperand() )		
			clsOperatorList = static_cast<OverloadOperand &>(*op).GetOverloadList();
		
		// ����� ��� ��� ��������� � ��� ���������� ������
		else
			INTERNAL( "'OverloadOperatorFounder::Found' - ����������� �������" );
	}

	// ������ ����������� �� �� ��������, ���� ��������� ������ �� ��������� ���������
	// ������� ���������
	if( evrywhere )
	{
		// �������� ��������� �� �������, ������� �� ��������� � ���������
		list<SymbolTable *> stl;

		// ������� ������� �������� ����� �� �����, ���� �� ��������� ������
		// ������ ��� ������
		if( GetCurrentSymbolTable().IsClassSymbolTable()				  ||
			(GetCurrentSymbolTable().IsFunctionSymbolTable() &&
				static_cast<const FunctionSymbolTable &>(
				GetCurrentSymbolTable()).GetFunction().IsClassMember())   ||
			(GetCurrentSymbolTable().IsLocalSymbolTable() && 
			    static_cast<const FunctionSymbolTable &>(
				GetScopeSystem().GetFunctionalSymbolTable()).GetFunction().IsClassMember()) )
		
			// �������� ��� ������� ��������� ������ ���������
			for( ;; )
			{
				if( !GetCurrentSymbolTable().IsClassSymbolTable() &&
					!GetCurrentSymbolTable().IsFunctionSymbolTable() &&
					!GetCurrentSymbolTable().IsLocalSymbolTable() )				
					break;				
				
				stl.push_front( &GetCurrentSymbolTable());
				GetScopeSystem().DestroySymbolTable();
			}				  
		

		IdentifierOperandMaker iom( np, NULL, true );
		POperand op = iom.Make();

		// ������� �������, ��������������� ������� ���������
		GetScopeSystem().PushSymbolTableList(stl);

		// ���� ������, �� �������� ������, ����� �.�. ���������������
		if( op->IsErrorOperand() )
		{
			if( !iom.IsNotFound() )
			{			
				ambigous = true;
				return;
			}
		}

		// ����� ���� ���� ��������, ��������� ��� � ������
		else if( op->IsPrimaryOperand() )
			globalOperatorList.push_back( &static_cast<const Function &>(op->GetType()) );

		// ����� ���� ��������� ����������, ��������� ��
		else if( op->IsOverloadOperand() )		
			globalOperatorList = static_cast<const OverloadOperand &>(*op).GetOverloadList();
		
		// ����� ��� ��� ��������� � ��� ���������� ������
		else
			INTERNAL( "'OverloadOperatorFounder::Found' - ����������� �������" );
	}

	// � ����� ������ ���� ������
	found = !clsOperatorList.empty() || !globalOperatorList.empty();
}


// ��������, � ��������� ���� �� �������� � ������ ������, ����
// error operand � ������ �������
const POperand &DefaultArgumentChecker::Check() const
{
	// ���� ��������� ���������� ��������, �������
	if( defArg->IsErrorOperand() )
		return defArg;

	INTERNAL_IF( !(defArg->IsPrimaryOperand() || defArg->IsExpressionOperand()) );
	
	// � ���� �����, ���������� ���� �������� � ���������
	PCaster caster = AutoCastManager( parametr, defArg->GetType(), true ).RevealCaster();
	caster->ClassifyCast();

	// ���� �������������� ����������, ������� ������ � ���������� error operand
	if( !caster->IsConverted() )
	{
		if( caster->GetErrorMessage().empty() )
		{
			theApp.Error(errPos,
				"'%s' - ���������� ������������� '%s' � '%s' � �������� �� ���������",
				parametr.GetName().c_str(), 
				defArg->GetType().GetTypyziedEntityName(false).c_str(),
				parametr.GetTypyziedEntityName(false).c_str());
			return ErrorOperand::GetInstance();
		}
		else
		{
			theApp.Error(errPos,
				"'%s' - %s", parametr.GetName().c_str(), caster->GetErrorMessage().c_str());
			return ErrorOperand::GetInstance();
		}
	}

	// ����� ��������� �������������� ���������
	caster->DoCast( new PrimaryOperand(true, parametr), const_cast<POperand&>(defArg), errPos);

	// ���� ��� ��������� �� ��������� ��������� � �� �� �������� �����
	// ������� ������������, ��������, ����� �-��� ����������� ��� ��������
	if( parametr.GetBaseType().IsClassType() && parametr.GetDerivedTypeList().IsEmpty() )
	{
		// ���������, ���� ����� �-���
		if( defArg->IsExpressionOperand() &&
			static_cast<const Expression&>(*defArg).IsFunctionCall() )
		{
			const FunctionCallExpression &fnc = 
					static_cast<const FunctionCallExpression&>(*defArg);

			// ����� ����������, ��� ����� ����� ������������, �������� �� �-��� 
			// ����������� �� ���������
			if( (&fnc.GetFunctionOperand()->GetType().GetBaseType() ==
				&static_cast<const ClassType&>(parametr.GetBaseType()) )	&&
				(dynamic_cast<const ConstructorMethod *>(
					&fnc.GetFunctionOperand()->GetType()) != NULL) )
				return defArg;
		}

		// � ��������� ������, ���������, ����� � ������� ��� �������� �-��� �����������
		// � ����������
		ExpressionMakerUtils::ObjectCreationIsAccessible(
			static_cast<const ClassType&>(parametr.GetBaseType()), errPos, false, true, true);
	}

	return defArg;
}


// ��������� ������������� �� ��������� ��� ���������� ����,
// ���������� ����� �������� ������ �������� � ��������
void AgregatController::DefaultInit( const TypyziedEntity &type, const Position &errPos )
{	
	::Object ob( "�������������", 0, (BaseType *)&type.GetBaseType(), type.IsConst(),
		type.IsVolatile(), type.GetDerivedTypeList(), ::Object::SS_NONE );
	static PExpressionList el = new ExpressionList;

	// �������� ������ ������ ���������, ��� �������� ����������� ����. �� ���������
	InitializationValidator( el, ob, errPos ).Validate();
}


// ���������� ������
AgregatController *ArrayAgregatController::DoList( const ListInitComponent &ilist )
{
	// ���� ��� �������� �� �������� ���������, ������������� ����������
	if( !ListInitializationValidator::IsAgregatType(*elementType) )
	{
		// ���� ���� ���� �������, ��� ������ �� ������, ����������
		// ������� ����������
		if( ilist.GetICList().size() == 1 )
			return this;

		// ���� ��������� ���, ���������� ������������� �� ���������
		else if( ilist.GetICList().size() == 0 )
		{
			DefaultInit(*elementType, ilist.GetPosition());
			initElementCount++;			
			return this;
		}

		// ����� ��� �� �������� ���������, ������
		else
			throw TypeNotArgegat(*elementType);
	}

	// ����� ����� �������, ���������� ����� ���������� ��� ����. �
	// ����� ����������� ��� ��������
	initElementCount++;
	return ListInitializationValidator::MakeNewController(*elementType, NULL);
}


// ���������� ���������, ���� ������� ����������� ��� ����� ����������,
// � ������ ���� ��������� �� �������� ��� ������� ������� �������
AgregatController *ArrayAgregatController::DoAtom( const AtomInitComponent &iator, bool endList )
{
	// ����� ���� ���������� ��������� �� ��������� � ������, 
	// ������� ������������ ����������
	if( initElementCount == arraySize )
	{	
		// ���� �������� ���, ��� ������
		if( parentController == NULL )
			throw NoElement(iator.GetPosition());

		return const_cast<AgregatController *>(parentController);
	}

	// ���� ������� ������� �������, ������� ��� ���� ���� ����������,
	// ������� ���
	if( ListInitializationValidator::IsAgregatType(*elementType) &&
		!ListInitializationValidator::IsCharArrayInit(*elementType, iator.GetExpression()) )
	{
		initElementCount++;
		return ListInitializationValidator::MakeNewController(*elementType, this);
	}
	
	initElementCount++;

	// � ��������� ������, ��������� ���������
	static PExpressionList el = new ExpressionList;
	el->clear();
	el->push_back(iator.GetExpression());

	// ���������
	const TypyziedEntity &etype = *elementType;
	::Object ob( "�������������", NULL, (BaseType *)&etype.GetBaseType(), etype.IsConst(),
		etype.IsVolatile(), etype.GetDerivedTypeList(), ::Object::SS_NONE );
	InitializationValidator( el, ob, iator.GetPosition() ).Validate();

	// ���������� NULL, ��� ��������� ���������� ������
	return NULL;
}


// ���������� ��������� - ����� ������, ��������� ������������� �� ���������
// ��� ������ ��������, ���� ������������ ���� ��� �������� ����������������
// ���� ������ ������� ����������
void ArrayAgregatController::EndList( const Position &errPos )
{		
	// ���� ������ ������� ����������, ������ ���
	if( pArray.IsUncknownSize() )
	{
		if( initElementCount < 1 )
			theApp.Error(errPos, "������ ������� ������� ����������� ����� �������������"),
			initElementCount = 1;
		pArray.SetArraySize(initElementCount);
	}

	// ���� ��� �������� ����������������, ��������� �������������
	// �� ��������� �� ���������, ����� � ��� ����������� �������
	if( initElementCount == arraySize || arraySize < 1 )	
		return; 	
	
	DefaultInit(*elementType, errPos);

	// �������������, ��� ��� �������� �������������������, ��� ���������
	// ���������
	initElementCount = arraySize;
}


// �������� ��-�� �� ��������� ���������������� �� ����������� ������-����,
// ���� ������ ��� � ���������, ���������� NULL
const DataMember *StructureAgregatController::NextDataMember( )
{
	const ClassMemberList &cml = pClass.GetMemberList();
	for( int i = curDMindex+1; i<cml.GetClassMemberCount(); i++ )
	{
		if( const DataMember *dm = 
				dynamic_cast<const DataMember *>(&*cml.GetClassMember(i)) )
		{
			if( dm->GetStorageSpecifier() == ::Object::SS_STATIC ||
				dm->GetStorageSpecifier() == ::Object::SS_TYPEDEF )
				continue;
			curDMindex = i;
			return dm;
		}
	}

	return NULL;
}


// ���������� ������ 
AgregatController *StructureAgregatController::DoList( const ListInitComponent &ilist )
{
	// �������� ��������� �������
	const DataMember *dm = NextDataMember();
	if( !dm )
		throw NoElement(ilist.GetPosition());

	// ���� ��� �������� �� �������� ���������, ������������� ����������
	if( !ListInitializationValidator::IsAgregatType(*dm) )
	{
		// ���� ���� ���� �������, ��� ������ �� ������, ����������
		// ������� ����������
		if( ilist.GetICList().size() == 1 )
			return this;

		// ���� ��������� ���, ���������� ������������� �� ���������
		else if( ilist.GetICList().size() == 0 )
		{
			DefaultInit(*dm, ilist.GetPosition());			
			return this;
		}

		// ����� ��� �� �������� ���������, ������
		else
			throw TypeNotArgegat(*dm);
	}

	// ����� ����� �������, ���������� ����� ���������� ��� ����. �
	// ����� ����������� ��� ��������
	return ListInitializationValidator::MakeNewController(*dm, NULL);
}


// ���������� ���������, ���� ������� ����������� ��� ����� ����������,
// � ������ ���� ��������� �� �������� ��� ������� ������� �������
AgregatController *StructureAgregatController::DoAtom( 
					const AtomInitComponent &iator, bool endList  )
{
	// �������� ��������� �������
	const DataMember *dm = NextDataMember();
	if( !dm )
	{
		// �������� ���, ���� ���������� ��������, ���� ���������� ������
		if( parentController == NULL )
			throw NoElement(iator.GetPosition());

		return const_cast<AgregatController *>(parentController);
	}

	// ���� ������� ������� �������, ������� ��� ���� ���� ����������,
	// ������� ���
	if( ListInitializationValidator::IsAgregatType(*dm) &&
		!ListInitializationValidator::IsCharArrayInit(*dm, iator.GetExpression()) )	
		return ListInitializationValidator::MakeNewController(*dm, this);

	// � ��������� ������, ��������� ���������
	static PExpressionList el = new ExpressionList;
	el->clear();
	el->push_back(iator.GetExpression());

	// ���������
	InitializationValidator( el, const_cast<DataMember&>(*dm), iator.GetPosition() ).Validate();

	// ���������� NULL, ��� ��������� ���������� ������
	return NULL;
}


// ���������� ��������� - ����� ������, ��������� ������������� �� ���������
// ��� ���������� ������
void StructureAgregatController::EndList( const Position &errPos )
{
	// ���� ��������� ����� ��������, ��������� �� �� ������������� �� ���������
	while( const DataMember *dm = NextDataMember() )
		DefaultInit(*dm, errPos);
}


// �������� ������ ������ ����, ���� �� ��� �� �������
const DataMember *UnionAgregatController::GetFirstDataMember()
{
	if( memberGot )
		return NULL;

	memberGot = true;
	const ClassMemberList &cml = pUnion.GetMemberList();
	for( int i = 0; i<cml.GetClassMemberCount(); i++ )
	{
		if( const DataMember *dm = 
				dynamic_cast<const DataMember *>(&*cml.GetClassMember(i)) )
		{
			if( dm->GetStorageSpecifier() == ::Object::SS_STATIC ||
				dm->GetStorageSpecifier() == ::Object::SS_TYPEDEF )
				continue;			
			return dm;
		}
	}

	return NULL;
}


// ���������� ������ 
AgregatController *UnionAgregatController::DoList( const ListInitComponent &ilist )
{
	// �������� ������ �������, ���� �� ��� �� �������
	const DataMember *dm = GetFirstDataMember();
	if( !dm )
		throw NoElement(ilist.GetPosition());

	// ���� ��� �������� �� �������� ���������, ������������� ����������
	if( !ListInitializationValidator::IsAgregatType(*dm) )
	{
		// ���� ���� ���� �������, ��� ������ �� ������, ����������
		// ������� ����������
		if( ilist.GetICList().size() == 1 )
			return this;

		// ���� ��������� ���, ���������� ������������� �� ���������
		else if( ilist.GetICList().size() == 0 )
		{
			DefaultInit(*dm, ilist.GetPosition());			
			return this;
		}

		// ����� ��� �� �������� ���������, ������
		else
			throw TypeNotArgegat(*dm);
	}

	// ����� ����� �������, ���������� ����� ���������� ��� ����. �
	// ����� ����������� ��� ��������
	return ListInitializationValidator::MakeNewController(*dm, NULL);
}


// ���������� ���������, ���� ������� ����������� ��� ����� ����������,
// � ������ ���� ��������� �� �������� ��� ������� ������� �������
AgregatController *UnionAgregatController::DoAtom( const AtomInitComponent &iator, bool endList )
{
	// �������� ��������� �������
	const DataMember *dm = GetFirstDataMember();
	if( !dm )
	{
		// �������� ���, ���� ���������� ��������, ���� ���������� ������
		if( parentController == NULL )
			throw NoElement(iator.GetPosition());

		return const_cast<AgregatController *>(parentController);
	}

	// ���� ������� ������� �������, ������� ��� ���� ���� ����������,
	// ������� ���
	if( ListInitializationValidator::IsAgregatType(*dm) &&
		!ListInitializationValidator::IsCharArrayInit(*dm, iator.GetExpression()) )	
		return ListInitializationValidator::MakeNewController(*dm, this);

	// � ��������� ������, ��������� ���������
	static PExpressionList el = new ExpressionList;
	el->clear();
	el->push_back(iator.GetExpression());

	// ���������
	InitializationValidator( el, const_cast<DataMember&>(*dm), iator.GetPosition() ).Validate();

	// ���������� NULL, ��� ��������� ���������� ������
	return NULL;
}


// ���������� ��������� - ����� ������, ��������� ������������� �� ���������
// ��� ���������� ������
void UnionAgregatController::EndList( const Position &errPos )
{
	if( const DataMember *dm = GetFirstDataMember() )
		DefaultInit(*dm, errPos);
}


// ������� true, ���� ��� �������� ��������� (�������� ��� ����������),
// ����� false. � ������ ���� ��������� �� �������� �������� (��-POD),
// ������������� ���������� ���� ClassType
bool ListInitializationValidator::IsAgregatType( const TypyziedEntity &type )
{
	if( type.GetDerivedTypeList().IsArray() )
		return true;

	// ���������, ����� ��������� ���� ���������
	else if( type.GetDerivedTypeList().IsEmpty() &&
			 type.GetBaseType().IsClassType() )
	{
		const ClassType &cls = static_cast<const ClassType &>(type.GetBaseType());
		// � ������ �� ������ ���� ������� �������, ����������� �������,
		// ��������/���������� ������������� ������-������, ������������
		// ������������� �������������
		if( !cls.GetBaseClassList().IsEmpty() ||
			 cls.IsPolymorphic() )
			 return false;

		// ���������, ����� �� ����������� ������-����� ���� ��������
		int i;
		for( i = 0; i<cls.GetMemberList().GetClassMemberCount(); i++ )
			if( const DataMember *dm = 
					dynamic_cast<const DataMember *>(&*cls.GetMemberList().GetClassMember(i)) )
				if( dm->GetAccessSpecifier()  != ClassMember::AS_PUBLIC &&
					dm->GetStorageSpecifier() != ::Object::SS_STATIC	&&
					dm->GetStorageSpecifier() != ::Object::SS_TYPEDEF )
					return false;

		// ��������� ������������
		for( ConstructorList::const_iterator p = cls.GetConstructorList().begin();
			 p != cls.GetConstructorList().end(); p++  )
			if( (*p)->IsUserDefined() )
				return false;

		// ��� �������� ��������, ������� true
		return true;
	}

	else
		return false;
}


// ����� ���������� true, ���� ��� �������� �������� ���� char ��� wchar_t,
// � ������� �������� ��������� ���������
bool ListInitializationValidator::IsCharArrayInit( 
			const TypyziedEntity &type, const POperand &iator )
{
	return type.GetDerivedTypeList().IsArray() &&
		type.GetDerivedTypeList().GetDerivedTypeCount() == 1 &&
		(type.GetBaseType().GetBaseTypeCode() == BaseType::BT_CHAR ||
		 type.GetBaseType().GetBaseTypeCode() == BaseType::BT_WCHAR_T) &&
		iator->IsPrimaryOperand()		&&
		iator->GetType().IsLiteral()	&&
		(static_cast<const Literal&>(iator->GetType()).IsStringLiteral() ||
		 static_cast<const Literal&>(iator->GetType()).IsWideStringLiteral());
}


// ������� ����� ���������� ��������,��� �������� ����. ���
// ������ ���� ����� ���������
AgregatController *ListInitializationValidator::MakeNewController( 
			const TypyziedEntity &elemType, const AgregatController *prntCntrl )
{
	AgregatController *controller = NULL;

	// ���������������, ��� elemType, ��� �������� �� ������������
	// ������� ���������� �������, ���� ��� �������� ������
	if( elemType.GetDerivedTypeList().IsArray() )
	{
		PTypyziedEntity et = new TypyziedEntity(elemType);

		// ����������� '������ ���� T' � 'T'
		const_cast<DerivedTypeList &>(et->GetDerivedTypeList()).PopHeadDerivedType();
		controller = new ArrayAgregatController(et, prntCntrl, const_cast<Array &>(
			static_cast<const Array &>(*elemType.GetDerivedTypeList().GetHeadDerivedType()) ) );
	}

	// �����, ��� ������ ���� ���������� ��� ������������
	else
	{
		INTERNAL_IF( !elemType.GetBaseType().IsClassType() ||
			!elemType.GetDerivedTypeList().IsEmpty() );
		const ClassType &cls = static_cast<const ClassType &>(elemType.GetBaseType());
		if( cls.GetBaseTypeCode() == BaseType::BT_UNION )
			controller = new UnionAgregatController(
				static_cast<const UnionClassType &>(cls), prntCntrl);
		
		// ����� ������� ��������� ��� �����
		else
			controller = new StructureAgregatController(cls, prntCntrl);		
	}

	// ��������� ���������� � ������
	allocatedControllers.push_back(controller);
	return controller;
}


// ����������� �����, �������� �� ������ �������������, � ������
// ��������� ��������� �������� �������� ��� ���������
void ListInitializationValidator::InspectInitList( const ListInitComponent &ilist )
{
	int cnt = 0;

	// �������� �� ������
	for( ListInitComponent::ICList::const_iterator p = ilist.GetICList().begin();
		 p != ilist.GetICList().end(); ++p, cnt++ )
	{
		// ���� ����� ����, �������� ��� ��������� �����������, ����
		// �� �� ������ NULL
		if( (*p)->IsAtom() )
		{
			const AtomInitComponent &aic = *static_cast<const AtomInitComponent *>(*p);
			bool isLastElement = cnt == ilist.GetICList().size()-1;
			AgregatController *temp = pCurrentController->DoAtom(aic, isLastElement);				
			int i = 0;

			// ���� ������� ���������� ���������� �� NULL, ������
			// �� �� ����� �������������� ���������� ������������� � �������
			// �����, ���� ���������� ������������
			while( temp != NULL )
			{
				pCurrentController = temp,
				temp = pCurrentController->DoAtom(aic, isLastElement);
				INTERNAL_IF( i++ == MAX_ITERATION_COUNT	);
			}
		}

		// ����� ������, �������� ��������� �������� �����������,
		// �������� ��������
		else
		{
			// ��������� ������� ����������, �.�. DoList, ������ ����
			// �������, ���� �����, �� ��� ������, ������� ����� �����������
			// ������ ������� ���������� ������� ������������
			AgregatController *pPrev = pCurrentController;
			const ListInitComponent &lic = *static_cast<const ListInitComponent *>(*p);

			// �������� ��������� � ��������� ���������� ������, ��������
			// ������ ����������
			pCurrentController = pCurrentController->DoList(lic);
			InspectInitList( lic );

			// ����� ��������� ����� ������, �������� �� ���� ��������� �����������.
			// ���� ���������� � ������������� �����������, ���� �������� � ����,
			// ���� �� ��������
			if( pCurrentController != pPrev )
			{
				pCurrentController->EndList(errPos);
				pCurrentController = pPrev;
			}
		}
	}
}


// ��������� ���������� ������������� �������
void ListInitializationValidator::Validate()
{
	try {
		// ���������, ���� ������������ ������ �� �������,
		if( !IsAgregatType(object) )
		{
			// � ������ ������ ���� <= 1 ���������
			if( listInitComponent.GetICList().size() > 1 )
			{
				theApp.Error(errPos, "������� ����� ���������������");
				return;
			}

			// ����� ��������� ������ ���������
			ExpressionList el;
			if( listInitComponent.GetICList().size() == 1 )
			{
				const InitComponent *icomp = listInitComponent.GetICList().front();
				while( icomp->IsList() )
					icomp = static_cast<const ListInitComponent *>(icomp)->GetICList().front();
				el.push_back(static_cast<const AtomInitComponent *>(icomp)->GetExpression());
			}

			PExpressionList pel = &el;
			InitializationValidator(pel, object, errPos).Validate();
			// ����������� ��������� �� ��������� ������
			pel.Release();

			// �������
			return;
		}

		// ����� ����� �������, ������� ��������� ������������� �������
		// ������� ���������� ��� ��������
		pCurrentController = MakeNewController(object, NULL);
		INTERNAL_IF( pCurrentController == NULL );

		// �������� �� ������
		AgregatController *prev = pCurrentController;
		InspectInitList(listInitComponent);
		prev->EndList(errPos);

	// ������������� ���������, ������� ��������� � ������ ���� 
	// ���������� ��������������� �� ��������� � �������
	} catch( const AgregatController::NoElement &nl ) {
		theApp.Error(nl.errPos, "������� ����� ��������������� � ������");

	// ������������� ������� - ��� �� �������� ���������
	} catch( const AgregatController::TypeNotArgegat &t ) {
		theApp.Error(errPos, 
			"'%s' - ��� �� �������� ���������; ������������� ������� ����������",
			t.type.GetTypyziedEntityName(false).c_str());
	}


}


// ��������� ������������� ������������� ��� ����� ���������,
// ��� ��� ��������
void InitializationValidator::ValidateCtorInit()
{
	// �����������
	INTERNAL_IF( &expList == NULL );
	const DerivedTypeList &dtl = object.GetDerivedTypeList();

	// ���� ������ ������������� ����, ��������� ������������� �� ���������,	
	if( expList->empty() )
	{		
		// ���� ��� ��� �������, ��� extern ������� 
		if( object.GetStorageSpecifier() == ::Object::SS_TYPEDEF ||
			object.GetStorageSpecifier() == ::Object::SS_EXTERN  ||
			dtl.IsFunction() )
			return;

		// ���� ����� ������ ������������� �����������
		if( dtl.IsReference()  )
			theApp.Error(errPos,
				"'%s' - ������������� ������ �����������",
				object.GetName().c_str());
		
		// ���� ����� ����������� ������
		else if( ExpressionMakerUtils::IsConstant(object) )
		{
			// ���� ����� �����, ����� �� ���������������� �������������,
			// ����� ������
			if( !(object.GetBaseType().IsClassType() && dtl.IsEmpty()) )
				theApp.Error(errPos,
					"'%s' - ������������� ������������ ������� �����������",
					object.GetName().c_str());
		}

		// ���������, ����� ������ ������� ��� �����
		if( dtl.IsArray() && 
			static_cast<const Array &>(*dtl.GetHeadDerivedType()).IsUncknownSize() )
		{
			theApp.Error(errPos,
				"'%s' - ������ ������������ �������",
				object.GetName().c_str());

			// ������ ������
			const_cast<Array&>
				(static_cast<const Array &>(*dtl.GetHeadDerivedType())).SetArraySize(1);
		}

		// ��������� �������������
		ictor = ExpressionMakerUtils::CorrectObjectInitialization(
			object, expList, true, errPos).GetCtor();
	}

	// ����� ������������� ������� 
	else
	{		
		// ���� ����� ��� ��� �������, ������������� ����������.
		if( object.GetStorageSpecifier() == ::Object::SS_TYPEDEF ||
			dtl.IsFunction() )
		{
			theApp.Error(errPos,
				"������������� %s ����������", dtl.IsFunction() ? 
				"�������" : "����");
			return;
		}

		// ���� ������ ���� char ��� wchar_t, � ������ �������������
		// �������, �� ��������� �������������
		if( dtl.IsArray() && dtl.GetDerivedTypeCount() == 1 &&
			expList->size() == 1 && 
			expList->at(0)->IsPrimaryOperand() && expList->at(0)->GetType().IsLiteral() )
		{
			const Literal &lit = static_cast<const Literal &>(expList->at(0)->GetType());
			int lsz = -1;
			Array &obar = const_cast<Array&>
				( static_cast<const Array &>(*dtl.GetHeadDerivedType()) );

			// ��������, ����� ������� ���� ��� char [], � ������ ��� ����
			// char, ���� ������� wchar_t � ������ ������ �� ����
			if( (object.GetBaseType().GetBaseTypeCode() == BaseType::BT_CHAR &&
				 lit.IsStringLiteral())  || 
				(object.GetBaseType().GetBaseTypeCode() == BaseType::BT_WCHAR_T &&
				 lit.IsWideStringLiteral()) )
			{			
				lsz = lit.GetDerivedTypeList().GetHeadDerivedType()->GetDerivedTypeSize();
				// ���� ������ ������� ����������, ������ ���
				if( obar.IsUncknownSize() )
					obar.SetArraySize(lsz);

				// �����, ���� ��������, �� ������ ���� �� ������ ������� ������
				else if( lsz > obar.GetArraySize() )
					theApp.Error(errPos, 
						"'%s' - ������ �� ����� �������� ��� ������; "
						"��������� ��� ������� '%d' ����(�)",
						object.GetName().c_str(), lsz);

				// ���������� ��������
				return;
			}
			
			// ����� ������� �������� ������������			
		}

		ExpressionMakerUtils::InitAnswer answer =
			ExpressionMakerUtils::CorrectObjectInitialization( object, expList, true, errPos);
		ictor = answer.GetCtor();

		// ���� ������������� ���������, � �� ����� ����������� �������������,
		// � �� ����� ���������� ������ ��������������� ����, ��������
		// ������� ��������������		
		if( answer )
		{
			double ival;
			if( expList->size() == 1										   && 
				ExpressionMakerUtils::IsInterpretable( expList->at(0), ival )  &&
				object.IsConst()									       &&
				ExpressionMakerUtils::IsArithmetic(object)				   &&
				dtl.IsEmpty() )
				object.SetObjectInitializer(ival);
		}		
	}
}


// ������ ����� �� �������� �������, ����� ����������� ������
void CtorInitListValidator::SelectVirtualBaseClasses( 
				const BaseClassList &bcl, unsigned &orderNum )
{
	static PExpressionList emptyList = NULL;
	for( int i = 0; i<bcl.GetBaseClassCount(); i++ )
	{
		const BaseClassCharacteristic &bcc = *bcl.GetBaseClassCharacteristic(i);
		SelectVirtualBaseClasses(bcc.GetPointerToClass().GetBaseClassList(), orderNum);

		// ���������, ���� ����� ���������� �����������, ������ ��� � ������
		if( bcc.IsVirtualDerivation() )
		{
			const ClassType &cls = bcc.GetPointerToClass();

			// ���������, ����� ������ �� ���� � ������, �.�. �����������
			// ����� ������� ���������������� �������
			if( find( oieList.begin(), oieList.end(), ObjectInitElement(cls, 
					emptyList, ObjectInitElement::IV_VIRTUALBC, 0)) == oieList.end() )
			{
				oieList.push_back( ObjectInitElement(cls, emptyList, 
					ObjectInitElement::IV_VIRTUALBC, orderNum) );
				orderNum++;
			}
		}
	}
}


// ��������� ������ ���������� ���������� �������������,
// ������������ �������� ��������, ������� �������� ��������, 
// �������������� �������-�������
void CtorInitListValidator::FillOIEList( )
{
	unsigned orderNum = 1, i;

	// ������� �������� �� �������� ������� �������, �������
	// ����������� ������� ������. ������� ������
	const BaseClassList &bcl = pClass.GetBaseClassList();
	SelectVirtualBaseClasses(bcl, orderNum);

	// ����� ��������� ������ ������� ������
	for( i = 0; i<bcl.GetBaseClassCount(); i++ )
	{
		const BaseClassCharacteristic &bcc = *bcl.GetBaseClassCharacteristic(i);
		
		// �� ��������� ����������� ������� ������, �.�. ��� ��� ���������
		if( bcc.IsVirtualDerivation() )
			continue;
		oieList.push_back( ObjectInitElement(bcc.GetPointerToClass(), NULL, 
			ObjectInitElement::IV_DIRECTBC, orderNum++) );		
	}

	// � ��������� �������, ��������� ������������� ������ �����
	const ClassMemberList &cml = pClass.GetMemberList();
	for( i = 0; i<cml.GetClassMemberCount(); i++ )
	{
		if( const DataMember *dm = dynamic_cast<const DataMember *>(&*cml.GetClassMember(i)) )
		{
			if( dm->GetStorageSpecifier() != ::Object::SS_STATIC &&
				dm->GetStorageSpecifier() != ::Object::SS_TYPEDEF )
				oieList.push_back( ObjectInitElement(*dm, NULL, 
					ObjectInitElement::IV_DATAMEMBER, orderNum++) );
		}
	}
}


// �������� ���� ���������������� �������, ���� �� �� ������������ � ������,
// ������� ������. ����� ������� ������ ���� ������������ ��� ����� ��������
// � ������, ��� �������� ���������������. 
void CtorInitListValidator::AddInitElement( const POperand &id, 
			const PExpressionList &expList, const Position &errPos, unsigned &orderNum )
{
	INTERNAL_IF( expList.IsNull() );

	// ���������, ����� ������������� ��� �������� ��� �����
	if( id->IsErrorOperand() )
		return;

	else if( id->IsOverloadOperand() )
	{
		theApp.Error(errPos, 
			"'%s' - �� �������� ������ ������ '%s'",
			static_cast<const OverloadOperand&>(*id).GetOverloadList().front()->
			GetQualifiedName().c_str(), pClass.GetQualifiedName().c_str());
		return;
	}

	// �������� ������� ������ ���� ��������
	else if( id->IsPrimaryOperand() )
	{
		const Identifier *obj = dynamic_cast<const Identifier *>(&id->GetType());
		INTERNAL_IF( !obj );
		ObjectInitElementList::iterator p = find( 
			oieList.begin(), oieList.end(), ObjectInitElement(*obj, 
			NULL, ObjectInitElement::IV_DATAMEMBER, 0));
		
		// ������� � ������ �� ������
		if( p == oieList.end() )
		{
			theApp.Error(errPos, 
				"'%s' - �� �������� ������������� ������-������ ������ '%s'",
				obj->GetQualifiedName().c_str(), pClass.GetQualifiedName().c_str());
			return;
		}

		// ����� ����� ������, ���������, ����� �� �� ��� ������������������
		INTERNAL_IF( !(*p).IsDataMember() );
		if( (*p).IsInitialized() )
		{
			theApp.Error(errPos, 
				"'%s' - ��� ������������������", obj->GetQualifiedName().c_str());
			return;
		}

		// ��������� ������������ �������������
		DataMember &dm = const_cast<DataMember &>(
			static_cast<const DataMember &>(id->GetType()) );

		// ��� �������� ������ ��������������� ������ ���� ������,
		// ��������� ������������� ���������� ����������
		if( dm.GetDerivedTypeList().IsArray() && !expList->empty() )
			theApp.Error(errPos, 
				"'%s' - ������ ��������������� ��� ������� ������ ���� ������",
				obj->GetQualifiedName().c_str());
		
		// ����� ��������� ������� �������� ������������ �����
		else
			InitializationValidator(expList, dm, errPos).Validate();

		// ������ �������������� � ���������� �����
		(*p).SetExpressionList( expList );
		(*p).SetOrderNum( orderNum );
		explicitInitCounter = orderNum++;
	}

	// ������� ������� ������ ���� ���� �������, ���� ��������� ����� ������
	else if( id->IsTypeOperand() )
	{
		const TypyziedEntity &type = id->GetType();
		if( !(type.GetBaseType().IsClassType() && type.GetDerivedTypeList().IsEmpty()) )
		{
			theApp.Error(errPos, 
				"'%s' - ��� �� ����� �������������� � ������ ������������� ������ '%s'",
				type.GetTypyziedEntityName(false).c_str(), pClass.GetQualifiedName().c_str());
			return;
		}

		const ClassType &cls = static_cast<const ClassType &>(type.GetBaseType());				
		ObjectInitElement oie(cls, NULL, ObjectInitElement::IV_DIRECTBC, 0);
		ObjectInitElementList::iterator p = find( oieList.begin(), oieList.end(), oie);
		
		// ������� � ������ �� ������
		if( p == oieList.end() )
		{
			theApp.Error(errPos, 
				"'%s' - �� �������� ������ ��� ����������� ������� ������� ������ '%s'",
				cls.GetQualifiedName().c_str(), pClass.GetQualifiedName().c_str());
			return;
		}

		// �������� �� ������ ��� ��� � ������� ������� ��� ����, ����� ��������� �����
		// �� �������������
		if( find( ++ObjectInitElementList::iterator(p), oieList.end(), oie ) != oieList.end() )
		{
			theApp.Error(errPos, 
				"'%s' - ������ � ����������� ������� ����� ������������; "
				"������������� ���������� ��-�� ���������������",
				cls.GetQualifiedName().c_str());
			return;
		}			

		// ����� ����� �����, ���������, ����� �� �� ��� ������������������
		INTERNAL_IF( (*p).IsDataMember() );
		if( (*p).IsInitialized() )
		{
			theApp.Error(errPos, 
				"'%s' - ��� ������������������", cls.GetQualifiedName().c_str());
			return;
		}

		// �������� �������������
		ExpressionMakerUtils::CorrectObjectInitialization( 
			TypyziedEntity( const_cast<ClassType *>(&cls), false, false, DerivedTypeList()),
			expList, false, errPos );
		
		(*p).SetExpressionList( expList );
		(*p).SetOrderNum( orderNum );
		explicitInitCounter = orderNum++;
	}

	// ��������� �� �����������
	else
		INTERNAL( "'CtorInitListValidator::AddInitElement' - ����������� �������");
}


// ��������� �������������� ��������, ����� ���� ��� ���� ������ ������.
// ��������� �������������������� ��������. ������ ���������� ������� �������������
void CtorInitListValidator::Validate()
{
	unsigned nonum = explicitInitCounter+1;
	static PExpressionList emptyList = new ExpressionList;

	// �������� �� ������, ���� �������� ������������������ ��������,
	// ��������� �� ������������� �� ���������, ������ ����� �������������
	for( ObjectInitElementList::iterator p = oieList.begin(); p != oieList.end(); p++ )
	{
		if( (*p).IsInitialized() )
			continue;

		// ���� ��� ����, ��������� ������������� �� ���������
		if( (*p).IsDataMember() )
			InitializationValidator(emptyList, const_cast<DataMember &>(
				static_cast<const DataMember &>((*p).GetIdentifier()) ), errPos).Validate();

		// ����� ��� �����
		else
			ExpressionMakerUtils::CorrectObjectInitialization( 
				TypyziedEntity( &const_cast<ClassType &>(
					static_cast<const ClassType &>((*p).GetIdentifier()) ),
					false, false, DerivedTypeList() ),
				emptyList, false, errPos );
	
		(*p).SetExpressionList(emptyList);
		(*p).SetOrderNum(nonum++);
	}
}


// ���������������� ������ ���������� 
void GlobalObjectMaker::Initialize( const ExpressionList &initList )
{
	INTERNAL_IF( targetObject == NULL );
	// ������ ������, ���������� ������, ���� initList == NULL
	static PExpressionList emptyList = new ExpressionList;

	// ���� ������ �������������, �� ��������� �������� �������������
	if( redeclared )
		return;

	// ������� ������ ���������
	PExpressionList pl = const_cast<ExpressionList *>(&initList);
	if( pl.IsNull() )
		pl = emptyList;

	// ��������� ���������
	InitializationValidator validator(pl, *targetObject, tempObjectContainer->errPos);
	validator.Validate();

	// ��������� �����������, ������� ���������������� ������. � ������
	// ���� ����������������� ��������� ������
	ictor = validator.GetConstructor();		

	// ����������� ���������
	if( &*pl != &*emptyList )
		pl.Release();
}


// ���������������� ������ ��� ����������� ���������� 
void MemberDefinationMaker::Initialize( const ExpressionList &initList )
{
	INTERNAL_IF( targetID == NULL );
	// ���������, ����� targetID, ��� ����������� ������ ������
	::Object *dm = dynamic_cast<::Object *>(targetID);
	if( !dm ||
		(dm->GetSymbolTableEntry().IsClassSymbolTable() &&
		 dm->GetStorageSpecifier() != ::Object::SS_STATIC) )
	{
		if( &initList != NULL )
			theApp.Error(toc->errPos, 
				"������������� �� ������������ ����� ������ ����������");	
		return;
	}

	// ���� � ������� ��� ���� �������������
	if( dm->IsHaveInitialValue() )
	{
		theApp.Error(toc->errPos, 
			"'%s' - ��� ���������������", dm->GetQualifiedName().c_str());
		return;
	}

	// ������� ������ ���������
	static PExpressionList emptyList = new ExpressionList;
	PExpressionList pl = const_cast<ExpressionList *>(&initList);
	if( pl.IsNull() )
		pl = emptyList;

	// ��������� ���������
	InitializationValidator iv(pl, *dm, toc->errPos);
	iv.Validate();
	ictor = iv.GetConstructor();

	// ������, ��� ������ ���������������
	if( !dm->IsHaveInitialValue() )
		dm->SetObjectInitializer(0);

	// ����������� ���������
	if( &*pl != &*emptyList )
		pl.Release();
}


// ����� ��������� ������������ �������� �������� ����
void DataMemberMaker::CheckBitField( const Operand &exp )
{	
	// ���������, ����� ������� ���� ���� �����, �� �����������,
	// �� ���. ��������, ����� ��������� ���� �����, ���������������� > 0,
	if( !ExpressionMakerUtils::IsIntegral(*targetDM) ||
		!targetDM->GetDerivedTypeList().IsEmpty() )	
		theApp.Error(toc->errPos,
			"'%s' - ������� ���� ������ ����� ����� ���",
			targetDM->GetName().c_str());
	
	// ����� ���������, ����� ���������� ������������ ��������
	if( targetDM->GetStorageSpecifier() != ::Object::SS_NONE )
		theApp.Error(toc->errPos,
			"'%s' - ������� ���� �� ����� ����� ������������ �������� '%s'",
			targetDM->GetName().c_str(), 
			ManagerUtils::GetObjectStorageSpecifierName(targetDM->GetStorageSpecifier()).c_str());

	// ����� ���������, ����� ��������� ���� ����� ���������������� �������������
	double ival = 1;
	POperand pexp = const_cast<Operand *>(&exp);
	if( !ExpressionMakerUtils::IsInterpretable( pexp, ival ) ||
		!ExpressionMakerUtils::IsIntegral(exp.GetType())	 ) 
		theApp.Error(toc->errPos, 
			"������ �������� ���� ������ ���������� ����� ���������� ����������");
	if( ival <= 0 )
	{
		theApp.Error(toc->errPos, "������ �������� ���� ������ ���� ������ ����");
		ival = 1;
	}

	pexp.Release();

	// � ��������� ������� ������ ������ �������� ����
	targetDM->SetObjectInitializer(ival, true);
}


// ����� ��������� ������������ ������������� �������-����� ���������
void DataMemberMaker::CheckDataInit( const Operand &exp )
{
	// ���������, ����� ���� ��� ����� ����������� ����������
	if( !ExpressionMakerUtils::IsIntegral(*targetDM) ||
		!targetDM->GetDerivedTypeList().IsEmpty()	 ||
		targetDM->GetStorageSpecifier() != ::Object::SS_STATIC ||
		!targetDM->IsConst() )
	{
		theApp.Error(toc->errPos,
			"'%s' - ������������� ����� ���������� ������ ������",
			targetDM->GetName().c_str());
		return;
	}
	
	// ����� ���������, ����� ��������� ���� ����� ���������������� 
	double ival = 1;
	POperand pexp = const_cast<Operand *>(&exp);
	if( !ExpressionMakerUtils::IsInterpretable( pexp, ival ) ||
		!ExpressionMakerUtils::IsIntegral(exp.GetType())	 )	
		theApp.Error(toc->errPos, 
			"������������� ������ ���� ����� ���������� ����������");				

	pexp.Release();

	// � ��������� ������� ������ �������������
	targetDM->SetObjectInitializer(ival);
}


// ����������� ����������
InitComponent::~InitComponent()
{
}
