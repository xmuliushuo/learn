#include "utils.h"

using namespace std;

const int defaultBlockSize = 10485760;
const int defaultThreadNum = 1;
const string defaultFileDir = "./";
const int defaultLockMemGB = 0;

int main(int argc, char *argv[])
{
	int threadNum = defaultThreadNum;
	string filedir = defaultFileDir;
	int lockMemGB = defaultLockMemGB;
	int blockSize = defaultBlockSize;

	int result;
	while ((result = getopt(argc, argv, "t:d:m:hb:")) != -1) {
		switch (result) {
		case 't':
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
		case 'h':	
			cout << "Usage: vodtest [option]" << endl;
			cout << "-t <threadnum>, default: " << defaultThreadNum << endl;
			cout << "-d <filedir>, default: " << defaultFileDir << endl;
			cout << "-m <lockMemGB>, default: " << defaultLockMemGB << endl;
			cout << "-b <blocksize>, default: " << defaultBlockSize << endl;
			cout << "-h, output this message" << endl;
			return 0;
		default:
			break;
		}
	}
	cout << "Thread Number: " << threadNum << endl;
	cout << "File Dir: " << filedir << endl;
	cout << "Lock Memeory(GB): " << lockMemGB << endl;
	cout << "Block Size(byte): " << blocksize << endl;

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
		// testArg * threadarg = new targ();
		// threadarg->id = i;
		// threadarg->filenames = &filenames;
		// createDettachThread(tid, testThread, (void*)threadarg);
	}
	return 0;
}