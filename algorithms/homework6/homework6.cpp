#include <iostream>
#include <fstream>
#include <set>

using namespace std;

const int Num = 500000;
const int low = 2500;
const int high = 4000;
int hashTable[high];
int count = 0;

int main()
{
	ifstream infile("HashInt.txt");
	int temp;
	for (int i = 0; i < Num; ++i) {
		infile >> temp;
		if (temp < high) {
			hashTable[temp] = 1;
		}
	}
	for (int i = low; i <= high; i++) {
		for (int j = 1; j < i; j++) {
			if ((hashTable[j] == 1) && (hashTable[i - j] == 1) && (j != i - j)) {
				count++;
				break;
			}
		}
	}
	cout << count << endl;
}