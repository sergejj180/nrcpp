// ���������� ��������� ��������� ����������� � ���� ������� - Body.cpp

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
#include "Parser.h"
#include "Checker.h"
#include "Body.h"
#include "ExpressionMaker.h"


// ������������ ��������� ���������� ��������
POperand ErrorOperand::errorOperand = NULL;


// ���� ���������� ����������� �����������
ObjectInitializator::~ObjectInitializator() { }
BodyComponent::~BodyComponent() { }
Instruction::~Instruction() { }	
AdditionalOperation::~AdditionalOperation() { }
LabelBodyComponent::~LabelBodyComponent() { }


// � ����������� ������������ �������� �� ���������
Parametr::~Parametr() 
{
	if( defaultValue && !defaultValue->IsErrorOperand() )
		delete defaultValue;
}


// ������ �������, ������� ������ �������������
ConstructorFunctionBody::ConstructorFunctionBody( const Function &pFn, const Position &ccPos )
	: FunctionBody(pFn, ccPos), oieList(new ObjectInitElementList)
{
}


// ������� ������ ������������� ������������
ConstructorFunctionBody::~ConstructorFunctionBody()
{
	delete oieList;
}


// ������ ������ ������������� ������������
void ConstructorFunctionBody::SetConstructorInitList( const ObjectInitElementList &ol )
{
	oieList->assign(ol.begin(), ol.end());
}