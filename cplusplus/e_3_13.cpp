/*
 * e_3_13.cpp
 *
 *  Created on: 2012-11-24
 *      Author: liushuo
 */


#include <iostream>
#include <vector>

using std::cin;
using std::endl;
using std::cout;
using std::vector;

void e_3_13_1()
{
	vector<int> ivec;
	int temp;
	while (cin >> temp) {
		ivec.push_back(temp);
	}
	vector<int>::size_type length;
	if ((ivec.size() % 2) != 0) {
		length = ivec.size() - 1;
		cout << "we did not sum the last number" << endl;
	}
	for (vector<int>::size_type i = 0; i < length; i += 2) {
		cout << ivec[i] + ivec[i + 1] << endl;
	}
}

