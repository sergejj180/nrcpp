// NoRules Compiler Library 
// ������������ ����, ����������� ����������� ���������� - NRC.H


#ifndef _NRC_H
#define _NRC_H

#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;


// ��� �������������� ����������� ���������� ��������� � ����������� ���� NRC
namespace nrc
{
	// ����������� �����
	class CharString;
	class StringList;


	// ������� ����� ��� ���� ������� ����������
	class SObject
	{
	public:
		// ������������ ����� ������ - ���������� ��� ������ ������
		virtual CharString Name();

		// ����������� ���������� ��� ����������� �������� ����� ��������
		virtual ~SObject( ) {	
		}
	};


	// ��������� �����, ��� �������������� ������ ���� � ���,
	// ����������� �� SObject
	template <class T>
	class TypeToSObject : public SObject
	{
		// �������� ������������� � ��������
		T val;

	public:
		// ����������� � �������� ��������
		TypeToSObject( T v = T() ) {
			val = v;
		}

		// �������������� ���������� � ���� �������
		operator T&() { return val; }
		
		virtual CharString Name();
		
	};

	
	// ��������� ������ ������� ����� ����������� �� SObject
	typedef TypeToSObject<char> SObjectCHAR;
	typedef TypeToSObject<int> SObjectINT;
	typedef TypeToSObject<float> SObjectFLOAT;
	typedef TypeToSObject<double> SObjectDOUBLE;


	// ���������� ������� ��� ��������� �������� �� ������� ���� TypeToSObject
	template <class T>
	inline T &GetValueFromSObject( SObject *p ) {
		return (T&)*(TypeToSObject<T> *)p;
	}

	
	// ����� ���������� �����, ��������� ����� ����� �� ����������� ����������
	class CharString : public SObject, public string
	{
		// ����� ������ ������, ��������������� �� �������
		SObject *assocObj;

	public:
		// ����������� �� ���������
		CharString( SObject *aobj = NULL ) { assocObj = aobj; }

		// ����������� ��������� � ��������� ��������� �� ������
		CharString( const char *s, SObject *aobj = NULL ) : string(s) {
			assocObj = aobj;
		}

		// ����������� ����������� ����� � ������
		CharString( int d, SObject *aobj = NULL ) {
			assocObj = aobj;
			*this = d;			
		}

		// ����������� ����������� ������������ ����� � ������
		CharString( double f, SObject *aobj = NULL ) {
			assocObj = aobj;
			*this = f;
		}

		// ���������� ��� ������ ������
		virtual CharString Name();

		// �������� ������ ������������� �� �������
		nrc::SObject *GetStringObject() { return assocObj; }

		// ������� �������� � ������ �������
		void Trim();

		// ������� �������� �������
		void TrimLeft();

		// ������� ������ �������
		void TrimRight();

		// ����������� ������ � ���������� �� � �����
		void Format( const char *s, ... );

		// ��������� ������ � ������ ������� 
		void LowerCase();

		// ��������� ������ � ������� �������
		void UpperCase();

		// ����������� ������
		void Reverse();

		// �������� ��� ������ ��� ����� ��������, �� � ������ ��������� ���������
		bool CompareNoCase( const char *s );

		// ������� �� ������ ��� ������� ����������� � ������ 'set'
		void DeleteInSet( const char *set );

		// ������� ������� � ������ �� ���� ��� �������� ������ �� ������
		// ������� ��������� ������
		CharString DeleteLeftWhileNot( const char *set );

		// ������� ������� � ����� �� ���� ��� �������� ������ �� ������
		// ������� ��������� ������
		CharString DeleteRightWhileNot( const char *set );		

		// �������� ������� �� ������ inset, ��������� �� ������ outset
		// ������ ������� ������� �� inset ������ ��������������� ������
		// �� outset, ���� ������ inset ������, �� ������� ����������
		// defchar
		void ChangeInSet( const char *inset, const char *outset, char defchar = ' ' );

		// �������� � ������ ������ ������� �� ������
		void KeepInSet( const char *set );

		// ������������� �����, ��������� � ������ �������, ��� �������
		// chkfunc ���������� ��������� ��������
		void KeepInSet( int (*chkfunc)(int c) );

		// ������� ��������� �� �����
		operator const char *();

		// ������������� ����� ����� � ������ � ��������
		CharString &operator=( int d );

		// ������������� ������������ ����� � ������ � ��������
		CharString &operator=( double f );
	};

	template <class T>
	virtual CharString TypeToSObject<T>::Name() {
		return CharString("NRC::TypeToSObject<T>");
	}


	// ������ �����
	class StringList : protected vector<CharString *>, public SObject
	{
		// ������� ��������� ��� ����� �� ���������, ��� ������
		static bool EqualCmpF(const CharString &s1, const CharString &s2) {			
			return s1 == s2;
		}

