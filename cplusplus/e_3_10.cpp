/*
 * e_3_10.cpp
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

void e_3_10()
{
	string s, result;
	cin >> s;
	for (string::size_type i = 0; i != s.size(); i++)
	{
		if (!ispunct(s[i])) {
			result += s[i];
		}
	}
	cout << result << endl;
}


