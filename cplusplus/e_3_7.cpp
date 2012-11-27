/*
 * e_3_7.cpp
 *
 *  Created on: 2012-11-21
 *      Author: liushuo
 */



#include <iostream>
#include <string>

using std::cin;
using std::string;
using std::endl;
using std::cout;

void e_3_7_1()
{
	string s1, s2;
	cin >> s1 >> s2;
	if (s1 != s2) {
		cout << "this string is bigger:" << (s1 > s2 ? s1 : s2) << endl;
	}
	else {
		cout << "the two strings are equal" << endl;
	}
}

void e_3_7_2()
{
	string s1, s2;
	cin >> s1 >> s2;
	if (s1.size() != s2.size()) {
		cout << "this string is longer:" << (s1.size() > s2.size() ? s1 : s2) << endl;
	}
	else {
		cout << "the two strings are equal" << endl;
	}
}
