#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <dirent.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

using namespace std;

static int acctime;
static size_t blockSize;

static const int K = 1024;

static double timeDiff(const struct timeval&, const struct timeval&);

struct timeval global_start_time, global_end_time; 

void testThread()
{
	struct timeval startTime, endTime, beginTime;
	size_t filelength = 0, totalread = 0, secread = 0;
	char *buf = new char[blockSize];

	gettimeofday(&startTime, NULL);

	int fd;

	//char name[256];
	string name;
	double diffsec;
	double diffall;

	while(true) {
		cin >> name >> diffall;
		//scanf("%s,%lf,%lf", name, &diffall, &diffsec);
		//getline(line);
		syscall(299, 3600);
		gettimeofday(&beginTime, 0);
		fd = open(name.c_str(), O_RDONLY);
		if (fd < 0) { 
			cout << "Error: open file error!" << endl;
			return;
		}
        
		if (read(fd, buf, blockSize) != blockSize) {
			cout << "Error: read file error!" << endl;
			return;
		}
		close(fd);
		gettimeofday(&endTime, NULL);
		diffsec = timeDiff(beginTime, endTime);
		diffall = timeDiff(startTime, endTime);
		cout << name << "," << diffall << "," << diffsec << endl;
		if (diffall > acctime){
			break;
		}
	}
	gettimeofday(&global_end_time , NULL);
}

int main(int argc, char *argv[])
{
    srand(time(NULL));
	int threadNum = 1;
	int memoryBlockGB = 0;
	pthread_t tid;
	// 接受参数
	if (argc != 4) {
		cout << "Usage : randtest <acctime(s)> <blocksize(B)> <memoryblockupGB(GB)>" << endl;
		exit(-1);
	}
	acctime = atoi(argv[1]);
	blockSize = atoll(argv[2]);
	memoryBlockGB = atoi(argv[3]);

	// 锁定内存
	size_t totalSize = K * K * K * memoryBlockGB;
	void *startAddr = malloc(totalSize);
	if (startAddr == NULL) {
		cout << "Error: malloc failed" << endl;
		exit(1);
	}
	int ret = mlock((void *)startAddr, totalSize);
	memset(startAddr, 1, totalSize);

	if (ret) {
		switch(ret) {
		case ENOMEM : // not enough memory
			cout << "Error : not enough memory, " ;
			break;
		case EPERM : // priviledged not enough
			cout << "Error : priviledged not enough, " ;
			break;
		case EAGAIN: // not enough memory
			cout << "Error : not enough memory, " ;
			break;
		default :
			cout << "Error : unkown, " ;
			break;
		}
		cout << strerror(errno) << endl;
		exit(1);
	} 
	gettimeofday(&global_start_time , NULL);
	testThread();
	munlockall();

	return 0;
}

static double 
timeDiff(const struct timeval &startTime, const struct timeval &endTime)
{
	return ((endTime.tv_sec - startTime.tv_sec) * 1000000 + 
		(endTime.tv_usec - startTime.tv_usec)) / 1000000.0;
}

