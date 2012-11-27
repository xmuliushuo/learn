/*
 * e_3_8.cpp
 *
 *  Created on: 2012-11-22
 *      Author: liushuo
 */

#include <iostream>
#include <string>

using std::cin;
using std::string;
using std::endl;
using std::cout;

void e_3_8_1()
{
	string bigString, s;
	while (cin >> s) {
		bigString += s;
	}
	cout << "the final string: " << bigString << endl;
}

void e_3_8_2()
{
	string bigString, s;
	while (cin >> s) {
		bigString += " ";
		bigString += s;
	}
	cout << "the final string: " << bigString << endl;
}


