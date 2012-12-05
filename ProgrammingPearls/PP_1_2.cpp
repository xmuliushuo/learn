/*
 * PP_1_2.cpp
 *
 *  Created on: 2012-12-5
 *      Author: liushuo
 */

#include <iostream>
#include <bitset>

using std::bitset;
using std::cin;
using std::cout;

void PP_1_2()
{
	const int num = 1000000;
	const int range = 10000000;

	int temp;
	bitset<range> *bit = new bitset<range>();
	for (int i = 0; i < num; i++) {
		cin >> temp;
		if ((temp < 10000000) && (temp >= 0))
			(*bit)[temp - 1] = 1;
	}
	for (int i = 0; i < num; i++) {
		if (bit->test(i)) {
			cout << i << " ";
		}
	}
}


