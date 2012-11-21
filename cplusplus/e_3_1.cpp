/*
 * e_3_1.cpp
 *
 *  Created on: 2012-11-20
 *      Author: liushuo
 */

#include <iostream>
using namespace std;

void e_3_1()
{
	int base;
	int exponent;
	cout << "input the base" << endl;
	cin >> base;
	cout << "input the exponent" << endl;
	cin >> exponent;
	int result = 1;
	for (int i = 0; i < exponent; i++) {
		result *= base;
	}
	cout << "the result is " << result << endl;
	return;
}
