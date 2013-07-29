#include <fcntl.h>
#include <pthread.h>

#include "utils.h"

using namespace std;

void * TestThread(void *arg);

const int defaultBlockSize = 10485760;
const int defaultThreadNum = 1;
const string defaultFileDir = "./";
const int defaultLockMemGB = 0;
const int DefaultTestTime = 60;

int *read_per_thread;
int *timeout_per_thread;

struct ThreadArg {
	int id;
	int period;
	int blocksize;
	int testtime;
	vector<string> *filenames;
};

int main(int argc, char *argv[])
{
	int threadNum = defaultThreadNum;
	string filedir = defaultFileDir;
	int lockMemGB = defaultLockMemGB;
	int blockSize = defaultBlockSize;
	int testtime = DefaultTestTime;

	int result;
	while ((result = getopt(argc, argv, "n:d:m:hb:t:")) != -1) {
		switch (result) {
		case 'n':
			threadNum = atoi(optarg);
			assert(threadNum > 0);
			break;
		case 'd':
			filedir = optarg;
			break;
		case 'm':
			lockMemGB = atoi(optarg);
			assert(lockMemGB >= 0);
			break;
		case 'b':
			blockSize = atoi(optarg);
			assert(blockSize > 0);
			break;
		case 't':
			testtime = atoi(optarg);
			assert(testtime > 0);
		case 'h':	
			cout << "Usage: vodtest [option]" << endl;
			cout << "-n <threadnum>, default: " << defaultThreadNum << endl;
			cout << "-d <filedir>, default: " << defaultFileDir << endl;
			cout << "-m <lockMemGB>, default: " << defaultLockMemGB << endl;
			cout << "-b <blocksize>, default: " << defaultBlockSize << endl;
			cout << "-t <testtime>, default: " << DefaultTestTime << endl;
			cout << "-h, output this message" << endl;
			return 0;
		default:
			break;
		}
	}
	cout << "Thread Number: " << threadNum << endl;
	cout << "File Dir: " << filedir << endl;
	cout << "Lock Memeory(GB): " << lockMemGB << endl;
	cout << "Block Size(byte): " << blockSize << endl;
	cout << "Test Time(s): " << testtime << endl;

	read_per_thread = new int[threadNum]();
	timeout_per_thread = new int[threadNum]();

	/*
	 * lock memory
	 */
	if (lockMemGB > 0) {
		if (LockMem(lockMemGB) != 0) {
			cout << "Error: lock memeory error" << endl;
			return 1;
		}
	}

	/*
	 * find test files
	 */
	vector<string> filenames;
	if (!GetFileNamesInDir(filedir, filenames, blockSize)) {
		cout << "Error : get dir info fail." << endl;
		return 1;
	}
	if (filenames.size() == 0) {
		cout << "Error\t: There are no test file fit the blocksize in DIR [ " 
			<< filedir << " ]."  << endl
			<<  "\tNeed add test file first ." << endl;
		return 0;
	}
	cout << "Find " << filenames.size() << " test files" << endl; 

	for(int i = 0; i < threadNum; ++i) {
		ThreadArg *threadarg = new ThreadArg();
		threadarg->id = i;
		threadarg->period = 4;
		threadarg->blocksize = blockSize;
		threadarg->filenames = &filenames;
		threadarg->testtime = testtime;
		pthread_t tid;
		if (pthread_create(&tid, NULL, TestThread, threadarg) != 0) {
			cout << "Error: pthread_create error." << endl;
			return 1;
		}
	}
	sleep(testtime + 10);
	int read_total = 0;
	int timeout_total = 0;
	for (int i = 0; i < threadNum; ++i) {
		read_total+= blockSize / 1024 * read_per_thread[i];
		timeout_total += timeout_per_thread[i];
	}
	double speed = ((double)read_total) / 1024.0 / testtime;
	cout << "Total speed: " << speed << endl;
	cout << "Timeout: " << timeout_total << endl;
	return 0;
}

void *TestThread(void *_arg)
{
	if (pthread_detach(pthread_self()) != 0) {
		cout << "Error: pthread_detach error." << endl;
		return (void *)1;
	}
	ThreadArg *arg = (ThreadArg *)_arg;
	vector<string> &filenames = *(arg->filenames);
	int blocksize = arg->blocksize;
	int threadid = arg->id;
	char *buf = new char[blocksize];
	struct timeval starttime, begintime, endtime;
	gettimeofday(&starttime, NULL);
	int i, fd, filenum = filenames.size();
	double diffsec, diffall;
	while (true) {
		i = rand() % filenum;
		string name = filenames[i];
		gettimeofday(&begintime, NULL);
		//syscall(299, arg->period * 1000 - 100);
		fd = open(name.c_str(), O_RDONLY);
		if (fd < 0) {
			cout << "Error: open file error." << endl;
			return (void *)1;
		}
		if (read(fd, buf, blocksize) != blocksize) {
			cout << "Error: read error." << endl;
			return (void *)1;
		}
		close(fd);
		++read_per_thread[threadid];
		gettimeofday(&endtime, NULL);
		diffsec = TimeDiff(begintime, endtime);
		diffall = TimeDiff(starttime, endtime);
		if (diffsec > arg->period) {
			++timeout_per_thread[threadid];
		} else {
			// TODO sleep here
		}
		usleep(50000);
		if (diffall > arg->testtime) {
			break;
		}
	}
	return (void *)0;
}