		// ������� ��������� ��� ����� �� ���������, ��� ����������
		static bool EqualCmpS(const CharString *s1, const CharString *s2) {			
			return strcmp(s1->c_str(), s2->c_str()) < 0;
		}


	public:
		// ����������� �� ���������
		StringList() { 
		}

		// ����������� �����������
		StringList( const StringList &sl ) {
			this->operator=( sl );
		}

		// ���������� ����������� ���������� ��� ������ ������
		~StringList() { Clear(); }

		// ���������� ��� ������ ������
		virtual CharString Name();

		// �������� ������ � ����� ������
		void Append( CharString &s );

		// ������� ������ � ������� N
		void Delete( int n );

		// �������� ���� ������
		void Clear();

		// ��������� ������ �� �����
		// ���� �������� �� ������� - ������� false
		bool LoadFromFile( const char *fname );

		// ��������� ������ � �����
		// ���� ���������� �� ������� - ������� false
		bool SaveToFile( const char *fname );

		// ����� ������, ������� �� ������������� ������� ��������� 'Cmp'
		int Find( const CharString &s, 
			bool (*fcmp)(const CharString &, const CharString&) = StringList::EqualCmpF,
			int startPos = 0 ) const;
		
		// ���������� �����
		void Sort( 
			bool (*fcmp)(const CharString *, const CharString*) = StringList::EqualCmpS );
		

		// ������� ���������� �����
		int Count() const { return size(); }

		// ������� ������ ����� � ���� ������
		CharString Text();

		// ��������� ������ �� �����, ����� ������-�����������
		// � ��������� �� � ������ �����. ��� ���� ���������� ���������
		// ������ ���������. ������ ����������� � ������� ��� ��������� �������
		// ��������� ����� ������, ���������� �� �����������
		void DelimText( const CharString &s, char delim = ';' );

		// �������� ������� � �������
		CharString &operator[]( int ix );

		// const
		const CharString &operator[]( int ix ) const;		

		// ����������� ������ �����
		const StringList &operator=(  const StringList &sl );  
	};

	// ���������������� ���������
	template <class T>
	class SmartPtr
	{
		// ������� ������ �� ���������
		int *refCount;

		// ��� ���������
		T *ptr;

	public:  

		// ����������� ������ ��������� ��������� �� ������
		SmartPtr( T *p ) : ptr(p) {
			refCount = new int(1);
		}

		// ����������� �����������, ����������� ������� ������
		// �� ���������
		SmartPtr( const SmartPtr &sptr ) {
			ptr = sptr.ptr;
			refCount = sptr.refCount;
			*refCount += 1;
		}

		// ����������� �������
		~SmartPtr() {
			// ���� ��������� ����������
			if( *refCount < 0 )	
			{
				if( !++*refCount )
					delete refCount ;
				return;
			}

			*refCount -= 1;
			if( *refCount == 0 )
			{
				delete refCount;
				delete ptr;
			}			
		}

		// ���� ��������� - �������
		bool IsNull() const {
			return ptr == NULL;
		}

		// ������� ���������� ������ �� ���� ���������
		int GetReferenceCount() const {
			return *refCount;
		}

		// ���������� ���������
		T *Release() {
			*refCount = -*refCount;		// ������� ����, ��� ��������� ����������
			T *t = ptr;
			ptr = NULL;
			return t;
		}

		// �������� �����������, ����������� ������� ����������
		SmartPtr &operator=( const SmartPtr &sptr ) {			
			if( &sptr == this )
				return *this;

			// ���� ������������� ������������ ���������,
			// ��� ��������� �������
			if( *sptr.refCount < 0 )
				throw "'SmartPtr<T>::operator=' ���������� �������������� ���������";

			// ���� ��������� ����������
			if( *refCount < 0 )	
			{
				if( !++*refCount )
					delete refCount ;				
			}

			// ����� ��������� ������� ��������
			else
			{
				*refCount -= 1;
				if( *refCount == 0 )
				{
					delete refCount;
					delete ptr;
				}
			}

			ptr = sptr.ptr;
			refCount = sptr.refCount;
			*refCount += 1;
			return *this;
		}


		// ������������� ���������	(���� ��������� �������, ���� ����� �������� ������)
		T &operator * () {
			return *ptr;
		}

		// ������������� ��������� (����������� �����) 
		const T &operator * () const {
			return *ptr;
		}


		// ������ � ������ ��������� (���� ��������� �������, ���� ����� �������� ������)
		T *operator ->() {
			return ptr;
		}		

		// ������ � ������ ��������� (����������� �����)
		const T *operator ->() const {
			return ptr;
		}				
	};
}


#endif			// _NRC_H_
