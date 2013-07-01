#ifndef UTILS_H_
#define UTILS_H_

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include <errno.h>
#include <dirent.h>
#include <assert.h>

using std::string;
using std::vector;

const int MAXLENGTH = 4096;

int LockMem(int);
off_t GetFileLength(const string &);
double TimeDiff(const struct timeval &, const struct timeval &);
bool GetFileNamesInDir(const string &, vector<std::string> &, off_t);

#endif