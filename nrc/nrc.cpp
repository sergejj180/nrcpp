// NoRules Compiler Library 
// реализация библиотеки - NRC.H

#include "nrc.h"
using namespace nrc;

// единственный метод класса - возвращает имя самого класса
nrc::CharString nrc::SObject::Name()
{
	return CharString("nrc::Object");
}


//------------------------------------------------------------------------------------------------
// возвращает имя самого класса
nrc::CharString nrc::CharString::Name()
{
	return CharString("nrc::CharString");
}


// удаляет передние и задние пробелы
void nrc::CharString::Trim()
{
	TrimLeft();
	TrimRight();
}


// удаляет передние пробелы
void nrc::CharString::TrimLeft()
{
	const char *p = c_str(), *q = p;
	
	while( isspace(*p) )
		p++;

	erase(0, p - q);
}


// удаляет задние пробелы
void nrc::CharString::TrimRight()
{
	const char *p = c_str() + length() - 1, *q = p;
	
	while( isspace(*p) && p != c_str() )
		p--;

	erase(length() - (q - p), q - p); 
}


// форматирует строку и записывает ее в буфер
void nrc::CharString::Format( const char *s, ... )
{
	static char buf[1024];
	va_list ls;
	va_start(ls, s);
	_vsnprintf(buf, 1024, s, ls);
	va_end(ls);

	this->operator =(buf);
}


// перевести строку в нижний регистр 
void nrc::CharString::LowerCase()
{
	for( int i = 0; i<length(); i++ )
		this->operator[](i) = tolower(at(i));

}

	
// перевести строку в верхний регистр
void nrc::CharString::UpperCase()
{	
	for( int i = 0; i<length(); i++ )
		this->operator[](i) = toupper(at(i));
}


// перевернуть строку
void nrc::CharString::Reverse()
{
	CharString s = c_str();
	strrev((char*)s.c_str());
	*this = s;
}


// сравнить две строки без учета регистра, но с учетом локальных установок
bool nrc::CharString::CompareNoCase( const char *s )
{
	return !_stricoll(c_str(), s);
}


// удалить из строки все символы находящиеся в наборе 'set'
void nrc::CharString::DeleteInSet( const char *set )
{
	CharString out;

	out.reserve(length()+1);
	for( int i = 0; i<length(); i++ )
	{
		const char *p;
		if( (p = strchr(set, at(i))) == NULL )
			out += at(i);			
	}

	*this = out;
}


// заменить символы из набора inset, символами из набора outset
// причем каждому символу из inset должен соответствовать символ
// из outset, если длинна inset меньше, то символы заменяются
// defchar
void nrc::CharString::ChangeInSet( const char *inset, const char *outset, char defchar )
{
	int r = strlen(inset) - strlen(outset);
	CharString s;

	if( r > 0 )
	{
		s = outset;
		while(r)
			s += defchar, r--;
		outset = s.c_str();
	}

	for( int i = 0; i<length(); i++ )
	{
		const char *p;
		if( (p = strchr(inset, at(i))) != NULL )
			this->operator[](i) = outset[p-inset];
	}
}


// оставить в строке только символы из набора
void nrc::CharString::KeepInSet( const char *set )
{
	CharString out;

	out.reserve(length()+1);
	for( int i = 0; i<length(); i++ )
	{
		const char *p;
		if( (p = strchr(set, at(i))) != NULL )
			out += at(i);
	}

	*this = out;
}


// перегруженный метод, оставляет в строке символы, для которых
// chkfunc возвращает ненулевое значение
void nrc::CharString::KeepInSet( int (*chkfunc)(int c) )
{
	CharString out;

	out.reserve(length()+1);
	for( int i = 0; i<length(); i++ )
	{
		if( chkfunc( at(i) ) != 0 )
			out += at(i);
	}

	*this = out;
}


// удалить символы с начала до того как появится символ из набора
// вернуть удаленную строку
nrc::CharString nrc::CharString::DeleteLeftWhileNot( const char *set )
{
	int cnt = 0;
	while( strchr(set, at(cnt)) == NULL && cnt < length() )
		cnt++;
			
	CharString s = substr(0, cnt).c_str();
	erase(0, cnt);
	return s;
}


