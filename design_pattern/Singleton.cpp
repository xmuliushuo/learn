#include <iostream>

using std::cout;
using std::endl;

class Singleton {
public:
	static Singleton* Instance();
	void Display() {cout << "Singleton" << endl;}
private:
	Singleton(){ };
	static Singleton* _instance;
};

Singleton* Singleton::_instance = 0;
Singleton* Singleton::Instance() {
	if (_instance == 0)
		_instance = new Singleton;
	return _instance;
}

int main()
{
	Singleton::Instance()->Display();
	return 0;
}
