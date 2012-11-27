/*
 * e_3_14.cpp
 *
 *  Created on: 2012-11-24
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

void e_3_14()
{
	vector<string> text;
	string word;
	while (cin >> word) {
		text.push_back(word);
	}
	int count = 0;
	for (vector<string>::size_type i = 0; i != text.size(); i++) {
		text[i][0] = toupper(text[i][0]);
		cout << text[i] << " ";
		count++;
		if ((count % 8) == 0) {
			cout << endl;
		}
	}
	cout << endl;
}


