// 实现三种工厂模式
// xmuliushuo
// xmuliushuo@gmail.com

#include <iostream>

using namespace std;

enum CTYPE {COREA, COREB};

class SingleCore
{
public:
    virtual void Show() = 0;
};

// 单核A
class SingleCoreA: public SingleCore
{
public:
    void Show()
    {
    	cout << "SingleCore A" << endl;
	}
};

// 单核B
class SingleCoreB: public SingleCore
{
public:
    void Show()
    {
    	cout << "SingleCore B" << endl;
	}
};

//多核
class MultiCore
{
public:
    virtual void Show() = 0;
};

class MultiCoreA : public MultiCore
{
public:
    void Show()
    {
    	cout<<"Multi Core A"<<endl;
    }

};

class MultiCoreB : public MultiCore
{
public:
    void Show()
    {
    	cout<<"Multi Core B"<<endl;
	}
};

// *****************************************************
// 简单工厂模式
// 可以生产两种型号的处理器核，在内部判断
class Factory_Simple
{
public:
    SingleCore* CreateSingleCore(enum CTYPE ctype)
    {
        if (ctype == COREA) //工厂内部判断
            return new SingleCoreA(); //生产核A
        else if (ctype == COREB)
            return new SingleCoreB(); //生产核B
        else
            return NULL;
    }
};

// *****************************************************
// 工厂方法模式
class Factory
{
public:
    virtual SingleCore* CreateSingleCore() = 0;
};
//生产A核的工厂
class FactoryA: public Factory
{
public:
    SingleCoreA* CreateSingleCore()
    {
    	return new SingleCoreA;
    }
};
//生产B核的工厂
class FactoryB: public Factory
{
public:
    SingleCoreB* CreateSingleCore()
    {
    	return new SingleCoreB;
    }
};

// *****************************************************
// 抽象工厂模式
class CoreFactory
{
public:
    virtual SingleCore* CreateSingleCore() = 0;
    virtual MultiCore* CreateMultiCore() = 0;
};

//工厂A，专门用来生产A型号的处理器
class CoreFactoryA :public CoreFactory
{
public:
    SingleCore* CreateSingleCore()
    {
    	return new SingleCoreA();
	}
    MultiCore* CreateMultiCore()
    {
    	return new MultiCoreA();
	}
};

//工厂B，专门用来生产B型号的处理器
class CoreFactoryB : public CoreFactory
{
public:
    SingleCore* CreateSingleCore()
    {
    	return new SingleCoreB();
    }
    MultiCore* CreateMultiCore()
    {
    	return new MultiCoreB();
	}
};


// *****************************************************
int main()
{
	// 简单工厂模式
	Factory_Simple factory_simple;
	cout << "简单工厂模式" << endl;
	(factory_simple.CreateSingleCore(COREA))->Show();
	(factory_simple.CreateSingleCore(COREB))->Show();

	// 工厂方法模式
	cout << "工厂方法模式" << endl;
	FactoryA factoryA;
	(factoryA.CreateSingleCore())->Show();
	FactoryB factoryB;
	(factoryB.CreateSingleCore())->Show();

	// 抽象工厂模式
	cout << "抽象工厂模式" << endl;
	CoreFactoryA coreFactoryA;
	(coreFactoryA.CreateSingleCore())->Show();
	(coreFactoryA.CreateMultiCore())->Show();
	CoreFactoryB coreFactoryB;
	(coreFactoryB.CreateSingleCore())->Show();
	(coreFactoryB.CreateMultiCore())->Show();
	return 0;
}

