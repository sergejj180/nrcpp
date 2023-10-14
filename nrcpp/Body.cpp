// реализация поведения сущностей относящихся к телу функции - Body.cpp

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


// единственный экземпляр ошибочного операнда
POperand ErrorOperand::errorOperand = NULL;


// явно определяем абстрактные деструкторы
ObjectInitializator::~ObjectInitializator() { }
BodyComponent::~BodyComponent() { }
Instruction::~Instruction() { }	
AdditionalOperation::~AdditionalOperation() { }
LabelBodyComponent::~LabelBodyComponent() { }


// в деструкторе уничтожается значение по умолчанию
Parametr::~Parametr() 
{
	if( defaultValue && !defaultValue->IsErrorOperand() )
		delete defaultValue;
}


// задаем функцию, создаем список инициализации
ConstructorFunctionBody::ConstructorFunctionBody( const Function &pFn, const Position &ccPos )
	: FunctionBody(pFn, ccPos), oieList(new ObjectInitElementList)
{
}


// удалить список инициализации конструктора
ConstructorFunctionBody::~ConstructorFunctionBody()
{
	delete oieList;
}


// задать список инициализации конструктора
void ConstructorFunctionBody::SetConstructorInitList( const ObjectInitElementList &ol )
{
	oieList->assign(ol.begin(), ol.end());
}