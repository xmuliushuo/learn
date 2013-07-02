#include <iostream>

using namespace std;

class Image
{
public:
	Image(string name): m_imageName(name) {}
	virtual ~Image() {}
	virtual void Show() {}
protected:
	string m_imageName;
};
class BigImage: public Image
{
public:
	BigImage(string name):Image(name) {}
	~BigImage() {}
	void Show() { cout<<"Show big image : "<<m_imageName<<endl; }
};
class BigImageProxy: public Image
{
private:
	BigImage *m_bigImage;
public:
	BigImageProxy(string name):Image(name),m_bigImage(0) {}
	~BigImageProxy() { delete m_bigImage; }
	void Show() 
	{
		if(m_bigImage == NULL)
			m_bigImage = new BigImage(m_imageName);
		m_bigImage->Show();
	}
};

int main()
{
	Image *image = new BigImageProxy("proxy.jpg"); //代理
	image->Show(); //需要时由代理负责打开
	delete image;
	return 0;
}
