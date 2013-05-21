#include <iostream>

using namespace std;

//简历
class Resume
{
protected: //保护成员
	virtual void SetPersonalInfo() {}
	virtual void SetEducation() {}
	virtual void SetWorkExp() {}
public:
	void FillResume() 
	{
		SetPersonalInfo();
		SetEducation();
		SetWorkExp();
	}
};
class ResumeA: public Resume
{
protected:
	void SetPersonalInfo() { cout<<"A's PersonalInfo"<<endl; }
	void SetEducation() { cout<<"A's Education"<<endl; }
	void SetWorkExp() { cout<<"A's Work Experience"<<endl; }
};
class ResumeB: public Resume
{
protected:
	void SetPersonalInfo() { cout<<"B's PersonalInfo"<<endl; }
	void SetEducation() { cout<<"B's Education"<<endl; }
	void SetWorkExp() { cout<<"B's Work Experience"<<endl; }
};

int main()
{
	Resume *r1;
	r1 = new ResumeA();
	r1->FillResume();
	delete r1;
	r1 = new ResumeB();
	r1->FillResume();
	delete r1;
	r1 = NULL;
	return 0;
}