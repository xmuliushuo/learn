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

#include "utils.h"

using namespace std;


static const int K = 1024;


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
    filelength = GetFileLength(filename);
	double diffall = TimeDiff(startTime, endTime);
	double total = filelength / 1024;
    cout << "Time: " << diffall << endl;
	cout << "Speed: " << total / diffall << "KB/s" << endl;

	return 0;
}
