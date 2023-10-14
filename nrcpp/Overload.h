// интерфейс работающий с перегрузкой и перекрытием имен - Overload.h
// является продолжением интерфейса Checker
#ifndef OVERLOAD_H
#define OVERLOAD_H
// объявлен в Object.h
class Function;


// класс, выявляет принадлежность имени к определенной группе 
// (например класс, тип, именованная область видимости). При этом основной задачей 
// класса, является выявление нужного идентификатора из списка согласно правилам языка. 
// Класс является чекером потому, что проверяет имена на неоднозначность. 
// Хотя с друой стороны, класс также является и менеджером.
class AmbiguityChecker
{
	// список ролей имени
	const RoleList &rlist;

	// позиция для вывода ошибок об неоднозначностях
	Position errPos;

	// если true, сообщения об ошибке выводятся
	bool diagnostic;

public:
	// 
	AmbiguityChecker( const RoleList &r, const Position &p = Position(), bool d = false ) 
		: rlist(r), errPos(p), diagnostic(d) {
	}


	// если имя является синонимом типа typedef, вернуть указатель на него
	const ::Object *IsTypedef(  ) const;

	// проверить является ли тип перечислением, если параметр withOverload == true,
	// тогда достаточно, чтобы роль присутствовала в списке, в противном случае,
	// роль должна быть одна
	const EnumType *IsEnumType( bool withOverload  ) const;

	// проверить является ли тип классом, если параметр withOverload == true,
	// тогда достаточно, чтобы роль присутствовала в списке, в противном случае,
	// роль должна быть одна
	const ClassType *IsClassType( bool withOverload  ) const;

	// статический метод для определения является ли имя - именем типа
	// т.е. классом, перечислением или typedef
	const Identifier * IsTypeName( bool withOverload ) const;

	// если имя является шаблонным классом
	const TemplateClassType *IsTemplateClass( ) const;

	// если имя является именованной областью видимости
	const NameSpace *IsNameSpace( ) const;

private:
	// функция, используется для определения семантической группы имени.
	// Во-первых, все коды в списке, должны быть равны r, во вторых,
	// если правило один верно, значит каждый указатель в списке, должен
	// быть равен первому указателю в списке. В случае если оба правила
	// равны, возвращается идентификатор, иначе - NULL
	const Identifier *IsDestRole( Role r ) const;
};


// чекер, который определяет возможность переопределения объекта, функции
// перегруженного оператора, конструктора. 
class RedeclaredChecker
{
	// сформированный идентификатор, который следует проверить на 
	// возможность объявления
	const TypyziedEntity &destID;

	// роль идентификатора
	Role destIDRole;

	// список ролей которые уже объявлены под этим именем
	const RoleList &roleList;

	// позиция для вывода ошибок
	Position errPos;

	// флаг, который устанавливается, если идентификатор переопределен
	// и его не следует вставлять в таблицу
	bool redeclared;

	// предыдущая декларация ,если есть
	const Identifier *prevID;

public:
	// в конструкторе задаются необх. параметры
	RedeclaredChecker( const TypyziedEntity &did, 
		const RoleList &rl, const Position &ep, Role r )

		: destID(did), roleList(rl), errPos(ep), destIDRole(r), 
		  redeclared(false), prevID(NULL) {

		Check();
	}

	// инспектор
	bool IsRedeclared() const {
		return redeclared;
	}

	// получить предыдущую декларацию, если идентификатор переопределен,
	// либо NULL, в случае если идентификатор не переопрделен
	const Identifier *GetPrevDeclaration() const {
		return prevID;
	}

	// сравнивает destID с типизированной сущностью и если их типы не эквивалентны,
	// вернуть false. Метод учитывает тип входного параметра. Например у 
	// параметра функции не проверяется константность и throw-спецификация.
	// Следует учесть, что метод не проверяет спецификаторы хранения
	static bool DeclEqual( const TypyziedEntity &ob1, const TypyziedEntity &ob2 );

	// методы учавствующие в выявлении переопределения идентификатора
private:

	// проверить, возможно ли объявление функции или объекта в текущей
	// области видимости. Если невозможно устанавливает redeclared в false
	void Check() ;

	// пробегает по списку ролей и пытается найти функцию
	// совпадающую по прототипу с имеющийся. Если функция найдена, возвращается
	// указатель на нее, иначе NULL
	const Function *FnMatch( ) const;
};

#endif