/*
 * e_5_18.cpp
 *
 *  Created on: 2012-12-6
 *      Author: liushuo
 */


#include <string>
#include <vector>
#include <iostream>

using std::string;
using std::vector;
using std::cout;

void e_5_18()
{
	vector<string *> strvec;
	string str1("Hello ");
	string str2("World!");
	strvec.push_back(&str1);
	strvec.push_back(&str2);
	for (vector<string *>::iterator iter = strvec.begin(); iter != strvec.end(); iter++) {
		cout << **iter;
	}
}

