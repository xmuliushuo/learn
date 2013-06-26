#include <unistd.h>
#include <iostream>

using namespace std;

int main(int argc, char **argv)
{
	int result;
	while ((result = getopt(argc, argv, "ba:c:d")) != -1) {
		switch (result) {
		case 'a':
			cout << "option=a, optopt=" << optopt << endl;
//				<< ", optarg=" << optarg << endl;
			break;
		case 'b':	
			cout << "option=b, optopt=" << optopt
				<< ", optarg=" << optarg << endl;
			break;
		case 'c':
			cout << "option=c, optopt=" << optopt
				<< ", optarg=" << optarg << endl;
			break;
		case 'd':
			cout << "option=d, optopt=" << optopt << endl;
//				<< ", optarg=" << optarg << endl;
			break;
		}
		cout << "argv[" << optind << "]=" << argv[optind] << endl;
	}

	return 0;
}
