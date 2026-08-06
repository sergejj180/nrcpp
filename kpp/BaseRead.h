#ifndef BASEREAD_H
#define BASEREAD_H

// класс считывания из файла
class BaseRead
{
public:
	BaseRead() { }
	virtual ~BaseRead() { }

	// считывание из потока в символ
	virtual int operator>>( register int &c ) = 0;

	// возврат символа в поток
	virtual void operator<<( register int &c ) = 0;
};

// класс считывания из буфера
class BufferRead : public BaseRead
{
	string buf;

	// текущий указатель на место в строке
	int i;
public:
	BufferRead( string b ) : buf(b) { i = 0; }

	// считывание из буфера в символ
	int operator>>( register int &c ) {
		if( i == buf.length() )
			return (c = EOF);

		c = (unsigned char)buf[i++];
		return c;
	}

	// возврат символа в поток
	void operator<<( register int &c ) { if(c != EOF) i--; }
};

// класс считывания из файла
class FileRead : public BaseRead
{
	FILE *in;

public:
	FileRead( FILE *i ) : in(i) { }
	~FileRead( ) { fclose(in); }

	// считывание из буфера в символ
	int operator>>( register int &c ) {
		c = fgetc(in);
		return c;
	}

	// возврат символа в поток
	void operator<<( register int &c ) { ungetc(c, in); }
};

#endif
