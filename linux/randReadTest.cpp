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

static const int K = 1024;

static off_t getFileLength(const string);
static bool getFileNamesInDir(const string strDir, vector<string> &vecFileName);
static double timeDiff(const struct timeval&, const struct timeval&);

typedef struct targ {
	int id;
	vector<string> * filenames;
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
	vector<string> &filenames = *(testarg->filenames);

	int i, filenum = filenames.size();
	struct timeval startTime, endTime, beginTime;
	size_t filelength = 0, totalread = 0, secread = 0;
	char *buf = new char[blockSize];

	gettimeofday(&startTime, NULL);
	// lastTime.tv_sec = startTime.tv_sec;
	// lastTime.tv_usec = startTime.tv_usec;

	int fd;

	while(true) {
		i = rand() % filenum;
		string name = filenames[i];
		syscall(299, 3800);
		cout << threadid << " " << readPerThread[threadid] 
			<< " start: " << syscall(300) << endl;
		gettimeofday(&beginTime, 0);
		fd = open(name.c_str(), O_RDONLY);
		if (fd < 0) { 
			cout << "Error: open file error!" << endl;
			return (void *)1;
		}
        
		if (read(fd, buf, blockSize) != blockSize) {
			cout << "Error: read file error!" << endl;
		    return (void *)1;
		}
       /*
		int filelength = blockSize;
		while(filelength > 0) {
			if (filelength >= 4096) {
				read(fd, buf, 4096);
				filelength -= 4096;
			} else {
				read(fd, buf, filelength);
				filelength = 0;
			}
		}*/
		close(fd);
		cout << threadid << " " << readPerThread[threadid] 
			<< " end: " << syscall(300) << endl;
		gettimeofday(&endTime, NULL);
		double diffsec = timeDiff(beginTime, endTime);
		double diffall = timeDiff(startTime, endTime);
		readPerThread[threadid] += 1;
		if (diffall > acctime){
		//	cout << "thread " << threadid << " times out." << endl; 
			break;
		}
		if (diffsec > 4) {
			cout << "timeout: " << diffsec << endl;
			//usleep(1000);
		}
		// else {
		// 	double left = 4 - diffsec;
		// 	if (left > 1) {
		// //		printf("sleep %d", (int)left);
		// 		sleep((int)left);
		// 	}
		// //	printf("usleep %")
		// 	usleep((left - (int)left) * 1000000);
		// }
        usleep(50000);
	}
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
		cout << "Usage : randtest <datadir> <threadnum> <acctime(s)> <blocksize(B)> <memoryblockupGB(GB)>" << endl;
		exit(-1);
	}
	string datadir(argv[1]);
	threadNum = atoi(argv[2]);
	readPerThread = new int[threadNum]();
	acctime = atoi(argv[3]);
	blockSize = atoll(argv[4]);
	memoryBlockGB = atoi(argv[5]);

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

	vector<string> filenames;
	if (!getFileNamesInDir(datadir, filenames)) {
		cout << "Error : get dir info fail." << endl;
		exit(1);
	}
	if (filenames.size() == 0) {
		cout << "Error\t: There are no test file fit the blocksize in DIR [ " 
			<< datadir << " ]."  << endl
			<<  "\tNeed add test file first ." << endl;
		exit(-1);
	}
	cout << "Find " << filenames.size() << " test files" << endl; 

	for(int i = 0; i < threadNum; ++i) {
		testArg * threadarg = new targ();
		threadarg->id = i;
		threadarg->filenames = &filenames;
		createDettachThread(tid, testThread, (void*)threadarg);
	}

	sleep(acctime + 10);
	int readTotal = 0;
	for (int i = 0; i < threadNum; ++i) {
		cout << "Thread " << i << " read " << readPerThread[i] << endl;
		readTotal += blockSize / 1024 * readPerThread[i];
	}
	double speed = ((double)readTotal) / 1024.0 / (double)acctime;
	cout << "Total speed: " << speed << endl;
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

