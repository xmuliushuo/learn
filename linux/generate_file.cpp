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

const int MAXLENGTH = 4096;

int main()
{
	long long filelength;
	string filename;
	cout << "input the file length" << endl;
	cin >> filelength;
	cout << "input the file name" << endl;
	cin >> filename;
	char *p = new char[MAXLENGTH];
	int fd = open(filename.c_str(), O_CREAT | O_RDWR, 0644);
	if (fd < 0) {
		cout << "open error" << endl;
	}
	int ret;
	while (filelength > MAXLENGTH) {
		ret = write(fd, p, MAXLENGTH);
		filelength -= ret;
	}
	write(fd, p, filelength);
	close(fd);
	return 0;
}