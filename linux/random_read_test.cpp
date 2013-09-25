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

using namespace std;

static int acctime;
static size_t blockSize;
int *readPerThread;
int *timeout_per_thread;

static const int K = 1024;

static off_t getFileLength(const string);
static bool getFileNamesInDir(const string strDir, vector<string> &vecFileName);
static double timeDiff(const struct timeval&, const struct timeval&);
static int RandomI(int first, int second);

struct timeval global_start_time, global_end_time; 
pthread_mutex_t lock;

typedef struct targ {
	int id;
	string * filename;
} testArg;

void set_pthread_dettach(pthread_attr_t & attr)
{
	if (pthread_attr_init( &attr ) != 0) {
		cout << "\tError : pthread_attr_init fail " << strerror(errno) << endl;
		return;
	}

	if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) {
		cout << "\tError : pthread_attr_setdetachstate fail " << strerror(errno) << endl;
		return;
	}
}

void createDettachThread(pthread_t & tid, void* (* fun) (void *), void * arg)
{
	pthread_attr_t attr;
	set_pthread_dettach(attr);
	pthread_create(&tid, &attr, fun, arg);
}


void * testThread(void * arg)
{
	testArg *testarg = (testArg *)arg;
	int threadid = testarg->id;
	string filename = *(testarg->filename);

	struct timeval startTime, endTime, beginTime;
	size_t totalread = 0, secread = 0;
	char *buf = new char[blockSize];

	gettimeofday(&startTime, NULL);

	int fd;
	int latency_start;
	int latency_end;
	int i;
	size_t filelength = getFileLength(filename);

	fd = open(filename.c_str(), O_RDONLY);
	if (fd < 0) { 
		cout << "Error: open file error!" << endl;
		return (void *)1;
	}

	while(true) {
		i = RandomI(0, filelength - blockSize);
		// syscall(299, 3600);
		// latency_start = syscall(300);
		gettimeofday(&beginTime, 0);
		
		if (read(fd, buf, blockSize) != blockSize) {
			cout << "Error: read file error!" << endl;
			return (void *)1;
		}
		
		// latency_end = syscall(300);
		// syscall(299, 0);
		gettimeofday(&endTime, NULL);
		double diffsec = timeDiff(beginTime, endTime);
		double diffall = timeDiff(startTime, endTime);
		readPerThread[threadid] += 1;
		if (diffall > acctime){
			break;
		}
		if (diffsec > 4) {
			++timeout_per_thread[threadid];
		}
		pthread_mutex_lock(&lock);
		cout << diffall << "," << diffsec << endl;
		pthread_mutex_unlock(&lock);
		usleep(50000);
	}
	close(fd);
	pthread_mutex_lock(&lock);
	gettimeofday(&global_end_time , NULL);
	pthread_mutex_unlock(&lock);
	return (void *)0;
}

int main(int argc, char *argv[])
{
	srand(time(NULL));
	int threadNum = 1;
	int memoryBlockGB = 0;
	pthread_t tid;
	// 接受参数
	if (argc != 6) {
		cout << "Usage : randtest <filename> <threadnum> <acctime(s)> <blocksize(B)> <memoryblockupGB(GB)>" << endl;
		exit(-1);
	}
	string filename(argv[1]);
	threadNum = atoi(argv[2]);
	readPerThread = new int[threadNum]();
	timeout_per_thread = new int[threadNum]();
	acctime = atoi(argv[3]);
	blockSize = atoll(argv[4]);
	memoryBlockGB = atoi(argv[5]);

	pthread_mutex_init(&lock, NULL);

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
	for(int i = 0; i < threadNum; ++i) {
		testArg * threadarg = new targ();
		threadarg->id = i;
		threadarg->filename = &filename;
		createDettachThread(tid, testThread, (void*)threadarg);
	}

	sleep(acctime + 10);
	double total_time = timeDiff(global_start_time, global_end_time);
	long long readTotal = 0;
	int timeout_total = 0;
	int times_total = 0;
	for (int i = 0; i < threadNum; ++i) {
		cout << "Thread " << i << " read " << readPerThread[i] << endl;
		readTotal += blockSize * readPerThread[i];
		timeout_total += timeout_per_thread[i];
		times_total += readPerThread[i];
	}
	double speed = (readTotal * 1.0) / total_time;
	double speedinMB = speed / K / K;
	cout << "Total speed: " << speed << "B/s in " << total_time << " seconds." << endl;
	cout << "Total speed: " << speedinMB << "MB/s in " << total_time << " seconds." << endl;
	cout << "Total read: " << times_total << endl; 
	cout << "Total timeout: " << timeout_total << endl;
	munlockall();

	return 0;
}

static off_t getFileLength(const string filename)
{
	struct stat st;
	if (lstat(filename.c_str(), &st) != 0) {
		cout << "Error: lstat error" << endl;
		exit(1);
	}
	return st.st_size;
}

static bool getFileNamesInDir(const string strDir, vector<string> &vecFileName)
{
	DIR* dir = NULL;
	struct dirent entry;
	struct dirent* entryPtr = NULL;
	char realPath[1024];
	realpath(strDir.c_str(), realPath);
	string strRealPath = realPath;

	dir = opendir(realPath);
	if (NULL == dir) {
		cout << strerror(errno) << ", strDir : " << strDir << endl;
		return false;
	}

	readdir_r(dir, &entry, &entryPtr);
	while (entryPtr != NULL) {
		if (entry.d_type == DT_REG) {
			string strFileName = entry.d_name;
			if ("." == strFileName || ".." == strFileName) {
			}
			else {
				if (getFileLength(strRealPath + "/" + strFileName) == blockSize) {
					vecFileName.push_back(strRealPath + "/" + strFileName);
				}
			}
		}
		else if(entry.d_type == DT_DIR) {
			string dir = entry.d_name;
			if (!("." == dir || ".." == dir)) {
				getFileNamesInDir(strRealPath + "/" + dir, vecFileName);
			}
		}
		readdir_r(dir, &entry, &entryPtr);
	}

	return true;
}


static double 
timeDiff(const struct timeval &startTime, const struct timeval &endTime)
{
	return ((endTime.tv_sec - startTime.tv_sec) * 1000000 + 
		(endTime.tv_usec - startTime.tv_usec)) / 1000000.0;
}

static int RandomI(int first, int second)
{
	double temp = random() / (RAND_MAX * 1.0);
	return (int)(first + (second - first) * temp);
}