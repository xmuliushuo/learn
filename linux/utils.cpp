#include "utils.h"

using namespace std;

int LockMem(int size)
{
	assert(size > 0);

	void *ptr = malloc(size);
	if (ptr == NULL) {
		cout << "malloc error!" << endl;
		return -1;
	}
	int ret = mlock((void *)ptr, size);
	memset(ptr, 1, size);

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
	}

	return ret;
}

inline off_t GetFileLength(const string &filename)
{
	struct stat st;
	if (lstat(filename.c_str(), &st) != 0) {
		cout << "Error: lstat error" << endl;
		//return -1;
		exit(1);
	}
	return st.st_size;
}

inline double 
TimeDiff(const struct timeval &startTime, const struct timeval &endTime)
{
	return ((endTime.tv_sec - startTime.tv_sec) * 1000000 + 
		(endTime.tv_usec - startTime.tv_usec)) / 1000000.0;
}

bool GetFileNamesInDir(const string &strDir, vector<string> &vecFileName, 
	off_t fileLength)
{
	DIR* dir = NULL;
	struct dirent entry;
	struct dirent* entryPtr = NULL;
	char realPath[MAXLENGTH];
	assert(strDir.size() < MAXLENGTH);
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
				if (GetFileLength(strRealPath + "/" + strFileName) 
						== fileLength) {
					vecFileName.push_back(strRealPath + "/" + strFileName);
				}
			}
		}
		else if(entry.d_type == DT_DIR) {
			string dir = entry.d_name;
			if (!("." == dir || ".." == dir)) {
				GetFileNamesInDir(strRealPath + "/" + dir, 
					vecFileName, fileLength);
			}
		}
		readdir_r(dir, &entry, &entryPtr);
	}

	return true;
}

void TimeSubstract(const struct timeval &tv1, const struct timeval &tv2, 
	struct timeval &tvres)
{
	const struct timeval *tmptv1, *tmptv2;  
	if (TimeCompare(tv1, tv2) > 0) {
		tmptv1 = &tv1;
		tmptv2 = &tv2;
	}
	else {
		tmptv1 = &tv2;
		tmptv2 = &tv1;
	}
	if (tmptv1->tv_usec < tmptv2->tv_usec) {
		tvres.tv_sec = tmptv1->tv_sec - tmptv2->tv_sec - 1;
		tvres.tv_usec = tmptv1->tv_usec + 1000000 - tmptv2->tv_usec;
	} else {
		tvres.tv_sec = tmptv1->tv_sec - tmptv2->tv_sec;  
		tvres.tv_usec = tmptv1->tv_usec - tmptv2->tv_usec;  
	}
}

int TimeCompare(const struct timeval &tv1, const struct  timeval &tv2)
{
	if (tv1.tv_sec > tv2.tv_sec)  
		return 1;  
	else if (tv1.tv_sec < tv2.tv_sec)  
		return -1;  
	else if (tv1.tv_usec > tv2.tv_usec)  
		return 1;  
	else if (tv1.tv_usec < tv2.tv_usec)  
		return -1;  
	else  
		return 0; 
}