/*
 * e_3_18.cpp
 *
 *  Created on: 2012-11-26
 *      Author: liushuo
 */

#include <iostream>
#include <vector>

using std::cin;
using std::endl;
using std::cout;
using std::vector;

void e_3_18()
{
	vector<int> ivec;
	int num;
	for (int i = 0; i < 10; i++) {
		cin >> num;
		ivec.push_back(num);
	}
	for (vector<int>::iterator iter = ivec.begin(); iter != ivec.end(); iter++) {
		(*iter) *= 2;
		cout << (*iter) << " ";
	}
	cout << endl;
}