// удалить символы с конца до того как появится символ из набора
// вернуть удаленную строку
nrc::CharString nrc::CharString::DeleteRightWhileNot( const char *set )
{
	int cnt = 0, i = length()-1;
	while( i >= 0 && strchr(set, at(i)) == NULL )
		cnt++, i--;
	
	CharString s = substr(i+1, cnt+1).c_str();
	erase(i, cnt);
	return s;
}


// вернуть указатель на буфер
nrc::CharString::operator const char *()
{
	return c_str();
}


// преобразовать целое число в строку и записать
nrc::CharString &nrc::CharString::operator=( int d )
{
	char buf[25];
	_snprintf(buf, 25, "%d", d);
	this->operator=( buf );
	return *this;
}

	
// преобразовать вещественное число в строку и заменить
nrc::CharString &nrc::CharString::operator=( double f )
{
	char buf[100];
	_snprintf(buf, 100, "%lf", f);
	this->operator=( buf );
	return *this;
}

//------------------------------------------------------------------------------------------------
// возвращает имя самого класса
nrc::CharString nrc::StringList::Name() 
{
	return CharString("NRC::StringList");
}
	

// добавить строку в конец списка
void nrc::StringList::Append( CharString &s )
{
	CharString *n = new CharString(s);
	push_back(n);
}

	
// удалить строку в позиции N. 
// Индексация начинается с нуля
void nrc::StringList::Delete( int n )
{
	if( n < 0 || n >= size() )
		return;

	iterator p = begin();
	for( int i = 0; i<n; i++, p++ ) 
		continue;

	delete at(n);
	erase(p);
}

	
// очистить весь список
void nrc::StringList::Clear()
{
	for( int i = 0; i<size(); i++ )
		delete at(i);
	clear();
}


// загрузить строки из файла
bool nrc::StringList::LoadFromFile( const char *fname )
{
	ifstream fin(fname);
	fin.flags( fin.flags() | ios::skipws );

	if( fin.fail() )
		return false;

	CharString s;
	
	while( getline(fin, s, '\n') )
		Append(s);

	fin.close();
	return true;
}


// сохранить строки в файле
bool nrc::StringList::SaveToFile( const char *fname )
{
	ofstream fout(fname);
	if( fout.fail() )
		return false;

	for( int i = 0; i<Count(); i++ )
		if( !(fout << (*this)[i] << '\n') )
		{
			fout.close();
			return false;
		}

	fout.close();
	return true;
}


// вернуть список строк в виде буфера
nrc::CharString nrc::StringList::Text()
{
	CharString s;

	for( int i = 0; i<Count(); i++ )
		s += (*this)[i] + '\n';

	return s;
}


// разделить строку на части, через символ-разделитель
// и загрузить ее в список строк. При этом предыдущее состояние
// списка стирается. Строка заключенная в двойные или одинарные кавычки
// считается одним словом, независимо от разделителя
void nrc::StringList::DelimText( const CharString &s, char delim )
{
	Clear();
	
	CharString out;
	bool quote = false;

	for( int i = 0; i<s.length(); i++ )
	{
		// строка заключенная в кавычки
		if( s[i] == '\"' )
		{
			if( i != 0 && s[i-1] == '\\' )
				quote = true;
			else
				quote = !quote;

			if( !quote )
			{
				out += '\"';
				goto addword;
			}
		}

		// добавляем слово
		if( s[i] == delim || isspace(s[i]) )
		{			
			if( out.empty() || quote )
				continue;

		addword:
			Append(out);
			out = "";
		}

		else
			out += s[i];
	}
}


// найти строку соответствующую шаблону
int nrc::StringList::Find( const CharString &s, 
			bool (*fcmp)(const CharString &, const CharString&), int startPos )	const
{
	if( startPos < 0 ||  startPos >= Count() )
		return -1;

	for( int i = 0; i<Count(); i++ )
		if( !fcmp( (*this)[i], s ) )
			return i;
	return -1;
}


// сортировка строк
void nrc::StringList::Sort( bool (*fcmp)(const CharString *, const CharString*) )
{
	sort(begin(), end(), fcmp);
}


// оператор доступа к строкам
nrc::CharString &nrc::StringList::operator[]( int ix )
{
	static CharString def;
	if( ix < 0 || ix >= size() )
		return def;

	return *at(ix);
}


// const
const CharString &nrc::StringList::operator[]( int ix ) const
{
	static CharString def;
	if( ix < 0 || ix >= size() )
		return def;

	return *at(ix);
}