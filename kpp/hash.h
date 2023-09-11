// ������������ ���� ��� ���� ������� � �������


// ������ ������� �������� �� ���������
#define DEFAULT_SIZE	531


// ����� ��������� ��� ������� ��� ����������, �������, ������������ ����
// ��� ���� T
template <class T, int size>
class HashTab
{
	// ���� ������� � ������� ��������� ������
	list<T> table[size];

protected:
	// ���-�������, ���������� ������� � ������� ���������� key
	list<T> &HashFunc( char *name ) {
		register char *p;
		unsigned int h = 0, g;

		for(p = name; *p != '\0'; p++)
			if(g = (h = (h << 4) + *p) & 0xF0000000)
				h ^= g >> 24 ^ g;
		
		return table[h % size];
	}

public:
	
	HashTab( ) { }
	~HashTab( ) {  }
};
