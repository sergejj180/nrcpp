// ����������� ����� ����������������� ���������� - read.h


enum READ_MODE { MODE_BUFFER, MODE_FILE, MODE_UNCKNOW };
typedef READ_MODE WRITE_MODE;


class ReadFileWriteFile 
{
};


class Read
{
	// ������� �����
	string inbuf;


	// �������� ����� 
	string outbuf;


	// ������� ���� 
	FILE *in;


	// �������� ����
	FILE *out;


	// ������� ����� �������� ����������
	READ_MODE inmode;


	// ������� ����� ������
	WRITE_MODE outmode;


public:
	Read() { in = out = NULL; inmode = outmode = MODE_UNCKNOW; }
	Read( FILE *i, FILE *o ) { in = i, o = out; inmode = outmode = MODE_FILE; }
	
	
};