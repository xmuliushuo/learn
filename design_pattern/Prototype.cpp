/*
	原型模式：用原型实例指定创建对象的种类，并且通过拷贝这些原型创建新的对象
	Created by Phoenix_FuliMa
*/

#include <iostream>
#include <string>
using namespace std;


class Prototype
{
public:
	virtual Prototype *Clone() = 0;
	virtual void display() = 0;
};

class Prototype1:public Prototype
{
protected:
	string name;
	int id;
public:
	Prototype1(string name, int id)
	{
		this->name = name;
		this->id = id;
	}
	Prototype1(const Prototype1&type)
	{
		this->name = type.name;
		this->id   = type.id;
	}

	virtual void display()
	{
		cout<<"my name and id are : " << this->id<<" "<< this->name <<endl;
	}
	Prototype *Clone()
	{
		return new Prototype1(*this);
	}
};

class Prototype2:public Prototype
{
protected:
	string name;
public:
	Prototype2(string name)
	{
		this->name = name;
	}
	Prototype2(const Prototype2&type)
	{
		this->name = type.name;
	}

	virtual void display()
	{
		cout<<"my name is : "<< this->name <<endl;
	}
	Prototype *Clone()
	{
		return new Prototype2(*this);
	}
};

int main()
{
	Prototype *obj1 = new Prototype1("mafuli", 1);
	Prototype *obj2 = obj1->Clone();
	Prototype *obj3 = obj2->Clone();

	obj1->display();
	obj2->display();
	obj3->display();

	Prototype *obj4 = new Prototype2("fulima");
	Prototype *obj5 = obj4->Clone();
	Prototype *obj6 = obj5->Clone();

	obj4->display();
	obj5->display();
	obj6->display();
	
	delete obj1;
	delete obj2;
	delete obj3;
	delete obj4;
	delete obj5;
	delete obj6;


//	system("pause");
	return 0;
}

/*
my name and id are : 1 mafuli
my name and id are : 1 mafuli
my name and id are : 1 mafuli
my name is : fulima
my name is : fulima
my name is : fulima
*/
