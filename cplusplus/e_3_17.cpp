/*
 * e_3_17.cpp
 *
 *  Created on: 2012-11-26
 *      Author: liushuo
 */

#include <iostream>
#include <vector>
#include <string>

using std::cin;
using std::endl;
using std::cout;
using std::vector;
using std::string;

void e_3_17()
{
	vector<string> text;
	string word;
	while (cin >> word) {
		text.push_back(word);
	}
	int count = 0;
	for (vector<string>::iterator iter = text.begin(); iter != text.end(); iter++) {
		(*iter)[0] = toupper((*iter)[0]);
		cout << (*iter) << " ";
		count++;
		if ((count % 8) == 0) {
			cout << endl;
		}
	}
	cout << endl;
}
