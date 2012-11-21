/*
 * e_3_5.cpp
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

void e_3_5_1()
{
	string line;
	while (getline(cin, line))
	{
		cout << line << endl;
	}
}

void e_3_5_2()
{
	string word;
	while (cin >> word)
	{
		cout << word << endl;
	}
}
