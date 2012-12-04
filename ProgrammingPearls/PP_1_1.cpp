/*
 * e_1_1.cpp
 *
 *  Created on: 2012-12-4
 *      Author: liushuo
 */

#include <vector>
#include <iostream>
#include <algorithm>

using std::vector;
using std::cin;
using std::cout;
using std::endl;

void PP_1_1()
{
	vector<int> ivec;
	int num;
	while (cin >> num)
		ivec.push_back(num);
	sort(ivec.begin(), ivec.end());
	for (vector<int>::iterator iter = ivec.begin(); iter != ivec.end(); iter++) {
		cout << *iter << " ";
	}
	cout << endl;
}


