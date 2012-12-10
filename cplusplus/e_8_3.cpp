/*
 * e_8_3.cpp
 *
 *  Created on: 2012-12-10
 *      Author: liushuo
 */
#include <iostream>
#include <string>

using std::istream;
using std::cout;
using std::string;
using std::cin;
using std::endl;

static istream& streamtest(istream& in)
{
	string str;
	while (in >> str) {
		cout << str;
	}
	cout << endl;
	in.clear();
	return in;
}

void e_8_3()
{
	streamtest(cin);
}


