// NoRules Compiler Library 
// ���������� ���������� - NRC.H

#include "nrc.h"
using namespace nrc;

// ������������ ����� ������ - ���������� ��� ������ ������
nrc::CharString nrc::SObject::Name()
{
	return CharString("nrc::Object");
}


//------------------------------------------------------------------------------------------------
// ���������� ��� ������ ������
nrc::CharString nrc::CharString::Name()
{
	return CharString("NRC::CharString");
}


// ������� �������� � ������ �������
void nrc::CharString::Trim()
{
	TrimLeft();
	TrimRight();
}


// ������� �������� �������
void nrc::CharString::TrimLeft()
{
	const char *p = c_str(), *q = p;
	
	while( isspace(*p) )
		p++;

	erase(0, p - q);
}


// ������� ������ �������
void nrc::CharString::TrimRight()
{
	const char *p = c_str() + length() - 1, *q = p;
	
	while( isspace(*p) && p != c_str() )
		p--;

	erase(length() - (q - p), q - p); 
}


// ����������� ������ � ���������� �� � �����
void nrc::CharString::Format( const char *s, ... )
{
	static char buf[1024];
	va_list ls;
	va_start(ls, s);
	_vsnprintf(buf, 1024, s, ls);
	va_end(ls);

	this->operator =(buf);
}


// ��������� ������ � ������ ������� 
void nrc::CharString::LowerCase()
{
	for( int i = 0; i<length(); i++ )
		this->operator[](i) = tolower(at(i));

}

	
// ��������� ������ � ������� �������
void nrc::CharString::UpperCase()
{	
	for( int i = 0; i<length(); i++ )
		this->operator[](i) = toupper(at(i));
}


// ����������� ������
void nrc::CharString::Reverse()
{
	CharString s = c_str();
	strrev((char*)s.c_str());
	*this = s;
}


// �������� ��� ������ ��� ����� ��������, �� � ������ ��������� ���������
bool nrc::CharString::CompareNoCase( const char *s )
{
	return !_stricoll(c_str(), s);
}


// ������� �� ������ ��� ������� ����������� � ������ 'set'
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


// �������� ������� �� ������ inset, ��������� �� ������ outset
// ������ ������� ������� �� inset ������ ��������������� ������
// �� outset, ���� ������ inset ������, �� ������� ����������
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


// �������� � ������ ������ ������� �� ������
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


// ������������� �����, ��������� � ������ �������, ��� �������
// chkfunc ���������� ��������� ��������
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


// ������� ������� � ������ �� ���� ��� �������� ������ �� ������
// ������� ��������� ������
nrc::CharString nrc::CharString::DeleteLeftWhileNot( const char *set )
{
	int cnt = 0;
	while( strchr(set, at(cnt)) == NULL && cnt < length() )
		cnt++;
			
	CharString s = substr(0, cnt).c_str();
	erase(0, cnt);
	return s;
}


// ������� ������� � ����� �� ���� ��� �������� ������ �� ������
// ������� ��������� ������
nrc::CharString nrc::CharString::DeleteRightWhileNot( const char *set )
{
	int cnt = 0, i = length()-1;
	while( i >= 0 && strchr(set, at(i)) == NULL )
		cnt++, i--;
	
	CharString s = substr(i+1, cnt+1).c_str();
	erase(i, cnt);
	return s;
}


// ������� ��������� �� �����
nrc::CharString::operator const char *()
{
	return c_str();
}


// ������������� ����� ����� � ������ � ��������
nrc::CharString &nrc::CharString::operator=( int d )
{
	char buf[25];
	_snprintf(buf, 25, "%d", d);
	this->operator=( buf );
	return *this;
}

	
// ������������� ������������ ����� � ������ � ��������
nrc::CharString &nrc::CharString::operator=( double f )
{
	char buf[100];
	_snprintf(buf, 100, "%lf", f);
	this->operator=( buf );
	return *this;
}

//------------------------------------------------------------------------------------------------
// ���������� ��� ������ ������
nrc::CharString nrc::StringList::Name() 
{
	return CharString("NRC::StringList");
}
	

// �������� ������ � ����� ������
void nrc::StringList::Append( CharString &s )
{
	CharString *n = new CharString(s);
	push_back(n);
}

	
// ������� ������ � ������� N. 
// ���������� ���������� � ����
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

	
// �������� ���� ������
void nrc::StringList::Clear()
{
	for( int i = 0; i<size(); i++ )
		delete at(i);
	clear();
}


// ��������� ������ �� �����
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


// ��������� ������ � �����
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


// ������� ������ ����� � ���� ������
nrc::CharString nrc::StringList::Text()
{
	CharString s;

	for( int i = 0; i<Count(); i++ )
		s += (*this)[i] + '\n';

	return s;
}


// ��������� ������ �� �����, ����� ������-�����������
// � ��������� �� � ������ �����. ��� ���� ���������� ���������
// ������ ���������. ������ ����������� � ������� ��� ��������� �������
// ��������� ����� ������, ���������� �� �����������
void nrc::StringList::DelimText( const CharString &s, char delim )
{
	Clear();
	
	CharString out;
	bool quote = false;

	for( int i = 0; i<s.length(); i++ )
	{
		// ������ ����������� � �������
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

		// ��������� �����
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


// ����� ������ ��������������� �������
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


// ���������� �����
void nrc::StringList::Sort( bool (*fcmp)(const CharString *, const CharString*) )
{
	sort(begin(), end(), fcmp);
}


// �������� ������� � �������
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