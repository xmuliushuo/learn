/*
	状态模式：当一个对象的内在状态发生变化时，允许改变其行为，这个对象看来像是改变了其类。
	状态模式与策略模式的UML图几乎一模一样，下面列举了两者的不同（来自网络）
	（1）可以通过环境类状态的个数来决定是使用策略模式还是状态模式。
	（2）策略模式的环境类自己选择一个具体策略类，具体策略类无须关心环境类；而状态模式的环
	     境类由于外在因素需要放进一个具体状态中，以便通过其方法实现状态的切换，因此环境类
		 和状态类之间存在一种双向的关联关系。
	（3）使用策略模式时，客户端需要知道所选的具体策略是哪一个，而使用状态模式时，客户端无
	     须关心具体状态，环境类的状态会根据用户的操作自动转换。如果系统中某个类的对象存在
		 多种状态，不同状态下行为有差异，而且这些状态之间可以发生转换时使用状态模式；如果
		 系统中某个类的某一行为存在多种实现方式，而且这些实现方式可以互换时使用策略模式。

	Created by Phoenix_FuliMa
*/
#include <iostream>
using namespace std;

class State
{
public:
	virtual void Handle() {
		cout<<"**"<<endl;
	}
};

class ConcreteState1:public State
{
public:
	virtual void Handle()
	{
		cout<<"ConcreteState1 is called handle()"<<endl;
	}
};

class ConcreteState2:public State
{
public:
	virtual void Handle()
	{
		cout<<"ConcreteState2 is called handle()"<<endl;
	}
};

class Context
{
private:
	State *state;
	
public:
	Context(State *state)
		:state(state)
	{}
	void SetState(State *state)
	{
		this->state = state;
	}
	
	/* 在不同的状态下具有不同的行为 */
	void Request()
	{
		state->Handle();
	}
};

int main()
{
	State *state1 = new ConcreteState1();
	State *state2 = new ConcreteState2();

	Context *context = new Context(state1);
	
	context->Request();
	context->SetState(state2);
	context->Request();

	delete state1;
	delete state2;
	delete context;

//	system("pause");
	return 0;
}
