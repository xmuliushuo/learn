/*
 * e_4_28.cpp
 *
 *  Created on: 2012-12-3
 *      Author: liushuo
 */


#include <vector>
#include <iostream>

using std::cin;
using std::cout;
using std::vector;

void e_4_28()
{
	vector<int> ivec;
	int num;
	while (cin >> num) {
		ivec.push_back(num);
	}
	int *array = new int[ivec.size()];
	int *p = array;
	for (vector<int>::iterator iter = ivec.begin(); iter != ivec.end(); iter++, p++) {
		*p = *iter;
		cout << *p << " ";
	}

}
