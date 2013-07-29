#ifndef UTILS_H_
#define UTILS_H_

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <dirent.h>
#include <assert.h>

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

using std::string;
using std::vector;

const int MAXLENGTH = 4096;

int LockMem(int);
off_t GetFileLength(const string &);
double TimeDiff(const struct timeval &, const struct timeval &);
bool GetFileNamesInDir(const string &, vector<std::string> &, off_t);
int TimeCompare(const struct timeval &, const struct  timeval &);

inline double 
TimeDiff(const struct timeval &startTime, const struct timeval &endTime)
{
	return ((endTime.tv_sec - startTime.tv_sec) * 1000000 + 
		(endTime.tv_usec - startTime.tv_usec)) / 1000000.0;
}

#endif