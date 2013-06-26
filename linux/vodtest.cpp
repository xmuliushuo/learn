#include "utils.h"

using namespace std;

int main(int argc, char *argv[])
{
	unsigned int threadNum = 1;
	int result;
	while ((result = getopt(argc, argv, "t:h")) != -1) {
		switch (result) {
		case 't':
			threadNum = (unsigned int)atoi(optarg);
			break;
		case 'h':	
			cout << "Usage: vodtest [option]" << endl;
			cout << "-t <threadnum>, default: 1" <<endl;
			cout << "-h, output this message" << endl;
			break;
		default:
			break;
		}
	}
	cout << "Thread Number: " << threadNum << endl;
	return 0;
}