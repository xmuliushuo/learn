/*
	命令模式：将一个请求封装为一个对象，从而使我们可用不同的请求对可以进行参数化，对请求排队或者记录请求日志
	以及支持可撤销的操作

	Created by Phoenix_FuliMa
*/
#include <iostream>
using namespace std;

class Receiver
{
public:
	virtual void Action()
	{
		cout<<"Receiver action"<<endl;
	}
};

class Command
{
	Receiver *receiver;
public:
	Command(Receiver *reciver)
	{
		this->receiver = reciver;
	}
	virtual void cmmand()
	{
		receiver->Action();
	}
};

class ConcreteCommand:public Command
{
public:
	ConcreteCommand(Receiver *receiver)
		:Command(receiver)
	{}
};

class Invoker
{
	Command *_command;

public:
	void SetCommand(Command *command)
	{
		this->_command = command;
	}

	void Notify()
	{
		_command->cmmand();
	}
};

int main()
{

	Receiver *receiver = new Receiver();
	ConcreteCommand *command = new ConcreteCommand(receiver);

	Invoker* invoker =new Invoker();
	invoker->SetCommand(command);
	invoker->Notify();

	delete receiver;
	delete command;
	delete invoker;

//	system("pause");
	return 0;
}

