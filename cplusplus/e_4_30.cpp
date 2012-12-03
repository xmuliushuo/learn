/*
 * e_4_30.cpp
 *
 *  Created on: 2012-12-3
 *      Author: liushuo
 */
#include <string.h>
#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::string;

void e_4_30()
{
	char cstr1[10] = "hello ";
	char cstr2[10] = "world!";
	char cresult[100];
	cresult[0] = 0;
	strcat(cresult, cstr1);
	strcat(cresult, cstr2);
	cout << cresult << endl;

	string str1(cstr1);
	string str2(cstr2);
	string result = str1 + str2;
	cout << result << endl;
}


