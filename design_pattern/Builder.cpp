#include <iostream>

using std::cout;
using std::endl;

class Builder  
{
public:
	virtual void BuildHead() {}
	virtual void BuildBody() {}
	virtual void BuildLeftArm(){}
	virtual void BuildRightArm() {}
	virtual void BuildLeftLeg() {}
	virtual void BuildRightLeg() {}
};
//构造瘦人
class ThinBuilder : public Builder
{
public:
	void BuildHead() { cout<<"build thin body"<<endl; }
	void BuildBody() { cout<<"build thin head"<<endl; }
	void BuildLeftArm() { cout<<"build thin leftarm"<<endl; }
	void BuildRightArm() { cout<<"build thin rightarm"<<endl; }
	void BuildLeftLeg() { cout<<"build thin leftleg"<<endl; }
	void BuildRightLeg() { cout<<"build thin rightleg"<<endl; }
};
//构造胖人
class FatBuilder : public Builder
{
public:
	void BuildHead() { cout<<"build fat body"<<endl; }
	void BuildBody() { cout<<"build fat head"<<endl; }
	void BuildLeftArm() { cout<<"build fat leftarm"<<endl; }
	void BuildRightArm() { cout<<"build fat rightarm"<<endl; }
	void BuildLeftLeg() { cout<<"build fat leftleg"<<endl; }
	void BuildRightLeg() { cout<<"build fat rightleg"<<endl; }
};
//构造的指挥官
class Director  
{
private:
	Builder *_pBuilder;
public:
	Director(Builder *builder) { _pBuilder = builder; }
	void Create(){
		_pBuilder->BuildHead();
		_pBuilder->BuildBody();
		_pBuilder->BuildLeftArm();
		_pBuilder->BuildRightArm();
		_pBuilder->BuildLeftLeg();
		_pBuilder->BuildRightLeg();
	}
};

int main()
{
	ThinBuilder thin;
	FatBuilder fat;
	Director thinDirector(&thin);
	thinDirector.Create();
	Director fatDirector(&fat);
	fatDirector.Create();
	return 0;
}
