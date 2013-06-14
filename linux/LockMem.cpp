#include <iostream>
#include <cstdlib>
#include <cstring>

#include <sys/mman.h>
#include <errno.h>

using namespace std;

int LockMem(int size)
{
    cout << "lock memeory :" << size << endl;
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
