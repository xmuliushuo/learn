#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
int main()
{
	int fd;
	int ret;
	char buf[]="h";
	if((fd = open("2.txt",O_RDWR|O_CREAT|O_TRUNC, S_IREAD | S_IWRITE)) <= 0)
	{
		perror("open");
		printf("open error");
	}
	printf("fd:%d\n", fd);
	write(fd, buf, 1);
	ret = lseek(fd,1022L,SEEK_CUR);
	if(ret == -1)
	{
		perror("lseek");
	}
	printf("%d\n",ret);
	printf("%s\n", strerror(ret));
	write(fd,buf,1);
	close(fd);
	return;
}
