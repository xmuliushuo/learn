#include <stdio.h>

#pragma pack(4)
struct info_t
{
	unsigned char version;
	unsigned char padding;
	unsigned char extension;
	unsigned char count;
	unsigned char marker;
	unsigned char payload;
	unsigned short sequence;
	unsigned int timestamp;
	unsigned int ssrc;
};

union info_u
{
	unsigned char version;
	unsigned char padding;
	unsigned char extension;
	unsigned char count;
	unsigned char marker;
	unsigned char payload;
	unsigned short sequence;
	unsigned int timestamp;
	unsigned int ssrc;
};
#pragma pack()

struct A {
    char a;
    short b;
    int c;
};

struct B {
    char a;
    int b;
    short c;
};

int main(void)
{
	printf("%d\n",sizeof(info_t));
	printf("%d\n",sizeof(info_u));
    printf("%d\n", sizeof(A));
    printf("%d\n", sizeof(B));
	return 0;
}