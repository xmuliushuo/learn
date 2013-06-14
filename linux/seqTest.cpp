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


static const int K = 1024;

extern int LockMem(int size);

static off_t getFileLength(const string);
//static bool getFileNamesInDir(const string strDir, vector<string> &vecFileName);
static double timeDiff(const struct timeval&, const struct timeval&);

int main(int argc, char *argv[])
{
	struct timeval startTime, endTime;
	// 接受参数
	if (argc != 5) {
		cout << "Usage : randtest <filename> <perReadSize> <isDirect> <lockMem(GB)>" << endl;
		exit(-1);
	}
	string filename(argv[1]);
	int bufferSize = atoi(argv[2]);
    int isDirect = atoi(argv[3]);
    int lockMemSize = atoi(argv[4]);
    int fd;
    
    cout << "test file name: " << filename << endl;
    cout << "lock memeory: " << lockMemSize  << " GB"<< endl;

    if (lockMemSize > 0) {
        if (LockMem(lockMemSize * 1024 * 1024 * 1024) != 0) {
            cout << "lock memeory error" << endl;
            exit(1);
        }
    }

	char *buf;
	int ret = posix_memalign((void **)&buf, 4096, bufferSize);

	if (ret) {
		cout << "Error: posix_memalign error: " << strerror(ret) << endl;
		return 1;
	}

	gettimeofday(&startTime, NULL);
    if (isDirect) {
	    fd = open(filename.c_str(), O_RDONLY | O_DIRECT);
    }
    else {
        fd = open(filename.c_str(), O_RDONLY);
    }

	off_t filelength = getFileLength(filename);
    cout << "file length: " << filelength << endl;
	while(filelength > 0) {
		if (filelength >= bufferSize) {
			read(fd, buf, bufferSize);
			filelength -= bufferSize;
		} else {
			read(fd, buf, filelength);
			filelength = 0;
		}
	}
	close(fd);
	gettimeofday(&endTime, NULL);
    filelength = getFileLength(filename);
	double diffall = timeDiff(startTime, endTime);
	double total = filelength / 1024;
    cout << "Time: " << diffall << endl;
	cout << "Speed: " << total / diffall << "KB/s" << endl;

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

static double 
timeDiff(const struct timeval &startTime, const struct timeval &endTime)
{
	return ((endTime.tv_sec - startTime.tv_sec) * 1000000 + 
		(endTime.tv_usec - startTime.tv_usec)) / 1000000.0;
}
