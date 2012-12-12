/*
 * e_9_28.cpp
 *
 *  Created on: 2012-12-12
 *      Author: liushuo
 */

#include <vector>
#include <list>
#include <iostream>

using namespace std;

void e_9_28()
{
	char *str[] = {"abc", "def", "ghi", "jkl"};
	list<char *> strList(str, str + 4);
	vector<string> strVec;

	strVec.assign(strList.begin(), strList.end());

	for (vector<string>::iterator iter = strVec.begin(); iter != strVec.end(); iter++) {
		cout << (*iter);
	}
	cout << endl;

	for (list<char *>::iterator iter = strList.begin(); iter != strList.end(); iter++) {
		cout << (*iter);
	}
	cout << endl;
}


